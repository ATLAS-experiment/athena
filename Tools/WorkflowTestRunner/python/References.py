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
    "s3761": "v34",
    "s4005": "v22",
    "s4006": "v31",
    "s4007": "v30",
    "s4008": "v2",
    "s4454": "v15",
    "a913": "v25",
    # Digi
    "d1920": "v24",
    # Overlay
    "d1726": "v19",
    "d1759": "v27",
    "d1912": "v9",
    "d2029": "v12",
    "d2030": "v28",
    # Reco
    "q442": "v130",
    "q449": "v190",
    "q452": "v96",
    "q454": "v116",
    "q447": "v46",
    # Derivations
    "data_PHYS_Run2": "v103",
    "data_PHYSLITE_Run2": "v64",
    "data_PHYS_Run3": "v121",
    "data_PHYSLITE_Run3": "v83",
    "mc_PHYS_Run2": "v142",
    "mc_PHYSLITE_Run2": "v72",
    "mc_PHYS_Run3": "v154",
    "mc_PHYSLITE_Run3": "v89",
    "af3_PHYS_Run2": "v94",
    "af3_PHYSLITE_Run2": "v52",
    "af3_PHYS_Run3": "v135",
    "af3_PHYSLITE_Run3": "v92",
}
