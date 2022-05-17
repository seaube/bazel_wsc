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
    strip_prefix = "boost-da62319c330d81ef032517cbe13b6f35d97387cb",
    urls = ["https://github.com/bazelboost/boost/archive/da62319c330d81ef032517cbe13b6f35d97387cb.zip"],
    sha256 = "4a79c389add7e3d54d0e12c83098d471d24536ba2d6b8593d3a95f151f25eebb",
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
    strip_prefix = "bazel-toolchain-5f82830f9d6a1941c3eb29683c1864ccf2862454",
    urls = ["https://github.com/grailbio/bazel-toolchain/archive/5f82830f9d6a1941c3eb29683c1864ccf2862454.zip"],
    sha256 = "16ec55656a6f8d4a6827eb991cfd19c41125ddc9aaf7f26339c26e72110874bb",
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
