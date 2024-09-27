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
    "s4006": "v16",
    "s4007": "v15",
    "s4008": "v1",
    "a913": "v10",
    # Digi
    "d1920": "v5",
    # Overlay
    "d1726": "v12",
    "d1759": "v18",
    "d1912": "v6",
    # Reco
    "q442": "v62",
    "q449": "v98",
    "q452": "v22",
    "q454": "v32",
    # Derivations
    "data_PHYS_Run2": "v35",
    "data_PHYSLITE_Run2": "v17",
    "data_PHYS_Run3": "v34",
    "data_PHYSLITE_Run3": "v17",
    "mc_PHYS_Run2": "v45",
    "mc_PHYSLITE_Run2": "v19",
    "mc_PHYS_Run3": "v45",
    "mc_PHYSLITE_Run3": "v21",
    "af3_PHYS_Run3": "v26",
    "af3_PHYSLITE_Run3": "v22",
}
