#include <filesystem>
#include <type_traits>
#include <functional>
#include <string_view>
#include <iostream>
#include <fstream>
#include <git2.h>
#include <cstdlib>
#include <csignal>
#include <cstring>

namespace fs = std::filesystem;
using namespace std::string_view_literals;

constexpr auto WSC_CMD = R"(@echo off
if exist "%~dp0\wsc.exe" (
	"%~dp0\wsc.exe"
) else (
	echo: >&2
	echo Missing wsc.exe executable. Workspace statuses will NOT be reported. >&2 
	echo Have you done wsc init? >&2 
	echo  ^> bazel run @wsc init >&2 
	echo: >&2 
)
)";

constexpr auto WSC_SH = R"(#!/usr/bin/env bash
set -e
WSC_EXEC="./.wsc/wsc"
if [ -f "$WSC_EXEC" ]; then
	$WSC_EXEC
else 
	>&2 echo -e "🔔"
	>&2 echo -e "🔔 Missing wsc executable. Workspace statuses will NOT be reported."
	>&2 echo -e "🔔 Have you done wsc init?"
	>&2 echo -e "🔔 \u276f bazel run @wsc init"
	>&2 echo -e "🔔"
fi
)";

void chdir_if_bazel_run() {
	auto workspace_dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
	if(workspace_dir != nullptr) {
		fs::current_path(workspace_dir);
	}
}

void exit_if_git_error(int error) {
	if(error < 0) {
		const git_error* e = git_error_last();
		std::cerr
			<< "Error " << error << "/" << e->klass << " " << e->message << "\n";
		std::exit(error);
	}
}

std::string get_head_commit_sha(git_repository* repo) {
	git_oid head_oid;
	exit_if_git_error(git_reference_name_to_id(&head_oid, repo, "HEAD"));
	return git_oid_tostr_s(&head_oid);
}

using std_fn_tag_foreach_cb =
	std::function<bool(const char* name, git_oid* oid)>;

int std_fn_tag_foreach_cb_impl(const char* name, git_oid* oid, void* payload) {
	auto std_cb = reinterpret_cast<std_fn_tag_foreach_cb*>(payload);
	return (*std_cb)(name, oid) ? 0 : 1;
}

static_assert(std::is_same_v<git_tag_foreach_cb, decltype(&std_fn_tag_foreach_cb_impl)>);

void std_fn_tag_foreach(git_repository* repo, std_fn_tag_foreach_cb cb) {
	git_tag_foreach(repo, &std_fn_tag_foreach_cb_impl, &cb);
}

std::string get_git_tag_if(git_repository* repo) {
	git_oid head_oid;
	exit_if_git_error(git_reference_name_to_id(&head_oid, repo, "HEAD"));

	std::string tag;

	std_fn_tag_foreach(repo, [&](const char* name, git_oid* oid) -> bool {
		if(git_oid_equal(oid, &head_oid)) {
			tag = std::string{name + "refs/tags/"sv.size()};
			return false;
		}
		return true;
	});

	return tag;
}

bool is_dirty(git_repository* repo) {
	static bool is_dirty_value = false;
	git_index* index = nullptr;
	exit_if_git_error(git_repository_index(&index, repo));

	git_diff* diff = nullptr;
	git_diff_options diff_options = GIT_DIFF_OPTIONS_INIT;
	exit_if_git_error(
		git_diff_index_to_workdir(&diff, repo, index, &diff_options)
	);

	is_dirty_value = false;
	git_diff_foreach(diff, [](const git_diff_delta *delta,
	float progress,
	void *payload) -> int{

		switch(delta->status) {
			case GIT_DELTA_UNREADABLE:
			case GIT_DELTA_IGNORED:
			case GIT_DELTA_UNMODIFIED:
				return 0;
			case GIT_DELTA_ADDED:
			case GIT_DELTA_DELETED:
			case GIT_DELTA_MODIFIED:
			case GIT_DELTA_RENAMED:
			case GIT_DELTA_COPIED:
			case GIT_DELTA_UNTRACKED:
			case GIT_DELTA_TYPECHANGE:
			case GIT_DELTA_CONFLICTED:
				is_dirty_value = true;
				return 1;
		}
		
		return 0;
	}, nullptr, nullptr, nullptr, nullptr);

	return is_dirty_value;
}

int print_workspace_status() {
	git_libgit2_init();

	git_repository* repo = nullptr;
	exit_if_git_error(git_repository_open(&repo, fs::current_path().c_str()));

	const auto head_commit_sha = get_head_commit_sha(repo);
	const auto tag = get_git_tag_if(repo);
	const auto dirty = is_dirty(repo);
	
	std::string version;

	if(tag.empty()) {
		version = head_commit_sha;
	} else {
		version = tag;
	}

	if(dirty) {
		version += " (dirty)";
	}

	std::cout
		<< "STABLE_GIT_COMMIT " << head_commit_sha << "\n"
		<< "STABLE_GIT_TAG " << tag << "\n"
		<< "STABLE_GIT_DIRTY " << (dirty ? "true" : "false") << "\n"
		<< "STABLE_VERSION " << version << "\n";

	git_repository_free(repo);

	git_libgit2_shutdown();
	return 0;
}

void ensure_gitignore_items(std::vector<std::string> items) {
	std::fstream gitignore(".gitignore");

	while(gitignore.good() && !gitignore.eof()) {
		std::string line;
		std::getline(gitignore, line);
		if(!line.empty()) {
			for(auto itr = items.begin(); itr != items.end(); ++itr) {
				if(line == "/" + (*itr)) {
					items.erase(itr);
					break;
				}
			}
		}
	}

	gitignore.clear();
	gitignore.seekg(0, std::ios_base::end);
	gitignore.seekg(gitignore.tellg());

	for(auto& item : items) {
		std::cout << "Adding " << item << " to .gitignore ...\n";
		gitignore << "/" << item << "\n";
	}
}

void ensure_bazelrc_lines(std::vector<std::string> lines) {
	std::fstream bazelrc(".bazelrc");

	while(bazelrc.good() && !bazelrc.eof()) {
		std::string line;
		std::getline(bazelrc, line);
		if(!line.empty()) {
			for(auto itr = lines.begin(); itr != lines.end(); ++itr) {
				if(line.starts_with(*itr)) {
					lines.erase(itr);
					break;
				}
			}
		}
	}

	bazelrc.clear();
	bazelrc.seekg(0, std::ios_base::end);
	bazelrc.seekg(bazelrc.tellg());

	for(auto& line : lines) {
		std::cout << "Adding " << line << " to .bazelrc ...\n";
		bazelrc << line << " # added by wsc init\n";
	}
}

void ensure_script(const fs::path& script_path, std::string_view script_contents) {
	if(fs::exists(script_path)) {
		std::cout << "Updating " << fs::relative(script_path).string() << " ...\n";
	} else {
		std::cout << "Creating " << fs::relative(script_path).string() << " ...\n";
	}
	std::ofstream(script_path) << script_contents;
	fs::permissions(script_path, fs::perms::owner_all);
}

int init_wsc(int argc, char* argv[]) {
	const bool is_valid_workspace_directory =
		fs::exists("WORKSPACE") ||
		fs::exists("WORKSPACE.bazel") ||
		fs::exists("MODULE.bazel");

	if(!is_valid_workspace_directory) {
		std::cerr
			<< "Invalid workspace directory. Could not find WORKSPACE, "
			<< "WORKSPACE.bazel or MODULE.bazel in the current directory.\n";
		return 1;
	}

	fs::path wsc_directory = ".wsc";
	fs::create_directories(wsc_directory);

	fs::path argv0_path{argv[0]};
	if(fs::is_symlink(argv0_path)) {
		argv0_path = fs::read_symlink(argv0_path);
	}
	if(argv0_path.is_absolute()) {
		argv0_path = fs::absolute(argv0_path);
	}

	const auto argv0_filename = argv0_path.filename();
	const auto wsc_executable = wsc_directory / argv0_filename;

	if(fs::exists(wsc_executable)) {
		std::cout << "Updating " << wsc_executable.string() << " ...\n";
		fs::remove(wsc_executable);
	} else {
		std::cout << "Adding " << wsc_executable.string() << " ...\n";
	}
	fs::copy_file(argv0_path, wsc_executable);
	fs::permissions(wsc_executable, fs::perms::owner_all);

	const auto wsc_executable_no_extension = 
		fs::path{wsc_executable}.replace_extension("");

	const auto wsc_sh_path =
		fs::path{wsc_executable_no_extension}.replace_extension("sh");

	const auto wsc_cmd_path =
		fs::path{wsc_executable_no_extension}.replace_extension("cmd");

	ensure_bazelrc_lines({
		"build --enable_platform_specific_config",
		"build --workspace_status_command=./" + fs::relative(wsc_sh_path).string(),
		"build:windows --workspace_status_command=./" + fs::relative(wsc_cmd_path).string(),
	});
	ensure_gitignore_items({
		wsc_executable_no_extension.string(),
		fs::path{wsc_executable_no_extension}.replace_extension("exe").string(),
	});
	ensure_script(wsc_sh_path, WSC_SH);
	ensure_script(wsc_cmd_path, WSC_CMD);

	std::cout << "✨ wsc init done! ✨\n";

	return 0;
}

int main(int argc, char* argv[]) {
	chdir_if_bazel_run();
	if(argc > 1 && std::strcmp(argv[1], "init") == 0) {
		return init_wsc(argc, argv);
	} else {
		return print_workspace_status();
	}
}
