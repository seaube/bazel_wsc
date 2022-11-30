#!/usr/bin/env bash
set -e
WSC_EXEC="./.wsc/wsc"
if [ -f "$WSC_EXEC" ]; then
	$WSC_EXEC
else 
	>&2 echo -e "🔔"
	>&2 echo -e "🔔 Missing wsc executable. Workspace statuses will not be reported."
	>&2 echo -e "🔔 Have you done wsc init?"
	>&2 echo -e "🔔 \u276f bazel run @wsc init"
	>&2 echo -e "🔔"
fi
