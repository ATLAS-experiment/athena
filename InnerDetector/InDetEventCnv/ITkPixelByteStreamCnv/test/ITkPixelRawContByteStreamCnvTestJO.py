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
   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags.Input.Files = defaultTestFiles.RDO_RUN4

   flags.Concurrency.NumThreads = 1
   
   # --- end flag customization
   flags.lock()


   flags.dump()
   # minimum stuff to read files:
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   cfg = MainServicesCfg(flags)

   from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
   cfg.merge(PoolReadCfg(flags))

   #add cabling
   from ITkPixelCabling.ITkPixelCablingAlgConfig import ITkPixelCablingAlgCfg
   cfg.merge(ITkPixelCablingAlgCfg(flags, name="ITkPixelCablingAlg", UseTestCabling=True))


   from PixelReadoutGeometry.PixelReadoutGeometryConfig import ITkPixelReadoutManagerCfg
   cfg.merge(ITkPixelReadoutManagerCfg(flags))


   # example runs pixel clusterization
   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelTranslatorAlgCfg
   cfg.merge( ITkPixelTranslatorAlgCfg(flags) )

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
   
   #try and write it in a BS file
   cfg.merge(ByteStreamWriteCfg(flags, ['ITkPixelRDO_Container#ITkPixelRDOs']))

   #explicitly add cabling
   bs_alg=cfg.getEventAlgo('BSOutputStreamAlg')
   bs_alg.ExtraInputs.add(('ITkPixelCablingData', 'ConditionStore+ITkPixelCablingData'))

   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
   
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True


   # loop over 1 event
   cfg.run(1)
