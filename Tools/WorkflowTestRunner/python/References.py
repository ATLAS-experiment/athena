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
    "s3761": "v32",
    "s4005": "v21",
    "s4006": "v30",
    "s4007": "v29",
    "s4008": "v2",
    "s4454": "v14",
    "a913": "v24",
    # Digi
    "d1920": "v23",
    # Overlay
    "d1726": "v19",
    "d1759": "v27",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v27",
    # Reco
    "q442": "v124",
    "q449": "v185",
    "q452": "v89",
    "q454": "v113",
    "q447": "v15",
    # Derivations
    "data_PHYS_Run2": "v94",
    "data_PHYSLITE_Run2": "v55",
    "data_PHYS_Run3": "v108",
    "data_PHYSLITE_Run3": "v69",
    "mc_PHYS_Run2": "v129",
    "mc_PHYSLITE_Run2": "v65",
    "mc_PHYS_Run3": "v137",
    "mc_PHYSLITE_Run3": "v78",
    "af3_PHYS_Run2": "v81",
    "af3_PHYSLITE_Run2": "v44",
    "af3_PHYS_Run3": "v119",
    "af3_PHYSLITE_Run3": "v81",
}
