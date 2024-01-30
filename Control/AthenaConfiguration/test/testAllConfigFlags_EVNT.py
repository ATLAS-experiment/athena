#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.EVNT

# Load Sim flags if available
flagsAvailableSim = True
try:
    import SimulationConfig # noqa: F401
except ImportError:
    flagsAvailableSim = False
if flagsAvailableSim:
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags._loadDynaFlags("Sim")

# Init and print
flags.initAll()
flags.lock()
flags.dump()
