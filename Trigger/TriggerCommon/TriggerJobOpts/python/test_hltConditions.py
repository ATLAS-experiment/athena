#!/usr/bin/env athena
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
###########################################################################
# Job options to test HLT conditions updates based on local sqlite file
#
# To be used to test HLT conditions updates (in sqlite files) before
# deployment in the online DB.
#
# Usage:
#  athena --evtMax=10 TriggerJobOpts/test_hltConditions.py [flags...]
#
# Author: Frank Winklmeier
###########################################################################

# Update these values to your need:
sqlite = 'noise.db'                   # sqlite file with new conditions
onldb = 'ATLAS_COOLONL_CALO'
folder = '/CALO/Noise/CellNoise'


def createSqlite(flags):
   """Create local sqlite file"""
   import os
   try:
      os.remove(flags.IOVDb.SqliteInput)   # Delete temporary file
   except Exception:
      pass

   # Copy last IOV from online DB
   rc = os.system(f"AtlCoolCopy 'oracle://ATLAS_COOLPROD;schema={onldb};dbname=CONDBR2' 'sqlite://;schema={flags.IOVDb.SqliteInput};dbname=CONDBR2' -create -tag {flags.IOVDb.GlobalTag} -hitag -prunetags -folder {folder} -r 9999999")
   if rc!=0:
      raise RuntimeError("Cannot copy folder from COOL")

   # Merge with user given sqlite file (in case only some channels have been updated)
   rc = os.system(f"AtlCoolCopy 'sqlite://;schema={sqlite};dbname=CONDBR2' 'sqlite://;schema={flags.IOVDb.SqliteInput};dbname=CONDBR2' -folder {folder} -r 9999999")
   if rc!=0:
      raise RuntimeError("Cannot merge sqlite file")


# Set flags
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

flags.Concurrency.NumThreads = 1
#flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data22_13p6TeV.00428770.express_express.merge.RAW._lb0221._SFO-ALL._0001.1']
flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data22_cos.00433897.express_express.merge.RAW._lb0134._SFO-ALL._0001.1']
flags.IOVDb.SqliteInput = 'cool_tmp.db'  # created by createSqlite
flags.IOVDb.SqliteFolders = (folder,)

flags.Trigger.L1.errorOnMissingTOB=False

from TriggerJobOpts.TriggerConfigFlags import trigGlobalTag
flags.IOVDb.GlobalTag=trigGlobalTag(flags)

# Create sqlite file
createSqlite(flags)

# Configure HLT
from TriggerJobOpts.runHLT import athenaCfg
acc = athenaCfg(flags)

# Force IOV for folder
iovDbSvc = acc.getService('IOVDbSvc')
for i,f in enumerate(iovDbSvc.Folders):
   if folder in f:
      iovDbSvc.Folders[i] += '<forceRunNumber>9999999</forceRunNumber>'
# allow missing ROB in the input file
l1bsdec = acc.getEventAlgo("L1TriggerByteStreamDecoder")
l1bsdec.MaybeMissingROBs += [0x941000]

# Run
import sys
sys.exit(acc.run().isFailure())
