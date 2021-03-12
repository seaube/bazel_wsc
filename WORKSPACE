workspace(name = "com_seaube_bazel_wsc")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "df0dd370588b2e3b2307cc38872970397e76757b",
    remote = "https://github.com/zaucy/rules_boost",
    shallow_since = "1594588553 -0700",
)

http_archive(
    name = "boost",
    build_file = "@com_github_nelhage_rules_boost//:BUILD.boost",
    sha256 = "4eb3b8d442b426dc35346235c8733b5ae35ba431690e38c6a8263dce9fcbb402",
    strip_prefix = "boost_1_73_0",
    patches = [
        # Temporary until https://github.com/boostorg/process/pull/167 gets 
        # merged and released
        "//patches:boost_process_windows_file_descriptor.patch",
    ],
    urls = [
        "https://mirror.bazel.build/dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.bz2",
        "https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.bz2",
    ],
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-2c8efaf333818925490de36e2fb7c266f0b0bd40",
    url = "https://github.com/zaucy/bzlws/archive/2c8efaf333818925490de36e2fb7c266f0b0bd40.zip",
    sha256 = "14db184ec822cfa53449e34b551f3804a2acc087acfbec4dc7f139d1dbb65028",
)

load("@bzlws//:repo.bzl", "bzlws_deps")
bzlws_deps()

http_archive(
    name = "com_github_zaucy_rules_7zip",
    strip_prefix = "rules_7zip-7c9fb2b1bdab4d4c3fdd49b1e12d21a5b728dc57",
    urls = ["https://github.com/zaucy/rules_7zip/archive/7c9fb2b1bdab4d4c3fdd49b1e12d21a5b728dc57.zip"],
    sha256 = "7adf6bcd53403ab9a44d8b42db0e8afe7fd547dbee21a5c6f5479966d96462a5",
)

load("@com_github_zaucy_rules_7zip//:setup.bzl", "setup_7zip")
setup_7zip()

http_archive(
    name = "com_grail_bazel_toolchain",
    strip_prefix = "bazel-toolchain-master",
    urls = ["https://github.com/grailbio/bazel-toolchain/archive/master.tar.gz"],
)

load("@com_grail_bazel_toolchain//toolchain:deps.bzl", "bazel_toolchain_dependencies")

bazel_toolchain_dependencies()

load("@com_grail_bazel_toolchain//toolchain:rules.bzl", "llvm_toolchain")

llvm_toolchain(
    name = "llvm_toolchain",
    llvm_version = "11.0.0",
)

load("@llvm_toolchain//:toolchains.bzl", "llvm_register_toolchains")

llvm_register_toolchains()
