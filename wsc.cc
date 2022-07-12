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

constexpr auto WSC_BAT = R"(:; ./wsc.sh "$@"
:; exit

@ECHO OFF

@REM Workaround to have workspace status command on both UNIX based systems and
@REM Windows: https://github.com/bazelbuild/bazel/issues/5958

%~dp0\wsc.cmd %*
)";

constexpr auto WSC_CMD = R"(
%~dp0\wsc.exe
)";

constexpr auto WSC_SH = R"(#!/usr/bin/env sh
set -e
./wsc
)";

constexpr auto BAZELRC_WSC_LINE = "build --workspace_status_command=./wsc.bat";

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

void update_wsc_bazelrc(std::string wsc, std::string wsc_bazelrc) {
	std::fstream bazelrc_stream(wsc_bazelrc);

	while(bazelrc_stream.good() && !bazelrc_stream.eof()) {
		std::string line;
		std::getline(bazelrc_stream, line);
		if(!line.empty()) {
			if(line.starts_with(BAZELRC_WSC_LINE)) {
				return;
			}
		}
	}

	bazelrc_stream.clear();
	bazelrc_stream.seekg(0, std::ios_base::end);
	bazelrc_stream.seekg(bazelrc_stream.tellg());

	bazelrc_stream << BAZELRC_WSC_LINE << " # added by wsc init\n";
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

	fs::path argv0_path{argv[0]};
	if(fs::is_symlink(argv0_path)) {
		argv0_path = fs::read_symlink(argv0_path);
	}
	if(argv0_path.is_absolute()) {
		argv0_path = fs::absolute(argv0_path);
	}

	const auto argv0_filename = argv0_path.filename();

	if(fs::exists(argv0_filename)) {
		std::cout << "Updating ./" << argv0_filename.string() << " ...\n";
		fs::remove(argv0_filename);
	} else {
		std::cout << "Adding ./" << argv0_filename.string() << " ...\n";
	}
	fs::copy_file(argv0_path, argv0_filename);
	fs::permissions(argv0_filename, fs::perms::all);

	if(!fs::exists(".bazelrc")) {
		std::cout << "Creating ./" << ".bazelrc" << " ...\n";
		std::ofstream bazelrc_stream(".bazelrc");
		bazelrc_stream << BAZELRC_WSC_LINE << " # added by wsc init\n";
	} else {
		std::cout << "Updating ./.bazelrc ...\n";
		update_wsc_bazelrc(argv0_filename.string(), ".bazelrc");
	}

	auto argv0_filename_no_ext = fs::path{argv0_filename}.replace_extension("");
	ensure_gitignore_items({"wsc", "wsc.exe"});
	std::ofstream("wsc.bat") << WSC_BAT;
	std::ofstream("wsc.cmd") << WSC_CMD;
	std::ofstream("wsc.sh") << WSC_SH;

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
