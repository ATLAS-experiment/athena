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
    "s3761": "v6",
    "s4005": "v4",
    "s4006": "v6",
    "s4007": "v6",
    "s4008": "v1",
    "a913": "v7",
    # Overlay
    "d1726": "v4",
    "d1759": "v5",
    "d1912": "v6",
    "d2029": "v7",
    "d2030": "v8",
    # Reco
    "q442": "v14",
    "q449": "v42",
    "q452": "v16",
    "q454": "v30",
    # Derivations
    "data_PHYS_Run2": "v3",
    "data_PHYS_Run3": "v5",
    "mc_PHYS_Run2": "v3",
    "mc_PHYS_Run3": "v3",
    "af3_PHYS_Run3": "v3",
}
