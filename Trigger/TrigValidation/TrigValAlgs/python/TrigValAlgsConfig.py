# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def TrigEDMCheckerCfg(flags, name="TrigEDMChecker", doDumpAll=True):
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    cfg = ComponentAccumulator()
    edmchecker = CompFactory.TrigEDMChecker(
        name,
        doDumpAll = doDumpAll,
        TriggerDecisionTool = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags)) )

    if doDumpAll:
        from MuonConfig.MuonRecToolsConfig import MuonEDMPrinterToolCfg
        from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg, MuonIdHelperSvcCfg
        cfg.merge(MuonGeoModelCfg(flags))
        cfg.merge(MuonIdHelperSvcCfg(flags))
        edmchecker.MuonPrinter = CompFactory.Rec.MuonPrintingTool(
            MuonStationPrinter = cfg.popToolsAndMerge(MuonEDMPrinterToolCfg(flags)) )

    cfg.addEventAlgo(edmchecker)

    return cfg


def TrigEDMAuxCheckerCfg(flags, name="TrigEDMAuxChecker"):
    cfg = ComponentAccumulator()

    alg = CompFactory.TrigEDMAuxChecker(name, AuxContainerList=getEDMAuxList(flags))
    cfg.addEventAlgo(alg)
    return cfg


def getEDMAuxList(flags):
    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    tlist=getTriggerEDMList(flags.Trigger.AODEDMSet, flags.Trigger.EDMVersion)
    objlist=[]
    for t,kset in tlist.items():
        for k in kset:
             if 'Aux.' in k:
                 s = k.split('.',1)[0] + "."
                 # Types of these collections from Run 2 do not inherit from xAOD::AuxContainerBase, so can't test them here
                 if flags.Trigger.EDMVersion == 2 and "HLT_xAOD__JetContainer" in s or "xTrigDecisionAux" in s or "TrigNavigationAux" in s:
                     continue
                 objlist += [s]
    return objlist


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.lock()

    cfg = ComponentAccumulator()
    cfg.merge( TrigEDMCheckerCfg(flags) )
    cfg.merge( TrigEDMAuxCheckerCfg(flags) )
    cfg.wasMerged()
