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
