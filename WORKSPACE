workspace(name = "com_seaube_bazel_wsc")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_skylib",
    urls = [
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.3/bazel-skylib-1.0.3.tar.gz",
    ],
    sha256 = "1c531376ac7e5a180e0237938a2536de0c54d93f5c278634818e0efc952dd56c",
)
load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")
bazel_skylib_workspace()

http_archive(
    name = "boost",
    strip_prefix = "boost-060aa1ce60c635726e6287ff0a19d39b4d61c30d",
    urls = ["https://github.com/bazelboost/boost/archive/060aa1ce60c635726e6287ff0a19d39b4d61c30d.zip"],
    sha256 = "908dedbd43267c34fb5f024367a5e78981219f495780955738a7b2a34bb91d5a",
)

load("@boost//:index.bzl", "boost_http_archives")
boost_http_archives()

http_archive(
    name = "bzlws",
    strip_prefix = "bzlws-557f409ffcb2425b0fc4cb4526b2e2e9fa9601f0",
    url = "https://github.com/zaucy/bzlws/archive/557f409ffcb2425b0fc4cb4526b2e2e9fa9601f0.zip",
    sha256 = "887dd772883d975b766b7224b18f6d288d3723929c300474262b3ca5c8fc8e2c",
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
    strip_prefix = "bazel-toolchain-c3131a6894804ee586d059c57ffe8e88d44172e1",
    urls = ["https://github.com/grailbio/bazel-toolchain/archive/c3131a6894804ee586d059c57ffe8e88d44172e1.zip"],
    sha256 = "7fa5a8624b1148c36e09c7fa29ef6ee8b83f865219c9c219c9125aac78924758",
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
