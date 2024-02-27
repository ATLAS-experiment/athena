"""Define method to configure and test ITkStrip_ModuleVetoTestAlg

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ITkStripVetoTestAlgCfg(flags, name="ITkStripModuleVetoTestAlg", **kwargs):
    """Return a configured ITkStripModuleVetoTestAlg"""
    acc = ComponentAccumulator()
    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ModuleVetoCfg
    acc.addEventAlgo(CompFactory.SCT_ModuleVetoTestAlg(name,
                                                       ModuleVetoTool=acc.popToolsAndMerge(SCT_ModuleVetoCfg(flags, **kwargs))))
    return acc

if __name__=="__main__":
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.isMC = True
    flags.Input.ProjectName = "mc16_13TeV"
    flags.Input.RunNumbers = [350200] # MC23 PhaseII mu=200 run number
    flags.Input.TimeStamps = [1625130000] # MC23 PhaseII mu=200 time stamp
    flags.IOVDb.GlobalTag = "OFLP200"
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN4
    flags.Detector.GeometryITkStrip = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
    cfg.merge(McEventSelectorCfg(flags))

    kwargs = {}

    ### Use COOL database for SCT_ModuleVetoTool
    kwargs["useDB"] = False
    if kwargs["useDB"]:
        kwargs["folderStrings"] = "/ITk/Manual/BadModules"
        kwargs["BadModuleIdentifiers"] = ["database"]
    else:
        kwargs["BadModuleIdentifiers"] = ["1", "2"]

    cfg.merge(ITkStripVetoTestAlgCfg(flags, **kwargs))

    cfg.run(maxEvents=20)
