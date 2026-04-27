#!/usr/bin/env python

#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys

if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # --- set flags
   # the input file


   if len(sys.argv) < 2:
      print('No input file given')
      print('Usage: python ITkPixelDecodingPhaseIIRDOAlgTestJO.py inputRAW')
      sys.exit()
   
   inputRAW = sys.argv[1]
   print(f'ITkPixelDecodingPhaseIIRDOAlgTestJO.py ----------   Running over input file {inputRAW}')

   flags.Input.Files = [inputRAW]
   flags.Output.RDOFileName = "RDO.pool.root"

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
   #flags.GeoModel.Align.Dynamic = False #uncomment for SR1 outputs

   from AthenaConfiguration.TestDefaults import defaultConditionsTags
   flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

   # Set the necessary flags
   flags.PerfMon.doFullMonMT = True
   flags.PerfMon.OutputJSON = 'perfmonmt_test.json'

   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   #add cabling
   from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
   cfg.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge(ByteStreamReadCfg(flags))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingPhaseIIRDOAlgCfg
   cfg.merge( ITkPixelDecodingPhaseIIRDOAlgCfg(flags, nRDOs = 1200000) )

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))

   ## RDO output
   itemList = [] # items to store in RDO
   acceptAlgs = [] # skimming algs
   itemList.append('PhaseIIPixelRawDataContainer#PixelRDOs')

   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   cfg.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))
   
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True

   cfg.run(50)





