# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaCommon.Logging import logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
from LArRecUtils.LArRecUtilsConfig import LArOFCCondAlgCfg
from LArConfiguration.LArConfigFlags import RawChannelSource


def LArOFFCRawChannelBuilderCfg(flags, name="LArOFFCRawChannelBuilder", **kwargs):

    acc = LArADC2MeVCondAlgCfg(flags)

    # Index of the digit sample the OFC window starts at, same convention as
    # the other raw channel builders
    kwargs.setdefault(
        "firstSample",
        (
            flags.LAr.ROD.nPreceedingSamples
            if flags.LAr.ROD.nPreceedingSamples != 0
            else flags.LAr.ROD.FirstSample
        ),
    )
    obj = "AthenaAttributeList"
    dspkey = "Run2DSPThresholdsKey"
    from IOVDbSvc.IOVDbSvcConfig import addFolders

    kwargs.setdefault("ShapeKey", "LArShapeSym")

    if flags.Input.isMC:
        acc.merge(LArOFCCondAlgCfg(flags))
        kwargs.setdefault("LArRawChannelKey", "LArRawChannels")

        if flags.GeoModel.Run is LHCPeriod.Run1:  # back to flat threshold
            kwargs.setdefault("useDB", False)
            dspkey = ""
        else:
            fld = "/LAR/NoiseOfl/DSPThresholds"
            sgkey = fld
            dbString = "OFLP200"
            dbInstance = "LAR_OFL"
            acc.merge(
                addFolders(flags, fld, dbInstance, className=obj, db=dbString)
            )

        if flags.LAr.ROD.ApplyRODBCIDCorr:
            # Read the digits produced by LArRODBCIDCorrAlg instead of the raw ones
            mlog = logging.getLogger("LArOFFCRawChannelBuilderCfg")
            mlog.info("LAr.ROD.ApplyRODBCIDCorr is set: reading pile-up "
                      "corrected digits LArDigitContainer_PileupCorrected")
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_PileupCorrected")
        elif flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
            kwargs.setdefault(
                "LArDigitKey", flags.Overlay.BkgPrefix + "LArDigitContainer_MC"
            )
        else:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")

    else:
        acc.merge(LArElecCalibDBCfg(flags, ("OFC", "Shape", "Pedestal")))
        if flags.Overlay.DataOverlay:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels")
        else:
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels_FromDigits")

        if "COMP200" in flags.IOVDb.DatabaseInstance:
            fld = "/LAR/Configuration/DSPThreshold/Thresholds"
            obj = "LArDSPThresholdsComplete"
            dspkey = "Run1DSPThresholdsKey"
            sgkey = "LArDSPThresholds"
            dbString = "COMP200"
        else:
            fld = "/LAR/Configuration/DSPThresholdFlat/Thresholds"
            sgkey = fld
            dbString = "CONDBR2"
        dbInstance = "LAR_ONL"
        acc.merge(addFolders(flags, fld, dbInstance, className=obj, db=dbString))

    # Run 1 MC falls back to a flat threshold and sets no folder at all
    if len(dspkey) > 0:
        kwargs.setdefault(dspkey, sgkey)

    if (
        flags.LAr.ROD.forceIter
        or flags.LAr.RawChannelSource is RawChannelSource.Calculated
    ):
        # Iterative OFC procedure. There is no OFFC variant of it, so this
        # falls back to the standard iterative builder and none of the OFFC
        # properties below may be set here: it declares none of them.
        kwargs.setdefault("minSample", 2)
        kwargs.setdefault("maxSample", 12)
        kwargs.setdefault("minADCforIterInSigma", 4)
        kwargs.setdefault("minADCforIter", 15)
        kwargs.setdefault("defaultPhase", 12)
        nominalPeakSample = 2
        from LArConditionsCommon.LArRunFormat import getLArFormatForRun

        larformat = getLArFormatForRun(
            flags.Input.RunNumbers[0],
            connstring="COOLONL_LAR/" + flags.IOVDb.DatabaseInstance,
        )
        if larformat is not None:
            nominalPeakSample = larformat.firstSample()
        else:
            print("WARNING: larformat not found, use nominalPeakSample = 2")
            nominalPeakSample = 2
        if nominalPeakSample > 1:
            kwargs.setdefault("DefaultShiftTimeSample", nominalPeakSample - 2)
        else:
            kwargs.setdefault("DefaultShiftTimeSample", 0)

        acc.addEventAlgo(CompFactory.LArRawChannelBuilderIterAlg(**kwargs))
    else:
        # Default OFFC Configuration
        kwargs.setdefault("BelowThreshold", flags.LAr.ROD.OFFCBelowThreshold)
        kwargs.setdefault("BelowTillReset", flags.LAr.ROD.OFFCBelowTillReset)
        kwargs.setdefault("NPulse", flags.LAr.ROD.OFFCNPulse)
        kwargs.setdefault("Q3Cut", flags.LAr.ROD.OFFCQ3Cut)
        kwargs.setdefault("Q3Offset", flags.LAr.ROD.OFFCQ3Offset)
        kwargs.setdefault("FilterThreshold", flags.LAr.ROD.OFFCFilterThreshold)
        # Per-layer tuning. The scalars above stay the fallback for any layer
        # not named in these maps, so a job that clears them behaves exactly
        # as it did before.
        kwargs.setdefault("FilterThresholdByLayer",
                          flags.LAr.ROD.OFFCFilterThresholdByLayer)
        kwargs.setdefault("Q3CutByLayer", flags.LAr.ROD.OFFCQ3CutByLayer)
        kwargs.setdefault("Q3OffsetByLayer", flags.LAr.ROD.OFFCQ3OffsetByLayer)
        kwargs.setdefault("NPulseByLayer", flags.LAr.ROD.OFFCNPulseByLayer)
        kwargs.setdefault("EnabledLayers", flags.LAr.ROD.OFFCEnabledLayers)

        acc.addEventAlgo(CompFactory.LArOFFCRawChannelBuilder(name, **kwargs))

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    log.setLevel(DEBUG)

    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    # in case of testing iterative OFC:
    #flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data15_1beam/data15_1beam.00260466.physics_L1Calo.merge.RAW._lb1380._SFO-ALL._0001.1']
    flags.Input.isMC = False
    flags.Detector.GeometryTile = False
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg

    acc = MainServicesCfg(flags)
    acc.merge(LArRawDataReadingCfg(flags))
    acc.merge(LArOFFCRawChannelBuilderCfg(flags))

    DumpLArRawChannels = CompFactory.DumpLArRawChannels
    acc.addEventAlgo(
        DumpLArRawChannels(
            LArRawChannelContainerName="LArRawChannels_FromDigits",
        ),
        sequenceName="AthAlgSeq",
    )

    acc.run(3)
