# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArMaskedSCCfg

def LArRAWtoSuperCellCfg(flags,name="LArRAWtoSuperCell",mask=True,SCellContainerOut="",SCIn="",doReco=False, bcidShift=0):
    result=ComponentAccumulator()
    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'LArRAWtoSuperCellCfg:' )
    result.merge(LArOnOffIdMappingSCCfg(flags))
    SCInput=""
    if (SCIn != ""):
       SCInput = SCIn
    elif flags.Input.RunNumbers:
       from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
       runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
       for i in range(0,len(runinfo.streamTypes())):
         if runinfo.streamTypes()[i] ==  "SelectedEnergy":
           SCInput="SC_ET_ID"
    else :
       SCInput=flags.LAr.LATOME.DTInfoForL1
    if mask :
       LArBadChannelKey="LArBadChannelSC"
    else :
       LArBadChannelKey=""
    result.merge(LArBadChannelCfg(flags,isSC=True) )

    if SCellContainerOut=="": SCellContainerOut=flags.LAr.DT.ET_IDKey 

    algo = CompFactory.LArRAWtoSuperCell(name,isReco=doReco,SCellContainerOut=SCellContainerOut,LArBadChannelKey=LArBadChannelKey,BCIDOffset=bcidShift)

    if mask and not flags.Input.isMC:
        # also setup to read OTF masked supercells if running on data
        result.merge(LArMaskedSCCfg(flags))
        algo.LArMaskedChannelKey="LArMaskedSC"

    if ( SCInput == ""):
       mlog.info("Not setting SCInput container name")
    else :
       mlog.info("Setting SCInput container name to %s",SCInput)
       algo.SCellContainerIn = SCInput

    result.addEventAlgo(algo)
       
    return result
