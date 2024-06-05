# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/master/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v1",
    "s3779": "v1",
    "s4005": "v1",
    "s4006": "v1",
    "s4007": "v1",
    "s4008": "v1",
    # Overlay
    "d1590": "v1",
    "d1726": "v1",
    "d1759": "v2",
    # Reco
    "q442": "v7",
    "q443": "v4",
    "q445": "v6",
    "q449": "v8",
    # Derivations
    "data_PHYS_Run2": "v2",
    "data_PHYS_Run3": "v2",
    "mc_PHYS_Run2": "v2",
    "mc_PHYS_Run3": "v2",
}
