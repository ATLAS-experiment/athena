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

# simple mu test cases
chains = \
['HLT_2e12_lhloose_nod0_mu10', 'HLT_2mu14', 'HLT_2mu20_L12MU20_OVERLAY', 'HLT_2mu4_bJpsimumu', 'HLT_2mu4_bUpsimumu', 'HLT_2mu6_bJpsimumu', 'HLT_2mu6_bUpsimumu', 'HLT_3mu3_mu3noL1_L13MU4', 'HLT_3mu4', 'HLT_3mu4_mu2noL1', 'HLT_3mu6', 'HLT_3mu6_msonly', 'HLT_3mu6_msonly_L1MU4_UNPAIRED_ISO', 'HLT_3mu6_msonly_L1MU6_EMPTY', 'HLT_4mu4', 'HLT_e12_lhloose_nod0_2mu10', 'HLT_e17_lhloose_nod0_mu14', 'HLT_e26_lhmedium_nod0_mu8noL1', 'HLT_e7_lhmedium_nod0_mu24', 'HLT_e9_lhvloose_nod0_mu20_mu8noL1', 'HLT_e9_lhvloose_nod0_mu20_mu8noL1_L1EM7_MU20', 'HLT_mu10', 'HLT_mu10_bJpsi_TrkPEB', 'HLT_mu10_bJpsi_Trkloose', 'HLT_mu11_bJpsi_TrkPEB', 'HLT_mu11_mu6_bJpsimumu', 'HLT_mu11_mu6_bUpsimumu', 'HLT_mu13_mu13_idperf_Zmumu', 'HLT_mu14', 'HLT_mu14_ivarloose_L1MU11_tau35_medium1_tracktwoEF_L1MU11_TAU20IM', 'HLT_mu14_ivarloose_L1MU11_tau35_medium1_tracktwo_L1MU11_TAU20IM', 'HLT_mu14_ivarloose_L1MU11_tau35_mediumRNN_tracktwoMVA_L1MU11_TAU20IM', 'HLT_mu14_ivarloose_tau25_medium1_tracktwo', 'HLT_mu14_ivarloose_tau25_medium1_tracktwoEF', 'HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1DR-MU10TAU12I_TAU12I-J25', 'HLT_mu14_ivarloose_tau25_medium1_tracktwoEF_L1MU10_TAU12IM_3J12', 'HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1DR-MU10TAU12I_TAU12I-J25', 'HLT_mu14_ivarloose_tau25_medium1_tracktwo_L1MU10_TAU12IM_3J12', 'HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA', 'HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1DR-MU10TAU12I_TAU12I-J25', 'HLT_mu14_ivarloose_tau25_mediumRNN_tracktwoMVA_L1MU10_TAU12IM_3J12', 'HLT_mu14_ivarloose_tau35_medium1_tracktwo', 'HLT_mu14_ivarloose_tau35_medium1_tracktwoEF', 'HLT_mu14_ivarloose_tau35_mediumRNN_tracktwoMVA', 'HLT_mu18', 'HLT_mu20', 'HLT_mu20_2mu2noL1_JpsimumuFS', 'HLT_mu20_2mu4_JpsimumuL2', 'HLT_mu20_2mu4noL1', 'HLT_mu20_bJpsi_TrkPEB', 'HLT_mu20_ivarmedium_mu8noL1', 'HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan05_L1MU4_UNPAIRED_ISO', 'HLT_mu20_msonly_iloosems_mu6noL1_msonly_nscan05_L1MU6_EMPTY', 'HLT_mu20_msonly_mu15noL1_msonly_nscan05_noComb', 'HLT_mu20_msonly_mu15noL1_msonly_nscan05_noComb_L1MU4_UNPAIRED_ISO', 'HLT_mu20_msonly_mu15noL1_msonly_nscan05_noComb_L1MU6_EMPTY', 'HLT_mu20_msonly_mu6noL1_msonly_nscan05', 'HLT_mu22', 'HLT_mu22_mu8noL1', 'HLT_mu24', 'HLT_mu24_ivarmedium', 'HLT_mu26_ivarmedium', 'HLT_mu4', 'HLT_mu4_bJpsi_TrkPEB', 'HLT_mu4_bJpsi_Trkloose', 'HLT_mu4_j15_boffperf_split_dr05_dz02', 'HLT_mu4_j25_boffperf_split_dr05_dz02', 'HLT_mu4_j35_boffperf_split_dr05_dz02', 'HLT_mu4_j45_gsc55_boffperf_split_dr05_dz02', 'HLT_mu4_mu4_idperf_bJpsimumu_noid', 'HLT_mu50', 'HLT_mu6', 'HLT_mu60_0eta105_msonly', 'HLT_mu6_2mu4', 'HLT_mu6_bJpsi_TrkPEB', 'HLT_mu6_bJpsi_Trkloose', 'HLT_mu6_bJpsi_lowpt_TrkPEB', 'HLT_mu6_dRl1_mu20_msonly_iloosems_mu6noL1_dRl1_msonly', 'HLT_mu6_idperf', 'HLT_mu6_j110_gsc150_boffperf_split_dr05_dz02', 'HLT_mu6_j150_gsc175_boffperf_split_dr05_dz02', 'HLT_mu6_j175_gsc260_boffperf_split_dr05_dz02', 'HLT_mu6_j60_gsc85_boffperf_split_dr05_dz02', 'HLT_mu6_j85_gsc110_boffperf_split_dr05_dz02', 'HLT_mu6_mu4_bJpsimumu', 'HLT_mu6_mu4_bUpsimumu', 'HLT_mu6_nomucomb_2mu4_nomucomb_L1MU6_3MU4', 'HLT_mu80_msonly_3layersEC']
# chains=['HLT_mu22_mu8noL1_TagandProbe']
# chains=['HLT_mu50']
# chains=["HLT_tau35_medium1_tracktwo_tau25_medium1_tracktwo_03dR30_L1DR-TAU20ITAU12I-J25"]
# chains = [] # all chains processing
from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
TriggerAPI.setConfigFlags(flags)
chains = list(TriggerAPI.getAllHLT(TriggerPeriod.y2018).keys())

# these are cases to debug further
#chains= ["HLT_g45_tight_L1EM22VHI_xe45noL1"] 

from TrigNavTools.NavConverterConfig import NavConverterCfg
cfg.merge(NavConverterCfg(flags, chainsList=chains))

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
