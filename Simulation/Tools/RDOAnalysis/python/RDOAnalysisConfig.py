# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def RDOAnalysisOutputCfg(flags, output_name="RDOAnalysis"):
    from MuonConfig.MuonConfigUtils import setupHistSvcCfg
    return setupHistSvcCfg(flags, outFile =flags.Output.HISTFileName, outStream =  output_name)


def EventInfoRDOAnalysisCfg(flags, name="EventInfoRDOAnalysis", **kwargs):
    result = ComponentAccumulator()

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/histos/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("EventInfo", f"{flags.Overlay.BkgPrefix}EventInfo")

    result.addEventAlgo(CompFactory.EventInfoRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def BCM_RDOAnalysisCfg(flags, name="BCM_RDOAnalysis", **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    result = PixelReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "BCM")
    kwargs.setdefault("HistPath", "/RDOAnalysis/BCM/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}BCM_RDOs")
    kwargs.setdefault("InputTruthKey", f"{prefix}BCM_SDO_Map")

    result.addEventAlgo(CompFactory.BCM_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def PixelRDOAnalysisCfg(flags, name="PixelRDOAnalysis", **kwargs):
    from PixelGeoModel.PixelGeoModelConfig import PixelReadoutGeometryCfg
    result = PixelReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "Pixel")
    kwargs.setdefault("HistPath", "/RDOAnalysis/Pixel/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("InputKey", f"{flags.Overlay.BkgPrefix}PixelRDOs")
        kwargs.setdefault("InputTruthKey", f"{flags.Overlay.BkgPrefix}PixelSDO_Map")
        kwargs.setdefault("InputMcEventCollectionKey", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("InputKey", "PixelRDOs")
        kwargs.setdefault("InputTruthKey", "PixelSDO_Map")
        kwargs.setdefault("InputMcEventCollectionKey", "TruthEvent")

    result.addEventAlgo(CompFactory.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def SCT_RDOAnalysisCfg(flags, name="SCT_RDOAnalysis", **kwargs):
    from SCT_GeoModel.SCT_GeoModelConfig import SCT_ReadoutGeometryCfg
    result = SCT_ReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "SCT")
    kwargs.setdefault("HistPath", "/RDOAnalysis/SCT/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("InputKey", f"{flags.Overlay.BkgPrefix}SCT_RDOs")
        kwargs.setdefault("InputTruthKey", f"{flags.Overlay.BkgPrefix}SCT_SDO_Map")
        kwargs.setdefault("InputMcEventCollectionKey", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("InputKey", "SCT_RDOs")
        kwargs.setdefault("InputTruthKey", "SCT_SDO_Map")
        kwargs.setdefault("InputMcEventCollectionKey", "TruthEvent")

    result.addEventAlgo(CompFactory.SCT_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def TRT_RDOAnalysisCfg(flags, name="TRT_RDOAnalysis", **kwargs):
    from TRT_GeoModel.TRT_GeoModelConfig import TRT_ReadoutGeometryCfg
    result = TRT_ReadoutGeometryCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "TRT")
    kwargs.setdefault("HistPath", "/RDOAnalysis/TRT/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}TRT_RDOs")
    kwargs.setdefault("InputTruthKey", f"{prefix}TRT_SDO_Map")

    result.addEventAlgo(CompFactory.TRT_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def ITkPixelRDOAnalysisCfg(flags, name="ITkPixelRDOAnalysis", **kwargs):
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    result = ITkPixelReadoutGeometryCfg(flags)

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/ITkPixel/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("CollectionName", f"{flags.Overlay.BkgPrefix}ITkPixelRDOs")
        kwargs.setdefault("SDOCollectionName", f"{flags.Overlay.BkgPrefix}ITkPixelSDO_Map")
        kwargs.setdefault("McEventCollectionName", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("CollectionName", "ITkPixelRDOs")
        kwargs.setdefault("SDOCollectionName", "ITkPixelSDO_Map")
        kwargs.setdefault("McEventCollectionName", "TruthEvent")

    result.addEventAlgo(CompFactory.ITk.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def ITkStripRDOAnalysisCfg(flags, name="ITkStripRDOAnalysis", **kwargs):
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    result = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/ITkStrip/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("CollectionName", f"{flags.Overlay.BkgPrefix}ITkStripRDOs")
        kwargs.setdefault("SDOCollectionName", f"{flags.Overlay.BkgPrefix}ITkStripSDO_Map")
        kwargs.setdefault("McEventCollectionName", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("CollectionName", "ITkStripRDOs")
        kwargs.setdefault("SDOCollectionName", "ITkStripSDO_Map")
        kwargs.setdefault("McEventCollectionName", "TruthEvent")

    result.addEventAlgo(CompFactory.ITk.StripRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def HGTD_RDOAnalysisCfg(flags, name="HGTD_RDOAnalysis", **kwargs):
    from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    result = HGTD_ReadoutGeometryCfg(flags)

    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("HistPath", "/RDOAnalysis/HGTD/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("CollectionName", f"{flags.Overlay.BkgPrefix}HGTD_RDOs")
        kwargs.setdefault("SDOCollectionName", f"{flags.Overlay.BkgPrefix}HGTD_SDO_Map")
        kwargs.setdefault("McEventCollectionName", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("CollectionName", "HGTD_RDOs")
        kwargs.setdefault("SDOCollectionName", "HGTD_SDO_Map")
        kwargs.setdefault("McEventCollectionName", "TruthEvent")

    result.addEventAlgo(CompFactory.HGTD_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def PLR_RDOAnalysisCfg(flags, name="PLR_RDOAnalysis", **kwargs):
    from PLRGeoModelXml.PLR_GeoModelConfig import PLR_ReadoutGeometryCfg
    result = PLR_ReadoutGeometryCfg(flags)

    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("CollectionName", f"{flags.Overlay.BkgPrefix}PLR_RDOs")
        kwargs.setdefault("SDOCollectionName", f"{flags.Overlay.BkgPrefix}PLR_SDO_Map")
        kwargs.setdefault("McEventCollectionName", f"{flags.Overlay.BkgPrefix}TruthEvent")
    else:
        kwargs.setdefault("CollectionName", "PLR_RDOs")
        kwargs.setdefault("SDOCollectionName", "PLR_SDO_Map")
        kwargs.setdefault("McEventCollectionName", "TruthEvent")
    kwargs.setdefault("HistPath", "/RDOAnalysis/PLR/")
    kwargs.setdefault("SharedHistPath", "/RDOAnalysis/histos/")
    kwargs.setdefault("NtuplePath", "/RDOAnalysis/ntuples/")
    kwargs.setdefault("NtupleName", "PLR")
    kwargs.setdefault("DetectorName", "PLR")
    kwargs.setdefault("PixelIDName", "PLR_ID")

    result.addEventAlgo(CompFactory.ITk.PixelRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def RDOAnalysisCfg(flags):
    acc = ComponentAccumulator()

    acc.merge(EventInfoRDOAnalysisCfg(flags))

    if flags.Detector.EnablePixel:
        acc.merge(PixelRDOAnalysisCfg(flags))

    if flags.Detector.EnableSCT:
        acc.merge(SCT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableTRT:
        acc.merge(TRT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableLAr:
        acc.merge(LArRDOAnalysisCfg(flags))

    if flags.Detector.EnableTile:
        acc.merge(TileRDOAnalysisCfg(flags))

    if flags.Detector.EnableMDT:
        from MuonConfig.MuonByteStreamCnvTestConfig import MdtRdoToMdtDigitCfg

        if  "MDTCSM" in flags.Input.Collections:
            acc.merge(MdtRdoToMdtDigitCfg(flags))
        elif f"{flags.Overlay.BkgPrefix}MDTCSM" in flags.Input.Collections:
            acc.merge(MdtRdoToMdtDigitCfg(flags, MdtRdoContainer =f"{flags.Overlay.BkgPrefix}MDTCSM",
                                                 MdtDigitContainer=f"{flags.Overlay.BkgPrefix}MDT_DIGITS" ))

        acc.merge(MDT_RDOAnalysisCfg(flags))

    if flags.Detector.EnableRPC:
        if "RPCPAD" in flags.Input.Collections or f"{flags.Overlay.BkgPrefix}RPCPAD" in flags.Input.Collections:
            from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg
            acc.merge(RpcRdoToRpcDigitCfg(flags))
        acc.merge(RPC_RDOAnalysisCfg(flags))

    if flags.Detector.EnableTGC:
        from MuonConfig.MuonByteStreamCnvTestConfig import TgcRdoToTgcDigitCfg
        if "TGCRDO" in flags.Input.Collections:
            acc.merge(TgcRdoToTgcDigitCfg(flags))
        elif f"{flags.Overlay.BkgPrefix}TGCRDO" in flags.Input.Collections:
            acc.merge(TgcRdoToTgcDigitCfg(flags,TgcRdoContainer = f"{flags.Overlay.BkgPrefix}TGCRDO",
                                                TgcDigitContainer=f"{flags.Overlay.BkgPrefix}TGC_DIGITS"))
        acc.merge(TGC_RDOAnalysisCfg(flags))

    if flags.Detector.EnablesTGC:
        from MuonConfig.MuonByteStreamCnvTestConfig import STGC_RdoToDigitCfg
        if "sTGCRDO" in flags.Input.Collections:
            acc.merge(STGC_RdoToDigitCfg(flags))
        elif f"{flags.Overlay.BkgPrefix}sTGCRDO" in flags.Input.Collections:
            acc.merge(STGC_RdoToDigitCfg(flags,
                                          sTgcRdoContainer=f"{flags.Overlay.BkgPrefix}sTGCRDO",
                                          sTgcDigitContainer=f"{flags.Overlay.BkgPrefix}sTGC_DIGITS"))

            
    if flags.Detector.EnableMM:
        from MuonConfig.MuonByteStreamCnvTestConfig import MM_RdoToDigitCfg
        if "MMRDO" in flags.Input.Collections:
            acc.merge(MM_RdoToDigitCfg(flags))
        elif f"{flags.Overlay.BkgPrefix}MMRDO" in flags.Input.Collections:
            acc.merge(MM_RdoToDigitCfg(flags,MmRdoContainer=f"{flags.Overlay.BkgPrefix}MMRDO",
                                             MmDigitContainer=f"{flags.Overlay.BkgPrefix}MM_DIGITS"))
            

    if flags.Detector.EnableMuon:
        from MuonPRDTest.MuonPRDTestCfg import AddHitValAlgCfg
        acc.merge(AddHitValAlgCfg(flags, name = "MuonHitValAlg", outFile=flags.Output.HISTFileName, doSDOs = True, doDigits=True))

    if flags.Detector.EnableITkPixel:
        acc.merge(ITkPixelRDOAnalysisCfg(flags))

    if flags.Detector.EnableITkStrip:
        acc.merge(ITkStripRDOAnalysisCfg(flags))

    if flags.Detector.EnableHGTD:
        acc.merge(HGTD_RDOAnalysisCfg(flags))

    if flags.Detector.EnablePLR:
        acc.merge(PLR_RDOAnalysisCfg(flags))

    return acc


def LArRDOAnalysisCfg(flags, name="LArRDOAnalysis", **kwargs):
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result = LArGMCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "LAr")
    kwargs.setdefault("HistPath", "/RDOAnalysis/LAr/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("PreSampling", True)
        prefix=flags.Overlay.BkgPrefix
        kwargs.setdefault("InputRawChannelKey", '') # Not in presampled RDO files
        kwargs.setdefault("InputTTL1HADKey", '') # Not in presampled RDO files
        kwargs.setdefault("InputTTL1EMKey", '') # Not in presampled RDO files
    kwargs.setdefault("InputRawChannelKey", "LArRawChannels")
    kwargs.setdefault("InputTTL1HADKey", "LArTTL1HAD")
    kwargs.setdefault("InputTTL1EMKey", "LArTTL1EM")
    if flags.Digitization.AddCaloDigiThinned or \
       f"{prefix}LArDigitContainer_MC_Thinned" in flags.Input.Collections or \
       flags.Common.ProductionStep is ProductionStep.PileUpPresampling or \
       flags.Common.ProductionStep is ProductionStep.Digitization: \
        
        kwargs.setdefault("InputDigitKey", f"{prefix}LArDigitContainer_MC_Thinned")
    else:
        kwargs.setdefault("InputDigitKey", "")
 
    result.addEventAlgo(CompFactory.LArRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def TileRDOAnalysisCfg(flags, name="TileRDOAnalysis", **kwargs):
    from TileGeoModel.TileGMConfig import TileGMCfg
    result = TileGMCfg(flags)
    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge(TileCablingSvcCfg(flags))

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "Tile")
    kwargs.setdefault("HistPath", "/RDOAnalysis/Tile/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        kwargs.setdefault("PreSampling", True)
        prefix=flags.Overlay.BkgPrefix
        kwargs.setdefault("InputRawChKey", '') # Not in presampled RDO files
        kwargs.setdefault("InputMuRcvRawChKey", '') # Not in presampled RDO files
        kwargs.setdefault("InputMuRcvKey", '') # Not in presampled RDO files
        kwargs.setdefault("InputMBTS_TTL1Key", '') # Not in presampled RDO files
        kwargs.setdefault("InputTileTTL1Key", '') # Not in presampled RDO files
        kwargs.setdefault("InputL2Key", '') # Not in presampled RDO files
    kwargs.setdefault("InputRawChKey", 'TileRawChannelCnt')
    kwargs.setdefault("InputMuRcvRawChKey", 'MuRcvRawChCnt')
    kwargs.setdefault("InputMuRcvKey", 'TileMuRcvCnt')
    if flags.Detector.EnableMBTS:
        kwargs.setdefault("InputMBTS_TTL1Key", 'TileTTL1MBTS')
    else:
        kwargs.setdefault("InputMBTS_TTL1Key", "")
    kwargs.setdefault("InputTileTTL1Key", 'TileTTL1Cnt')
    kwargs.setdefault("InputL2Key", 'TileL2Cnt')
    kwargs.setdefault("InputDigitsMuRcvKey", f'{prefix}MuRcvDigitsCnt')
    if f"{prefix}TileDigitsCnt" in flags.Input.Collections:
        kwargs.setdefault("InputDigitsFltKey", f"{prefix}TileDigitsCnt")
    else:
        kwargs.setdefault("InputDigitsFltKey", "TileDigitsFlt")

    result.addEventAlgo(CompFactory.TileRDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def CSC_RDOAnalysisCfg(flags, name="CSC_RDOAnalysis", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = MuonGeoModelCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "CSC")
    kwargs.setdefault("HistPath", "/RDOAnalysis/CSC/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}CSCRDO")
    kwargs.setdefault("InputTruthKey", f"{prefix}CSC_SDO")

    result.addEventAlgo(CompFactory.CSC_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def MDT_RDOAnalysisCfg(flags, name="MDT_RDOAnalysis", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = MuonGeoModelCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "MDT")
    kwargs.setdefault("HistPath", "/RDOAnalysis/MDT/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}MDTCSM")
    kwargs.setdefault("InputTruthKey", f"{prefix}MDT_SDO")

    result.addEventAlgo(CompFactory.MDT_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def RPC_RDOAnalysisCfg(flags, name="RPC_RDOAnalysis", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = MuonGeoModelCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "RPC")
    kwargs.setdefault("HistPath", "/RDOAnalysis/RPC/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}RPCPAD")
    kwargs.setdefault("InputTruthKey", f"{prefix}RPC_SDO")

    result.addEventAlgo(CompFactory.RPC_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def TGC_RDOAnalysisCfg(flags, name="TGC_RDOAnalysis", **kwargs):
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result = MuonGeoModelCfg(flags)

    kwargs.setdefault("NtupleFileName", "/RDOAnalysis")
    kwargs.setdefault("NtupleDirectoryName", "/ntuples/")
    kwargs.setdefault("NtupleTreeName", "TGC")
    kwargs.setdefault("HistPath", "/RDOAnalysis/TGC/")
    prefix=''
    if flags.Common.ProductionStep is ProductionStep.PileUpPresampling:
        prefix=flags.Overlay.BkgPrefix
    kwargs.setdefault("InputKey", f"{prefix}TGCRDO")
    kwargs.setdefault("InputTruthKey", f"{prefix}TGC_SDO")

    result.addEventAlgo(CompFactory.TGC_RDOAnalysis(name, **kwargs))

    result.merge(RDOAnalysisOutputCfg(flags))

    return result


def SetupArgParser():
    from argparse import ArgumentParser
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    parser = ArgumentParser()
    parser.add_argument("--threads", type=int, help="number of threads", default=1)
    parser.add_argument("--inputFile", "-i", default= defaultTestFiles.RDO_RUN3, 
                        help="Input file to run on ", nargs="+")
    parser.add_argument("--outFile", default="RDOAnalysis.root", help="Output ROOT file to dump the geomerty")
    parser.add_argument("--nEvents", help="Number of events to run", type = int ,default = 1)
    parser.add_argument("--skipEvents", help="Number of events to skip", type = int, default = 0)
    parser.add_argument("--geoModelFile", default ="", help="GeoModel SqLite file containing the muon geometry.")
 
    return parser

if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    args = SetupArgParser().parse_args()
    flags = initConfigFlags()
    flags.Concurrency.NumThreads = args.threads
    flags.Concurrency.NumConcurrentEvents = args.threads  # Might change this later, but good enough for the moment.
    flags.Input.Files = args.inputFile 
    flags.Scheduler.ShowDataDeps = True 
    flags.Scheduler.ShowDataFlow = True
    flags.Exec.FPE= 500
    flags.Exec.MaxEvents = args.nEvents
    flags.Exec.SkipEvents = args.skipEvents
    flags.Output.HISTFileName = args.outFile
    if len (args.geoModelFile) > 0:
        flags.GeoModel.SQLiteDB = True
        flags.GeoModel.SQLiteDBFullPath = args.geoModelFile
    from MuonConfig.MuonConfigUtils import configureCondTag
    configureCondTag(flags)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    ### Setup the file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    cfg.merge(RDOAnalysisCfg(flags))

    cfg.printConfig(withDetails=True, summariseProps=True)
    if not cfg.run().isSuccess(): exit(1)

