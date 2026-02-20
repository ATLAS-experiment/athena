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

   #make logging more verbose
   from AthenaCommon.Logging import log
   from AthenaCommon.Constants import INFO #DEBUG
   log.setLevel(INFO)
   
   # --- set flags
   # the input file
   flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1']
   #from AthenaConfiguration.TestDefaults import defaultTestFiles
   #flags.Input.Files = defaultTestFiles.RDO_RUN4
   
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
   cfg.merge(ITkPixelReadoutManagerCfg(flags, name="ITkPixelReadoutManager"))

   from ITkPixelByteStreamCnv.ITkPixelByteStreamCnvConfig import ITkPixelEncodingAlgCfg
   cfg.merge( ITkPixelEncodingAlgCfg(flags) )

   from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamWriteCfg
   #
   ##try and write it in a BS file
   cfg.merge(ByteStreamWriteCfg(flags))
   cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

 
   #dump what's in SG
   sg = cfg.getService("StoreGateSvc")
   sg.Dump = True

   # loop over 1 events
   cfg.run(1)

