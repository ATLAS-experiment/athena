# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v23",
    "s4005": "v16",
    "s4006": "v25",
    "s4007": "v24",
    "s4008": "v2",
    "s4454": "v7",
    "a913": "v19",
    # Digi
    "d1920": "v14",
    # Overlay
    "d1726": "v15",
    "d1759": "v22",
    "d1912": "v9",
    "d2029": "v8",
    "d2030": "v13",
    # Reco
    "q442": "v91",
    "q449": "v150",
    "q452": "v55",
    "q454": "v71",
    # Derivations
    "data_PHYS_Run2": "v61",
    "data_PHYSLITE_Run2": "v36",
    "data_PHYS_Run3": "v66",
    "data_PHYSLITE_Run3": "v44",
    "mc_PHYS_Run2": "v81",
    "mc_PHYSLITE_Run2": "v44",
    "mc_PHYS_Run3": "v83",
    "mc_PHYSLITE_Run3": "v51",
    "af3_PHYS_Run2": "v29",
    "af3_PHYSLITE_Run2": "v23",
    "af3_PHYS_Run3": "v63",
    "af3_PHYSLITE_Run3": "v53",
}
