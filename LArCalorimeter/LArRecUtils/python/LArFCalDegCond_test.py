# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration


from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.TestDefaults import defaultGeometryTags
from Campaigns.Utils import Campaign
import sys

flags=initConfigFlags()
#Set a number of flags to avoid input-file peeking
flags.Input.isMC=True
flags.Input.RunNumbers=[1]
flags.Input.TimeStamps=[0]
flags.LAr.doAlign=False
flags.IOVDb.GlobalTag = "COND-MC21-SDR-RUN4-CHAI-06"
flags.IOVDb.UseCREST=True
flags.IOVDb.CrestServer="https://atlas-crest-dev.cern.ch/api-v6.0"
flags.GeoModel.AtlasVersion=defaultGeometryTags.RUN4
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

from LArRecUtils.LArRecUtilsConfig import LArFcalDegCondAlgCfg
acc.merge(LArFcalDegCondAlgCfg(flags))
acc.addEventAlgo(CompFactory.LArFCalDegTestAlg(degradationMap="LArFcalDegMap"))

from AthenaCommon.Constants import DEBUG
acc.getService("IOVDbSvc").Source="CREST"
acc.getService("IOVDbSvc").OutputLevel=DEBUG
acc.getService("IOVDbSvc").DBInstance=""
acc.getEventAlgo("LArFCalDegTestAlg").OutputLevel=DEBUG

sys.exit(acc.run(1).isFailure())
