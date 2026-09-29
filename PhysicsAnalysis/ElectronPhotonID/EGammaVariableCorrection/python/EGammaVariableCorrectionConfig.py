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
    defaultTuneFile = "EGammaVariableCorrection/TUNE28EG/ElPhVariableNominalCorrection.conf"
    tuneFile = ""
    mcCampaign = flags.Input.MCCampaign
    runPeriod = flags.GeoModel.Run
    if runPeriod is LHCPeriod.Run2:
        # TUNE25: gamma FUDGE FACTORS RUN2 FULL DATA vs MC15-18, derived with r21.2
        tuneFile = "EGammaVariableCorrection/TUNE25/ElPhVariableNominalCorrection.conf"
    elif runPeriod is LHCPeriod.Run3:
        if mcCampaign in [Campaign.MC23a, Campaign.MC23d]:
            # TUNE28AD: gamma FUDGE FACTORS RUN3 2022-2023 vs MC23a-d, derived with r25
            tuneFile = "EGammaVariableCorrection/TUNE28AD/ElPhVariableNominalCorrection.conf"
        elif mcCampaign in [Campaign.MC23e, Campaign.MC23g]:
            # TUNE28EG: gamma FUDGE FACTORS RUN3 2024-2026 vs MC23e-g, derived with r25
            tuneFile = "EGammaVariableCorrection/TUNE28EG/ElPhVariableNominalCorrection.conf"
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
        flags, name="PhotonVariableNFCorrectionTool",
        nFolds=None, **kwargs):
    """Configure the Normalizing Flow-based photon shower shape correction tool

    nFolds: number of folds to use. If None (default), only 1 fold (fold 0) is used.
    Otherwise must be >=1 and not exceed the NFolds value in the tool's config file.
    """
    acc = ComponentAccumulator()

    from AthenaConfiguration.Enums import LHCPeriod

    if not flags.Input.isMC:
        raise RuntimeError("ElectronPhotonVariableNFCorrectionToolCfg: "
            "NF correction tool should not be called for data"
        )

    isFullSim = flags.Sim.ISF.Simulator.isFullSim()
    isRun3 = flags.GeoModel.Run is LHCPeriod.Run3
    isRun2 = flags.GeoModel.Run is LHCPeriod.Run2

    if isFullSim and isRun3:
        default_conf = "EGammaVariableCorrection/NF_y_TUNE2/Run3FS/ElectronPhotonVariableNFCorrectionTool.conf"
    elif isFullSim and isRun2:
        default_conf = "EGammaVariableCorrection/NF_y_TUNE2/Run2FS/ElectronPhotonVariableNFCorrectionTool.conf"
    elif not isFullSim and isRun3:
        default_conf = "EGammaVariableCorrection/NF_y_TUNE2/Run3AF3/ElectronPhotonVariableNFCorrectionTool.conf"
    elif not isFullSim and isRun2:
        # temporary the same Run3 AF3 models are applied to Run2 AF3
        default_conf = "EGammaVariableCorrection/NF_y_TUNE2/Run3AF3/ElectronPhotonVariableNFCorrectionTool.conf"
    else:
        raise RuntimeError(
            f"ElectronPhotonVariableNFCorrectionToolCfg: no NF correction config available for Run period {flags.GeoModel.Run} "
            f"(isFullSim={isFullSim}). Only Run2 and Run3 are supported."
        )

    conf_key = kwargs.setdefault("ConfigFile", default_conf)

    conf_file = PathResolver.FindCalibFile(conf_key)
    if not conf_file:
        raise RuntimeError(f"PathResolver cannot find {conf_key}")

    # Parse NFolds and ONNXnamePattern from config file
    n_folds_config = None
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
                n_folds_config = int(value)
            elif key == 'ONNXnamePattern':
                pattern = value

    if n_folds_config is None or pattern is None:
        raise RuntimeError(f'NFolds or ONNXnamePattern not found in config: {conf_file}')

    # Number of folds to actually build/use. Defaults to 1 fold (fold 0) if not requested.
    if nFolds is None:
        n_folds_used = 1
    else:
        if nFolds < 1:
            raise ValueError(f'nFolds must be >= 1, got {nFolds}')
        if nFolds > n_folds_config:
            raise ValueError(
                f'Requested nFolds={nFolds} exceeds NFolds={n_folds_config} available in config: {conf_file}'
            )
        n_folds_used = nFolds

    kwargs.setdefault("NFoldsOverride", n_folds_used)

    # Build forward and backward ONNX tools per fold
    forward_tools  = []
    backward_tools = []
    for i in range(n_folds_used):

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

    # Photons failing the shower shape cuts (NF not applied) are corrected with fudge factors
    if "FallbackFudgeTool" not in kwargs:
        kwargs["FallbackFudgeTool"] = acc.popToolsAndMerge(
            PhotonVariableCorrectionToolCfg(flags, name=f"{name}_FallbackFudgeTool"))

    acc.setPrivateTools(
        CompFactory.ElectronPhotonVariableNFCorrectionTool(name, **kwargs))
    return acc


