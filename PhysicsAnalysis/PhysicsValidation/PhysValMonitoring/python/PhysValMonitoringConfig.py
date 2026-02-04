#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''@file PhysValMonitoringConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for PhysValMonitoring
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import seqAND

from AthenaCommon.Logging import logging
logger = logging.getLogger("PhysValMonitoringConfig")


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
    kwargs.setdefault("DoExJet", flags.PhysVal.doJet)
    kwargs.setdefault("METContainerName", "")

    acc.setPrivateTools(CompFactory.PhysVal.PhysValExample(**kwargs))
    return acc


def GoodRunsListSelectionToolCfg(flags, grls, **kwargs):
    from GoodRunsLists.GoodRunsListsDictionary import getGoodRunsLists
    all_grls = getGoodRunsLists()  # this is a Dict[str, List[str]]
    all_grl_flattened = [grl for grls in all_grls.values() for grl in grls]

    acc = ComponentAccumulator()

    if isinstance(grls, str):
        raise TypeError("grls must be an iterable of GRLs, e.g. [%s]" % grls)

    resolved_grls = []
    for grl in grls:
        if grl.endswith(".xml"):
            if grl not in all_grl_flattened:
                logger.warning("GRL '%s' is not in the reccomended GRLs, using it as is.", grl)
            resolved_grls.append(grl)
        else:
            if grl in all_grls:
                resolved_grls.extend(all_grls[grl])
            else:
                raise ValueError(f"GRL name '{grl}' not found in the available GRLs. Available GRL names: %s" % all_grls.keys())

    tool = CompFactory.GoodRunsListSelectionTool(name="GoodRunsListSelectionTool",
                                                 GoodRunsListVec=resolved_grls,
                                                 PassThrough=False,
                                                 **kwargs)
    acc.setPrivateTools(tool)
    return acc


def GoodRunListSelectionAlgCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault('GoodRunsListSelectionTool', acc.popToolsAndMerge(
                      GoodRunsListSelectionToolCfg(flags, grls=flags.PhysVal.GRLs)))

    alg = CompFactory.GRLSelectorAlg(
        'GRLSelectorAlg', Tool=kwargs['GoodRunsListSelectionTool'], grlKey="EventInfo.passGRL")
    acc.addEventAlgo(alg)
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
        tools.append(acc.popToolsAndMerge(EgammaPhysValMonitoringToolCfg(flags, useOQQuality=flags.PhysVal.applyAllDataCleaning)))
    if flags.PhysVal.doTau:
        from TauDQA.TauDQAConfig import PhysValTauCfg
        tools.append(acc.popToolsAndMerge(PhysValTauCfg(flags, tauContainer="TauJets")))
        if flags.Tau.TauMuonRM_isAvailable:
            tools.append(acc.popToolsAndMerge(PhysValTauCfg(flags, tauContainer="TauJets_MuonRM")))
        if flags.Tau.TauEleRM_isAvailable:
            tools.append(acc.popToolsAndMerge(PhysValTauCfg(flags, tauContainer="TauJets_EleRM")))
    if flags.PhysVal.doDiTau:
        from DiTauDQA.DiTauDQAConfig import PhysValDiTauCfg
        tools.append(acc.popToolsAndMerge(PhysValDiTauCfg(flags)))
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
    if flags.PhysVal.doLLPSecVtx:
        from InDetSecVertexValidation.InDetSecVertexValidationConfig import PhysValSecVtxCfg
        tools.append(acc.popToolsAndMerge(PhysValSecVtxCfg(flags)))

    kwargs.setdefault("AthenaMonTools", tools)

    # create the sequence, so that the main algorithm is not executed if the GRL or event cleaning fails
    acc.addSequence(seqAND("PhysValSequence"))

    if flags.PhysVal.applyAllDataCleaning or flags.PhysVal.applyGRL:
        if (flags.Input.isMC):
            raise ValueError("applyGRL (or applyAllDataCleaning) is not supported for MC data, please disable it.")
        acc.merge(GoodRunListSelectionAlgCfg(flags, **kwargs),
                sequenceName="PhysValSequence")
    if flags.PhysVal.applyAllDataCleaning or flags.PhysVal.applyEventStatusSelection:
        if (flags.Input.isMC):
            raise ValueError("applyEventStatusSelection (or applyAllDataCleaning) is not supported for MC data, please disable it.")
        acc.addEventAlgo(CompFactory.CP.EventStatusSelectionAlg("EventStatusSelectionAlg", FilterKey="EventErrorState",
                        FilterDescription="selecting events without any error state set"), sequenceName="PhysValSequence")

    # add the main algorithm
    acc.addEventAlgo(CompFactory.AthenaMonManager(
        name, **kwargs), sequenceName="PhysValSequence")
    acc.addService(CompFactory.THistSvc(
        Output=[f"PhysVal DATAFILE='{flags.PhysVal.OutputFileName}' OPT='RECREATE'"]))

    return acc
