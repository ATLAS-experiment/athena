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

    # the RUN3 and RUN4 groups hold the definitions that depend on the run, only one of them is read
    import json
    with open(path) as menu:
        groups = [group for group in json.load(menu) if group != "template"]
    thisRun = "RUN3" if flags.GeoModel.Run <= LHCPeriod.Run3 else "RUN4"
    definitions = []
    for group in groups:
        if group.startswith("RUN") and group != thisRun:
            continue
        definitions += getHistogramDefinitions(path, 'PHYSVAL', group)

    kwargs.setdefault("HistogramDefinitions", definitions)
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

    # the keys name the histograms and stay fixed, the values are the decorations to read
    taggerDecorations = {"GN2v01": flags.BTagging.AK4TaggerName}
    taggerFractionC = {"GN2v01": 0.2}
    taggerFractionTau = {"GN2v01": 0.01}
    taggerWorkingPoints = {}
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
        taggerDecorations["GN3EPCLV01"] = "GN3EPCLV01"
        taggerFractionC["GN3EPCLV01"] = 0.3
        taggerFractionTau["GN3EPCLV01"] = 0.05
        if kwargs["DetailLevel"] <= 10:
            GN3EPCLV01WorkingPoints = {"70": GN3EPCLV01WorkingPoints["70"]}
        taggerWorkingPoints.update({f"GN3EPCLV01_{wp}": cut for wp, cut in GN3EPCLV01WorkingPoints.items()})

        # GN3V03 is not in any CDI yet, so it gets the discriminant but no working points
        taggerDecorations["GN3V03"] = "GN3V03"
        taggerFractionC["GN3V03"] = 0.3
        taggerFractionTau["GN3V03"] = 0.05

    kwargs.setdefault("TaggerDecorations", taggerDecorations)
    kwargs.setdefault("TaggerFractionC", taggerFractionC)
    kwargs.setdefault("TaggerFractionTau", taggerFractionTau)
    kwargs.setdefault("TaggerWorkingPoints", taggerWorkingPoints)

    # background fractions of the GN3XPV01 discriminants, see https://ftag.docs.cern.ch/xbb/taggers/gn3xpv01-working-points/
    kwargs.setdefault("GN3XPV01HbbFractions", {
        "phcc": 0.02, "phtautauhad": 0.0, "pWqq": 0.10, "ptop": 0.25,
        "pqcdll": 0.1575, "pqcdcx": 0.1575, "pqcdbx": 0.1575, "pqcdbb": 0.1575})
    kwargs.setdefault("GN3XPV01HccFractions", {
        "phbb": 0.02, "phtautauhad": 0.0, "pWqq": 0.0, "ptop": 0.15,
        "pqcdll": 0.65, "pqcdcx": 0.03, "pqcdbx": 0.03, "pqcdbb": 0.12})

    tool = CompFactory.JetTagDQA.PhysValBTag(**kwargs)
    acc.setPrivateTools(tool)
    return acc
