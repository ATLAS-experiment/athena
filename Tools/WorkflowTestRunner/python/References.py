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
    "d1920": "v3",
    # Overlay
    "d1726": "v10",
    "d1759": "v16",
    "d1912": "v6",
    # Reco
    "q442": "v55",
    "q449": "v86",
    "q452": "v15",
    "q454": "v22",
    # Derivations
    "data_PHYS_Run2": "v24",
    "data_PHYSLITE_Run2": "v6",
    "data_PHYS_Run3": "v23",
    "data_PHYSLITE_Run3": "v6",
    "mc_PHYS_Run2": "v28",
    "mc_PHYSLITE_Run2": "v6",
    "mc_PHYS_Run3": "v29",
    "mc_PHYSLITE_Run3": "v6",
    "af3_PHYS_Run3": "v10",
    "af3_PHYSLITE_Run3": "v6",
}
