# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
import os

def LArCalibBaseCfg(flags):  
    result=ComponentAccumulator()

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    result.merge(LArGMCfg(flags))

    if flags.LArCalib.isSC:
        #Setup SuperCell cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg, LArCalibIdMappingSCCfg, LArLATOMEMappingCfg
        result.merge(LArOnOffIdMappingSCCfg(flags))
        result.merge(LArCalibIdMappingSCCfg(flags))
        result.merge(LArLATOMEMappingCfg(flags))
    else: 
        #Setup regular cabling
        from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArCalibIdMappingCfg
        result.merge(LArOnOffIdMappingCfg(flags))
        result.merge(LArCalibIdMappingCfg(flags))
    
    #Set up bad-channel config
    from LArCalibProcessing.LArCalib_BadChannelConfig import LArCalibBadChannelCfg

    result.merge(LArCalibBadChannelCfg(flags))
    if "FRONTIER_SERVER" not in os.environ:
        # when running offline job at P1,
        # point to the location where PoolCat_oflcond.xml can be found
        p = ":/det/lar/project/databases/jobsDatabase/"
        if p not in os.environ["DATAPATH"]:
            os.environ["DATAPATH"] += p
    return result




#Helper method to maipulate channel selection string
def chanSelStr(flags):
    inp=flags.LArCalib.Input.ChannelSelection
    if inp=="": return ""
    if inp.startswith("<channelSelection>"): return inp
    
    return "<channelSelection>"+inp+"</channelSelection>"
