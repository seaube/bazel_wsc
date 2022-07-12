#!/usr/bin/env bash
set -e
WSC_EXEC="./.wsc/wsc"
if [ -f "$WSC_EXEC" ]; then
  $WSC_EXEC
else 
	>&2 echo "🔔"
  >&2 echo "🔔 Missing wsc executable. Workspace statuses will not be reported."
	>&2 echo "🔔 Have you done wsc init?"
  >&2 echo "🔔 ❯ bazel run @wsc init"
	>&2 echo "🔔"
fi
