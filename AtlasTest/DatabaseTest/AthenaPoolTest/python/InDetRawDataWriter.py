# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.DetectorConfigFlags import enableDetectors
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Exec.MaxEvents = 20
flags.Input.Files = []
flags.GeoModel.AtlasVersion = "ATLAS-R2-2016-01-00-01"
flags.IOVDb.GlobalTag = "OFLCOND-SDR-BS7T-04-00"
enableDetectors(flags, ["Pixel", "SCT", "TRT"])
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainEvgenServicesCfg
acc = MainEvgenServicesCfg(flags)

# GeoModel
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
acc.merge( GeoModelCfg(flags) )

# Pool writing
acc.addEventAlgo( CompFactory.InDetRawDataFakeWriter(OutputLevel = DEBUG),
                  sequenceName = 'AthAlgSeq' )

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge( OutputStreamCfg(flags, "InDetRDO", disableEventTag = True,
                           ItemList = ["PixelRDO_Container#*",
                                       "SCT_RDO_Container#*",
                                       "TRT_RDO_Container#*",
                                       "EventInfo#*",
                                       "PixelRDOElemLinkVec#*"]) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
