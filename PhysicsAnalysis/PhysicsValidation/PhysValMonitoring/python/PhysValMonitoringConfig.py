#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file PhysValMonitoringConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for PhysValMonitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def PhysValExampleCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    from AthenaCommon.Constants import WARNING
    kwargs.setdefault("EnableLumi", False)
    kwargs.setdefault("OutputLevel", WARNING)
    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("TauContainerName", "TauJets")
    kwargs.setdefault("PhotonContainerName", "Photons")
    kwargs.setdefault("ElectronContainerName", "Electrons")

    # Keep this disabled for now
    kwargs.setdefault("DoExBtag", False)
    kwargs.setdefault("DoExMET", False)
    kwargs.setdefault("METContainerName", "")

    acc.setPrivateTools(CompFactory.PhysVal.PhysValExample(**kwargs))
    return acc


def PhysValMonitoringCfg(flags, name="PhysValMonManager", tools=None, **kwargs):
    if tools is None:
        tools = []
    
    acc = ComponentAccumulator()

    kwargs.setdefault("FileKey", "PhysVal")
    kwargs.setdefault("Environment", "altprod")
    kwargs.setdefault("ManualDataTypeSetup", True)
    kwargs.setdefault("DataType", "monteCarlo")
    kwargs.setdefault("ManualRunLBSetup", True)
    kwargs.setdefault("Run", 1)
    kwargs.setdefault("LumiBlock", 1)

    if flags.PhysVal.doExample:
        tools.append(acc.popToolsAndMerge(PhysValExampleCfg(flags)))
    if flags.PhysVal.doInDet:
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetPhysValMonitoringToolCfg
        tools.append(acc.popToolsAndMerge(InDetPhysValMonitoringToolCfg(flags)))
    if flags.PhysVal.doInDetLargeD0:
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetLargeD0PhysValMonitoringToolCfg
        tools.append(acc.popToolsAndMerge(InDetLargeD0PhysValMonitoringToolCfg(flags)))
    if flags.PhysVal.doBtag:
        from JetTagDQA.JetTagDQAConfig import PhysValBTagCfg
        tools.append(acc.popToolsAndMerge(PhysValBTagCfg(flags)))
    if flags.PhysVal.doMET:
        from MissingEtDQA.MissingEtDQAConfig import PhysValMETCfg
        tools.append(acc.popToolsAndMerge(PhysValMETCfg(flags)))
    if flags.PhysVal.doEgamma:
        from EgammaPhysValMonitoring.EgammaPhysValMonitoringConfig import EgammaPhysValMonitoringToolCfg
        tools.append(acc.popToolsAndMerge(EgammaPhysValMonitoringToolCfg(flags)))
    if flags.PhysVal.doTau:
        from TauDQA.TauDQAConfig import PhysValTauCfg
        tools.append(acc.popToolsAndMerge(PhysValTauCfg(flags)))
    if flags.PhysVal.doJet:
        from JetValidation.JetValidationConfig import PhysValJetCfg
        tools.append(acc.popToolsAndMerge(PhysValJetCfg(flags)))
    if flags.PhysVal.doTopoCluster:
        from PFODQA.ClusterDQAConfig import PhysValClusterCfg
        tools += acc.popToolsAndMerge(PhysValClusterCfg(flags))
    if flags.PhysVal.doZee:
        from ZeeValidation.ZeeValidationMonToolConfig import PhysValZeeCfg
        tools.append(acc.popToolsAndMerge(PhysValZeeCfg(flags)))
    if flags.PhysVal.doPFlow:
        from PFODQA.PFPhysValConfig import PhysValPFOCfg
        tools += acc.popToolsAndMerge(PhysValPFOCfg(flags))
    if flags.PhysVal.doMuon:
        from MuonPhysValMonitoring.MuonPhysValConfig import PhysValMuonCfg
        tools.append(acc.popToolsAndMerge(PhysValMuonCfg(flags)))
    if flags.PhysVal.doLRTMuon:
        from MuonPhysValMonitoring.MuonPhysValConfig import PhysValLRTMuonCfg
        tools.append(acc.popToolsAndMerge(PhysValLRTMuonCfg(flags)))
    if flags.PhysVal.IDPVM.doActs:
        from ActsConfig.ActsAnalysisConfig import PhysValActsCfg
        tools.append(acc.popToolsAndMerge(PhysValActsCfg(flags)))
    if flags.PhysVal.doLLPSecVtx:
        from InDetSecVertexValidation.InDetSecVertexValidationConfig import PhysValSecVtxCfg
        tools.append(acc.popToolsAndMerge(PhysValSecVtxCfg(flags)))

    kwargs.setdefault("AthenaMonTools", tools)

    acc.addEventAlgo(CompFactory.AthenaMonManager(name, **kwargs))
    acc.addService(CompFactory.THistSvc(Output=[f"PhysVal DATAFILE='{flags.PhysVal.OutputFileName}' OPT='RECREATE'"]))
    return acc
