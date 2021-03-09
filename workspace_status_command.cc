#include <iostream>
#include <sstream>
#include <string>
#include <future>
#include <boost/process.hpp>
#include <boost/asio.hpp>

namespace bp = boost::process;
namespace asio = boost::asio;

struct build_info {
	/// Commit hash
	std::string commit;

	/// If the current commit belongs to a tag 
	std::string tag;

	/// Current working tree has uncommitted changes
	bool dirty;
};

static build_info get_build_info();

int main() {
	try {
		auto build_info = get_build_info();

		std::cout
			<< "STABLE_GIT_COMMIT " << build_info.commit << "\n"
			<< "STABLE_GIT_TAG " << build_info.tag << "\n"
			<< "STABLE_GIT_DIRTY " << (build_info.dirty ? "true" : "false") << "\n";
	} catch(const std::exception& err) {
		std::cerr << "EXCEPTION THROWN: " << err.what() << std::endl;
		return 2;
	}

	return 0;
}

std::string parse_git_commit_output
	( std::string&& output
	)
{
	return output;
}

std::string parse_git_tag_output
	( std::string&& output
	)
{
	return output;
}

bool parse_git_dity_output
	( std::string&& output
	)
{
	return output.ends_with("-dirty")
	    || output.ends_with("-broken");
}

std::string get_system_output
	( std::string cmd
	)
{
	bp::ipstream stdout_stream;
	bp::ipstream stderr_stream;
	auto exit_code = bp::system(
		cmd,
		bp::std_out > stdout_stream,
		bp::std_err > stderr_stream
	);

	std::string line;
	if(exit_code == 0) {
		std::getline(stdout_stream, line);
		return line;
	}

	std::getline(stderr_stream, line);
	std::cerr
		<< "'" << cmd << "' exited with code " << exit_code << ": " << line << cmd;
	std::exit(1);

	return line;
}

build_info get_build_info() {
	return build_info{
		.commit = get_system_output("git rev-parse HEAD"),
		.tag = get_system_output("git tag --points-at HEAD"),
		.dirty = parse_git_dity_output(
			get_system_output("git describe --always --dirty --broken")
		),
	};
}
