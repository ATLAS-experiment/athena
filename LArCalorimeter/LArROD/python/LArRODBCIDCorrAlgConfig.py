# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
from LArRecUtils.LArRecUtilsConfig import LArMCSymCondAlgCfg
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg


def LArRODBCIDCorrAlgCfg(flags, name="LArRODBCIDCorrAlg", **kwargs):
    from IOVDbSvc.IOVDbSvcConfig import addFolderList

    # MC only: the min-bias average is read from /LAR/ElecCalibMC/LArPileupAverage
    # and the symmetrised MC conditions keys are used throughout.
    if not flags.Input.isMC:
        raise RuntimeError(
            "LArRODBCIDCorrAlgCfg is only supported for MC "
            "(LAr.ROD.ApplyRODBCIDCorr must be False for data)"
        )

    acc = LArADC2MeVCondAlgCfg(flags)
    acc.merge(LArOnOffIdMappingCfg(flags))

    # The pulse shape (LArShapeSym) is the only calibration input besides
    # ADC2MeV and the min-bias average; no OFCs are needed since the correction
    # is applied to the ADC samples, before any filtering.
    acc.merge(LArElecCalibDBCfg(flags, ("Shape",)))

    acc.merge(
        addFolderList(
            flags,
            (("/LAR/ElecCalibMC/LArPileupAverage", "LAR_OFL", "LArMinBiasAverageMC"),),
        )
    )

    acc.merge(LArMCSymCondAlgCfg(flags))

    LArMinBiasAverageSymAlg = CompFactory.getComp(
        "LArSymConditionsAlg<LArMinBiasAverageMC, LArMinBiasAverageSym>"
    )
    acc.addCondAlgo(
        LArMinBiasAverageSymAlg(
            "LArPileUpAvgSymCondAlg",
            ReadKey="LArPileupAverage",
            WriteKey="LArPileupAverageSym",
        )
    )

    kwargs.setdefault("ShapeKey", "LArShapeSym")
    kwargs.setdefault("ADC2MeVKey", "LArADC2MeV")
    kwargs.setdefault("CablingKey", "LArOnOffIdMap")
    kwargs.setdefault("MinBiasAvgKey", "LArPileupAverageSym")
    kwargs.setdefault("EventInfo", "EventInfo")

    # MUST match LArHitEMapToDigitAlg.firstSample (LArDigitizationConfig.py):
    # it is the shape index seen by digit sample 0, hence negative when
    # preceding samples are read out. This is NOT the "skip the first N samples"
    # firstSample used by the raw channel builders.
    kwargs.setdefault(
        "firstSample",
        (
            -flags.LAr.ROD.nPreceedingSamples
            if flags.LAr.ROD.nPreceedingSamples != 0
            else flags.LAr.ROD.FirstSample
        ),
    )

    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault(
            "LArDigitKey", flags.Overlay.BkgPrefix + "LArDigitContainer_MC"
        )
    else:
        kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")

    # FIXME: MinBunchCrossing/MaxBunchCrossing are deliberately left at their
    # defaults, which impose no restriction and so reproduce CaloBCIDCoeffs.
    # The digitisation only overlays min-bias over a finite bunch crossing
    # window (-30..+4 for EM, see the PileUpXingFolder ranges in
    # LArDigitizationConfig.py), so with nPreceedingSamples = 24 the earliest
    # preceding samples are over-corrected by up to ~78% of the peak pile-up.
    # See the class documentation in LArRODBCIDCorrAlg.h; this should probably
    # be addressed, but it is a physics choice rather than a bug.

    kwargs.setdefault("OutputDigitKey", "LArDigitContainer_PileupCorrected")

    kwargs.setdefault("BeamIntensityPattern", flags.Digitization.PU.BeamIntensityPattern)

    acc.addEventAlgo(CompFactory.LArRODBCIDCorrAlg(name, **kwargs))

    return acc
