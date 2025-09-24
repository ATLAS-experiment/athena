# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def NswOccupancyAlgCfg(flags, binWidth = 100):
    result = ComponentAccumulator()
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags,outFile="NswFairyTales.root", outStream="NSWSTORIES"))
    the_alg = CompFactory.NswOccupancyAlg("NswOccupancyAlgBin{width}".format(width = binWidth), BinWidth = binWidth)
    result.addEventAlgo(the_alg, primary = True)
    return result

def AddMetaAlgCfg(flags, alg_name="MuonTPMetaAlg", OutStream="NSWPRDValAlg", **kwargs):
    result = ComponentAccumulator()
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    from EventBookkeeperTools.EventBookkeeperToolsConfig import CutFlowSvcCfg
    result.merge(CutFlowSvcCfg(flags))
    result.merge(MetaDataSvcCfg(flags))
    if len(OutStream):
        kwargs.setdefault("OutStream", OutStream)
        alg_name += "_" + OutStream
    kwargs.setdefault("isData", not flags.Input.isMC)    
    kwargs.setdefault("ExtraOutputs", [('xAOD::EventInfo', 'StoreGateSvc+EventInfo.MetaData' + OutStream)])
    the_alg = CompFactory.MuonVal.MetaDataAlg(alg_name, **kwargs)
    result.addEventAlgo(the_alg, primary=True)  # top sequence
    return result


def AddHitValAlgCfg(flags, name = "HitValAlg", outFile="NSWPRDValAlg.ntuple.root", **kwargs):
    result = ComponentAccumulator()
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    result.merge(setupHistSvcCfg(flags, outFile=outFile, outStream="MUONHITVALIDSTREAM"))

    kwargs.setdefault("doMMHit", flags.Detector.EnableMM) 
    kwargs.setdefault("doMMDigit", flags.Detector.EnableMM)
    kwargs.setdefault("doMMRDO", flags.Detector.EnableMM)
    kwargs.setdefault("doMMPRD", flags.Detector.EnableMM)
    kwargs.setdefault("doMMSDO", flags.Detector.EnableMM) 


    kwargs.setdefault("doSTGCHit", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCDigit", flags.Detector.EnablesTGC) 
    kwargs.setdefault("doSTGCRDO", flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCPRD",flags.Detector.EnablesTGC)
    kwargs.setdefault("doSTGCSDO", flags.Detector.EnablesTGC) 

    kwargs.setdefault("doRPCHit", flags.Detector.EnableRPC)
    kwargs.setdefault("doRPCSDO", flags.Detector.EnableRPC)
    kwargs.setdefault("doRPCDigit", flags.Detector.EnableRPC)
    kwargs.setdefault("doRPCPRD", flags.Detector.EnableRPC)

    kwargs.setdefault("doMDTHit", flags.Detector.EnableMDT)
    kwargs.setdefault("doMDTSDO", flags.Detector.EnableMDT)
    kwargs.setdefault("doMDTDigit", flags.Detector.EnableMDT) 
    kwargs.setdefault("doMDTPRD", flags.Detector.EnableMDT) 

    kwargs.setdefault("doTGCHit", flags.Detector.EnableTGC)
    kwargs.setdefault("doTGCSDO", flags.Detector.EnableTGC)
    kwargs.setdefault("doTGCDigit", flags.Detector.EnableTGC)
    kwargs.setdefault("doTGCRDO", flags.Detector.EnableTGC)
    kwargs.setdefault("doTGCPRD", flags.Detector.EnableTGC)


    #Turn off by default but keep the option to turn on for validation of the NSW 

    if not  flags.Detector.EnableCSC:
        kwargs.setdefault("CscRDODecoder","")
    kwargs.setdefault("doCSCHit", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCSDO", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCDigit", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCRDO", flags.Detector.EnableCSC)
    kwargs.setdefault("doCSCPRD", flags.Detector.EnableCSC)
    from AthenaConfiguration.Enums import ProductionStep

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault("EvtInfo", f"{flags.Overlay.BkgPrefix}EventInfo")
        kwargs.setdefault("MdtDigitKey", f"{flags.Overlay.BkgPrefix}MDT_DIGITS")
        kwargs.setdefault("RpcDigitKey", f"{flags.Overlay.BkgPrefix}RPC_DIGITS")
        kwargs.setdefault("TgcDigitKey", f"{flags.Overlay.BkgPrefix}TGC_DIGITS")
        kwargs.setdefault("MmDigitKey", f"{flags.Overlay.BkgPrefix}MM_DIGITS")
        kwargs.setdefault("sTgcDigitKey", f"{flags.Overlay.BkgPrefix}sTGC_DIGITS")
        
        kwargs.setdefault("MdtSdoKey", f"{flags.Overlay.BkgPrefix}MDT_SDO")
        kwargs.setdefault("RpcSdoKey", f"{flags.Overlay.BkgPrefix}RPC_SDO")
        kwargs.setdefault("TgcSdoKey", f"{flags.Overlay.BkgPrefix}TGC_SDO")
        kwargs.setdefault("MmSdoKey", f"{flags.Overlay.BkgPrefix}MM_SDO")
        kwargs.setdefault("sTgcSdoKey", f"{flags.Overlay.BkgPrefix}sTGC_SDO")

    the_alg = CompFactory.MuonVal.HitValAlg(name, **kwargs)
    result.addEventAlgo(the_alg)

    return result