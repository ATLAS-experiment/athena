# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory

log = logging.getLogger('TrigADHypoAlgs')

# Object cuts sets used to configure the AD input multiplicity and pT threshold
object_cuts_sets = {
    "default": {
        "max_jets": 6,
        "max_electrons": 3,
        "max_muons": 3,
        "max_photons": 3,
    }
}

# cuts applied for each config value
config_dict = {
    "default":{
        "object_cuts": "default",
        "adScoreThres": 0.5,
    },
    "L":{
        "object_cuts": "default",
        "adScoreThres": 8.768, # 20 Hz est. w/ v2
    },
    "M":{
        "object_cuts": "default",
        "adScoreThres": 10.574, # 10 Hz est. w/ v2
    },
    "T":{
        "object_cuts": "default",
        "adScoreThres": 15.0277, # 5 Hz est. w/ v2
    }

}

def TrigADGetConfigValue(chainDict, key):
    values = [i.strip(key) for i in chainDict['topo']]

    if len(values) != 1:
        raise RuntimeError("Invalid chain dictionary for AD trigger, unable to find config value in {}".format(str(chainDict)))

    return values[0]

def TrigADComboHypoToolFromDict(chainDict):
    name = chainDict['chainName']

    log.debug("Inside AD ComboHypoToolFromDict")
    log.debug("chainDict:", chainDict)

    cfg_name = TrigADGetConfigValue(chainDict, "anomdet")
    cfg = config_dict[cfg_name]
    
    obj_cuts = object_cuts_sets[cfg["object_cuts"]]

    tool = CompFactory.TrigADComboHypoTool(
        name,
        max_jets = obj_cuts["max_jets"],
        max_electrons = obj_cuts["max_electrons"],
        max_muons = obj_cuts["max_muons"],
        max_photons = obj_cuts["max_photons"],
        ModelFileName = "TrigAnomalyDetectionHypo/2025-03-12/HLT_AD_v2.onnx",
        adScoreThres = float(cfg["adScoreThres"])
    )
    
    return tool
