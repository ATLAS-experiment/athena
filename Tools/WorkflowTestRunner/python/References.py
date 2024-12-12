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
    "s3761": "v5",
    "s4005": "v3",
    "s4006": "v5",
    "s4007": "v5",
    "s4008": "v1",
    "a913": "v6",
    # Overlay
    "d1726": "v3",
    "d1759": "v3",
    "d1912": "v4",
    # Reco
    "q442": "v7",
    "q449": "v19",
    "q452": "v9",
    "q454": "v19",
    # Derivations
    "data_PHYS_Run2": "v3",
    "data_PHYS_Run3": "v3",
    "mc_PHYS_Run2": "v3",
    "mc_PHYS_Run3": "v3",
    "af3_PHYS_Run3": "v3",
}
