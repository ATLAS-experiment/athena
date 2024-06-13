#!/usr/bin/env athena
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
flags = initConfigFlags()
flags.Exec.MaxEvents = 10
flags.Exec.FPE = -2  # disable FPE auditing
flags.fillFromArgs()
flags.lock()

from AthenaConfiguration.MainServicesConfig import MainServicesCfg
cfg = MainServicesCfg(flags)

from StoreGateTests.Lib import PyClidsTestWriter
cfg.addEventAlgo( PyClidsTestWriter() )

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
cfg.merge(
    OutputStreamCfg(
        flags,
        streamName="Stl",
        disableEventTag=True,
        ItemList=[
            "EventInfo#*",
            "std::vector<int>#*",
            "std::vector<unsigned int>#*",
            "std::vector<float>#*",
            "std::vector<double>#*",
        ],
        MetadataItemList=["IOVMetaDataContainer#*"],
    )
)

import sys
sys.exit( cfg.run().isFailure() )
