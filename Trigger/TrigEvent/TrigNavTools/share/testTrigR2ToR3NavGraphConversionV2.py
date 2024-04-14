#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

# Set the Athena configuration flags
from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()

flags.Input.Files=["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1"]
# can browse config for this file here: 
flags.Detector.GeometryLAr=True
flags.Detector.GeometryTile=True
flags.Exec.MaxEvents = -1
flags.Exec.SkipEvents = 0
flags.Trigger.doEDMVersionConversion=True
flags.fillFromArgs()
flags.lock()

# Initialize configuration object, add accumulator, merge, and run.
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.ComponentFactory import CompFactory

from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
cfg = MainServicesCfg(flags)
cfg.merge(PoolReadCfg(flags))

from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
cfg.merge(MetaDataSvcCfg(flags))

confSvc = CompFactory.TrigConf.xAODConfigSvc("xAODConfigSvc")
cfg.addService(confSvc)
from AthenaCommon.Constants import DEBUG

from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
triggerListsHelper = TriggerListsHelper(flags)
chains = triggerListsHelper.Run2TriggerNamesNoTau + triggerListsHelper.Run2TriggerNamesTau

# these are cases to debug further
#chains= ["HLT_g45_tight_L1EM22VHI_xe45noL1"] 

from TrigNavTools.NavConverterConfig import NavConverterCfg
cfg.merge(NavConverterCfg(flags, chainsList=chains, runTheChecker=True))

# input EDM needs calo det descrition for conversion (uff)
from LArGeoAlgsNV.LArGMConfig import LArGMCfg
from TileGeoModel.TileGMConfig import TileGMCfg
cfg.merge(LArGMCfg(flags))
cfg.merge(TileGMCfg(flags))

# enable to get the navigation graphs *.dot files
# from TrigValAlgs.TrigValAlgsConfig import TrigEDMCheckerCfg
# cfg.merge(TrigEDMCheckerCfg(flags, doDumpAll=False))
# cfg.getEventAlgo("TrigEDMChecker").doDumpTrigCompsiteNavigation=True

msg = cfg.getService('MessageSvc'); 
msg.debugLimit=0
msg.infoLimit=0 
msg.warningLimit=0 
msg.Format='% F%35W%C% F%9W%e%7W%R%T %0W%M'
cfg.printConfig(withDetails=True, summariseProps=False) # set True for exhaustive info
sc = cfg.run()
sys.exit(0 if sc.isSuccess() else 1)
