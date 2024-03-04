#! /usr/bin/env bash
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


DCSC2_SYSTEMS=${DCSC_SYSTEMS:-"-sMDT -sTile -sTGC -sRPC -sTDQ -sMagnets -sGlobal -sTRT -sSCT -sLAr -sLucid -sTrigger -sAFP -sMMG -sSTG"}
DCSC2_ARGS=${DCSC_ARGS:-""}
# the bottom should be overridden by environment variables
DEST_DB=${DCSC_DEST_DB:-"sqlite://;schema=test.db;dbname=CONDBR2"}

RUN=$1
shift

if [ -z $RUN ]; then 
    echo "Usage: ./ExecuteDCSC2.sh [run_number]"
    exit 1
fi

echo "Running for $RUN"

export AtlasSetup=/cvmfs/atlas.cern.ch/repo/ATLASLocalRootBase/x86_64/AtlasSetup/current/AtlasSetup
source $AtlasSetup/scripts/asetup.sh ${ATLAS_RELEASE:-24.0.25},Athena
if [ -n "${DCSC_BUILD_DIR}" ]; then 
    source ${DCSC_BUILD_DIR}/*/setup.sh
fi

export CORAL_AUTH_PATH=${DCSC_AUTH_PATH:-/afs/cern.ch/user/a/atlasdqm/private}
export CORAL_DBLOOKUP_PATH=${DCSC_AUTH_PATH:-/afs/cern.ch/user/a/atlasdqm/private}
echo "Authentication from" $CORAL_AUTH_PATH
unset FRONTIER_SERVER
#export FRONTIER_LOG_LEVEL=debug
export PBEAST_SERVER_HTTPS_PROXY='atlasgw.cern.ch:3128'

dcsc.py $@ $DCSC2_SYSTEMS $DCSC2_ARGS -r$RUN -o$DEST_DB --email-on-failure
