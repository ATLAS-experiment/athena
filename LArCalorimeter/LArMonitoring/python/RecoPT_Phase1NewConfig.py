# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def LArDTMonitoringConfig(flags,STREAM):
    
    acc=ComponentAccumulator()
    
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from LArCabling.LArCablingConfig import LArLATOMEMappingCfg
    acc.merge(LArLATOMEMappingCfg(flags))

    LArRawSCDataReadingAlg=CompFactory.LArRawSCDataReadingAlg
    acc.addEventAlgo(LArRawSCDataReadingAlg(LATOMEDecoder = CompFactory.LArLATOMEDecoder("LArLATOMEDecoder",ProtectSourceId = True)))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))
    acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg')) 

    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc.merge(LArBadChannelCfg(flags, isSC=True))

    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'RecoPT_Phase1' )

    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    mlog.info("Run number: "+str(flags.Input.RunNumbers[0]))

    try:
        runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
        if 'PEB' in STREAM:
           streams=runinfo.streamTypesPEB()
           nsamples=int(runinfo.streamLengthsPEB()[0])
        else:
           streams=runinfo.streamTypes()
           nsamples=int(runinfo.streamLengths()[0])
    except Exception as e:
        mlog.warning("Could not get DT run info")
        mlog(e)
        streams=["ADC","SelectedEnergy"]
        nsamples=32

    if nsamples >= 4:
        nenergies=1
    else:
        nenergies=0

    larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg",LArDigitKey="SC", isADCBas=False, nEnergies=nenergies)
    acc.addEventAlgo(larLATOMEBuilderAlg)

    from LArMonitoring.LArDigitalTriggMonAlg import LArDigitalTriggMonConfig
    acc.merge(LArDigitalTriggMonConfig(flags, larLATOMEBuilderAlg, nsamples, streams))

    return acc




def LArSCvsRawChannelMonAlgCfg(flags,STREAM):  # HERE stream doesn't do anything??
    acc=ComponentAccumulator()    
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc.merge(LArBadChannelCfg(flags))
    acc.merge(LArBadChannelCfg(flags,isSC=True))

    from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArOnOffIdMappingSCCfg

    acc.merge(LArOnOffIdMappingCfg(flags))
    acc.merge(LArOnOffIdMappingSCCfg(flags))
    acc.merge(LArRawSCDataReadingCfg(flags))
    
    from CaloRec.CaloBCIDAvgAlgConfig import CaloBCIDAvgAlgCfg
    acc.merge(CaloBCIDAvgAlgCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))
    acc.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg')) 

    from LArByteStream.LArRawDataReadingConfig import LArRawDataReadingCfg
    acc.merge(LArRawDataReadingCfg(flags))
    
    if flags.Input.isMC is False and not flags.Common.isOnline:
       from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
       acc.merge(LuminosityCondAlgCfg(flags))
       from LumiBlockComps.LBDurationCondAlgConfig import  LBDurationCondAlgCfg
       acc.merge(LBDurationCondAlgCfg(flags))


    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArSuperCellMonAlgCfg')
    from LArMonitoring.LArSCvsRawChannelMonAlg import LArSCvsRawChannelMonConfigCore
    acc.merge(LArSCvsRawChannelMonConfigCore(helper, flags))

    return acc
