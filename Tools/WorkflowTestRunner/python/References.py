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
    "s3761": "v7",
    "s4005": "v6",
    "s4006": "v7",
    "s4007": "v7",
    "s4008": "v1",
    "a913": "v8",
    # Overlay
    "d1726": "v5",
    "d1759": "v7",
    "d1912": "v6",
    "d2029": "v10",
    "d2030": "v19",
    # Reco
    "q442": "v25",
    "q449": "v57",
    "q452": "v23",
    "q454": "v42",
    # Derivations
    "data_PHYS_Run2": "v3",
    "data_PHYS_Run3": "v7",
    "mc_PHYS_Run2": "v5",
    "mc_PHYS_Run3": "v4",
    "af3_PHYS_Run3": "v4",
}
