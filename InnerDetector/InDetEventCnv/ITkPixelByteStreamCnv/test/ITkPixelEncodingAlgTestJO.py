#!/usr/bin/env python

#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RDO_RUN4

   import sys
   if len(sys.argv) > 1:
      flags.Output.BSFileName = sys.argv[1]
   
   flags.lock()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
   cfg.merge(PoolReadCfg(flags))

   from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
   cfg.merge( ITkPixelEncodingAlgCfg(flags) )

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
   cfg.merge(ByteStreamWriteCfg(flags))

   # cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   # sg = cfg.getService("StoreGateSvc")
   # sg.Dump = True

   status = cfg.run(1)
   sys.exit(0 if status.isSuccess() else 1)
