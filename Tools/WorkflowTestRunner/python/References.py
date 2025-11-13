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
    "s3761": "v24",
    "s4005": "v16",
    "s4006": "v25",
    "s4007": "v24",
    "s4008": "v2",
    "s4454": "v8",
    "a913": "v19",
    # Digi
    "d1920": "v18",
    # Overlay
    "d1726": "v17",
    "d1759": "v25",
    "d1912": "v9",
    "d2029": "v11",
    "d2030": "v16",
    # Reco
    "q442": "v100",
    "q449": "v158",
    "q452": "v66",
    "q454": "v85",
    # Derivations
    "data_PHYS_Run2": "v70",
    "data_PHYSLITE_Run2": "v37",
    "data_PHYS_Run3": "v77",
    "data_PHYSLITE_Run3": "v46",
    "mc_PHYS_Run2": "v94",
    "mc_PHYSLITE_Run2": "v46",
    "mc_PHYS_Run3": "v98",
    "mc_PHYSLITE_Run3": "v56",
    "af3_PHYS_Run2": "v43",
    "af3_PHYSLITE_Run2": "v26",
    "af3_PHYS_Run3": "v79",
    "af3_PHYSLITE_Run3": "v58",
}
