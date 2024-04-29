# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# Low threshold prescaled L1 items - slected at HLT based on TBP bit from L1 in random-seeded events        
# High(er) threshold prescaled L1 items - slected at HLT based on TBP bit from L1 in random-seeded events 
l1seeds = { 'low'  : \
               ['L1_2eEM9',\
                'L1_eEM12L',\
                "L1_eTAU20",\
                #'L1_EM12_XS20',\
                'L1_jJ40p30ETA49',\
                'L1_JPSI-1M5-eEM15',\
                'L1_jJ60',\
                #'L1_J30p0ETA49_2J20p0ETA49',\
                'L1_JPSI-1M5-eEM9',\
                'L1_MU8F',\
                'L1_ZeroBias',\
                ],\
             'medium' : \
               [
                'L1_2eEM18',\
                'L1_2MU3V',\
                'L1_BPH-0DR3-eEM9jJ40_2MU3V',\
                'L1_BPH-0DR3-eEM9jJ40_MU5VF',\
                'L1_BPH-0M9-eEM9-eEM7_MU5VF',\
                'L1_BPH-2M9-2DR15-2MU5VF',\
                'L1_BPH-2M9-0DR15-C-MU5VFMU3V',\
                'L1_BPH-8M15-0DR22-2MU5VF',\
                'L1_BPH-8M15-0DR22-MU5VFMU3V-BO',\
                'L1_BTAG-MU3VjJ40',\
                'L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20',\
                'L1_DY-BOX-2MU5VF',\
                'L1_DY-BOX-2MU3VF',\
                #'L1_EM15_XS30',\
                'L1_eEM18L',\
                'L1_eEM24L',\
                'L1_gXEJWOJ100',\
                'L1_jJ60p30ETA49',\
                'L1_jJ80p0ETA25_2jJ40p30ETA49',\
                'L1_J50',\
                'L1_jJ90_DETA20-jJ90J',\
                'L1_LFV-MU5VF',\
                'L1_MU5VF_J40',\
                'L1_MU8F_eTAU30M',\
                #'L1_MU5VF_J20',\
                #'L1_MU5VF_J30p0ETA49_2J20p0ETA49',\
                'L1_eTAU60',\
                #'L1_XE35',
                'L1_jXE70',
            ]
}


def enhancedBiasReco(flags):
    inputMakerAlg = CompFactory.InputMakerForRoI("IM_enhancedBias")
    inputMakerAlg.RoITool = CompFactory.ViewCreatorInitialROITool()
    inputMakerAlg.RoIs="enhancedBiasInputRoIs"

    reco = InEventRecoCA("EnhancedBiasReco", inputMaker=inputMakerAlg)
    
    return reco


def EnhancedBiasHypoToolGen(chainDict):
    tool = CompFactory.L1InfoHypoTool(chainDict['chainName'])
    tool.CTPUnpackingTool.UseTBPBits = True

    key = chainDict['chainParts'][0]['algType']
    if key not in l1seeds:
        log.error("No configuration exist for EB chain: ", key)
    else:
        tool.L1ItemNames = l1seeds[key]

    return tool


def enhancedBiasMenuSequenceGenCfg(flags):

    reco = enhancedBiasReco(flags)
    selAcc = SelectionCA("enhancedBiasSequence") 
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(CompFactory.L1InfoHypo("EnhancedBiasHypo"))

    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = EnhancedBiasHypoToolGen)


class EnhancedBiasChainConfiguration(ChainConfigurationBase):
    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self, chainDict)


    def assembleChainImpl(self, flags):
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        chainSteps.append( self.getStep(flags, 1, "EnhancedBias", [enhancedBiasMenuSequenceGenCfg]) )

        return self.buildChain(chainSteps)
