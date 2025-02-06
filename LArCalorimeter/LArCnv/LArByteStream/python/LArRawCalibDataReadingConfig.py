# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
from AthenaConfiguration.MainServicesConfig import MainServicesCfg

def LArRawCalibDataReadingCfg(configFlags,gain="HIGH",doAccDigit=False,doAccCalibDigit=False,doCalibDigit=False,doDigit=False):
    acc=ComponentAccumulator()
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(configFlags))
    acc.merge(ByteStreamReadCfg(configFlags))    
    print('LArRawCalibDataReadingCfg gain: ',gain,' doAccDigit: ',doAccDigit,' doAccCalibDigit: ',doAccCalibDigit,' doCalibDigit: ',doCalibDigit,' doDigit: ',doDigit)
    accKey=""
    accCalibKey=""
    calibKey=""
    digKey=""
    if doAccDigit:
       accKey=gain
    elif doAccCalibDigit:
       accCalibKey=gain
    elif doCalibDigit:
       calibKey=gain
    elif doDigit:
       digKey=gain
    else:   
       from AthenaCommon.Logging import logging 
       mlog = logging.getLogger( 'LArRawCalibDataReadingCfg' ) 
       mlog.error("No digits type choosen for LArRawCalibDataReadingAlg, no reading algo added !!!!")
       return acc

    if configFlags.hasCategory("LArCalib"):
       print('LArRawCalibDataReadingCfg accKey: ',accKey,' accCalibKey: ',accCalibKey,' calibKey: ',calibKey,' digKey: ',digKey)
       acc.addEventAlgo(CompFactory.LArRawCalibDataReadingAlg(LArDigitKey=digKey, LArAccDigitKey=accKey, 
                                      LArAccCalibDigitKey=accCalibKey,
                                      LArCalibDigitKey=calibKey, LArFebHeaderKey="LArFebHeader",
                                      SubCaloPreselection=configFlags.LArCalib.Input.SubDet,
                                      PosNegPreselection=configFlags.LArCalib.Preselection.Side,
                                      BEPreselection=configFlags.LArCalib.Preselection.BEC,
                                      FTNumPreselection=configFlags.LArCalib.Preselection.FT))
    else:    
       acc.addEventAlgo(CompFactory.LArRawCalibDataReadingAlg(LArDigitKey=digKey, LArAccDigitKey=accKey, 
                                      LArAccCalibDigitKey=accCalibKey,
                                      LArCalibDigitKey=calibKey, LArFebHeaderKey="LArFebHeader"))
    return acc


if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags=initConfigFlags()
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    flags.LAr.doAlign=False
    flags.Input.Files = ["/eos/atlas/atlastier0/rucio/data22_calib/calibration_LArElec-Pedestal-32s-High-All/00420537/data22_calib.00420537.calibration_LArElec-Pedestal-32s-High-All.daq.RAW/data22_calib.00420537.calibration_LArElec-Pedestal-32s-High-All.daq.RAW._lb0000._SFO-4._0001.data"]

    flags.Exec.OutputLevel=DEBUG
    flags.lock()

    acc = MainServicesCfg( flags )
    acc.merge(LArRawCalibDataReadingCfg(flags,doAccCalibDigit=True))
    
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg 
    acc.merge(LArOnOffIdMappingCfg(flags))

    #f=open("LArRawCalibDataReading.pkl","wb")
    #acc.store(f)
    #f.close()
    acc.run(-1)
