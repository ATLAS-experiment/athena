# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# A GNN based algorithm for selecting the Hard Scatter vertex (truth primary vertex 
# that is simulated for the hard process, i.e. the PV in the TruthEvent) 
 
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

from TrkConfig.VertexFindingFlags import VertexSortingSetup
# Overlap removal configurations

def AsgPtEtaSelectionToolCfg(flags, name="AsgPtEtaSelectionTool", **kwargs):
    cfg = ComponentAccumulator()
    kwargs.setdefault("maxEta", 2.5)
    cfg.setPrivateTools(CompFactory.CP.AsgPtEtaSelectionTool(name, **kwargs))
    return cfg

def AsgPtEtaSelectionToolGapCfg(flags, name="AsgPtEtaSelectionTool", **kwargs):
    kwargs.setdefault("etaGapLow", 1.37)
    kwargs.setdefault("etaGapHigh", 1.52)
    return AsgPtEtaSelectionToolCfg(flags, name, **kwargs)

def AsgViewFromSelectionAlgCfg(flags, name="AsgViewFromSelectionAlg", **kwargs):
    cfg = ComponentAccumulator()
    kwargs.setdefault("selection", "selectPtEta")
    kwargs.setdefault("deepCopy", False)
    cfg.addEventAlgo(CompFactory.CP.AsgViewFromSelectionAlg(name, **kwargs))
    return cfg

def GNNHSSelectionAlgCfg(flags, input, minPt):
    cfg = ComponentAccumulator()

    selectionTool = None
    if input in ["Electrons", "Photons", "AntiKt4EMTopoJets"]:
        selectionTool = cfg.popToolsAndMerge(
            AsgPtEtaSelectionToolGapCfg(flags, minPt = minPt))
    elif input in ["Muons"]:
        selectionTool = cfg.popToolsAndMerge(
            AsgPtEtaSelectionToolCfg(flags, minPt = minPt))
    
    cfg.addEventAlgo(CompFactory.CP.AsgSelectionAlg(
        name = "GNNHS_"+input+"_SelectionAlg",
        selectionTool = selectionTool,
        selectionDecoration = "selectPtEta,as_char",
        particles = input))

    return cfg


def GNNHSOverlapRemovalToolCfg(flags, name="GNNHS_OverlapRemovalToolCfg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("InputLabel", "selectPtEta")
    kwargs.setdefault("OutputLabel", "passesOR")
    kwargs.setdefault("OutputPassValue", True)

    subtool_kwargs={}
    for prop in ["InputLabel", "OutputLabel", "OutputPassValue"]:
        subtool_kwargs[prop] = kwargs[prop]

    kwargs.setdefault("EleEleORT", CompFactory.ORUtils.EleEleOverlapTool(**subtool_kwargs))
    kwargs.setdefault("EleMuORT", CompFactory.ORUtils.EleMuSharedTrkOverlapTool(**subtool_kwargs))
    kwargs.setdefault("EleJetORT", CompFactory.ORUtils.EleJetOverlapTool(**subtool_kwargs))
    kwargs.setdefault(
        "MuJetORT",
        CompFactory.ORUtils.MuJetOverlapTool(
            PVContainerName=(
                "PrimaryVertices_initial" 
                if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting 
                else "PrimaryVertices"
            ),
            **subtool_kwargs
        )
    )
    kwargs.setdefault("PhoEleORT", CompFactory.ORUtils.DeltaROverlapTool(**subtool_kwargs))
    kwargs.setdefault("PhoMuORT", CompFactory.ORUtils.DeltaROverlapTool(**subtool_kwargs))
    kwargs.setdefault("PhoJetORT", CompFactory.ORUtils.DeltaROverlapTool(**subtool_kwargs))
 
    cfg.setPrivateTools(CompFactory.ORUtils.OverlapRemovalTool(name, **kwargs))
    return cfg

def GNNHSOverlapRemovalAlgCfg(flags, name="GNNHS_OverlapRemovalAlg",
                              overlapInputNames = None, overlapOutputNames = None, **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("OutputLabel", "passesOR")
    kwargs.setdefault("affectingSystematicsFilter", ".*")

    for obj in overlapInputNames:
        kwargs.setdefault(obj, overlapInputNames[obj])
        kwargs.setdefault(obj+"Decoration", kwargs["OutputLabel"] + ",as_char")

    kwargs.setdefault("overlapTool", cfg.popToolsAndMerge(GNNHSOverlapRemovalToolCfg(flags)))
    
    cfg.addEventAlgo(CompFactory.CP.OverlapRemovalAlg(name, **kwargs))

    for obj in overlapInputNames:
        cfg.addEventAlgo(CompFactory.CP.AsgViewFromSelectionAlg(
            name = "GNNHS_"+obj+"_ORSelectionAlg",
            input = overlapInputNames[obj],
            output = overlapOutputNames[obj],
            selection = [kwargs["OutputLabel"]+",as_char"],
            deepCopy = False))
    
    return cfg


# GNNTool + VertexDecoratorAlg configs

def GNNToolCfg(flags, name="HardScatterSelectionGNNTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.InDetGNNHardScatterSelection.GNNTool(name, **kwargs))
    return acc

def GNNHSVertexDecoratorAlgCfg(flags, name="GNNHS_VertexDecoratorAlg", **kwargs):
    cfg = ComponentAccumulator()

    kwargs.setdefault("photonsIn", "Photons")

    if "gnnTool" not in kwargs:
        kwargs.setdefault("gnnTool", cfg.popToolsAndMerge(
            GNNToolCfg(flags,
                       nnFile="InDetGNNHardScatterSelection/v0/HSGN2_export_090824.onnx")))

    if "TrackVertexAssociationTool" not in kwargs:
        from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import TTVAToolCfg
        kwargs.setdefault(
            "TrackVertexAssociationTool",
            cfg.popToolsAndMerge(
                TTVAToolCfg(
                    flags,
                    "TrackVertexAssociationTool_GNNHS",
                    VertexContName=(
                        "PrimaryVertices_initial" 
                        if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting 
                        else "PrimaryVertices"
                    ),
                )
            ),
        )


    cfg.addEventAlgo(
        CompFactory.InDetGNNHardScatterSelection.VertexDecoratorAlg(name, **kwargs))
    return cfg
        

# Global Sequence
def GNNSequenceCfg(flags, **kwargs):
    cfg = ComponentAccumulator()

    sysSvc = CompFactory.CP.SystematicsSvc("SystematicsSvc")
    selectionSvc = CompFactory.CP.SelectionNameSvc("SelectionNameSvc")
    cfg.addService(sysSvc)
    cfg.addService(selectionSvc)

    inputCollections = {
        "jets": (
            "AntiKt4EMTopoCustomVtxGNNJets" 
            if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting 
            else "AntiKt4EMTopoJets"
        ),
        "electrons": "Electrons",
        "muons": "Muons",
        "photons": "Photons",
    }

    from PhotonVertexSelection.PhotonVertexSelectionConfig import (
        DecoratePhotonPointingAlgCfg)
    cfg.merge(DecoratePhotonPointingAlgCfg(flags, PhotonContainerKey=inputCollections["photons"]))
    
    ptThresholds = {
        "jets": 15000,
        "electrons": 4500,
        "muons": 3000,
        "photons": 10000,
    }

    for obj in inputCollections:
        cfg.merge(GNNHSSelectionAlgCfg(flags, input = inputCollections[obj],
                                       minPt = ptThresholds[obj]))

    overlapOutputNames = {
        "muons": f'{inputCollections["muons"]}_OR',
        "electrons": f'{inputCollections["electrons"]}_OR',
        "photons": f'{inputCollections["photons"]}_OR',
        "jets": f'{inputCollections["jets"]}_OR',
    }

    cfg.merge(GNNHSOverlapRemovalAlgCfg(flags, overlapInputNames = inputCollections,
                                        overlapOutputNames = overlapOutputNames))

    cfg.merge(
        GNNHSVertexDecoratorAlgCfg(
            flags,
            vertexIn=(
                "PrimaryVertices_initial" 
                if flags.Tracking.PriVertex.sortingSetup is VertexSortingSetup.GNNSorting 
                else "PrimaryVertices"
            ),
            electronsIn=overlapOutputNames["electrons"],
            muonsIn=overlapOutputNames["muons"],
            photonsIn=overlapOutputNames["photons"],
            jetsIn=overlapOutputNames["jets"],
        )
    )

    return cfg


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # AOD input
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags.Input.Files = defaultTestFiles.AOD_RUN3_MC
    flags.Exec.MaxEvents = 100
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    top_acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    top_acc.merge(PoolReadCfg(flags))

    from InDetPhysValMonitoring.addRecoJetsConfig import (
        AddRecoJetsIfNotExistingCfg)
    top_acc.merge(AddRecoJetsIfNotExistingCfg(
        flags, "AntiKt4EMTopoJets"))

    top_acc.merge(GNNSequenceCfg(flags))
    from AthenaCommon.Constants import DEBUG
    top_acc.foreach_component("AthEventSeq/*").OutputLevel = DEBUG
    top_acc.printConfig(withDetails=True, summariseProps=True)
    top_acc.store(open("GNNSequenceConfig.pkl", "wb"))

    import sys
    if "--norun" not in sys.argv:
        sc = top_acc.run(1)
        if sc.isFailure():
            sys.exit(-1)
