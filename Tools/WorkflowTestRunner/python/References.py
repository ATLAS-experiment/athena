# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v14",
    "s4005": "v8",
    "s4006": "v13",
    "s4007": "v12",
    "s4008": "v1",
    "a913": "v10",
    # Digi
    "d1920": "v2",
    # Overlay
    "d1726": "v9",
    "d1759": "v15",
    "d1912": "v5",
    # Reco
    "q442": "v50",
    "q449": "v78",
    "q452": "v10",
    "q454": "v16",
    # Derivations
    "data_PHYS_Run2": "v22",
    "data_PHYSLITE_Run2": "v3",
    "data_PHYS_Run3": "v21",
    "data_PHYSLITE_Run3": "v3",
    "mc_PHYS_Run2": "v26",
    "mc_PHYSLITE_Run2": "v4",
    "mc_PHYS_Run3": "v27",
    "mc_PHYSLITE_Run3": "v4",
    "af3_PHYS_Run3": "v8",
    "af3_PHYSLITE_Run3": "v4",
}
