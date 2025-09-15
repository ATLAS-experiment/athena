# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultGeometryTags
from AthenaCommon import Constants
from Campaigns.Utils import Campaign
import sys

flags=initConfigFlags()
#Set a number of flags to avoid input-file peeking
flags.Input.isMC=True
flags.Input.RunNumbers=[1]
flags.Input.TimeStamps=[0]
flags.LAr.doAlign=False
flags.IOVDb.GlobalTag = 'OFLCOND-SDR-BS14T-IBL-06'
flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN2
flags.Input.Files=[]
flags.Input.TypedCollections=[]
flags.Input.MCCampaign=Campaign.Unknown
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
acc=MainEvgenServicesCfg(flags)

from LArGeoAlgsNV.LArGMConfig import LArGMCfg
acc.merge(LArGMCfg(flags))
from TileGeoModel.TileGMConfig import TileGMCfg
acc.merge(TileGMCfg(flags))

acc.addPublicTool(CompFactory.LArFCalTowerBuilderTool())
acc.addEventAlgo(CompFactory.LArFCalTowerBuilderToolTestAlg ('towertest'))

acc.getService("GeoModelSvc").OutputLevel=Constants.WARNING
                 
sys.exit(acc.run(1).isFailure())
