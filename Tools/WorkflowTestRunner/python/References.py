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
    "s4454": "v2",
    "a913": "v15",
    # Digi
    "d1920": "v7",
    # Overlay
    "d1726": "v13",
    "d1759": "v20",
    "d1912": "v8",
    # Reco
    "q442": "v72",
    "q449": "v116",
    "q452": "v32",
    "q454": "v47",
    # Derivations
    "data_PHYS_Run2": "v43",
    "data_PHYSLITE_Run2": "v25",
    "data_PHYS_Run3": "v43",
    "data_PHYSLITE_Run3": "v26",
    "mc_PHYS_Run2": "v55",
    "mc_PHYSLITE_Run2": "v29",
    "mc_PHYS_Run3": "v58",
    "mc_PHYSLITE_Run3": "v32",
    "af3_PHYS_Run2": "v5",
    "af3_PHYSLITE_Run2": "v6",
    "af3_PHYS_Run3": "v39",
    "af3_PHYSLITE_Run3": "v33",
}
