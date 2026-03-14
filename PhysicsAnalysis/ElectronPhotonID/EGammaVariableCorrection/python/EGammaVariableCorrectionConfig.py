# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from Campaigns.Utils import Campaign
from PathResolver import PathResolver
from AthenaCommon.Logging import logging

def ElectronVariableCorrectionToolCfg(
        flags, name="ElectronVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool for electrons"""
    acc = ComponentAccumulator()
    log = logging.getLogger("ElectronVariableCorrectionToolCfg")
    # Can ultimately be configured differently between Run 2 and Run 3 configs
    # TUNE27: e FUDGE FACTORS RUN2 FULL DATA, derived with rel 22.2
    tuneFile = "EGammaVariableCorrection/TUNE27/ElVariableNominalCorrection.conf"
    kwargs.setdefault("ConfigFile", tuneFile)
    log.info("Setting as default FF file: %s", tuneFile)
    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableCorrectionTool(name, **kwargs))
    return acc

def PhotonVariableCorrectionToolCfg(
        flags, name="PhotonVariableCorrectionTool", **kwargs):
    """Configure the e/gamma variable correction tool for photons"""
    acc = ComponentAccumulator()
    log = logging.getLogger("PhotonVariableCorrectionToolCfg")
    # fallback tune file if there is no dedicated tuning
    defaultTuneFile = "EGammaVariableCorrection/TUNE27E/ElPhVariableNominalCorrection.conf"
    tuneFile = ""
    mcCampaign = flags.Input.MCCampaign
    runPeriod = flags.GeoModel.Run
    if runPeriod is LHCPeriod.Run2:
        # TUNE25: gamma FUDGE FACTORS RUN2 FULL DATA vs MC15-18, derived with r21.2
        tuneFile = "EGammaVariableCorrection/TUNE25/ElPhVariableNominalCorrection.conf"
    elif runPeriod is LHCPeriod.Run3:
        if mcCampaign in [Campaign.MC23a, Campaign.MC23d]:
            # TUNE27AD: gamma FUDGE FACTORS RUN3 2022-2023 vs MC23a-d, derived with r25
            tuneFile = "EGammaVariableCorrection/TUNE27AD/ElPhVariableNominalCorrection.conf"
        elif mcCampaign is Campaign.MC23e:
            # TUNE27E: gamma FUDGE FACTORS RUN3 2024 vs MC23e, derived with r25
            tuneFile = "EGammaVariableCorrection/TUNE27E/ElPhVariableNominalCorrection.conf"
        else:
            log.warning("No default FF file centrally provided for mc campaign %s", mcCampaign.value)
            tuneFile = defaultTuneFile
    else:
        log.warning("No default FF file centrally provided for run period %s", runPeriod.value)
        tuneFile = defaultTuneFile
    log.info("Setting as default FF file: %s", tuneFile)
    kwargs.setdefault("ConfigFile", tuneFile)
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


