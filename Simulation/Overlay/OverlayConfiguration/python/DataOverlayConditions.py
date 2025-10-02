# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import ProductionStep
from IOVDbSvc.IOVDbSvcConfig import addOverride


def DataOverlayConditionsBaseCfg(flags):
    """Data overlay conditions overrides for data overlay."""
    acc = ComponentAccumulator()

    # LAr alignment (common for all substeps)
    # Used from MC for simplicity
    acc.merge(addOverride(flags, "/LAR/LArCellPositionShift", tag="LArCellPositionShift-IOVDEP-00", db="COOLOFL_LAR/OFLP200"))

    # Some conditions are split by fast chain (sim+digi+overlay) and reco steps
    if flags.Common.ProductionStep is not ProductionStep.Reconstruction:
        # SCT
        # Only for digitization, not reconstruction
        # Available only in OFLP200 (not in CONDBR2).
        acc.merge(addOverride(flags, "/SCT/DAQ/Calibration/ChipNoise", "SctDaqCalibrationChipNoise-MC-01", db="COOLOFL_SCT/OFLP200"))
        acc.merge(addOverride(flags, "/SCT/DAQ/Calibration/ChipGain", "SctDaqCalibrationChipGain-MC-01", db="COOLOFL_SCT/OFLP200"))

        # LAr
        # Sampling fractions only for simulation+digitization, not reconstruction
        # Available only in OFLP200 (not in CONDBR2).
        acc.merge(addOverride(flags, "/LAR/ElecCalibMC/fSampl", tag="LARElecCalibMCfSampl-G4106-22056-v2"))

        # Tile
        # Sampling fractions only for simulation+digitization, not reconstruction
        # Available only in OFLP200 (not in CONDBR2).
        acc.merge(addOverride(flags, "/TILE/OFL02/CALIB/SFR", tag="TileOfl02CalibSfr-SIM-07"))

        # TGC
        # Only for digitization, not reconstruction
        # Available only in OFLP200 (not in CONDBR2).
        acc.merge(addOverride(flags, "/TGC/DIGIT/ASDPOS", tag="TgcDigitAsdPos-00-01"))
        acc.merge(addOverride(flags, "/TGC/DIGIT/TOFFSET", tag="TgcDigitTimeOffset-00-01"))
        acc.merge(addOverride(flags, "/TGC/DIGIT/XTALK", tag="TgcDigitXTalk-00-01"))

    else:
        # TRT
        # Only for reconstruction
        # TODO: Include in a global tag
        acc.merge(addOverride(flags, "/TRT/Calib/MC/RT", tag="TrtCalibRt-MC-run2-run3-01"))
        acc.merge(addOverride(flags, "/TRT/Calib/MC/T0", tag="TrtCalibT0-MC-run2-run3-01"))

    return acc


def DataOverlay2023Cfg(flags):
    """Conditions for 2023 data overlay."""
    return DataOverlayConditionsBaseCfg(flags)


def DataOverlay2024Cfg(flags):
    """Conditions for 2024 data overlay."""
    return DataOverlayConditionsBaseCfg(flags)


def DataOverlay2025OOCfg(flags):
    """Conditions for 2025 OO data overlay."""
    return DataOverlayConditionsBaseCfg(flags)
