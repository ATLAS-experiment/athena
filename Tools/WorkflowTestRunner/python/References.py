# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#####
# CI Reference Files Map
#####

# The top-level directory for the files is /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/
# Then the subfolders follow the format branch/test/version, i.e. for s3760 in master the reference files are under
# /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences/main/s3760/v1 for v1 version

# Format is "test" : "version"
references_map = {
    # Simulation
    "s3761": "v18",
    "s4005": "v12",
    "s4006": "v20",
    "s4007": "v19",
    "s4008": "v1",
    "s4454": "v4",
    "a913": "v15",
    # Digi
    "d1920": "v9",
    # Overlay
    "d1726": "v13",
    "d1759": "v20",
    "d1912": "v9",
    # Reco
    "q442": "v80",
    "q449": "v134",
    "q452": "v41",
    "q454": "v56",
    # Derivations
    "data_PHYS_Run2": "v51",
    "data_PHYSLITE_Run2": "v28",
    "data_PHYS_Run3": "v54",
    "data_PHYSLITE_Run3": "v34",
    "mc_PHYS_Run2": "v66",
    "mc_PHYSLITE_Run2": "v34",
    "mc_PHYS_Run3": "v69",
    "mc_PHYSLITE_Run3": "v38",
    "af3_PHYS_Run2": "v15",
    "af3_PHYSLITE_Run2": "v12",
    "af3_PHYS_Run3": "v49",
    "af3_PHYSLITE_Run3": "v40",
}
