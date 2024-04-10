# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def CaloRecoCalibCfg(configFlags):
  
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    result = ComponentAccumulator()
    if configFlags.Input.Format is Format.BS:
        #Data-case: Schedule ByteStream reading for LAr & Tile
        from LArByteStream.LArRawCalibDataReadingConfig import LArRawCalibDataReadingCfg 
        result.merge(LArRawCalibDataReadingCfg(configFlags,gain="FREE",doDigit=True))

        from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
        result.merge( TileRawDataReadingCfg(configFlags) )

        if configFlags.Output.doWriteESD:
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterOutputCfg
            result.merge(TileDigitsFilterOutputCfg(configFlags))
        else: #Mostly for wrapping in RecExCommon
            from TileRecAlgs.TileDigitsFilterConfig import TileDigitsFilterCfg
            result.merge(TileDigitsFilterCfg(configFlags))
            
        from LArROD.LArRawChannelBuilderAlgConfig import LArRawChannelBuilderAlgCfg
        result.merge(LArRawChannelBuilderAlgCfg(configFlags))
        from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
        result.merge(TileRawChannelMakerCfg(configFlags))
            
    if not configFlags.Input.isMC:
        from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
        result.merge(LArTimeVetoAlgCfg(configFlags))

  
    
    #Configure cell-building
    from CaloRec.CaloCellMakerConfig import CaloCellMakerCfg
    result.merge(CaloCellMakerCfg(configFlags))
    
    return result

    
 

    




if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Constants import DEBUG

    flags = initConfigFlags()
    flags.Input.Files = ['/eos/atlas/atlastier0/rucio/data22_calib/calibration_L1CaloEnergyScan/00408980/data22_calib.00408980.calibration_L1CaloEnergyScan.daq.RAW/data22_calib.00408980.calibration_L1CaloEnergyScan.daq.RAW._lb0000._SFO-1._0001.data']
    flags.LAr.doHVCorr = False
    from LArConfiguration.LArConfigFlags import RawChannelSource
    flags.LAr.RawChannelSource = RawChannelSource.Calculated
    flags.Exec.OutputLEvel=DEBUG

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    acc.merge(CaloRecoCalibCfg(flags))
    

    acc.run(10)
