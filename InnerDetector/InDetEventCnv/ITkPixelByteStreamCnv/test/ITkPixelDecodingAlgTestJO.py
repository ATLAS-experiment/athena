#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import sys

if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # --- set flags
   # the input file

   if len(sys.argv) < 2:
      print('No input file given')
      inputRAW = '/afs/cern.ch/work/f/fballi/private/athena/run_UI_ITk/encode/data_test.00242020.Single_Stream.daq.RAW._lb0002._Athena._0201.data'
   
   inputRAW = sys.argv[1]


   flags.Input.Files = [inputRAW] #ttbar, Fabrice
   flags.Output.RDOFileName = "RDO.pool.root"

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4

   #flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2017-05' #uncomment for SR1 outputs
   flags.IOVDb.GlobalTag = 'OFLCOND-MC15c-SDR-14-05'
   
   #flags.GeoModel.Align.Dynamic = False #uncomment for SR1 outputs

   # Set the necessary flags
   flags.PerfMon.doFullMonMT = True
   flags.PerfMon.OutputJSON = 'perfmonmt_test.json'
   
   # We want to keet the commented code for debugging
   from AthenaCommon.Constants import DEBUG
   flags.Exec.OutputLevel=DEBUG
   flags.ITk.Conditions.PixelTestCablingFallback=True
   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
   cfg.merge(ByteStreamReadCfg(flags))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingAlgCfg
   cfg.merge( ITkPixelDecodingAlgCfg(flags) )

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))


   itemList = [] # items to store in RDO
   acceptAlgs = [] # skimming algs
   itemList.append('PixelRDO_Container#ITkPixelRDOs')

   from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
   cfg.merge(OutputStreamCfg(flags, 'RDO', itemList, AcceptAlgs=acceptAlgs))
   
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True

   cfg.run(10)





