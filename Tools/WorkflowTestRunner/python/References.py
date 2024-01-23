# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v9",
    "s3779": "v2",
    "s4005": "v6",
    "s4006": "v9",
    "s4007": "v7",
    "s4008": "v1",
    "a913": "v5",
    # Overlay
    "d1590": "v5",
    "d1726": "v5",
    "d1759": "v11",
    # Reco
    "q442": "v42",
    "q443": "v32",
    "q445": "v56",
    "q449": "v60",
    # Derivations
    "data_PHYS_Run2": "v16",
    "data_PHYS_Run3": "v15",
    "mc_PHYS_Run2": "v19",
    "mc_PHYS_Run3": "v19",
}
