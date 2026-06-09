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
    "s3761": "v26",
    "s4005": "v18",
    "s4006": "v27",
    "s4007": "v26",
    "s4008": "v2",
    "s4454": "v11",
    "a913": "v21",
    # Digi
    "d1920": "v19",
    # Overlay
    "d1726": "v17",
    "d1759": "v25",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v24",
    # Reco
    "q442": "v120",
    "q449": "v181",
    "q452": "v85",
    "q454": "v107",
    # Derivations
    "data_PHYS_Run2": "v87",
    "data_PHYSLITE_Run2": "v51",
    "data_PHYS_Run3": "v100",
    "data_PHYSLITE_Run3": "v63",
    "mc_PHYS_Run2": "v121",
    "mc_PHYSLITE_Run2": "v61",
    "mc_PHYS_Run3": "v128",
    "mc_PHYSLITE_Run3": "v73",
    "af3_PHYS_Run2": "v72",
    "af3_PHYSLITE_Run2": "v40",
    "af3_PHYS_Run3": "v109",
    "af3_PHYSLITE_Run3": "v77",
}
