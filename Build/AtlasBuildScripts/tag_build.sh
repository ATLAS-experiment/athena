#! /bin/bash
#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

REPOURL="https://:@gitlab.cern.ch:8443/atlas/athena.git"
DATESTAMP=""

# Function printing the usage information for the script
usage() {
    cat <<EOF
Usage: tag_build.sh [-d date_stamp] [-u repository_url]
       Tag a 'nightly' build based on the current branch and timestamp. If the tag
       already exists it will be checked out instead. This script should only be used
       for tagging nightly builds, not for private builds.

       date_stamp        Format YYYY-MM-DDTHHMM
       repository_url    git repository URL (default: $REPOURL)
EOF
}

# Parse the command line arguments:
while getopts ":d:u:h" opt; do
    case $opt in
        d)
            DATESTAMP=$OPTARG
            ;;
        u)
            REPOURL=$OPTARG
            ;;
        h)
        	usage
        	exit 0
        	;;
        :)
            echo "Argument -$OPTARG requires a parameter!"
            usage
            exit 1
            ;;
        ?)
            echo "Unknown argument: -$OPTARG"
            usage
            exit 1
            ;;
    esac
done

# Check if datestamp is valid
echo "$DATESTAMP" | grep -E --quiet "^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{4}$"
if [ $? != "0" ]; then
	echo "Datestamp '$DATESTAMP' does not correspond to the ATLAS format YYYY-MM-DDTHHMM"
	exit 1
fi

# Abort if any of the next commands fail
set -e

# Use the source of this script to ensure that we cd to within the root of the
# local repository
ScriptSrcDir=$(dirname ${BASH_SOURCE[0]})
cd $ScriptSrcDir/../..

# Get branch name, then tag/push or checkout if it already exists
BRANCH=$(git symbolic-ref --short HEAD)
TAG="nightly/$BRANCH/$DATESTAMP"

# Update our local tags in case they are already out of date
git fetch --tags

if git show-ref --quiet --tags $TAG; then
    echo "Tag $TAG already exists. Doing checkout..."
    git checkout $TAG
else
    echo "Creating tag $TAG"
    git tag $TAG
    if ! git push $REPOURL $TAG; then
        # Extremly unlikely race condition if multiple tag_build.sh run in parallel
        # and create the same tag from a different commit hash because a commit was
        # done on the branch in the meantime.
        echo "Tag $TAG already exists on the remote with a different commit hash. Doing checkout..."
        git fetch --tags --force
        git checkout $TAG
    fi
fi
