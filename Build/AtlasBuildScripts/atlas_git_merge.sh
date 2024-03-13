#!/usr/bin/env bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Script to merge athena branches and applying a standard ignore list.
#
# Author: Frank Winklmeier
#

source=""
target="main"

# files/directories to ignore in merges (can be wildcards)
ignore=(
    "Projects/"
    "Build/"
    "Tools/WorkflowTestRunner/python/References.py"
)

usage() {
    cat <<EOF
Usage: atlas_git_merge.sh source [target]
Merge git source branch into target applying a standard ignore list.
Run this within a full athena repository checkout.

  source  git source branch (e.g. 24.0)
  target  git target branch [${target}]

The following directories/files are ignored in merges:
EOF
    for files in "${ignore[@]}"; do
        echo "  - ${files}"
    done
}

if [[ $# -eq 0 ]] || [[ $# -gt 2 ]] || [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
    usage
    exit 1
fi

source="$1"
if [[ $# -ge 2 ]]; then
    target="$2"
fi

# Colors
RED="[97;101;1m"
RESET="[m"

# Fetch upstream and create merge branch
git fetch upstream
git checkout -B "sweep_${source}_${target}_`date +%Y-%m-%d`" upstream/${target}

# Merge without commit
git merge --no-commit upstream/${source}

# File changes
diff0=`git diff HEAD --name-only`

# Restore ignored files
for files in "${ignore[@]}"; do
    git restore --staged --worktree --ignore-unmerged --source=upstream/${target} -- "${files}"
done

# Show what has been rejected
diff1=`git diff HEAD --name-only`
if [[ "${diff0}" != "${diff1}" ]]; then
    echo
    echo -e "${RED}The changes to the following files were rejected:${RESET}"
    diff --color=auto <(echo "${diff0}") <(echo "${diff1}")
    echo
fi

# Show current git status
echo
git status

cat <<EOF

Now finalize the merge:
 1) Resolve any remaining conflicts
 2) git commit OR git merge --continue
 3) git push origin
 4) ./Build/AtlasBuildScripts/prepare_release_notes.py --sweep -t GITLAB_TOKEN
EOF
