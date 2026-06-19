# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def MuonLearningOnnxRuntimeSvcCfg(flags, name="OnnxRuntimeSvc", **kwargs):
    """Configure the shared ONNX Runtime service used by MuonLearning tools."""
    result = ComponentAccumulator()
    kwargs.setdefault("LogLevel", 3)
    svc = CompFactory.AthOnnx.OnnxRuntimeSvc(name, **kwargs)
    result.addService(svc, primary=False, create=True)
    return result

def GraphInferenceAlgCfg(flags, name = "GraphInferenceAlg", **kwargs):
    result  = ComponentAccumulator()
    the_alg = CompFactory.MuonML.InferenceAlg(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def GraphSPFilterToolCfg(flags, name ="GraphSPFilterTool", **kwargs):
    
    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(OnnxRuntimeSessionToolCfg(flags, model_fname="/eos/atlas/atlascerngroupdisk/data-art/grid-input/MuonRecRTT/TestModel.onnx")))
    kwargs.setdefault("MLFilterCut", -3.6) # Working point cut
    
    the_tool = CompFactory.MuonML.GraphSPFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def GraphBucketFilterToolCfg(flags, name ="GraphBucketFilterTool", **kwargs):

    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    model_path = kwargs.pop("ModelPath", "dev/MuonRecRTT/edgecnn_mu200.onnx")
    
    if not model_path.startswith('/'):
        pass
    else:
        pass
    
    result.merge(MuonLearningOnnxRuntimeSvcCfg(flags))
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(
        OnnxRuntimeSessionToolCfg(flags, model_fname=model_path,
                                  OnnxRuntimeSvc=result.getService("OnnxRuntimeSvc"))))
    kwargs.setdefault("OutputLevel", 3)  # INFO level (1=VERBOSE, 2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL)
    kwargs.setdefault("ScoreThreshold", 0.160)
    kwargs.setdefault("OutputName", "logits")
    kwargs.setdefault("SingleOutputIsLogit", False)
    
    the_tool = CompFactory.MuonML.GraphBucketFilterTool(name, **kwargs)
    result.setPrivateTools(the_tool)
    return result


def SegmentEdgeClassifierToolCfg(flags, name="SegmentEdgeClassifierTool", **kwargs):
    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    model_path = kwargs.pop("ModelPath", "MuonInference/models/edge_gnn_refit_top01_from_t0020.onnx")
    result.merge(MuonLearningOnnxRuntimeSvcCfg(flags))
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(
        OnnxRuntimeSessionToolCfg(flags, model_fname=model_path,
                                  OnnxRuntimeSvc=result.getService("OnnxRuntimeSvc"))))
    # Keep the same ONNX/model properties used by GraphBucketFilterToolCfg in this file.
    kwargs.setdefault("InputNodeName", "x")
    kwargs.setdefault("InputEdgeIndexName", "edge_index")
    kwargs.setdefault("InputEdgeAttrName", "edge_attr")
    kwargs.setdefault("OutputName", "logits")
    kwargs.setdefault("MaxDeltaThetaDeg", 35.0)
    kwargs.setdefault("MaxDeltaSector", 1)
    kwargs.setdefault("SectorModulo", 16)
    tool = CompFactory.MuonML.SegmentEdgeClassifierTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def SegmentTrackCandidateBuilderToolCfg(flags, name="SegmentTrackCandidateBuilderTool", **kwargs):
    result = ComponentAccumulator()
    # High-purity candidate cores are built with OverlapThreshold.
    # A second low-threshold recovery pass is added with EdgeThreshold.
    # This strongly reduces candidate loss from borderline true edges.
    kwargs.setdefault("EdgeThreshold", 0.25)
    kwargs.setdefault("OverlapThreshold", 0.8)
    kwargs.setdefault("UseRecoveryComponents", True)
    kwargs.setdefault("SymmetrizeDirectedEdges", True)
    kwargs.setdefault("AddAllSegmentsRecoveryCandidate", False)
    kwargs.setdefault("KeepIsolatedSegments", False)
    kwargs.setdefault("MinCandidateSize", 2)
    tool = CompFactory.MuonML.SegmentTrackCandidateBuilderTool(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def SegmentEdgeInferenceAlgCfg(flags, name="SegmentEdgeInferenceAlg", **kwargs):
    result = ComponentAccumulator()
    # Accept EdgeModelPath as a convenience shortcut so callers don't need to
    # build the tool object themselves; a raw dict is also unwrapped for
    # backwards-compatibility with call-sites that used dict syntax.
    edge_tool_kwargs = {}
    if "EdgeModelPath" in kwargs:
        edge_tool_kwargs["ModelPath"] = kwargs.pop("EdgeModelPath")
    # Silently unwrap legacy dict-style: EdgeClassifierTool={"ModelPath": ...}
    if isinstance(kwargs.get("EdgeClassifierTool"), dict):
        edge_tool_kwargs.update(kwargs.pop("EdgeClassifierTool"))

    candidate_builder_kwargs = {}
    for key in ("EdgeThreshold",
                "OverlapThreshold",
                "UseRecoveryComponents",
                "SymmetrizeDirectedEdges",
                "AddAllSegmentsRecoveryCandidate",
                "KeepIsolatedSegments",
                "MinCandidateSize"):
        if key in kwargs:
            candidate_builder_kwargs[key] = kwargs.pop(key)

    if isinstance(kwargs.get("CandidateBuilderTool"), dict):
        candidate_builder_kwargs.update(kwargs.pop("CandidateBuilderTool"))

    if "EdgeClassifierTool" not in kwargs:
        kwargs["EdgeClassifierTool"] = result.popToolsAndMerge(
            SegmentEdgeClassifierToolCfg(flags, **edge_tool_kwargs))
    kwargs.setdefault("CandidateBuilderTool", result.popToolsAndMerge(
        SegmentTrackCandidateBuilderToolCfg(flags, **candidate_builder_kwargs)))
    kwargs.setdefault("SegmentKey", "MuonSegmentsFromR4")
    kwargs.setdefault("CandidateDecoration", "MuonSegmentsFromR4.trackCandidateIds")
    alg = CompFactory.MuonML.SegmentEdgeInferenceAlg(name=name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result


def DisplacedVertexInferenceToolCfg(flags, name="DisplacedVertexInferenceTool", **kwargs):
    """Configure the DisplacedVertex graph-level ONNX inference tool.
    The current DV ONNX export consumes raw graph tensors with the contract
      x [N,7], edge_index [2,E], edge_attr [E,5], n_muon_nodes [1] -> logits [1]
    """
    from AthOnnxComps.OnnxRuntimeSessionConfig import OnnxRuntimeSessionToolCfg

    result = ComponentAccumulator()
    model_path = kwargs.pop("ModelPath", "MuonInference/models/edge_class_dv_mu200.onnx")

    result.merge(MuonLearningOnnxRuntimeSvcCfg(flags))
    kwargs.setdefault("ModelSession", result.popToolsAndMerge(
        OnnxRuntimeSessionToolCfg(flags, model_fname=model_path,
                                  OnnxRuntimeSvc=result.getService("OnnxRuntimeSvc"))))
    kwargs.setdefault("InputNodeName", "x")
    kwargs.setdefault("InputEdgeIndexName", "edge_index")
    kwargs.setdefault("InputEdgeAttrName", "edge_attr")
    kwargs.setdefault("InputNMuonNodesName", "n_muon_nodes")
    kwargs.setdefault("OutputName", "logits")
    kwargs.setdefault("SingleOutputMode", "logit")
    if "SpacePointKeys" not in kwargs:
        sp_containers = []
        if flags.Detector.GeometryMDT or flags.Detector.GeometryRPC or flags.Detector.GeometryTGC:
            sp_containers.append("MuonSpacePoints")
        elif flags.Detector.GeometryMM or flags.Detector.GeometrysTGC:
            sp_containers.append("NswSpacePoints")
        kwargs.setdefault("SpacePointKeys", sp_containers)
    kwargs.setdefault("UseBucketSegmentSelection", True)
    kwargs.setdefault("MinTowerEnergyMeV", 1000.0)
    kwargs.setdefault("MaxTowerSegmentDR", 0.4)
    kwargs.setdefault("CaloRMaxMm", 4250.0)
    kwargs.setdefault("CaloZMaxMm", 6500.0)
    kwargs.setdefault("SectorModulo", 16)
    kwargs.setdefault("RequireEdges", False)
    tool = CompFactory.MuonML.DVInferenceToolBase(name, **kwargs)
    result.setPrivateTools(tool)
    return result


def DisplacedVertexCaloTowerCfg(flags):
    """Configure the calorimeter reconstruction used by the DV training converter.
    """
    result = ComponentAccumulator()

    # Same reconstruction chain used by MuonBucketDumpConfig.CaloCellsDumperCfg.
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    result.merge(CaloRecoCfg(flags))

    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    result.getPrimaryAndMerge(CaloTowerMakerCfg(flags))

    return result


def DisplacedVertexInferenceAlgCfg(flags, name="DisplacedVertexInferenceAlg", **kwargs):
    """Configure a runnable event-level DisplacedVertex inference algorithm."""
    result = ComponentAccumulator()
    do_calo_tower_build = kwargs.pop("DoCaloTowerBuild", True)
    do_ml_bucket_filter = kwargs.pop("DoMLBucketFilter", True)
    bucket_model_path = kwargs.pop("BucketModelPath", None)
    bucket_threshold = kwargs.pop("BucketThreshold", None)
    filtered_bucket_key = kwargs.pop("FilteredBucketKey", "FilteredMlBuckets")
    use_filtered_buckets_for_dv_graph = kwargs.pop("UseFilteredBucketsForDVGraph", False)
    alg_output_level = kwargs.get("OutputLevel", None)
    tool_kwargs = {}
    for key in (
        "ModelPath",
        "InputNodeName",
        "InputEdgeIndexName",
        "InputEdgeAttrName",
        "InputNMuonNodesName",
        "OutputName",
        "SingleOutputMode",
        "SegmentKey",
        "SpacePointKeys",
        "UseBucketSegmentSelection",
        "TowerContainerKey",
        "MinTowerEnergyMeV",
        "MaxTowerSegmentDR",
        "CaloRMaxMm",
        "CaloZMaxMm",
        "SectorModulo",
        "RequireEdges",
        "MaxEdges",
        "FallbackToAllSegments",
        "DebugDumpFirstNNodes",
        "DebugDumpFirstNEdges",
        "SpacePointKeys",
        "UseBucketSegmentSelection",
        "OutputLevel",
    ):
        if key in kwargs:
            tool_kwargs[key] = kwargs.pop(key)

    if isinstance(kwargs.get("InferenceTool"), dict):
        tool_kwargs.update(kwargs.pop("InferenceTool"))
        
    tower_key = tool_kwargs.get("TowerContainerKey", "CombinedTower")
    if do_calo_tower_build and tower_key:
        result.merge(DisplacedVertexCaloTowerCfg(flags))

    if do_ml_bucket_filter:
        bucket_filter_kwargs = {
            "WriteSpacePointKey": filtered_bucket_key,
            "ReadSpacePoints": "MuonSpacePoints",
        }
        if bucket_model_path is not None:
            bucket_filter_kwargs["ModelPath"] = bucket_model_path
        if bucket_threshold is not None:
            bucket_filter_kwargs["ScoreThreshold"] = bucket_threshold
        bucket_tool = result.popToolsAndMerge(
            GraphBucketFilterToolCfg(flags, **bucket_filter_kwargs)
        )
        result.merge(
            GraphInferenceAlgCfg(
                flags,
                name="DVBucketPrefilterAlg",
                InferenceTools=[bucket_tool],
            )
        )
        if use_filtered_buckets_for_dv_graph:
            tool_kwargs.setdefault("SpacePointKeys", [filtered_bucket_key])
        tool_kwargs.setdefault("UseBucketSegmentSelection", True)

    if "InferenceTool" not in kwargs:
        kwargs["InferenceTool"] = result.popToolsAndMerge(
            DisplacedVertexInferenceToolCfg(flags, **tool_kwargs)
        )

    if alg_output_level is not None:
        kwargs["OutputLevel"] = alg_output_level

    kwargs.setdefault("ScoreDecoration", "EventInfo.dv_score")
    kwargs.setdefault("RawOutputDecoration", "EventInfo.dv_rawOutput")
    kwargs.setdefault("PassDecoration", "EventInfo.dv_pass")
    kwargs.setdefault("NNodesDecoration", "EventInfo.dv_nNodes")
    kwargs.setdefault("NEdgesDecoration", "EventInfo.dv_nEdges")
    alg = CompFactory.MuonML.DVInferenceAlg(name=name, **kwargs)
    result.addEventAlgo(alg, primary=True)
    return result
