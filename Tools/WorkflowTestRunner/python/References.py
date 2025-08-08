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
    "s3761": "v22",
    "s4005": "v16",
    "s4006": "v25",
    "s4007": "v24",
    "s4008": "v1",
    "s4454": "v7",
    "a913": "v19",
    # Digi
    "d1920": "v12",
    # Overlay
    "d1726": "v14",
    "d1759": "v21",
    "d1912": "v9",
    "d2029": "v8",
    "d2030": "v12",
    # Reco
    "q442": "v89",
    "q449": "v148",
    "q452": "v51",
    "q454": "v67",
    # Derivations
    "data_PHYS_Run2": "v56",
    "data_PHYSLITE_Run2": "v32",
    "data_PHYS_Run3": "v61",
    "data_PHYSLITE_Run3": "v39",
    "mc_PHYS_Run2": "v73",
    "mc_PHYSLITE_Run2": "v37",
    "mc_PHYS_Run3": "v75",
    "mc_PHYSLITE_Run3": "v43",
    "af3_PHYS_Run2": "v21",
    "af3_PHYSLITE_Run2": "v16",
    "af3_PHYS_Run3": "v55",
    "af3_PHYSLITE_Run3": "v45",
}
