# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v27",
    "s4005": "v19",
    "s4006": "v28",
    "s4007": "v27",
    "s4008": "v2",
    "s4454": "v12",
    "a913": "v22",
    # Digi
    "d1920": "v20",
    # Overlay
    "d1726": "v18",
    "d1759": "v26",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v24",
    # Reco
    "q442": "v121",
    "q449": "v182",
    "q452": "v86",
    "q454": "v108",
    # Derivations
    "data_PHYS_Run2": "v88",
    "data_PHYSLITE_Run2": "v52",
    "data_PHYS_Run3": "v101",
    "data_PHYSLITE_Run3": "v64",
    "mc_PHYS_Run2": "v122",
    "mc_PHYSLITE_Run2": "v62",
    "mc_PHYS_Run3": "v129",
    "mc_PHYSLITE_Run3": "v74",
    "af3_PHYS_Run2": "v73",
    "af3_PHYSLITE_Run2": "v41",
    "af3_PHYS_Run3": "v110",
    "af3_PHYSLITE_Run3": "v78",
}
