# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg, LArOnOffIdMappingSCCfg
from IOVDbSvc.IOVDbSvcConfig import addFolders, addFoldersSplitOnline

def LArBadChannelCfg(configFlags, tag=None, isSC=False):

    

    result=ComponentAccumulator()
    if not isSC:
      result.merge(LArOnOffIdMappingCfg(configFlags))
      rekey="/LAR/BadChannels/BadChannels"
      algname="LArBadChannelCondAlg"
    else:
      result.merge(LArOnOffIdMappingSCCfg(configFlags))
      rekey="/LAR/BadChannels/BadChannelsSC"
      algname="LArBadChannelCondSCAlg"

    if configFlags.Overlay.DataOverlay:
        # TODO: move this in a better location
        result.merge(addFolders(configFlags, "/LAR/BadChannels/BadChannels", "LAR_OFL", className="CondAttrListCollection", tag="LARBadChannelsBadChannels-HECAQ3Missing", db="OFLP200"))
    elif configFlags.Input.isMC:
       folder = "/LAR/BadChannels/BadChannelsSC" if isSC else "/LAR/BadChannels/BadChannels"
       result.merge(addFolders(configFlags,folder,"LAR_OFL",tag=tag,
                                className="CondAttrListCollection"))
    else:
       if not isSC:
          result.merge(addFoldersSplitOnline(configFlags,"LAR","/LAR/BadChannels/BadChannels",
                                        f"/LAR/BadChannelsOfl/BadChannels<key>{rekey}</key>",tag=tag,
                                        className="CondAttrListCollection"))  
       else :
          result.merge(addFolders(configFlags,"/LAR/BadChannels/BadChannelsSC",
                                        "LAR_ONL",tag=tag,
                                        className="CondAttrListCollection"))  
    theLArBadChannelCondAlgo=CompFactory.LArBadChannelCondAlg(name=algname,ReadKey=rekey,isSC=isSC)
    if isSC:
        theLArBadChannelCondAlgo.WriteKey="LArBadChannelSC"
        theLArBadChannelCondAlgo.CablingKey="LArOnOffIdMapSC"
    result.addCondAlgo(theLArBadChannelCondAlgo)
    return result


def LArBadFebCfg(configFlags, tag=None):
    result=ComponentAccumulator()
    rekey="/LAR/BadChannels/MissingFEBs"

    if configFlags.Input.isMC:
        result.merge(addFolders(configFlags,"/LAR/BadChannels/MissingFEBs","LAR_OFL",tag=tag,
                                className="AthenaAttributeList"))
    else:
        result.merge(addFoldersSplitOnline(configFlags,"LAR","/LAR/BadChannels/MissingFEBs",
                                           f"/LAR/BadChannelsOfl/MissingFEBs<key>{rekey}</key>",tag=tag,
                                           className="AthenaAttributeList"))  
    result.addCondAlgo(CompFactory.LArBadFebCondAlg(ReadKey=rekey))
    return result

def LArMaskedSCCfg(configFlags):
    result=ComponentAccumulator()
    result.merge(LArOnOffIdMappingSCCfg(configFlags))
    from IOVDbSvc.IOVDbSvcConfig import addFolders
    result.merge(addFolders(configFlags,"/LAR/BadChannels/MaskedSC","LAR_ONL",
                            tag="LARBadChannelsMaskedSC-RUN3-UPD1-00",
                            className="CondAttrListCollection",
                            extensible=configFlags.Common.isOnline )) # when run online, need folder to be extensible to force reload each event         
    condAlgo = CompFactory.LArBadChannelCondAlg(name="MaskedSCCondAlg",ReadKey="/LAR/BadChannels/MaskedSC",isSC=True,
                                                CablingKey="LArOnOffIdMapSC",WriteKey="LArMaskedSC")         
    result.addCondAlgo(condAlgo)
    return result


if __name__=="__main__":
    
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    flags=initConfigFlags()
    flags.Input.isMC = False
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data17_13TeV.00330470.physics_Main.daq.RAW._lb0310._SFO-1._0001.data"]
    flags.lock()

    cfg=ComponentAccumulator()
    
    cfg.merge(LArBadChannelCfg(flags))
    cfg.merge(LArBadFebCfg(flags))
    f=open("LArBCCondAlgos.pkl","wb")
    cfg.store(f)
    f.close()
    
