# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

Dependencies=[('LArOnOffIdMapping'  , 'ConditionStore+LArOnOffIdMap' ),
              ('LArFebRodMapping'   , 'ConditionStore+LArFebRodMap' ),
              ('LArMCSym'           , 'ConditionStore+LArMCSym'),
              ('LArBadChannelCont'  , 'ConditionStore+LArBadChannel'),
              ('CaloDetDescrManager', 'ConditionStore+CaloDetDescrManager')]

def testLArRodDetectorCfg( flags ):
    acc = ComponentAccumulator()
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
    # Needed by bad channel maskers, refrerenced from LArCellCont.
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg
    acc.merge(LArBadChannelCfg(flags))
    acc.merge(LArBadFebCfg(flags))
    from LArRecUtils.LArRecUtilsConfig import LArMCSymCondAlgCfg
    acc.merge( LArMCSymCondAlgCfg( flags ) )
    testAlg = CompFactory.TestLArRodDecoder(name="TestLArRodDecoder",ExtraInputs=Dependencies,ROBIDs=[0x00410000,0x410001,0x00410002, 0x00410003, 0x00410004, 0x00410005, 0x00410006, 0x00410007, 0x00410008, 0x00410009,0x00420000,0x00420001,0x00420002, 0x00420003, 0x00420004, 0x00420005, 0x00420006, 0x00420007, 0x00420008, 0x00420009],PrintDebug=False)
    acc.addEventAlgo(testAlg)
    return acc

if __name__ == "__main__":
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.ComponentFactory import CompFactory
    import sys

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.Input.isMC=False
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg( flags )

    from AthenaCommon.CFElements import parOR
    acc.addSequence(parOR("HLTBeginSeq"))

    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    acc.merge( ByteStreamReadCfg( flags ) )
    acc.merge( testLArRodDetectorCfg( flags ) )

    acc.printConfig(True)
    sc = acc.run(10)
    sys.exit(sc.isFailure())
