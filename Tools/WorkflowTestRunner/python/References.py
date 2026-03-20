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
    "d1920": "v18",
    # Overlay
    "d1726": "v17",
    "d1759": "v25",
    "d1912": "v9",
    "d2029": "v11",
    "d2030": "v20",
    # Reco
    "q442": "v110",
    "q449": "v168",
    "q452": "v77",
    "q454": "v98",
    # Derivations
    "data_PHYS_Run2": "v78",
    "data_PHYSLITE_Run2": "v41",
    "data_PHYS_Run3": "v90",
    "data_PHYSLITE_Run3": "v53",
    "mc_PHYS_Run2": "v108",
    "mc_PHYSLITE_Run2": "v52",
    "mc_PHYS_Run3": "v116",
    "mc_PHYSLITE_Run3": "v66",
    "af3_PHYS_Run2": "v59",
    "af3_PHYSLITE_Run2": "v32",
    "af3_PHYS_Run3": "v97",
    "af3_PHYSLITE_Run3": "v69",
}
