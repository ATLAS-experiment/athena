# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format
from IOVDbSvc.IOVDbSvcConfig import addFolders

def TMDBConfig(flags):
    acc = ComponentAccumulator()

    # Read MuRcvRawChCnt from the input file (for POOL directly, for BS via converter)
    if flags.Input.Format is Format.POOL:
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, ["TileRawChannelContainer/MuRcvRawChCnt"]))
    else:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        acc.merge(ByteStreamReadCfg(flags, ["TileRawChannelContainer/MuRcvRawChCnt"]))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    tmdbAlg = CompFactory.TileMuonReceiverDecision('TileMuonReceiverDecision'
                                                   , TileRawChannelContainer = "MuRcvRawChCnt" # input
                                                   , TileMuonReceiverContainer = "rerunTileMuRcvCnt" # output
                                                   , ManualRunPeriod = 2 # forcing Run 2 format (=2) for now, until TGC implements Run 3 format (=3)
                                                   # run 2 thresholds
                                                   , MuonReceiverEneThreshCellD6Low = 500
                                                   , MuonReceiverEneThreshCellD6andD5Low = 500
                                                   , MuonReceiverEneThreshCellD6High = 600
                                                   , MuonReceiverEneThreshCellD6andD5High = 600
                                                   # run 3 thresholds
                                                   , MuonReceiverEneThreshCellD5 = 500
                                                   , MuonReceiverEneThreshCellD6 = 500
                                                   , MuonReceiverEneThreshCellD5andD6 = 500)
    acc.addEventAlgo(tmdbAlg)
    return acc

def MuonBytestream2RdoConfig(flags):
    acc = ComponentAccumulator()
    if flags.Input.isMC:
        return acc

    postFix = "_L1MuonSim"
    from MuonConfig.MuonBytestreamDecodeConfig import MuonCacheNames
    cacheCreator = CompFactory.MuonCacheCreator(RpcCacheKey = MuonCacheNames.RpcCache,
                                                TgcCacheKey = MuonCacheNames.TgcCache,
                                                MdtCsmCacheKey = MuonCacheNames.MdtCsmCache,
                                                CscCacheKey = (MuonCacheNames.CscCache if flags.Detector.GeometryCSC else ""))
    acc.addEventAlgo(cacheCreator)
    # for RPC
    RPCRodDecoder = CompFactory.Muon.RpcROD_Decoder(name = "RpcROD_Decoder" + postFix, NOBXS=flags.Trigger.L1MuonSim.RPCNBX)
    MuonRpcRawDataProviderTool = CompFactory.Muon.RPC_RawDataProviderToolMT(name = "RPC_RawDataProviderToolMT" + postFix,
                                                                             RpcContainerCacheKey = MuonCacheNames.RpcCache,
                                                                             WriteOutRpcSectorLogic = False,
                                                                             Decoder = RPCRodDecoder,
                                                                             RdoLocation = "RPCPAD_L1" )
    RpcRawDataProvider = CompFactory.Muon.RpcRawDataProvider(name = "RpcRawDataProvider" + postFix,
                                                              ProviderTool = MuonRpcRawDataProviderTool)
    acc.addEventAlgo(RpcRawDataProvider)
    # for TGC
    TGCRodDecoder = CompFactory.Muon.TGC_RodDecoderReadout(name = "TGC_RodDecoderReadout" + postFix)
    MuonTgcRawDataProviderTool = CompFactory.Muon.TGC_RawDataProviderToolMT(name = "TGC_RawDataProviderToolMT" + postFix,
                                                                             TgcContainerCacheKey = MuonCacheNames.TgcCache,
                                                                             Decoder = TGCRodDecoder,
                                                                             RdoLocation = "TGCRDO_L1")
    TgcRawDataProvider = CompFactory.Muon.TgcRawDataProvider(name = "TgcRawDataProvider" + postFix,
                                                              ProviderTool = MuonTgcRawDataProviderTool)
    acc.addEventAlgo(TgcRawDataProvider)
    # for sTGC
    if flags.Detector.GeometrysTGC:
        Muon__STGC_RawDataProviderToolMT=CompFactory.Muon.STGC_RawDataProviderToolMT
        from MuonConfig.MuonBytestreamDecodeConfig import sTgcRODDecoderCfg
        MuonsTgcRawDataProviderTool = Muon__STGC_RawDataProviderToolMT(name    = "STGC_RawDataProviderToolMT"+postFix,
                                                                       Decoder = acc.popToolsAndMerge(sTgcRODDecoderCfg(flags,
                                                                                                     name = "sTgcROD_Decoder"+postFix)),
                                                                       RdoLocation = "sTGCRDO_L1")
        Muon__sTgcRawDataProvider=CompFactory.Muon.sTgcRawDataProvider
        sTgcRawDataProvider = Muon__sTgcRawDataProvider(name       = "sTgcRawDataProvider"+postFix,
                                                        ProviderTool = MuonsTgcRawDataProviderTool )
        acc.addEventAlgo(sTgcRawDataProvider)

    # for MM
    if flags.Detector.GeometryMM:
        from MuonConfig.MuonBytestreamDecodeConfig import MmRDODDecoderCfg
        Muon_MM_RawDataProviderToolMT = CompFactory.Muon.MM_RawDataProviderToolMT
        MuonMmRawDataProviderTool = Muon_MM_RawDataProviderToolMT(name  = "MM_RawDataProviderToolMT"+postFix,
                                                                  Decoder = acc.popToolsAndMerge(MmRDODDecoderCfg(flags,
                                                                                                 name="MM_RODDecoder"+postFix)),
                                                                  RdoLocation = "MMRDO_L1")
        Muon__MmRawDataProvider = CompFactory.Muon.MM_RawDataProvider
        MmRawDataProvider = Muon__MmRawDataProvider(name = "MmRawDataProvider"+postFix, ProviderTool = MuonMmRawDataProviderTool )
        acc.addEventAlgo(MmRawDataProvider)

    return acc


def MuonRdoToMuonDigitToolCfg(flags, name="MuonRdoToMuonDigitTool", **kwargs ):
    result = ComponentAccumulator()
    kwargs.setdefault("DecodeSTGC_RDO", flags.Detector.GeometrysTGC)
    kwargs.setdefault("DecodeMM_RDO", flags.Detector.GeometryMM)
    kwargs.setdefault("DecodeNrpcRDO", flags.Muon.enableNRPC)
    from MuonConfig.MuonByteStreamCnvTestConfig import STgcRdoDecoderCfg, MMRdoDecoderCfg, MdtRdoDecoderCfg
    kwargs.setdefault( "stgcRdoDecoderTool", result.popToolsAndMerge(STgcRdoDecoderCfg(flags))
                         if flags.Detector.GeometrysTGC else "" )
    kwargs.setdefault("mmRdoDecoderTool", result.popToolsAndMerge(MMRdoDecoderCfg(flags))
                         if flags.Detector.GeometryMM else "" )
    kwargs.setdefault("mdtRdoDecoderTool", result.popToolsAndMerge(MdtRdoDecoderCfg(flags)))
    #Set N BCs and central BC consistently with RPC readout settings
    rpcrdo_decode = CompFactory.Muon.RpcRDO_Decoder("RpcRDO_Decoder", BCZERO=flags.Trigger.L1MuonSim.RPCNBCZ)
    kwargs.setdefault("rpcRdoDecoderTool", rpcrdo_decode)
    
    the_tool = CompFactory.MuonRdoToMuonDigitTool (name, **kwargs)
    result.setPrivateTools(the_tool)
    return result

def MuonRdo2DigitConfig(flags):
    acc = ComponentAccumulator()

    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    acc.merge( AtlasFieldCacheCondAlgCfg(flags) )
    # Read RPCPAD and TGCRDO from the input POOL file (for BS it comes from [Rpc|Tgc]RawDataProvider)
    suffix = "" if flags.Input.Format is Format.POOL else "_L1"
    RPCRdoName = "RPCPAD"+suffix
    TGCRdoName = "TGCRDO"+suffix
    MMRdoName = "MMRDO"+suffix
    sTGCRdoName = "sTGCRDO"+suffix
    
    if flags.Input.Format is Format.POOL:
        rdoInputs = [
            ('RpcPadContainer','RPCPAD'),
            ('TgcRdoContainer','TGCRDO')
        ]
        # Read MMRDO and sTGCRDO
        if flags.Detector.GeometrysTGC or flags.Detector.GeometryMM:
            rdoInputs += [
                ('Muon::MM_RawDataContainer','MMRDO'),
                ('Muon::STGC_RawDataContainer','sTGCRDO')
            ]
        if flags.Muon.enableNRPC:
            rdoInputs += [
                ('xAOD::NRPCRDOContainer' , 'NRPCRDO'),
                ('xAOD::NRPCRDOAuxContainer',  'NRPCRDOAux.')
            ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))

    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    acc.merge(MuonGeoModelCfg(flags))
    
    from MuonConfig.MuonByteStreamCnvTestConfig import RpcRdoToRpcDigitCfg, TgcRdoToTgcDigitCfg, STGC_RdoToDigitCfg, MM_RdoToDigitCfg

    acc.merge(RpcRdoToRpcDigitCfg(flags, RpcDigitContainer = "RPC_DIGITS_L1", RpcRdoContainer = RPCRdoName ))
    acc.merge(TgcRdoToTgcDigitCfg(flags, TgcDigitContainer = "TGC_DIGITS_L1", TgcRdoContainer = TGCRdoName ))
    if flags.Detector.GeometrysTGC:
        acc.merge(STGC_RdoToDigitCfg(flags, sTgcRdoContainer = sTGCRdoName, sTgcDigitContainer = "sTGC_DIGITS_L1"))
    if flags.Detector.GeometryMM:
          acc.merge(MM_RdoToDigitCfg(flags, MmRdoContainer = MMRdoName,  MmDigitContainer = "MM_DIGITS_L1" ))
    
    return acc

def NSWTriggerConfig(flags):
    acc = ComponentAccumulator()
    if not flags.Detector.GeometrysTGC and not flags.Detector.GeometryMM:
        return acc

    if flags.Input.Format is Format.POOL and flags.Input.isMC:
        rdoInputs = [
            ('McEventCollection','TruthEvent'), # for MM trigger
            ('TrackRecordCollection','MuonEntryLayer'), # for MM trigger
            ('MuonSimDataCollection','sTGC_SDO') # for sTGC Pad trigger
        ]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=rdoInputs))

    PadTdsTool = CompFactory.NSWL1.PadTdsOfflineTool("NSWL1__PadTdsOfflineTool", IsMC = flags.Input.isMC, sTGC_DigitContainerName="sTGC_DIGITS_L1")
    PadTriggerLogicTool = CompFactory.NSWL1.PadTriggerLogicOfflineTool("NSWL1__PadTriggerLogicOfflineTool")
    StripTdsTool = CompFactory.NSWL1.StripTdsOfflineTool("NSWL1__StripTdsOfflineTool",IsMC=flags.Input.isMC,sTGC_DigitContainerName="sTGC_DIGITS_L1")
    StripClusterTool = CompFactory.NSWL1.StripClusterTool("NSWL1__StripClusterTool",IsMC=flags.Input.isMC)
    StripSegmentTool = CompFactory.NSWL1.StripSegmentTool("NSWL1__StripSegmentTool")
    MMTriggerTool = CompFactory.NSWL1.MMTriggerTool("NSWL1__MMTriggerTool",DoNtuple=flags.Trigger.L1MuonSim.WriteMMBranches, IsMC = flags.Input.isMC, MmDigitContainer="MM_DIGITS_L1")
    TriggerProcessorTool = CompFactory.NSWL1.TriggerProcessorTool("NSWL1__TriggerProcessorTool")

    dosTGC =  flags.Trigger.L1MuonSim.doPadTrigger or flags.Trigger.L1MuonSim.doStripTrigger
    if dosTGC:
        from RegionSelector.RegSelToolConfig import regSelTool_STGC_Cfg
        stgcRegSel = acc.popToolsAndMerge(regSelTool_STGC_Cfg( flags ))  # noqa: F841 (adds a conditions algo as a side-effect)

    nswAlg = CompFactory.NSWL1.NSWL1Simulation("NSWL1Simulation",
                                               DoNtuple = flags.Trigger.L1MuonSim.WriteNSWDebugNtuple,
                                               DoMM = flags.Trigger.L1MuonSim.doMMTrigger,
                                               DoMMDiamonds = flags.Trigger.L1MuonSim.doMMTrigger,
                                               DosTGC = dosTGC,
                                               DoPad = flags.Trigger.L1MuonSim.doPadTrigger,
                                               DoStrip = flags.Trigger.L1MuonSim.doStripTrigger,
                                               PadTdsTool = PadTdsTool,
                                               PadTriggerTool = PadTriggerLogicTool,
                                               StripTdsTool = StripTdsTool,
                                               StripClusterTool = StripClusterTool,
                                               StripSegmentTool = StripSegmentTool,
                                               MMTriggerTool = MMTriggerTool,
                                               TriggerProcessorTool = TriggerProcessorTool,
                                               NSWTrigRDOContainerName = "L1_NSWTrigContainer" )
    acc.addEventAlgo(nswAlg)
    return acc

def RPCTriggerConfig(flags):
    acc = ComponentAccumulator()
    rpcAlg = CompFactory.TrigT1RPC("TrigT1RPC",
                                Hardware          = True,
                                DataDetail        = False,
                                RPCbytestream     = False,
                                RPCbytestreamFile = "",
                                RPCDigitContainer = "RPC_DIGITS_L1",
                                useRun3Config = True,
                                NOBXS=flags.Trigger.L1MuonSim.RPCNBX,
                                BCZERO=flags.Trigger.L1MuonSim.RPCNBCZ)
    acc.addEventAlgo(rpcAlg)
    from MuonConfig.MuonCablingConfig import RPCCablingConfigCfg
    acc.merge( RPCCablingConfigCfg(flags) ) # trigger roads
    return acc

def TGCTriggerConfig(flags):
    acc = ComponentAccumulator()
    tgcAlg = CompFactory.LVL1TGCTrigger.LVL1TGCTrigger("LVL1TGCTrigger",
                                                       InputData_perEvent  = "TGC_DIGITS_L1",
                                                       InputRDO = "TGCRDO" if flags.Input.isMC else "TGCRDO_L1",
                                                       useRun3Config = True,
                                                       TileMuRcv_Input = "rerunTileMuRcvCnt",
                                                       TILEMU = True)
    if (flags.Detector.GeometrysTGC or flags.Detector.GeometryMM):
        tgcAlg.MaskFileName12 = "TrigT1TGCMaskedChannel.noFI._12.db"
        tgcAlg.USENSW = True
        tgcAlg.NSWSideInfo = "AC"
        tgcAlg.NSWTrigger_Input = "L1_NSWTrigContainer"
        tgcAlg.FORCENSWCOIN = not flags.Trigger.L1MuonSim.NSWVetoMode
        tgcAlg.USEBIS78 = flags.Trigger.L1MuonSim.doBIS78
    else:
        tgcAlg.MaskFileName12 = "TrigT1TGCMaskedChannel._12.db"

    if flags.Input.Format is Format.BS:
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
        readBSConfig = ByteStreamReadCfg(flags, ['ByteStreamMetadataContainer/ByteStreamMetadata'])
        acc.merge(readBSConfig)
    else:
        tgcAlg.ByteStreamMetadataRHKey = ''
    acc.addEventAlgo(tgcAlg)

    from PathResolver import PathResolver
    bwCW_Run3_filePath=PathResolver.FindCalibFile("TrigT1TGC_CW/BW/CW_BW_Run3.v01.db")
    acc.merge(addFolders(flags, '<db>sqlite://;schema={0};dbname=OFLP200</db> /TGC/TRIGGER/CW_BW_RUN3'.format(bwCW_Run3_filePath),
                                tag='TgcTriggerCwBwRun3-01',
                                className='CondAttrListCollection'))
    acc.addCondAlgo(CompFactory.TGCTriggerCondAlg())
    from MuonConfig.MuonCablingConfig import TGCCablingConfigCfg
    acc.merge( TGCCablingConfigCfg(flags) )
    return acc

def MuctpiConfig(flags):
    acc = ComponentAccumulator()
    rpcRecRoiTool = CompFactory.LVL1.TrigT1RPCRecRoiTool("TrigT1RPCRecRoiTool", UseRun3Config=True)
    tgcRecRoiTool = CompFactory.LVL1.TrigT1TGCRecRoiTool("TrigT1TGCRecRoiTool", UseRun3Config=True)
    trigThresholdDecTool = CompFactory.LVL1.TrigThresholdDecisionTool(name="TrigThresholdDecisionTool",
                                                                       RPCRecRoiTool = rpcRecRoiTool,
                                                                       TGCRecRoiTool = tgcRecRoiTool)
    muctpiTool = CompFactory.LVL1MUCTPIPHASE1.MUCTPI_AthTool(name="MUCTPI_AthTool",
                                                              MuCTPICTPLocation = 'L1MuCTPItoCTPLocation',
                                                              OverlapStrategyName = flags.Trigger.MUCTPI.OverlapStrategy,
                                                              LUTXMLFile = flags.Trigger.MUCTPI.LUTXMLFile,
                                                              BarrelRoIFile = flags.Trigger.MUCTPI.BarrelRoIFile,
                                                              EndcapForwardRoIFile = flags.Trigger.MUCTPI.EndcapForwardRoIFile,
                                                              Side0LUTFile = flags.Trigger.MUCTPI.Side0LUTFile,
                                                              Side1LUTFile = flags.Trigger.MUCTPI.Side1LUTFile,
                                                              InputSource = 'DIGITIZATION',
                                                              RPCRecRoiTool = rpcRecRoiTool,
                                                              TGCRecRoiTool = tgcRecRoiTool,
                                                              TrigThresholdDecisionTool = trigThresholdDecTool)
    muctpiAlg = CompFactory.LVL1MUCTPIPHASE1.MUCTPI_AthAlg(name="MUCTPI_AthAlg",
                                                         MUCTPI_AthTool = muctpiTool)
    acc.addEventAlgo(muctpiAlg)
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
    acc.merge(L1ConfigSvcCfg(flags))
    return acc


def L0MuonSimulationCfg(flags):
    acc = ComponentAccumulator()

    if flags.Trigger.L0MuonSim.doEmulation:
        from L0MuonEmulation.L0MuonSmearingConfig import L0MuonSmearingCfg
        acc.merge(L0MuonSmearingCfg(flags))   # L0MuonRoI truth emulation
        return acc

    # otherwise, run-3 step will run.
    acc.merge(MuonBytestream2RdoConfig(flags)) # data prep for muon bytestream data
    acc.merge(MuonRdo2DigitConfig(flags)) # input for rpc/tgc trigger simulation
    acc.merge(RPCTriggerConfig(flags)) # rpc trigger simulation, including bis78 to prepare for bis78-tgc coincidence
    acc.merge(TMDBConfig(flags)) # for tmdb decision to prepare for tile-muon coincidence
    acc.merge(NSWTriggerConfig(flags)) # nsw trigger simulation to prepare input for nsw-tgc coincidence
    acc.merge(TGCTriggerConfig(flags)) # tgc trigger simulation
    acc.merge(MuctpiConfig(flags)) # muctpi simulation

    return acc

if __name__ == "__main__":
    import sys
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900498.PG_single_muonpm_Pt100_etaFlatnp0_43.recon.RDO.e8481_s4149_r14697/RDO.33675668._000016.pool.root.1']
    flags.Exec.MaxEvents = 5
    flags.Concurrency.NumThreads = 1
    flags.Trigger.triggerMenuSetup = 'MC_pp_run4_v1'
    flags.Trigger.enableL0Muon = True
    flags.Trigger.L0MuonSim.doEmulation = True
    flags.fillFromArgs()
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)

    acc.merge(L0MuonSimulationCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True, printDefaults=True)
    sys.exit(acc.run().isFailure())
