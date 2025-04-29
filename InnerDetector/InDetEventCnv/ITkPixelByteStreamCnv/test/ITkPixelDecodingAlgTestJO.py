#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # --- set flags
   # the input file

   flags.Input.Files = ['test.bs']

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4

   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge (ByteStreamReadCfg (flags, ['std::vector<std::vector<uint32_t>>/ITkEncodedStream']))


   from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

   from ITkPixelByteStreamCnv.ITkPixelDecodingAlgConfig import ITkPixelDecodingAlgCfg
   cfg.merge( ITkPixelDecodingAlgCfg(flags) )
   
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
 
   cfg.run(15)





