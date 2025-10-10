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
    "s3761": "v23",
    "s4005": "v16",
    "s4006": "v25",
    "s4007": "v24",
    "s4008": "v2",
    "s4454": "v8",
    "a913": "v19",
    # Digi
    "d1920": "v17",
    # Overlay
    "d1726": "v16",
    "d1759": "v24",
    "d1912": "v9",
    "d2029": "v10",
    "d2030": "v15",
    # Reco
    "q442": "v92",
    "q449": "v150",
    "q452": "v57",
    "q454": "v76",
    # Derivations
    "data_PHYS_Run2": "v67",
    "data_PHYSLITE_Run2": "v37",
    "data_PHYS_Run3": "v73",
    "data_PHYSLITE_Run3": "v45",
    "mc_PHYS_Run2": "v90",
    "mc_PHYSLITE_Run2": "v46",
    "mc_PHYS_Run3": "v92",
    "mc_PHYSLITE_Run3": "v53",
    "af3_PHYS_Run2": "v39",
    "af3_PHYSLITE_Run2": "v26",
    "af3_PHYS_Run3": "v73",
    "af3_PHYSLITE_Run3": "v55",
}
