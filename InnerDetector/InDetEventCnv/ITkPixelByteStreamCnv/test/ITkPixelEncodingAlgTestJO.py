#!/usr/bin/env python

#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


# # Example to add additional algorithms:
# from AthenaConfiguration.ComponentFactory import CompFactory
# def MyAlgCfg(flags, name='MyAlg', **kwargs):
#    acc = ComponentAccumulator()
#    kwargs.setdefault(',"Property', default_value)
#    acc.addEventAlgo(CompFactory.MyAlg(name,
#                                       **kwargs))
#    return acc


if __name__=="__main__":
   # test job skeleton reading pool files

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   flags = initConfigFlags()

   # make logging more verbose
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import DEBUG
   log.setLevel(DEBUG)
   
   # --- set flags
   # the input file
   #flags.Input.Files = ['/eos/user/s/sroygara/ITk/BytestreamDev/run/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000028.pool.root.1']
   #flags.Input.Files = ['/eos/user/o/okovanda/data/ITk/DAQ/RDO/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33628990._000069.pool.root.1']
   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RDO_RUN4
   
   # --- end flag customization
   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
   cfg.merge(PoolReadCfg(flags))

   from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))


   # example runs pixel clusterization
   from ITkPixelByteStreamCnv.ITkPixelEncodingAlgConfig import ITkPixelEncodingAlgCfg
   cfg.merge( ITkPixelEncodingAlgCfg(flags) )

   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
   
   #try and write it in a BS file
   cfg.merge(ByteStreamWriteCfg(flags, ['ITkPixelRDO_Container#ITkPixelRDOs']))
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

 
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True

   # loop over 10 events
   cfg.run(1)

