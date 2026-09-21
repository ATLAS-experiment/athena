#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''@file JetTagDQAConfig.py
@author T. Strebler
@date 2022-06-16
@brief Main CA-based python configuration for JetTagDQA
'''

from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def PhysValBTagCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("DetailLevel", 10)
    kwargs.setdefault("isData", not flags.Input.isMC)

    if flags.Input.isMC:
        ttbarDSIDs = {601229, 601230}
        zprimeDSIDs = {801271, 800030, 802818}
        dsid = flags.Input.MCChannelNumber
        if dsid not in ttbarDSIDs | zprimeDSIDs:
            logging.getLogger("PhysValBTagCfg").warning("DSID %s is not a known ttbar or Z' sample, using the ttbar jet selection", dsid)
        kwargs.setdefault("OnZprime", dsid in zprimeDSIDs)

    # Run 4 has no tuned JVT, and EMTopo is the main small-R collection since PFlow jets are not tuned for Run 4 yet.
    # Both use the FTAG truth based JVT proxy there instead.
    kwargs.setdefault("UseJvtProxy", flags.Input.isMC and flags.GeoModel.Run >= LHCPeriod.Run4)

    import ROOT
    path = ROOT.PathResolver.find_file( 'JetTagDQA/PhysValBtag_VariablesMenu.json', 'DATAPATH' )
    from PhysValMonitoring.PhysValUtils import getHistogramDefinitions
    definitions = getHistogramDefinitions(path, 'PHYSVAL', 'ALL')

    # Run-dependent histo definitions
    path_Run = ROOT.PathResolver.find_file( \
                'JetTagDQA/PhysValBtag_VariablesMenu_Run3.json' if flags.GeoModel.Run <= LHCPeriod.Run3 \
                else 'JetTagDQA/PhysValBtag_VariablesMenu_Run4.json',
                'DATAPATH' )
    definitions_Run = getHistogramDefinitions(path_Run, 'PHYSVAL', 'ALL')

    kwargs.setdefault("HistogramDefinitions", definitions + definitions_Run)
    kwargs.setdefault("JetEtaCut", 2.5 if flags.GeoModel.Run <= LHCPeriod.Run3 else 4.0)
    kwargs.setdefault("JetContainerEMTopo", "" if flags.GeoModel.Run <= LHCPeriod.Run3 else "AntiKt4EMTopoJets")
    kwargs.setdefault("JetContainerPFlow", "AntiKt4EMPFlowJets")

    from FlavorTagDiscriminants.FTagMuonAssociationConfig import FTagMuonAssociationCfg
    for jetCollection in [kwargs["JetContainerEMTopo"], kwargs["JetContainerPFlow"]]:
        if jetCollection:
            acc.merge(FTagMuonAssociationCfg(flags, jetCollection=jetCollection, outputMuons="GhostMuons"))

    if "trackTruthOriginTool" not in kwargs:
        from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import InDetTrackTruthOriginToolCfg
        kwargs.setdefault("trackTruthOriginTool", acc.popToolsAndMerge(
            InDetTrackTruthOriginToolCfg(flags)))

    kwargs.setdefault("GN2v01TaggerName", flags.BTagging.AK4TaggerName)
    if flags.GeoModel.Run <= LHCPeriod.Run3:
        GN2v01WorkingPoints = ["70"] if kwargs["DetailLevel"] <= 10 else ["65", "70", "77", "85", "90"]
        kwargs.setdefault("GN2v01WorkingPoints", GN2v01WorkingPoints)
        kwargs.setdefault("GN2v01SelectionTools", [
            CompFactory.BTaggingSelectionTool(
                f"GN2v01SelectionTool_{wp}",
                TaggerName=flags.BTagging.AK4TaggerName,
                JetAuthor="AntiKt4EMPFlowJets",
                OperatingPoint=f"FixedCutBEff_{wp}",
                ErrorOnTagWeightFailure=False,
            ) for wp in GN2v01WorkingPoints
        ])

        # Taken from the GN3EPCLV01 CDI in the GroupData dev area (MC23_2026-08-04_GN3EPCLV01_GN3PflowMuonsV00_GN2v01_v1_noSF.root),
        # which cannot be used here since reading dev files fails the transform in Athena
        # TODO: read them with BTaggingSelectionTool, as for GN2v01, once GN3EPCLV01 is in a production CDI
        GN3EPCLV01WorkingPoints = {"70": 2.8414, "75": 2.0235, "80": 1.1988, "85": 0.3328, "90": -0.6835}
        kwargs.setdefault("GN3EPCLV01TaggerName", "GN3EPCLV01")
        kwargs.setdefault("GN3EPCLV01FractionC", 0.3)
        kwargs.setdefault("GN3EPCLV01FractionTau", 0.05)
        kwargs.setdefault("GN3EPCLV01WorkingPoints", {"70": GN3EPCLV01WorkingPoints["70"]} if kwargs["DetailLevel"] <= 10 else GN3EPCLV01WorkingPoints)

    tool = CompFactory.JetTagDQA.PhysValBTag(**kwargs)
    acc.setPrivateTools(tool)
    return acc
