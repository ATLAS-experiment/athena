# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Joboptions fragment that should be post-included after a typicaly configured Athena job
# It will read original input files, create a list of Events in memory, sort them and produce
# an intermediate Event reference file that Athena will read instead of the original inputs

# Event information is read from EventInfoTags (stored by default in all Athena data files)
# The default sort key value (LumiBlockN) can be overridden. The sorting order is ascending.

import os

from AthenaCommon.AppMgr import ServiceMgr
inputs = ServiceMgr.EventSelector.InputCollections

# set default sort parameters, read overrides from locals()
tmpCollFile = locals().get("AthenaInputSortCollName", "sortedEventRefs" + str(os.getpid()) + ".root")
sortTag     = locals().get("AthenaInputSortTag",      "LumiBlockN")

from OutputStreamAthenaPool.SortedInputFileCreator import SortedInputFileCreator
sorter = SortedInputFileCreator()
# Sort Inputs based on one of the EventInfoTag attributes
# Store sorted event references in a temporary file
# This should run as postInclude, so we assume EventSelector.InputCollections is set earlier
sorter.execute(inputs, outputFile=tmpCollFile, sortAttribute=sortTag)

# Reading Events through References require a populated FileCatalog
for inpfile in inputs:
    os.system('pool_insertFileToCatalog {}'.format(inpfile))

ServiceMgr.PoolSvc.AttemptCatalogPatch = False

# Tell Athena to use the sorted collection instead of the original inputs
ServiceMgr.EventSelector.InputCollections = [tmpCollFile]
