# See MODULE.bazel

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "hedron_compile_commands",
    sha256 = "cb29ade67efd170c98b86fe75524fc053c01dcbe1f6211d00ce658e57441ed42",
    strip_prefix = "bazel-compile-commands-extractor-670e86177b6b5c001b03f4efdfba0f8019ff523f",
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/670e86177b6b5c001b03f4efdfba0f8019ff523f.tar.gz",
)

load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")

hedron_compile_commands_setup()
