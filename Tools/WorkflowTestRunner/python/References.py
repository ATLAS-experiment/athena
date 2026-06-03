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
    "s3761": "v25",
    "s4005": "v17",
    "s4006": "v26",
    "s4007": "v25",
    "s4008": "v2",
    "s4454": "v10",
    "a913": "v20",
    # Digi
    "d1920": "v19",
    # Overlay
    "d1726": "v17",
    "d1759": "v25",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v23",
    # Reco
    "q442": "v119",
    "q449": "v180",
    "q452": "v84",
    "q454": "v106",
    # Derivations
    "data_PHYS_Run2": "v86",
    "data_PHYSLITE_Run2": "v51",
    "data_PHYS_Run3": "v98",
    "data_PHYSLITE_Run3": "v62",
    "mc_PHYS_Run2": "v119",
    "mc_PHYSLITE_Run2": "v60",
    "mc_PHYS_Run3": "v127",
    "mc_PHYSLITE_Run3": "v73",
    "af3_PHYS_Run2": "v70",
    "af3_PHYSLITE_Run2": "v39",
    "af3_PHYS_Run3": "v108",
    "af3_PHYSLITE_Run3": "v77",
}
