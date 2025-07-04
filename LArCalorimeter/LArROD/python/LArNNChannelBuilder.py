# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep
from LArRecUtils.LArADC2MeVCondAlgConfig import LArADC2MeVCondAlgCfg
from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBCfg
from LArRecUtils.LArRecUtilsConfig import LArOFCCondAlgCfg
from LArConfiguration.LArConfigFlags import RawChannelSource
from IOVDbSvc.IOVDbSvcConfig import addFolders

def LArNNRawChannelBuilderCfg(flags, name="LArNNRawChannelBuilder", **kwargs):
    acc = LArADC2MeVCondAlgCfg(flags)

    acc.merge(addFolders(flags,"/LAR/IdentifierOfl/OnnxMap", "LAR_OFL", className="CondAttrListCollection", db="OFLP200", tag="LARIdentifierOflOnnxMap-RUN4-000"))

    # the NN always requires 1 sample in the past
    kwargs.setdefault("firstSample", flags.LAr.ROD.nPreceedingSamples if flags.LAr.ROD.nPreceedingSamples!=0 else flags.LAr.ROD.FirstSample)
    obj = "AthenaAttributeList" 
    dspkey = 'Run2DSPThresholdsKey'

    if flags.Input.isMC:
        acc.merge(LArOFCCondAlgCfg(flags))
        kwargs.setdefault("LArRawChannelKey", "LArRawChannels")

        if flags.GeoModel.Run is LHCPeriod.Run1:  # back to flat threshold
           kwargs.setdefault("useDB", False)
           dspkey = ''
        else:
           fld="/LAR/NoiseOfl/DSPThresholds"
           sgkey=fld
           dbString="OFLP200"
           dbInstance="LAR_OFL"
           acc.merge(addFolders(flags,fld, dbInstance, className=obj, db=dbString))

        if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
            kwargs.setdefault("LArDigitKey", flags.Overlay.BkgPrefix + "LArDigitContainer_MC")
        else:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")
    else:
        acc.merge(LArElecCalibDBCfg(flags,("OFC","Shape","Pedestal")))
        if flags.Overlay.DataOverlay:
            kwargs.setdefault("LArDigitKey", "LArDigitContainer_MC")
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels")
        else:
            kwargs.setdefault("LArRawChannelKey", "LArRawChannels_FromDigits")
        
        if 'COMP200' in flags.IOVDb.DatabaseInstance:
            fld='/LAR/Configuration/DSPThreshold/Thresholds'
            obj='LArDSPThresholdsComplete'
            dspkey = 'Run1DSPThresholdsKey'
            sgkey='LArDSPThresholds'
            dbString = 'COMP200'
        else:
            fld="/LAR/Configuration/DSPThresholdFlat/Thresholds"
            sgkey=fld
            dbString="CONDBR2"
        dbInstance="LAR_ONL"
        acc.merge(addFolders(flags,fld, dbInstance, className=obj, db=dbString))

    kwargs.setdefault(dspkey, sgkey)

    if flags.LAr.ROD.forceIter or flags.LAr.RawChannelSource is RawChannelSource.Calculated:
        # iterative OFC procedure
        LArRawChannelBuilderIterAlg=CompFactory.LArRawChannelBuilderIterAlg
        kwargs.setdefault('minSample',2)
        kwargs.setdefault('maxSample',12)
        kwargs.setdefault('minADCforIterInSigma',4)
        kwargs.setdefault('minADCforIter',15)
        kwargs.setdefault('defaultPhase',12)
        nominalPeakSample=2
        from LArConditionsCommon.LArRunFormat import getLArFormatForRun
        larformat=getLArFormatForRun(flags.Input.RunNumbers[0],connstring="COOLONL_LAR/"+flags.IOVDb.DatabaseInstance)
        if larformat is not None:
          nominalPeakSample = larformat.firstSample()
        else:
          print("WARNING: larformat not found, use nominalPeakSample = 2")
          nominalPeakSample = 2
        if (nominalPeakSample > 1) :
          kwargs.setdefault('DefaultShiftTimeSample',nominalPeakSample-2)
        else :
          kwargs.setdefault('DefaultShiftTimeSample',0)

        acc.addEventAlgo(LArRawChannelBuilderIterAlg(**kwargs))
    else:
       
        acc.addEventAlgo(CompFactory.LArNNRawChannelBuilder(name, **kwargs))

    return acc
