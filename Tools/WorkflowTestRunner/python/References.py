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
    "s3761": "v1",
    "s4005": "v1",
    "s4006": "v1",
    "s4007": "v1",
    "s4008": "v1",
    "a913": "v1",
    # Overlay
    "d1726": "v2",
    "d1759": "v2",
    "d1912": "v2",
    # Reco
    "q442": "v3",
    "q449": "v3",
    "q452": "v4",
    "q454": "v7",
    # Derivations
    "data_PHYS_Run2": "v1",
    "data_PHYS_Run3": "v1",
    "mc_PHYS_Run2": "v1",
    "mc_PHYS_Run3": "v1",
    "af3_PHYS_Run3": "v1",
}
