#! /usr/bin/env python
# This script is designed to be used as a quick runtime ctest in the CI framework
# We use python to better handle getting our validation cutfile which should be stable
# We will augment the NEvents line in our cutfile to limit the job
# This is primarily to catch any runtime changes introduced before they are merged

from CI_test import CITest
import sys
import ROOT

cutfilename = "CI-validation-cuts-Run3.txt"
cutfilepath = ROOT.PathResolver.find_file(cutfilename,
                                          "DATAPATH",
                                          ROOT.PathResolver.RecursiveSearch)

returnCode = CITest("DAOD_PHYS DATA",
                    cutfilename,
                    cutfilepath,
                    "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev/AnalysisTop/ContinuousIntegration/R22-Run3/Data/p5858/DAOD_PHYS.34860568._000319.pool.root.1",
                    [("#NEvents.*", "NEvents 5000")])

if returnCode != 0:
    print("Error in DAOD_PHYS Data22")
    sys.exit(returnCode)

# -- Return 0 as all tests were successful -- #
sys.exit(0)
