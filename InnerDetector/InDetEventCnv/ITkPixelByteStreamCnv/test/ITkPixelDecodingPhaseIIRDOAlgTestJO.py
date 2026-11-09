#!/usr/bin/env python

#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import sys
from AthenaCommon.Constants import DEBUG

if __name__=="__main__":
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   if len(sys.argv) < 2:
      print('No input file given')
      print('Usage: python ITkPixelDecodingPhaseIIRDOAlgTestJO.py inputRAW')
      sys.exit()
   
   inputRAW = sys.argv[1]
   print(f'ITkPixelDecodingPhaseIIRDOAlgTestJO.py ----------   Running over input file {inputRAW}')

   flags.Input.Files = [inputRAW]
   flags.Output.RDOFileName = "RDO_phaseII.pool.root"

   from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
   flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC
   #flags.GeoModel.Align.Dynamic = False #uncomment for SR1 outputs

   # We want to keep the commented code for debugging
   # flags.PerfMon.doFullMonMT = True
   # flags.PerfMon.OutputJSON = 'perfmonmt_test.json'

   flags.lock()
   # flags.dump()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge(ByteStreamReadCfg(flags))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingPhaseIIRDOAlgCfg
   cfg.merge( ITkPixelDecodingPhaseIIRDOAlgCfg(flags, nRDOs = 1200000, OutputLevel = DEBUG) )

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))

   ## RDO output
   itemList = [] # items to store in RDO
   acceptAlgs = [] # skimming algs
   itemList.append('PhaseIIPixelRawDataContainer#PixelRDOs')

   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   cfg.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))
   
   # cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   # sg = cfg.getService("StoreGateSvc")
   # sg.Dump = True

   status = cfg.run()
   sys.exit(0 if status.isSuccess() else 1)
