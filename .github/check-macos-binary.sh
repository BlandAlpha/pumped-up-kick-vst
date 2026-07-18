#!/bin/bash
# Guards two regressions that are invisible on the machine that builds them:
# a missing architecture slice locks out every Intel Mac, and a deployment
# target that failed to apply makes the binary demand the build machine's own
# macOS version. Both produce a successful build and a working local install.
#
# Usage: ./.github/check-macos-binary.sh <build-dir>

set -uo pipefail

BUILD_DIR="${1:-build}"
ARTEFACTS="$BUILD_DIR/PumpedUpKick_artefacts/Release"
# arm64 is clamped to 11.0 by the toolchain — Apple Silicon did not exist
# before Big Sur — so it is the highest legitimate requirement.
MAX_ALLOWED="11.0"

rc=0

for bundle in "VST3/PumpedUpKick.vst3" "AU/PumpedUpKick.component"; do
    target="$ARTEFACTS/$bundle/Contents/MacOS/PumpedUpKick"

    if [ ! -f "$target" ]; then
        echo "::error::$target was not built"
        rc=1
        continue
    fi

    echo "--- $bundle"
    lipo -info "$target"

    for slice in arm64 x86_64; do
        if ! lipo -info "$target" | grep -q "$slice"; then
            echo "::error::$bundle is missing the $slice slice"
            rc=1
            continue
        fi

        # LC_BUILD_VERSION reports "minos"; deployment targets older than
        # 10.14 emit LC_VERSION_MIN_MACOSX instead, which reports "version".
        required=$(vtool -arch "$slice" -show-build "$target" \
                   | awk '/^[[:space:]]*(minos|version)[[:space:]]/ {print $2; exit}')

        if [ -z "$required" ]; then
            echo "::error::could not read the deployment target of the $bundle $slice slice"
            rc=1
            continue
        fi

        echo "    $slice requires macOS $required"

        if [ "$(printf '%s\n%s\n' "$required" "$MAX_ALLOWED" | sort -V | tail -1)" != "$MAX_ALLOWED" ]; then
            echo "::error::$bundle $slice slice requires macOS $required — deployment target did not take effect"
            rc=1
        fi
    done
done

exit $rc
