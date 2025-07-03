# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import ProductionStep
from IOVDbSvc.IOVDbSvcConfig import addOverride


def DataOverlay2023Cfg(flags):
    """Conditions for 2023 data overlay."""
    # TODO: for now this is mostly a placeholder based on the p-p test configuration.

    acc = ComponentAccumulator()

    if flags.Common.ProductionStep is ProductionStep.Reconstruction:
        # TRT
        acc.merge(addOverride(flags, "/TRT/Calib/MC/RT", tag="TrtCalibRt-MC-run2-run3-01"))
        acc.merge(addOverride(flags, "/TRT/Calib/MC/T0", tag="TrtCalibT0-MC-run2-run3-01"))
    else:
        # SCT
        acc.merge(addOverride(flags, "/SCT/DAQ/Calibration/ChipNoise", "SctDaqCalibrationChipNoise-Apr10-01", db="COOLOFL_SCT/OFLP200"))
        acc.merge(addOverride(flags, "/SCT/DAQ/Calibration/ChipGain", "SctDaqCalibrationChipGain-Apr10-01", db="COOLOFL_SCT/OFLP200"))

        # TRT
        # TODO: TRTCondDigVers

        # LAr
        acc.merge(addOverride(flags, "/LAR/BadChannels/MissingFEBs", tag="LArBadChannelsMissingFEBs-IOVDEP-04", db="COOLOFL_LAR/OFLP200"))
        acc.merge(addOverride(flags, "/LAR/LArCellPositionShift", tag="LArCellPositionShift-ideal", db="COOLOFL_LAR/OFLP200"))
        acc.merge(addOverride(flags, "/LAR/ElecCalibOfl/OFC/PhysWave/RTM/4samples1phase", tag="LARElecCalibOflOFCPhysWaveRTM4samples1phase-RUN2-UPD4-00"))
        acc.merge(addOverride(flags, "/LAR/ElecCalibOfl/Shape/RTM/4samples1phase", tag="LARElecCalibOflShapeRTM4samples1phase-RUN2-UPD4-00"))
        acc.merge(addOverride(flags, "/LAR/ElecCalibMC/fSampl", tag="LARElecCalibMCfSampl-G496-19213-FTFP_BERT_BIRK"))

        # Tile
        # TODO: Tile sampling fraction

        # TGC
        # TODO: /TGC/DIGIT/* folders are available only in OFLP200 (not in CONDBR2).
        acc.merge(addOverride(flags, "/TGC/DIGIT/ASDPOS", tag="TgcDigitAsdPos-00-01", db="COOLOFL_TGC/OFLP200"))
        acc.merge(addOverride(flags, "/TGC/DIGIT/TOFFSET", tag="TgcDigitTimeOffset-00-01", db="COOLOFL_TGC/OFLP200"))
        acc.merge(addOverride(flags, "/TGC/DIGIT/XTALK", tag="TgcDigitXTalk-00-01", db="COOLOFL_TGC/OFLP200"))

    return acc


def DataOverlay2024Cfg(flags):
    """Conditions for 2024 data overlay."""
    # TODO: for now this is mostly a placeholder based on the p-p test configuration.
    return DataOverlay2023Cfg(flags)
