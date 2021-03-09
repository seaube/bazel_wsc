:; ./workspace_status.sh "$@"
:; exit

@ECHO OFF

@REM Workaround to have workspace status command on both UNIX based systems and
@REM Windows: https://github.com/bazelbuild/bazel/issues/5958

%~dp0\workspace_status.cmd %*
