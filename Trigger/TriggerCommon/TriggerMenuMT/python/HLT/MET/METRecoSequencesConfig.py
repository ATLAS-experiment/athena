#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""Configuration sequences for the MET input reconstruction

By convention all of these functions return two values: first a defaultdict mapping from
reco step to the component accumulators containing the reconstruction sequences and
second the name(s) of the key outputs created. The names can either be a single string
if there is only one output or a tuple otherwise. Which outputs are returned and their
order should be clearly documented in the method documentation
"""

from typing import Any

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from eflowRec.PFHLTConfig import PFCfg
from JetRecConfig.JetRecConfig import getConstitModAlg_nojetdef
from TrigCaloRec.TrigCaloRecConfig import (
    hltCaloCellMakerCfg,
    jetmetTopoClusteringCfg,
    jetmetTopoClusteringCfg_LC,
)
from ..CommonSequences.FullScanDefs import em_clusters, lc_clusters, trkFSRoI
from ..Jet.JetRecoCommon import (
    defineJetConstit,
)
from ..Jet.JetRecoSequencesConfig import JetRecoCfg
from ..Jet.JetTrackingConfig import JetFSTrackingCfg
from .StepOutput import StepOutput
from TrackVertexAssociationTool.TTVAToolConfig import TTVAToolCfg


def jetRecoDictForMET(**recoDict) -> dict[str, Any]:
    """Get a jet reco dict that's usable for the MET slice"""
    from ..Jet.JetRecoCommon import getJetCalibDefaultString, jetRecoDictToString
    from ..Jet.JetRecoCommon import recoKeys as jetRecoKeys
    from ..Menu.SignatureDicts import JetChainParts_Default
    
    jrd = {k: recoDict.get(k, JetChainParts_Default[k]) for k in jetRecoKeys}
    # Rename the cluster calibration
    try:
        jrd["clusterCalib"] = recoDict["calib"]
    except KeyError:
        pass
    # Fill constitMod
    jrd["constitMod"] = recoDict.get("constitmod", "")
    # We only use em calibration for PFOs
    if jrd["constitType"] == "pf":
        jrd["clusterCalib"] = "em"
    # Interpret jet calibration
    if jrd["jetCalib"] == "default":
        jrd["jetCalib"] = getJetCalibDefaultString(jrd)
    if jrd["constitType"] != "tc" or "gsc" in jrd["jetCalib"]:
        jrd["trkopt"] = "ftf"
    jrd["jetDefStr"] = jetRecoDictToString(jrd)
    return jrd


@AccumulatorCache
def cellInputCfg(flags, **recoDict) -> StepOutput:
    """Create the cell inputs"""
    acc = hltCaloCellMakerCfg(flags, name="HLTCaloCellMakerFS", roisKey="")
    return StepOutput.create(acc, Cells=acc.getPrimary().CellsName)


@AccumulatorCache
def clusterInputCfg(flags, **recoDict) -> StepOutput:
    """Create the cluster inputs"""
    if recoDict["calib"] == "em":
        acc = jetmetTopoClusteringCfg(flags, RoIs="")
        clusters = em_clusters
    elif recoDict["calib"] == "lcw":
        acc = jetmetTopoClusteringCfg_LC(flags, RoIs="")
        clusters = lc_clusters
    else:
        raise ValueError(f"Invalid cluster calibration '{recoDict['calib']}'")

    if recoDict.get("constitmod"):
        # Force the constituent type to topoclusters
        jetRecoDict = jetRecoDictForMET(**(recoDict | {"constitType": "tc"}))
        constit = defineJetConstit(jetRecoDict, clustersKey=clusters)
        acc.addEventAlgo(
            getConstitModAlg_nojetdef(
                constit, flags, context=jetRecoDict.get("trkopt", "default"), 
            )
        )
        clusters = constit.containername

    return StepOutput.create(acc, Clusters=clusters)


@AccumulatorCache
def trackingInputCfg(flags, **recoDict) -> StepOutput:
    """Get the tracking inputs"""
    return StepOutput.create(
        JetFSTrackingCfg(flags, "ftf", RoIs=trkFSRoI),
        step_idx=2,
        **flags.Jet.Context.ftf,
    )


@AccumulatorCache
def pfoInputCfg(flags, **recoDict) -> StepOutput:
    """Get the PFO inputs"""
    inputs = StepOutput.merge(
        cellInputCfg(flags),
        clusterInputCfg(flags, calib="em"),
        trackingInputCfg(flags),
    )
    acc = PFCfg(
        flags,
        tracktype="ftf",
        clustersin=inputs["Clusters"],
        calclustersin="",
        tracksin=inputs["Tracks"],
        verticesin=inputs["Vertices"],
        cellsin=inputs["Cells"],
    )

    # The jet constituent modifier sequence here is to apply the correct weights and
    # decorate the PV matching decoration. If we've specified constituent modifiers
    # those are also applied.
    jetRecoDict = jetRecoDictForMET(
        **(recoDict | {"trkopt": "ftf", "constitType": "pf"})
    )
    constit = defineJetConstit(jetRecoDict, pfoPrefix="HLT_ftf")
    acc.addEventAlgo(
        getConstitModAlg_nojetdef(constit, flags, context=jetRecoDict.get("trkopt", "default"))
    )
    pfoPrefix = constit.containername
    if pfoPrefix.endswith("ParticleFlowObjects"):
        pfoPrefix = pfoPrefix[:-19]
    return StepOutput.create(
        acc,
        inputs,
        PFOPrefix=pfoPrefix,
        cPFOs=pfoPrefix + "ChargedParticleFlowObjects",
        nPFOs=pfoPrefix + "NeutralParticleFlowObjects",
    )


@AccumulatorCache
def mergedPFOInputCfg(flags, **recoDict) -> StepOutput:
    """Create the merged PFO inputs"""
    pfos = pfoInputCfg(flags, **recoDict)
    alg = CompFactory.HLT.MET.FlowElementPrepAlg(
        f"{pfos['PFOPrefix']}METTrigPFOPrepAlg",
        InputNeutralKey=pfos["nPFOs"],
        InputChargedKey=pfos["cPFOs"],
        OutputKey=f"{pfos['PFOPrefix']}METTrigCombinedParticleFlowObjects",
        OutputCategoryKey="PUClassification",
    )
    acc = ComponentAccumulator()
    acc.addEventAlgo(alg, primary=True)
    return StepOutput.create(
        acc, pfos, MergedPFOs=alg.OutputKey, PUCategory=alg.OutputCategoryKey
    )


@AccumulatorCache
def cvfClusterInputCfg(flags, **recoDict) -> StepOutput:
    """Create the clusters with CVF decorated"""
    inputs = StepOutput.merge(
        clusterInputCfg(flags, **recoDict),
        trackingInputCfg(flags),
    )
    acc = ComponentAccumulator()
    acc.addEventAlgo(
        CompFactory.HLT.MET.CVFAlg(
            f"{recoDict['calib']}ftfClusterCVFAlg",
            InputClusterKey=inputs["Clusters"],
            InputTrackKey=inputs["Tracks"],
            InputVertexKey=inputs["Vertices"],
            OutputCVFKey="CVF",
            TrackSelectionTool=CompFactory.InDet.InDetTrackSelectionTool(
                CutLevel="TightPrimary"
            ),
            TVATool=acc.popToolsAndMerge(
                TTVAToolCfg(
                    flags,
                    "TTVATool",
                    addDecoAlg=False,
                    WorkingPoint="Custom",
                    d0_cut=2.0,
                    dzSinTheta_cut=2.0,
                    TrackContName=inputs["Tracks"],
                    VertexContName=inputs["Vertices"],
                    HardScatterLinkDeco="",
                )
            ),
            ExtensionTool=CompFactory.ApproximateTrackToLayerTool(),
        )
    )
    acc.addEventAlgo(
        CompFactory.HLT.MET.CVFPrepAlg(
            f"{recoDict['calib']}ftfClusterCVFPrepAlg",
            InputClusterKey=inputs["Clusters"],
            InputCVFKey="CVF",
            OutputCategoryKey="PUClassification",
        )
    )
    return StepOutput.create(acc, inputs, CVF="CVF", PUCategory="PUClassification")


@AccumulatorCache
def jetInputCfg(flags, force_tracks: bool = False, **recoDict) -> StepOutput:
    """Create the input jets

    Set force_tracks to True to require tracks and ensure that they are ghost-associated
    to the jets.

    Returns the accumulators and (jets, jetDef)
    """
    if force_tracks:
        recoDict["trkopt"] = "ftf"
    # hard code to em (for now) - there are no LC jets in EDM
    jrd = jetRecoDictForMET(
        **(
            recoDict
            | {"calib": "em"}
        )
    )
    
    inputs = StepOutput()
    if jrd["trkopt"] == "ftf":
        inputs.merge_other(trackingInputCfg(flags))
    if jrd["constitType"] == "pf":
        inputs.merge_other(pfoInputCfg(flags))
    else:
        # Always use EM clusters for jets
        inputs.merge_other(clusterInputCfg(flags, calib="em"))

    acc = ComponentAccumulator()
    jet_acc, jetName, jetDef = JetRecoCfg(flags, clustersKey=inputs["Clusters"], **jrd)
    acc.merge(jet_acc)
    return StepOutput.create(
        acc, inputs, Jets=jetName, JetDef=jetDef, **flags.Jet.Context[jrd["trkopt"]] 
    )
