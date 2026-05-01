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
    "d2030": "v22",
    # Reco
    "q442": "v115",
    "q449": "v176",
    "q452": "v80",
    "q454": "v102",
    # Derivations
    "data_PHYS_Run2": "v82",
    "data_PHYSLITE_Run2": "v48",
    "data_PHYS_Run3": "v94",
    "data_PHYSLITE_Run3": "v59",
    "mc_PHYS_Run2": "v114",
    "mc_PHYSLITE_Run2": "v58",
    "mc_PHYS_Run3": "v122",
    "mc_PHYSLITE_Run3": "v71",
    "af3_PHYS_Run2": "v65",
    "af3_PHYSLITE_Run2": "v37",
    "af3_PHYS_Run3": "v103",
    "af3_PHYSLITE_Run3": "v75",
}
