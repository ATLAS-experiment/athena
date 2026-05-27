# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

#The Bad Channel handling of calibration jobs is sufficently non-standard to
#justify a separate config-file

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
LArBadChannelCondAlg, LArBadFebCondAlg =CompFactory.getComps("LArBadChannelCondAlg","LArBadFebCondAlg")
from IOVDbSvc.IOVDbSvcConfig import addFolders


def LArCalibBadChannelCfg(flags):
    result=ComponentAccumulator()

    if not flags.LArCalib.isSC:
       foldername="/LAR/BadChannelsOfl/BadChannels"
       foldertag=("<tag>" + "".join(foldername.split("/")) + flags.LArCalib.BadChannelTag + "</tag>"
                 if flags.LArCalib.BadChannelTag not in (None, "") else "")
       result.merge(addFolders(flags,foldername+foldertag,flags.LArCalib.BadChannelDB,
                            className="CondAttrListCollection"))
       theLArBadChannelCondAlgo=LArBadChannelCondAlg(ReadKey=foldername)
    else:
       foldername="/LAR/BadChannelsOfl/BadChannelsSC"
       foldertag=("<tag>" + "".join(foldername.split("/")) + flags.LArCalib.BadChannelTagSC + "</tag>"
                 if flags.LArCalib.BadChannelTagSC not in (None, "") else "")
       result.merge(addFolders(flags,foldername+foldertag,flags.LArCalib.BadChannelDB,
                            className="CondAttrListCollection"))
       theLArBadChannelCondAlgo=LArBadChannelCondAlg(ReadKey=foldername, isSC=flags.LArCalib.isSC, 
                                                    CablingKey="LArOnOffIdMapSC",WriteKey="LArBadChannelSC")

    result.addCondAlgo(theLArBadChannelCondAlgo)
    return result


