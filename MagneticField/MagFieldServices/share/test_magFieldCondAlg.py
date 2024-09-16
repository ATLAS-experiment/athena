#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Test magnetic field conditions algs with varying currents.
#
# Folder name
import sys
from MagFieldServices.createDBForTest import createDB
folder = '/EXT/DCS/MAGNETS/SENSORDATA'
sqlite = 'magfieldForTest.db'


# Testing IOVs and currents: (since LB, solenoid, toroids)
# Default test - should read both mag field files, and turn off fields for events 5 to 9, and back on for 10 to 14
currents = [(0, 7730, 20400),
            (5, 0, 0),
            (10, 7730, 20400)]

# Optional test: only toroid is on for whole run. Scale factor for solenoid will become 1 at event 5,
# but solenoid field will still be off
# currents = [(0, 0, 20400),
#             (5, 7730, 20400),
#             (10, 7730, 20400)]

# Optional test: only solenoid is on for whole run. Scale factor for toroid will become 1 at event 5,
# but toroid field will still be off
# currents = [(0, 7730, 0),
#             (5, 7730, 20400),
#             (10, 7730, 20400)]


# Create sqlite file with DCS currents
createDB(folder, sqlite, currents)

from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AllConfigFlags import initConfigFlags

flags = initConfigFlags()
flags.Input.Files = []
flags.Concurrency.NumThreads = 1
flags.Exec.MaxEvents = currents[-1][0]+5   # 5 events per IOV
flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-RUN2-01'
flags.IOVDb.SqliteInput = sqlite
flags.IOVDb.SqliteFolders = (folder,)
flags.lock()

acc = MainEvgenServicesCfg(flags)
acc.getService('EventSelector').EventsPerLB = 1

acc.merge(AtlasFieldCacheCondAlgCfg(flags, LockMapCurrents=False))

acc.addEventAlgo(CompFactory.MagField.CondReader('MagFieldCondReader'))

sys.exit(acc.run().isFailure())
