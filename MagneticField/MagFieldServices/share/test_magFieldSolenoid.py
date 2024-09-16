#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Test magnetic field conditions algs with varying currents.
#
# Folder name
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
import sys
from MagFieldServices.createDBForTest import createDB
folder = '/EXT/DCS/MAGNETS/SENSORDATA'
sqlite = 'magFieldSolenoid.db'


# Default test - should read both mag field fields
currents = [(0, 7730, 20400)]

# Create sqlite file with DCS currents
createDB(folder, sqlite, currents)


flags = initConfigFlags()
flags.Input.Files = []
flags.Exec.MaxEvents = 1
flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-RUN2-01'
flags.IOVDb.SqliteInput = sqlite
flags.IOVDb.SqliteFolders = (folder,)
flags.lock()

acc = MainEvgenServicesCfg(flags)
acc.getService('EventSelector').EventsPerLB = 1

acc.merge(AtlasFieldCacheCondAlgCfg(flags, LockMapCurrents=False))

acc.addEventAlgo(CompFactory.MagField.SolenoidTest('MagFieldSolenoidTest',
                                                   StepsR=5, StepsZ=5,
                                                   StepsPhi=5, HistStreamName="SolenoidTest"))
acc.addService(CompFactory.THistSvc(
    Output=["SolenoidTest DATAFILE='solenoidTest.root' OPT='RECREATE'"]))

sys.exit(acc.run().isFailure())
