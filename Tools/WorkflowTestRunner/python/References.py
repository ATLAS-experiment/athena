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
    "s3761": "v20",
    "s4005": "v14",
    "s4006": "v22",
    "s4007": "v21",
    "s4008": "v1",
    "s4454": "v6",
    "a913": "v17",
    # Digi
    "d1920": "v10",
    # Overlay
    "d1726": "v14",
    "d1759": "v21",
    "d1912": "v9",
    "d2029": "v5",
    "d2030": "v5",
    # Reco
    "q442": "v83",
    "q449": "v141",
    "q452": "v44",
    "q454": "v60",
    # Derivations
    "data_PHYS_Run2": "v54",
    "data_PHYSLITE_Run2": "v31",
    "data_PHYS_Run3": "v58",
    "data_PHYSLITE_Run3": "v38",
    "mc_PHYS_Run2": "v69",
    "mc_PHYSLITE_Run2": "v37",
    "mc_PHYS_Run3": "v72",
    "mc_PHYSLITE_Run3": "v41",
    "af3_PHYS_Run2": "v18",
    "af3_PHYSLITE_Run2": "v15",
    "af3_PHYS_Run3": "v52",
    "af3_PHYSLITE_Run3": "v43",
}
