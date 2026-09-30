#!/usr/bin/env bash
#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
set -ex
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputFiles=/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYSLITE/p7267/data23_13p6TeV.00456749.physics_Main.deriv.DAOD_PHYSLITE.r15774_p6304_p7267/DAOD_PHYSLITE.49630893._000015.pool.root.1

outputFile=${ATLAS_CTEST_TESTNAME}.txt

EventGUIDLookup_tf.py --inputFile "$inputFiles" --inputDataType DAOD_PHYSLITE --eventList ${1}/share/${ATLAS_CTEST_TESTNAME}-in.txt --dataType AOD --outputTXTFile "$outputFile"

cat "$outputFile"
