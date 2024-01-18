# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

CaloDataAccessSvcDependencies = [('TileEMScale'        , 'ConditionStore+TileEMScale'),
                                 ('TileHid2RESrcID'    , 'ConditionStore+TileHid2RESrcIDHLT'),
                                 ('TileBadChannels'    , 'ConditionStore+TileBadChannels'),
                                 ('IRegSelLUTCondData' , 'ConditionStore+RegSelLUTCondData_TTEM'),
                                 ('IRegSelLUTCondData' , 'ConditionStore+RegSelLUTCondData_TTHEC'),
                                 ('IRegSelLUTCondData' , 'ConditionStore+RegSelLUTCondData_TILE'),
                                 ('IRegSelLUTCondData' , 'ConditionStore+RegSelLUTCondData_FCALEM'),
                                 ('IRegSelLUTCondData' , 'ConditionStore+RegSelLUTCondData_FCALHAD'),
                                 ('LArOnOffIdMapping'  , 'ConditionStore+LArOnOffIdMap' ),
                                 ('LArFebRodMapping'   , 'ConditionStore+LArFebRodMap' ),
                                 ('LArMCSym'           , 'ConditionStore+LArMCSym'),
                                 ('LArBadChannelCont'  , 'ConditionStore+LArBadChannel'),
                                 ('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager')]


def CaloOffsetCorrectionCfg(flags):

    acc = ComponentAccumulator()
    if not flags.Input.isMC and flags.Common.isOnline:
        from IOVDbSvc.IOVDbSvcConfig import addFolders
        acc.merge(addFolders(flags, "/LAR/ElecCalibFlat/OFC",'LAR_ONL', className = 'CondAttrListCollection'))
        larCondSvc = CompFactory.LArFlatConditionSvc()
        larCondSvc.OFCInput="/LAR/ElecCalibFlat/OFC"
        acc.addService(larCondSvc)
        acc.addService(CompFactory.ProxyProviderSvc(ProviderNames=[larCondSvc.name]))
        acc.addCondAlgo(CompFactory.getComp('LArFlatConditionsAlg<LArOFCFlat>')(ReadKey="/LAR/ElecCalibFlat/OFC", WriteKey='LArOFC'))
        from LumiBlockComps.LuminosityCondAlgConfig import LuminosityCondAlgCfg
        acc.merge(LuminosityCondAlgCfg(flags))

    from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
    acc.merge(CaloBCIDAvgAlgCfg(flags))
    from LArRecUtils.LArRecUtilsConfig import LArMCSymCondAlgCfg
    acc.merge( LArMCSymCondAlgCfg( flags ) )
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    monTool.defineHistogram('TIME_exec', path='EXPERT', type='TH1F', title="CaloBCIDAvgAlg execution time; time [ us ] ; Nruns", xbins=80, xmin=0.0, xmax=4000)
    acc.getEventAlgo("CaloBCIDAvgAlg").MonTool = monTool
    return acc

@AccumulatorCache
def trigCaloDataAccessSvcCfg( flags ):

    acc = ComponentAccumulator()
    svc = CompFactory.TrigCaloDataAccessSvc()

    # since the svc depends on calo geometry configure it here
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge( LArGMCfg( flags ) )

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg( flags ) )

    from LArRecUtils.LArRecUtilsConfig import LArRoIMapCondAlgCfg
    acc.merge( LArRoIMapCondAlgCfg( flags ) )

    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArFebRodMappingCfg
    acc.merge( LArOnOffIdMappingCfg( flags ))
    acc.merge( LArFebRodMappingCfg( flags ))

    from TileByteStream.TileHid2RESrcIDConfig import TileHid2RESrcIDCondAlgCfg
    acc.merge( TileHid2RESrcIDCondAlgCfg(flags, ForHLT=True) )

    #setup region selector
    from RegionSelector.RegSelToolConfig import (regSelTool_TTEM_Cfg,regSelTool_TTHEC_Cfg,
                                                 regSelTool_FCALEM_Cfg,regSelTool_FCALHAD_Cfg,regSelTool_TILE_Cfg)

    svc.RegSelToolEM = acc.popToolsAndMerge(regSelTool_TTEM_Cfg(flags))
    svc.RegSelToolHEC = acc.popToolsAndMerge(regSelTool_TTHEC_Cfg(flags))
    svc.RegSelToolFCALEM = acc.popToolsAndMerge(regSelTool_FCALEM_Cfg(flags))
    svc.RegSelToolFCALHAD = acc.popToolsAndMerge(regSelTool_FCALHAD_Cfg(flags))
    svc.RegSelToolTILE = acc.popToolsAndMerge(regSelTool_TILE_Cfg(flags))


    # Needed by bad channel maskers, refrerenced from LArCellCont.
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg
    acc.merge(LArBadChannelCfg(flags))
    acc.merge(LArBadFebCfg(flags))

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if flags.Trigger.Calo.doOffsetCorrection:
        if flags.Trigger.doHLT:
            from AthenaCommon.CFElements import parOR
            eventAcc = ComponentAccumulator(parOR("HLTBeginSeq"))
            eventAcc.merge(CaloOffsetCorrectionCfg(flags), sequenceName="HLTBeginSeq")
            acc.merge(eventAcc)
        else:
            acc.merge(CaloOffsetCorrectionCfg(flags))

    acc.addService( svc, primary=True )
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import sys

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Input.isMC=False
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg( flags )

    from AthenaCommon.CFElements import parOR
    acc.addSequence(parOR("HLTBeginSeq"))

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge( ByteStreamReadCfg( flags ) )

    acc.merge( trigCaloDataAccessSvcCfg( flags ) )

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    import math
    mon = GenericMonitoringTool(flags, "TrigCaloDataAccessSvcMon")
    mon.defineHistogram("TIME_locking_LAr_RoI",
                        path="EXPERT",
                        title="Time spent in unlocking the LAr collection",
                        xbins=100, xmin=0, xmax=100 ),
    mon.defineHistogram("roiROBs_LAr",
                        path="EXPERT",
                        title="Number of ROBs unpacked in RoI requests",
                        xbins=20, xmin=0, xmax=20 ),
    mon.defineHistogram("TIME_locking_LAr_FullDet",
                        path="EXPERT",
                        title="Time spent in unlocking the LAr collection",
                        xbins=100, xmin=0, xmax=100 ),
    mon.defineHistogram("roiEta_LAr,roiPhi_LAr",
                        type="TH2F",
                        path="EXPERT",
                        title="Geometric usage",
                        xbins=50, xmin=-5, xmax=5,
                        ybins=64, ymin=-math.pi, ymax=math.pi )

    acc.getService("TrigCaloDataAccessSvc").MonTool = mon

    testAlg = CompFactory.TestCaloDataAccess()
    acc.addEventAlgo(testAlg)

    acc.printConfig(True)
    sc = acc.run(10)
    sys.exit(sc.isFailure())
