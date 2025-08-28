# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from JetRecConfig.StandardJetConstits import inputsFromContext
from PathResolver import PathResolver

from AthenaCommon import Logging
jetcaliblog = Logging.logging.getLogger('JetCalibToolsConfig')

all = ['getJetCalibTool']

calibdic = {
    "AntiKt4EMPFlow:T0" : "JetCalibTools/calibConfigExample.yaml",
    "AntiKt4EMPFlow:TrigRun2" : "JetCalibTools/calibConfigExample.yaml", # this is for testing
}

# This method actually sets up the tool
def defineJetCalibTool(jetdef, modspec):
    from JetCalibTools.JetCalibStepsConfig import calibToolFromConfigFile

    jetcollection = jetdef.basename
    cfg = calibdic[f"{jetdef.basename}:{modspec}"]
    path_configFile = PathResolver.FindCalibFile(cfg) 
    toolname = "jetcalib_new_{0}_{1}".format(jetcollection,modspec)
    jct = calibToolFromConfigFile(jetdef._cflags, path_configFile, toolname)

    return jct

# This method extends the basic config getter to specify the requisite jet
# moments or other inputs
def getJetCalibToolPrereqs(jetdef, modspec):
    from JetCalibTools.JetCalibStepsConfig import load_yaml_cfg

    cfg = calibdic[f"{jetdef.basename}:{modspec}"]
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

