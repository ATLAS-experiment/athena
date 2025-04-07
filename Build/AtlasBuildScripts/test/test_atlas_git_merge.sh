#!/usr/bin/env bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# Tests for atlas_git_merge.py based on a few historic sweeps.
#
# Run this script within your athena repository and check if you see the
# final "OK" line.
#
# Author: Frank Winklmeier
#

assert_unmerged() {
    N=`git diff --name-status --diff-filter=U | wc -l`
    if [[ N -ne $1 ]]; then
        echo "ERROR: mismatch in number of unmerged files found (expected $1):"
        git --no-pager diff --name-status --diff-filter=U
        exit 1
    fi
}

# Exit on error
set -e

# Clone athena if not done already
if [ ! -d athena_tmp ]; then
    git clone https://:@gitlab.cern.ch:8443/atlas/athena.git athena_tmp
fi
cd athena_tmp

# Easy case with a few MRs, no sweep:ignore, no conflicts
git checkout -f -B source 4a443c11d2cd8f54ddbb84b4bd2efac4888c3d27
git checkout -f -B target 0ad98aadfc1432939e54c34d796464313a37eded
../Build/AtlasBuildScripts/atlas_git_merge.py source target --remote ''
assert_unmerged 0

# MR with sweep:ignore and renames
# https://gitlab.cern.ch/atlas/athena/-/merge_requests/78710
git checkout -f -B source 7b877c54a8fabb16b8911a7a9f84403cb35a98d3
git checkout -f -B target 13471c5cddc8bcb9468734e115232d6f41e9cf0f
echo "a" | ../Build/AtlasBuildScripts/atlas_git_merge.py source target --remote ''
assert_unmerged 0

# Complicated sweep
# https://gitlab.cern.ch/atlas/athena/-/merge_requests/78769
git checkout -f -B source 0c9ebbf4b7b78e03b762f43a958e9ae6d0d41d8e
git checkout -f -B target ffda64e6dce47eb01aa3efba01051186dcdf1df2
echo "a" | ../Build/AtlasBuildScripts/atlas_git_merge.py source target --remote ''
assert_unmerged 8

echo "OK. All tests succeeded."
