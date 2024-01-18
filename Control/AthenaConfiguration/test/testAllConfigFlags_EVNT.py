#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.TestDefaults import defaultTestFiles
flags = initConfigFlags()
flags.Input.Files = defaultTestFiles.EVNT

# Load Detector flags if available
flagsAvailableDetector = True
try:
    import DetDescrCnvSvc # noqa: F401
except ImportError:
    flagsAvailableDetector = False
if flagsAvailableDetector:
    flags._loadDynaFlags("Detector")

# Load Sim flags if available
flagsAvailableSim = True
try:
    import SimulationConfig # noqa: F401
except ImportError:
    flagsAvailableSim = False
if flagsAvailableSim:
    flags._loadDynaFlags("Sim")

# Init and print
flags.initAll()
flags.lock()
flags.dump()
