#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys
from AthenaCommon.Constants import DEBUG

if __name__=="__main__":
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   if len(sys.argv) < 2:
      print('No input file given')
      inputRAW = '/afs/cern.ch/work/f/fballi/private/athena/run_UI_ITk/encode/data_test.00242020.Single_Stream.daq.RAW._lb0002._Athena._0201.data'
   else:
      inputRAW = sys.argv[1]

   flags.Input.Files = [inputRAW] #ttbar, Fabrice
   flags.Output.RDOFileName = "RDO.pool.root"

   from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
   flags.IOVDb.GlobalTag = defaultConditionsTags.RUN4_MC

   # We want to keep these comments for memory of the SR1 config 
   # flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2017-05' #uncomment for SR1 outputs
   # flags.GeoModel.Align.Dynamic = False #uncomment for SR1 outputs
   
   # We want to keet the commented code for debugging
   # flags.PerfMon.doFullMonMT = True
   # flags.PerfMon.OutputJSON = 'perfmonmt_test.json'

   flags.lock()
   # flags.dump()

   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge(ByteStreamReadCfg(flags))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingAlgCfg
   cfg.merge( ITkPixelDecodingAlgCfg(flags, OutputLevel=DEBUG) )

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))

   itemList = [] # items to store in RDO
   acceptAlgs = [] # skimming algs
   itemList.append('PixelRDO_Container#ITkPixelRDOs')

   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   cfg.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))
   
   # cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   # sg = cfg.getService("StoreGateSvc")
   # sg.Dump = True

   status = cfg.run(10)
   sys.exit(0 if status.isSuccess() else 1)
