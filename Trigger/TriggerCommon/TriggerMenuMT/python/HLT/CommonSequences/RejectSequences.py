#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequence
# Generic Rejection Algo Configuration
def RejectSequence(flags, name, sequence):
        TargetHypoAlg = CompFactory.TrigStreamerHypoAlg(name)

        sequence.addHypoAlgo(TargetHypoAlg)

        # Reject every event
        def getRejectingHypoTool(flags, chainDict): 
                return CompFactory.TrigStreamerHypoTool(chainDict['chainName'],Pass=False)

        return  MenuSequence(flags,sequence,HypoToolGen=getRejectingHypoTool)

