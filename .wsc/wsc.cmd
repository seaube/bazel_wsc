@echo off
if exist "%~dp0\wsc.exe" (
	"%~dp0\wsc.exe"
) else (
	echo: >&2
	echo Missing wsc.exe executable. Workspace statuses will NOT be reported. >&2 
	echo Have you done wsc init? >&2 
	echo  ^> bazel run @wsc init >&2 
	echo: >&2 
)
