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
    "s3761": "v30",
    "s4005": "v20",
    "s4006": "v29",
    "s4007": "v28",
    "s4008": "v2",
    "s4454": "v14",
    "a913": "v23",
    # Digi
    "d1920": "v21",
    # Overlay
    "d1726": "v19",
    "d1759": "v27",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v26",
    # Reco
    "q442": "v122",
    "q449": "v183",
    "q452": "v87",
    "q454": "v111",
    "q447": "v9",
    # Derivations
    "data_PHYS_Run2": "v91",
    "data_PHYSLITE_Run2": "v54",
    "data_PHYS_Run3": "v105",
    "data_PHYSLITE_Run3": "v68",
    "mc_PHYS_Run2": "v125",
    "mc_PHYSLITE_Run2": "v64",
    "mc_PHYS_Run3": "v133",
    "mc_PHYSLITE_Run3": "v77",
    "af3_PHYS_Run2": "v78",
    "af3_PHYSLITE_Run2": "v44",
    "af3_PHYS_Run3": "v115",
    "af3_PHYSLITE_Run3": "v80",
}
