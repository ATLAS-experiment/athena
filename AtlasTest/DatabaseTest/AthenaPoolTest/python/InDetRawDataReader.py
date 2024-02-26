# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.DetectorConfigFlags import enableDetectors
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Input.Files = ["myInDetRDO.pool.root"]
flags.Input.ProcessingTags = []  # to avoid input file peeking
flags.GeoModel.AtlasVersion = "ATLAS-R2-2016-01-00-01"
flags.IOVDb.GlobalTag = "OFLCOND-SDR-BS7T-04-00"
enableDetectors(flags, ["Pixel", "SCT", "TRT"])
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

# GeoModel
from AtlasGeoModel.GeoModelConfig import GeoModelCfg
acc.merge( GeoModelCfg(flags) )

# Pool reading
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
acc.merge( PoolReadCfg(flags) )

acc.addEventAlgo( CompFactory.InDetRawDataFakeReader(OutputLevel = DEBUG),
                  sequenceName = 'AthAlgSeq' )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
