# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from JetRecConfig.StandardJetConstits import inputsFromContext
from PathResolver import PathResolver

from AthenaCommon import Logging
jetcaliblog = Logging.logging.getLogger('JetCalibToolsConfig')


all = ['getJetCalibTool']

import yaml
from functools import lru_cache

# Index mapping jet collection and calibration context to the config file
CONFIG_FILE = "JetCalibTools/calibDict.yaml"

# Optional: once the config files are distributed via cvmfs
# it can be dropped from the yaml and the paths left for PathResolver.
COMMONPATH_KEY = "commonPath"

@lru_cache(maxsize=1)
def load_calib_cfg():
    path = PathResolver.FindCalibFile(CONFIG_FILE)
    if not path:
        raise FileNotFoundError("Could not locate %s via PathResolver" % CONFIG_FILE)
    with open(path, "r", encoding="utf-8") as f:
        return yaml.safe_load(f)

def full_calib_path(rel_path: str) -> str:
    commonPath = load_calib_cfg().get(COMMONPATH_KEY)
    if not commonPath or rel_path.startswith("/"):
        return rel_path
    return commonPath.rstrip("/") + "/" + rel_path

def get_jet_collection_name(name: str) -> str:
    # For some specific jet collections, e.g. lepton-free PFlow jets,
    # we want to apply the calibrations of the default PFlow jets
    for suffix in ("_noElectrons", "_noMuons", "_noLeptons", "_tauSeedEleRM"):
        name = name.replace(suffix, "")
    return name

def get_calib_cfg_path(context: str, jetcollection: str) -> str:
    """ Look up the calibration config file for a jet collection and context.
    The config file itself holds any run-dependent settings, in 'RunX:' blocks. """
    data = load_calib_cfg()

    contexts = data.get(jetcollection)
    if contexts is None or jetcollection == COMMONPATH_KEY:
        known = [k for k in data if k != COMMONPATH_KEY]
        raise KeyError(f"No calibrations listed in {CONFIG_FILE} for jet collection "
                       f"'{jetcollection}'. Known collections: {known}")

    rel = contexts.get(context)
    if rel is None:
        raise KeyError(f"No '{context}' calibration listed in {CONFIG_FILE} for jet collection "
                       f"'{jetcollection}'. Known contexts: {list(contexts)}")

    return full_calib_path(rel)

def get_calib_cfg(context: str, jetcollection: str):
    from JetCalibTools.JetCalibStepsConfig import load_yaml_cfg
    return load_yaml_cfg(get_calib_cfg_path(context, jetcollection))

# This method actually sets up the tool
def defineJetCalibTool(jetdef, modspec):
    from JetCalibTools.JetCalibStepsConfig import calibToolFromConfigFile

    # Get the yaml file and calibration sequence
    cfg, calibSeqKey = getJetCalibToolSettings(jetdef, modspec)
    path_configFile = PathResolver.FindCalibFile(cfg) 
    toolname = "jetcalib_new_{0}_{1}".format(jetdef.basename,modspec)
    jct = calibToolFromConfigFile(jetdef._cflags, path_configFile, toolname, calibSeqKey)

    return jct

# This method extends the basic config getter to specify the requisite jet
# moments or other inputs
def getJetCalibToolPrereqs(jetdef, modspec):
    from JetCalibTools.JetCalibStepsConfig import load_yaml_cfg

    cfg, _ = getJetCalibToolSettings(jetdef, modspec)
    configDic = load_yaml_cfg(cfg)

    prereqs = ["mod:ConstitFourMom"]
    pvname = "PrimaryVertices" # this can be set dinamically in future

    for step, step_config in configDic.items():
        # JetArea
        if step_config.get("DoJetArea", False):
            if modspec.startswith("Trig"):
                prereqs.append("input:HLT_EventDensity")
            elif pvname == "PrimaryVertices_initial":
                prereqs.append("input:EventDensityCustomVtxGNN")
            elif pvname != "PrimaryVertices":
                prereqs.append("input:EventDensityCustomVtx")
            else:
                prereqs.append(inputsFromContext("EventDensity")(jetdef))

        # read prereqs from context or default config block
        prereq_block = step_config.get("prereqs", {})
        step_prereqs = prereq_block.get(modspec, prereq_block.get("default", []))
        prereqs.extend(step_prereqs)

    # remove duplication and keep order
    seen = set()
    prereqs_unique = []
    for p in prereqs:
        if p not in seen:
            prereqs_unique.append(p)
            seen.add(p)

    return prereqs_unique

# Get specific settings for JetCalibTools
def getJetCalibToolSettings(jetdef, modspec):

    calibspecs = modspec.split(':')

    context = calibspecs[0] # T0/Trigger/etc. - used to extract calbration sequence from YAML config file

    jetcollection = get_jet_collection_name(jetdef.basename)

    cfg = get_calib_cfg_path(context, jetcollection)

    return cfg, context
