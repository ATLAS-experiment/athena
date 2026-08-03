# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory 
from LArCellRec.LArTimeVetoAlgConfig import LArTimeVetoAlgCfg
from IOVDbSvc.IOVDbSvcConfig import addOverride

def EventVetoCearAlgCfg(flags):

    if flags.Input.isMC: #Do not run this in MC! 
        return None 

    if 456729 not in flags.Input.RunNumbers: 
        return None

    #Possible improvement:
    #Our AODFix infrastructure was designed to fix software problems. Here, we are fixing a conditions problem.
    #Ideally, we should apply this AODFix only if the input-AOD was made using global conditions tags that 
    #contains a folder-level tag for the folder /LAR/BadChannelsOfl/EventVeto that is older than 
    #LARBadChannelsOflEventVeto-RUN2-UPD4-18
    #But the format of global conditions tags is much less well defined as the format of athena-release numbers, 
    #moreover the future derivation-campaigns may use CREST not COOL, so I don't know how to code this in a 
    #future-proof way

    #print("Conditions tag this data was made with:",flags.IOVDb.GlobalTag)

    beginSeq = CompFactory.AthSequencer('AthBeginSeq', Sequential=True)
    result=ComponentAccumulator(sequence=beginSeq)
    result.addEventAlgo(CompFactory.EventInfoClearAlg()) 

    result.merge(LArTimeVetoAlgCfg(flags))
    result.merge(addOverride(flags,"/LAR/BadChannelsOfl/EventVeto","LARBadChannelsOflEventVeto-RUN2-UPD4-18"))

    return result