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
    "s3761": "v33",
    "s4005": "v21",
    "s4006": "v30",
    "s4007": "v29",
    "s4008": "v2",
    "s4454": "v14",
    "a913": "v24",
    # Digi
    "d1920": "v24",
    # Overlay
    "d1726": "v19",
    "d1759": "v27",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v27",
    # Reco
    "q442": "v129",
    "q449": "v190",
    "q452": "v95",
    "q454": "v116",
    "q447": "v44",
    # Derivations
    "data_PHYS_Run2": "v102",
    "data_PHYSLITE_Run2": "v62",
    "data_PHYS_Run3": "v120",
    "data_PHYSLITE_Run3": "v81",
    "mc_PHYS_Run2": "v141",
    "mc_PHYSLITE_Run2": "v71",
    "mc_PHYS_Run3": "v153",
    "mc_PHYSLITE_Run3": "v88",
    "af3_PHYS_Run2": "v93",
    "af3_PHYSLITE_Run2": "v51",
    "af3_PHYS_Run3": "v134",
    "af3_PHYSLITE_Run3": "v91",
}
