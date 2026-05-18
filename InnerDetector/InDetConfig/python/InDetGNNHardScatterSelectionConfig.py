# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
    
    selection_outputs = [
        ("SG::AuxVectorBase", f"StoreGateSvc+{input}.selectPtEta"),
        ("xAOD::IParticleContainer", f"StoreGateSvc+{input}.selectPtEta"),
    ]

    cfg.addEventAlgo(CompFactory.CP.AsgSelectionAlg(
        name = "GNNHS_"+input+"_SelectionAlg",
        selectionTool = selectionTool,
        selectionDecoration = "selectPtEta,as_char",
        particles = input,
        ExtraOutputs = selection_outputs))

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

    extraInputs = set(kwargs.get("ExtraInputs", set()))
    for inputContainer in overlapInputNames.values():
        extraInputs.add(("xAOD::IParticleContainer", f"StoreGateSvc+{inputContainer}.selectPtEta"))
    kwargs["ExtraInputs"] = list(extraInputs)

    overlap_outputs = [
        ("SG::AuxVectorBase", f"StoreGateSvc+{overlapInputNames['jets']}.passesOR"),
        ("xAOD::IParticleContainer", f"StoreGateSvc+{overlapInputNames['jets']}.passesOR"),
        ("SG::AuxVectorBase", f"StoreGateSvc+{overlapInputNames['electrons']}.passesOR"),
        ("xAOD::IParticleContainer", f"StoreGateSvc+{overlapInputNames['electrons']}.passesOR"),
        ("SG::AuxVectorBase", f"StoreGateSvc+{overlapInputNames['muons']}.passesOR"),
        ("xAOD::IParticleContainer", f"StoreGateSvc+{overlapInputNames['muons']}.passesOR"),
        ("SG::AuxVectorBase", f"StoreGateSvc+{overlapInputNames['photons']}.passesOR"),
        ("xAOD::IParticleContainer", f"StoreGateSvc+{overlapInputNames['photons']}.passesOR"),
    ]

    kwargs.setdefault("overlapTool", cfg.popToolsAndMerge(GNNHSOverlapRemovalToolCfg(flags)))
    
    cfg.addEventAlgo(CompFactory.CP.OverlapRemovalAlg(name, ExtraOutputs=overlap_outputs, **kwargs))

    or_container_types = {
        "jets": ["xAOD::JetContainer", "xAOD::IParticleContainer"],
        "electrons": ["xAOD::ElectronContainer", "xAOD::IParticleContainer"],
        "muons": ["xAOD::MuonContainer", "xAOD::IParticleContainer"],
        "photons": ["xAOD::PhotonContainer", "xAOD::EgammaContainer", "xAOD::IParticleContainer"],
    }

    for obj in overlapInputNames:
        output_name = overlapOutputNames[obj]
        view_outputs = [("xAOD::AuxContainerBase", f"StoreGateSvc+{output_name}Aux.")]
        for out_type in or_container_types[obj]:
            view_outputs.append((out_type, f"StoreGateSvc+{output_name}"))
        view_outputs.extend([
            ("SG::AuxVectorBase", f"StoreGateSvc+{output_name}.passesOR"),
            ("xAOD::IParticleContainer", f"StoreGateSvc+{output_name}.passesOR"),
            ("SG::AuxVectorBase", f"StoreGateSvc+{output_name}.selectPtEta"),
            ("xAOD::IParticleContainer", f"StoreGateSvc+{output_name}.selectPtEta"),
        ])

        cfg.addEventAlgo(CompFactory.CP.AsgViewFromSelectionAlg(
            name = "GNNHS_"+obj+"_ORSelectionAlg",
            input = overlapInputNames[obj],
            output = output_name,
            selection = [kwargs["OutputLabel"]+",as_char"],
            deepCopy = True,
            ExtraOutputs = view_outputs))
    
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
                       nnFile="InDetGNNHardScatterSelection/v1.2/HSGNN_baseline_v1.2.onnx")))

    if "TrackVertexAssociationTool" not in kwargs:
        from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import GNNHS_TTVAToolCfg
        kwargs.setdefault("TrackVertexAssociationTool", cfg.popToolsAndMerge(
            GNNHS_TTVAToolCfg(flags)))

    vertex_extra_inputs = set(kwargs.get("ExtraInputs", set()))
    for cont in ["electronsIn", "muonsIn", "photonsIn", "jetsIn"]:
        if cont in kwargs:
            vertex_extra_inputs.add(("xAOD::IParticleContainer", f"StoreGateSvc+{kwargs[cont]}"))
    if "photonsIn" in kwargs:
        photons_key = kwargs["photonsIn"]
        vertex_extra_inputs.update({
            ("xAOD::IParticleContainer", f"StoreGateSvc+{photons_key}.zCommon"),
            ("xAOD::IParticleContainer", f"StoreGateSvc+{photons_key}.caloPointingZ"),
            ("xAOD::IParticleContainer", f"StoreGateSvc+{photons_key}.zCommonError"),
        })
    kwargs["ExtraInputs"] = list(vertex_extra_inputs)

    cfg.addEventAlgo(CompFactory.InDetGNNHardScatterSelection.VertexDecoratorAlg(name, **kwargs))
    return cfg
        

# Global Sequence
def GNNSequenceCfg(flags, doOverlapRemoval=True):
    cfg = ComponentAccumulator()

    selectionSvc = CompFactory.CP.SelectionNameSvc("SelectionNameSvc")
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

    ptThresholds = {
        "jets": 15000,
        "electrons": 4500,
        "muons": 3000,
        "photons": 10000,
    }

    for obj in inputCollections:
        cfg.merge(GNNHSSelectionAlgCfg(flags, input = inputCollections[obj],
                                       minPt = ptThresholds[obj]))

    if doOverlapRemoval:
        # OverlapRemovalAlg is configured with affectingSystematicsFilter,
        # so the CP systematics service is only needed in this branch.
        cfg.addService(CompFactory.CP.SystematicsSvc("SystematicsSvc"))
        overlapOutputNames = {
            "muons": f'{inputCollections["muons"]}_OR',
            "electrons": f'{inputCollections["electrons"]}_OR',
            "photons": f'{inputCollections["photons"]}_OR',
            "jets": f'{inputCollections["jets"]}_OR',
        }
        cfg.merge(GNNHSOverlapRemovalAlgCfg(flags, overlapInputNames = inputCollections,
                                            overlapOutputNames = overlapOutputNames))
    else:
        # If overlap removal is disabled, use original containers
        overlapOutputNames = inputCollections

    from PhotonVertexSelection.PhotonVertexSelectionConfig import DecoratePhotonPointingAlgCfg
    photon_key = overlapOutputNames["photons"]
    cfg.merge(
        DecoratePhotonPointingAlgCfg(
            flags,
            PhotonContainerKey=photon_key,
            ExtraInputs=[
                ("xAOD::EgammaContainer", f"StoreGateSvc+{photon_key}"),
            ],
            ExtraOutputs=[
                ("xAOD::IParticleContainer", f"StoreGateSvc+{photon_key}.zCommon"),
                ("xAOD::IParticleContainer", f"StoreGateSvc+{photon_key}.caloPointingZ"),
                ("xAOD::IParticleContainer", f"StoreGateSvc+{photon_key}.zCommonError"),
            ],
        )
    )

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
