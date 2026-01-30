# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
LArCellBuilderFromLArRawChannelTool, LArCellMerger, LArCellNoiseMaskingTool=CompFactory.getComps("LArCellBuilderFromLArRawChannelTool","LArCellMerger","LArCellNoiseMaskingTool",)
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg
from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg, LArBadFebCfg
from LArCalibUtils.LArHVScaleConfig import LArHVScaleCfg
from LArConfiguration.LArConfigFlags import RawChannelSource 

def LArCellBuilderCfg(configFlags):
    result=ComponentAccumulator()
    result.merge(LArOnOffIdMappingCfg(configFlags))
    result.merge(LArBadFebCfg(configFlags))
    theLArCellBuilder = LArCellBuilderFromLArRawChannelTool()
    theLArCellBuilder.LArCablingKey = "ConditionStore+LArOnOffIdMap"
    theLArCellBuilder.MissingFebKey = "ConditionStore+LArBadFeb"
    if configFlags.LAr.RawChannelSource is RawChannelSource.Calculated:
       theLArCellBuilder.RawChannelsName="LArRawChannels_FromDigits"
    else:
       theLArCellBuilder.RawChannelsName = "LArRawChannels"
    theLArCellBuilder.addDeadOTX = True #Create flag? Requires bad-feb DB access
    result.setPrivateTools(theLArCellBuilder)
    return result


def LArCellCorrectorCfg(configFlags):
    result=ComponentAccumulator()

    correctionTools=[]

    if configFlags.LAr.RawChannelSource in (RawChannelSource.Both, RawChannelSource.Input) and not configFlags.Input.isMC and not configFlags.Overlay.DataOverlay:
        theMerger=LArCellMerger(RawChannelsName="LArRawChannels_FromDigits")
        correctionTools.append(theMerger)

    if configFlags.LAr.doCellNoiseMasking or configFlags.LAr.doCellSporadicNoiseMasking:
        result.merge(LArBadChannelCfg(configFlags))
        theNoiseMasker=LArCellNoiseMaskingTool(qualityCut = 4000)
        if configFlags.LAr.doCellNoiseMasking:
            theNoiseMasker.ProblemsToMask=["highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys"]
            pass
        if configFlags.LAr.doCellSporadicNoiseMasking:
            theNoiseMasker.SporadicProblemsToMask=["sporadicBurstNoise",]
            pass
        correctionTools.append(theNoiseMasker)

    if configFlags.LAr.doBadFebMasking:
        if not configFlags.Input.isMC:
            from LArROD.LArFebErrorSummaryMakerConfig import LArFebErrorSummaryMakerCfg
            result.merge(LArFebErrorSummaryMakerCfg(configFlags))
        badFebMask=CompFactory.LArBadFebMaskingTool(noFebErrors=configFlags.Input.isMC)

        correctionTools.append(badFebMask)


    result.setPrivateTools(correctionTools)
    return result


def LArHVCellContCorrCfg(configFlags):
    acc=ComponentAccumulator()
    acc.merge(LArHVScaleCfg(configFlags)) #CondAlgo & co for HVScale Corr
    LArCellContHVCorrTool=CompFactory.LArCellContHVCorrTool
    theLArCellHVCorrTool = LArCellContHVCorrTool()
    acc.setPrivateTools(theLArCellHVCorrTool)
    return acc

def LArDeadOTXCorrCfg(configFlags):
    acc=ComponentAccumulator()
    acc.merge(LArBadFebCfg(configFlags))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    acc.merge(LArOnOffIdMappingSCCfg(configFlags))
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc.merge(LArBadChannelCfg(configFlags,isSC=True))
    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    runinfo=getLArDTInfoForRun(configFlags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
    SCInput="SC_ET"
    for i in range(0,len(runinfo.streamTypes())):
       if runinfo.streamTypes()[i] ==  "SelectedEnergy":
          SCInput="SC_ET_ID"

    #Schedule reading of Super-Cell info from ByteStream
    from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
    acc.merge(LArRawSCDataReadingCfg(configFlags))

    acc.addCondAlgo(CompFactory.LArDeadOTXCondAlg())
    
    deadOTXTool=CompFactory.LArCelldeadOTXTool("LArCelldeadOTXTool", keySC=SCInput)
    acc.setPrivateTools(deadOTXTool)
    return acc


def LArDeadOTXAlgCfg(configFlags,keySC="SC_ET_ID_RoI"):
    acc=ComponentAccumulator()
    acc.merge(LArBadFebCfg(configFlags))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    acc.merge(LArOnOffIdMappingSCCfg(configFlags))
    from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
    acc.merge(LArBadChannelCfg(configFlags,isSC=True))

    deadOTXAlg=CompFactory.LArCelldeadOTXAlg(name="LArCelldeadOTXAlg",keyMF="LArBadFeb",
                                   keyCabling="LArOnOffIdMap", keySCCabling="LArOnOffIdMapSC",
                                   keySC=keySC,SCEneCut=0)
    acc.addEventAlgo(deadOTXAlg)
    return acc
