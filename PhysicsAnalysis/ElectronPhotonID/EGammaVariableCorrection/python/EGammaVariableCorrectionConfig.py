# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from PathResolver import PathResolver

def ElectronVariableCorrectionToolCfg(
        flags, name="ElectronVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Can ultimately be configured differently between Run 2 and Run 3 configs
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE27/ElVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc

def PhotonVariableCorrectionToolCfg(
        flags, name="PhotonVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool"""
    acc = ComponentAccumulator()
    # Use TUNE 25 for now for photons
    kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/TUNE25/ElPhVariableNominalCorrection.conf")
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc


def ElectronPhotonVariableNFCorrectionToolCfg(
        flags, name="PhotonVariableNFCorrectionTool", **kwargs):
    """Configure the Normalizing Flow-based photon shower shape correction tool"""
    acc = ComponentAccumulator()

    conf_key = kwargs.setdefault("ConfigFile", "EGammaVariableCorrection/ElectronPhotonVariableNFCorrectionTool.conf")
    conf_file = PathResolver.FindCalibFile(conf_key)
    if not conf_file:
        raise RuntimeError(f"PathResolver cannot find {conf_key}")

    # Parse NFolds and ONNXnamePattern from config file
    n_folds = None
    pattern = None
    with open(conf_file, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            key, _, value = line.partition(':')
            key   = key.strip()
            value = value.strip()
            if key == 'NFolds':
                n_folds = int(value)
            elif key == 'ONNXnamePattern':
                pattern = value

    if n_folds is None or pattern is None:
        raise RuntimeError(f'NFolds or ONNXnamePattern not found in config: {conf_file}')

    # Build forward and backward ONNX tools per fold
    forward_tools  = []
    backward_tools = []
    for i in range(n_folds):
        fwd_session = CompFactory.AthOnnx.OnnxRuntimeSessionToolCPU(
            f'NFCorrectionORTSessionToolForward_{i}',
            ModelFileName=f'{pattern}_forward_{i}.onnx')
        fwd_tool = CompFactory.AthOnnx.OnnxRuntimeInferenceTool(
            f'NFCorrectionOnnxToolForward_{i}',
            ORTSessionTool=fwd_session)
        forward_tools.append(fwd_tool)

        bwd_session = CompFactory.AthOnnx.OnnxRuntimeSessionToolCPU(
            f'NFCorrectionORTSessionToolBackward_{i}',
            ModelFileName=f'{pattern}_backward_{i}.onnx')
        bwd_tool = CompFactory.AthOnnx.OnnxRuntimeInferenceTool(
            f'NFCorrectionOnnxToolBackward_{i}',
            ORTSessionTool=bwd_session)
        backward_tools.append(bwd_tool)

    kwargs.setdefault("OnnxInferenceToolsForward",  forward_tools)
    kwargs.setdefault("OnnxInferenceToolsBackward", backward_tools)

    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableNFCorrectionTool(name, **kwargs))
    return acc


