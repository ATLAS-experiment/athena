#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # --- set flags
   # the input file

   flags.Input.Files = ['/afs/cern.ch/work/o/okovanda/ITk/DAQ/encoding_in_athena/run/data_test.00242000.Single_Stream.daq.RAW._lb0001._Athena._0001.data']

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4

   flags.IOVDb.GlobalTag = 'OFLCOND-MC15c-SDR-14-05'

   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge(ByteStreamReadCfg(flags))

   from ITkPixelByteStreamCnv.ITkPixelDecodingAlgConfig import ITkPixelDecodingAlgCfg
   cfg.merge( ITkPixelDecodingAlgCfg(flags) )

   from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

   
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
 
   cfg.run(15)





