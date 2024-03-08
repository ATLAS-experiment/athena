#!/usr/bin/env python
#
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# Test magnetic field conditions algs with varying currents.
#

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

# Folder name
folder = '/EXT/DCS/MAGNETS/SENSORDATA'
sqlite = 'magfield.db'

def createDB():
   """Create sqlite file with DCS currents"""
   import os
   os.environ['CLING_STANDARD_PCH'] = 'none' #See bug ROOT-10789
   from PyCool import cool
   from CoolConvUtilities import AtlCoolLib, AtlCoolTool

   # Cleanup previous file
   if os.path.isfile(sqlite):
      os.remove(sqlite)

   db = cool.DatabaseSvcFactory.databaseService().createDatabase(f'sqlite://;schema={sqlite};dbname=CONDBR2')
   spec = cool.RecordSpecification()
   spec.extend("value", cool.StorageType.Float)
   spec.extend("quality_invalid", cool.StorageType.Bool)
   f = AtlCoolLib.ensureFolder(db, folder, spec, AtlCoolLib.athenaDesc(True, 'CondAttrListCollection'))

   for v in currents:
      sol = cool.Record(spec)
      sol['value'] = v[1]
      sol['quality_invalid'] = False
      tor = cool.Record(spec)
      tor['value'] = v[2]
      tor['quality_invalid'] = False
      f.storeObject(v[0], cool.ValidityKeyMax, sol, 1)  # channel 1
      f.storeObject(v[0], cool.ValidityKeyMax, tor, 3)  # channel 3

   # print database content
   act = AtlCoolTool.AtlCoolTool(db)
   print (act.more(folder))


# Create sqlite file with DCS currents
createDB()

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg

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

from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
acc.merge( AtlasFieldCacheCondAlgCfg(flags, LockMapCurrents=False) )

acc.addEventAlgo( CompFactory.MagField.CondReader('MagFieldCondReader') )

import sys
sys.exit(acc.run().isFailure())
