# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthOnnxComps.OnnxRuntimeInferenceConfig import OnnxRuntimeInferenceToolCfg
from PathResolver import PathResolver
import yaml


def CaloClusterMLCalibToolLiteCfg(
    flags,
    name="CaloClusterMLCalibToolLite",
    config_file="CaloClusterCorrection/config_hgm_mc20.yaml",
):
    with open(PathResolver.FindCalibFile(config_file)) as f:
        config = yaml.safe_load(f)

    onnx_model_path = config["model"]["onnx_path"]
    features = config["data"]["features"]
    sorted_features = sorted(features.items(), key=lambda item: item[1]["position"])
    transform_names = [
        item[1]["preprocessing"]["processors"][0] for item in sorted_features
    ]
    transform_params = [
        [float(x) for x in item[1]["preprocessing"]["parameters"]]
        for item in sorted_features
    ]

    ca = ComponentAccumulator()
    onnx_tool = ca.popToolsAndMerge(OnnxRuntimeInferenceToolCfg(flags, onnx_model_path))
    CaloClusterMLCalibToolLite = CompFactory.CaloClusterMLCalibToolLite(name)
    CaloClusterMLCalibToolLite.ORTInferenceTool = onnx_tool
    CaloClusterMLCalibToolLite.PreprocessingTransformNames = transform_names
    CaloClusterMLCalibToolLite.PreprocessingTransformParams = transform_params
    ca.setPrivateTools(CaloClusterMLCalibToolLite)

    return ca
