# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file CaloUtils/share/CaloTowerStore_test.py
# @author scott snyder <snyder@bnl.gov>
# @date Jul, 2013
# @brief Regression tests for CaloTowerStore.
#



from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultGeometryTags
from Campaigns.Utils import Campaign

flags=initConfigFlags()
flags.Input.isMC=True
flags.IOVDb.GlobalTag = 'OFLCOND-SDR-BS14T-IBL-06'
flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
flags.LAr.doAlign=False
flags.Input.MCCampaign=Campaign.Unknown
flags.lock()
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc=MainServicesCfg(flags)

from LArGeoAlgsNV.LArGMConfig import LArGMCfg
from TileGeoModel.TileGMConfig import TileGMCfg
    
acc.merge(LArGMCfg(flags))
acc.merge(TileGMCfg(flags))

# Suppress useless GeoModelSvc messages.
from AthenaCommon import Constants
acc.getService("GeoModelSvc").OutputLevel=Constants.WARNING

acc.addEventAlgo(CompFactory.CaloTowerStoreTestAlg ('towertest'),sequenceName = 'AthAlgSeq')

acc.run(1)

