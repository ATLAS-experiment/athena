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
    "q442": "v125",
    "q449": "v187",
    "q452": "v91",
    "q454": "v115",
    "q447": "v22",
    # Derivations
    "data_PHYS_Run2": "v100",
    "data_PHYSLITE_Run2": "v58",
    "data_PHYS_Run3": "v118",
    "data_PHYSLITE_Run3": "v76",
    "mc_PHYS_Run2": "v137",
    "mc_PHYSLITE_Run2": "v68",
    "mc_PHYS_Run3": "v149",
    "mc_PHYSLITE_Run3": "v85",
    "af3_PHYS_Run2": "v89",
    "af3_PHYSLITE_Run2": "v47",
    "af3_PHYS_Run3": "v130",
    "af3_PHYSLITE_Run3": "v88",
}
