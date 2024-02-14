#! /usr/bin/env bash


DCSC2_SYSTEMS="-sMDT -sTile -sTGC -sRPC -sTDQ -sMagnets -sGlobal -sTRT -sSCT -sLAr -sLucid -sTrigger -sAFP"
#DCSC2_SYSTEMS="-sMDT -sTile -sTGC -sRPC -sTDQ -sCSC -sMagnets -sGlobal -sPixels -sTRT -sLAr -sLucid" #outdated
#DEST_DB=COOLOFL_GLOBAL/COMP200 #outdated
DEST_DB=COOLOFL_GLOBAL/CONDBR2

RUN=$1
shift

if [ -z $RUN ]; then 
    echo "Usage: ./ExecuteDCSC2.sh [run_number]"
    exit 1
fi

echo "Running for $RUN"

export AtlasSetup=/afs/cern.ch/atlas/software/dist/AtlasSetup
pushd /afs/cern.ch/user/a/atlasdqm/ws/DCSCalc/prodarea > /dev/null
source $AtlasSetup/scripts/asetup.sh 24.0.0,Athena
# Parse the major and minor of the Python version
PyVersion=$(python --version | sed -E 's/Python ([0-9]+\.[0-9]+)\.[0-9]+/\1/')
# Add the 'tdaq-' prefix to the TDAQ version if not present
TdaqVersion=$TDAQ_VERSION
if [[ ! $TdaqVersion = tdaq-* ]]; then
    TdaqVersion=tdaq-$TdaqVersion
fi
# Add auth-get-sso-cookie to the path - required by pBeast's ServerProxy
AUTH_GET_SSO_COOKIE="$(ls -d $LCG_RELEASE_BASE/auth_get_sso_cookie/*/$BINARY_TAG | sort -rV | head -n1)"
export PATH="$AUTH_GET_SSO_COOKIE/bin:$PATH"
# Add LCG packages at the end of the Python path - auth-get-sso-cookie dependencies
for package in $LCG_RELEASE_BASE/*/*/$BINARY_TAG/lib/python$PyVersion/site-packages; do
    export PYTHONPATH="$PYTHONPATH:$package"
done
# Add TDAQ external Python packages to the Python path - pBeast and auth-get-sso-cookie dependencies
export PYTHONPATH="$PYTHONPATH:$TDAQ_RELEASE_BASE/tdaq/$TdaqVersion/installed/external/$BINARY_TAG/lib/python$PyVersion/site-packages"
source /afs/cern.ch/user/a/atlasdqm/DQCalculators/DCSCalc/prodarea/build/$BINARY_TAG/setup.sh

export CORAL_AUTH_PATH=/afs/cern.ch/user/a/atlasdqm/private
export CORAL_DBLOOKUP_PATH=/afs/cern.ch/user/a/atlasdqm/private
#export FRONTIER_LOG_LEVEL=debug
export PBEAST_SERVER='https://pc-atlas-www.cern.ch'
export PBEAST_SERVER_SSO_SETUP_TYPE=AutoUpdateKerberos
export PBEAST_SERVER_HTTPS_PROXY='atlasgw.cern.ch:3128'
export REQUESTS_CA_BUNDLE=/etc/pki/tls/certs/ca-bundle.crt

#dcsc.py -h
dcsc.py $@ $DCSC2_SYSTEMS -r$RUN -o$DEST_DB --email-on-failure
#dcsc.py $@ $DCSC2_SYSTEMS -r$RUN -o$DEST_DB
