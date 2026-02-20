#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # --- set flags
   # the input file

   #flags.Input.Files = ['/afs/cern.ch/work/o/okovanda/ITk/DAQ/encoding_in_athena/run/data_test.00242000.Single_Stream.daq.RAW._lb0001._Athena._0101.data'] #single muon
   flags.Input.Files = ['/afs/cern.ch/work/o/okovanda/ITk/DAQ/encoding_in_athena/run/data_test.00350200.Single_Stream.daq.RAW._lb0008._Athena._1001.data'] #ttbar
   #flags.Input.Files = ['/eos/user/o/okepka/public/itk/forOndra/SR1/data_test.1749797964.calibration_DcmDummyProcessor.daq.RAW._lb0000._SFO-SR1._0001.data']

   from AthenaConfiguration.TestDefaults import defaultGeometryTags
   flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4

   #flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2017-05' #uncomment for SR1 outputs
   flags.IOVDb.GlobalTag = 'OFLCOND-MC15c-SDR-14-05'
   
   #flags.GeoModel.Align.Dynamic = False #uncomment for SR1 outputs

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

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelDecodingAlgCfg
   cfg.merge( ITkPixelDecodingAlgCfg(flags) )

   from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
   cfg.merge(PerfMonMTSvcCfg(flags))


   #from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   #cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

   
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True

   cfg.run(1000)





