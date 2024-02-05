# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA, menuSequenceCAToGlobalWrapper

from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger(__name__)

# Low threshold prescaled L1 items - slected at HLT based on TBP bit from L1 in random-seeded events        
# High(er) threshold prescaled L1 items - slected at HLT based on TBP bit from L1 in random-seeded events 
l1seeds = { 'low'  : \
               ['L1_2eEM9',\
                'L1_eEM12L',\
                #'L1_EM12_XS20',\
                'L1_J15p31ETA49',\
                'L1_JPSI-1M5-EM12',\
                'L1_J30',\
                #'L1_J30p0ETA49_2J20p0ETA49',\
                'L1_JPSI-1M5-eEM9',\
                'L1_MU8F',\
                'L1_ZB',\
              #  'L1_ZB_eEM18'
                ],\
             'medium' : \
               [
                'L1_2eEM18L',\
                'L1_2MU3V',\
                'L1_BPH-0DR3-EM7J15_2MU3V',\
                'L1_BPH-0DR3-EM7J15_MU5VF',\
                'L1_BPH-0M9-EM7-EM5_MU5VF',\
                'L1_BPH-2M9-2DR15-2MU5VF',\
                'L1_BPH-2M9-0DR15-C-MU5VFMU3V',\
                'L1_BPH-8M15-0DR22-2MU5VF',\
                'L1_BPH-8M15-0DR22-MU5VFMU3V-BO',\
                'L1_BTAG-MU3VjJ40',\
                'L1_cTAU30M_2cTAU20M_DR-eTAU30eTAU20',\
                'L1_DY-BOX-2MU5VF',\
                'L1_DY-BOX-2MU3VF',\
                'L1_eEM18M_2eTAU20M_jJ55_3jJ30',\
                #'L1_EM15_XS30',\
                'L1_eEM18L',\
                'L1_eEM24L',\
                'L1_HT190-J15s5pETA21',\
                'L1_J30p31ETA49',\
                'L1_J40p0ETA25_2J15p31ETA49',\
                'L1_J50',\
                'L1_J50_DETA20-J50J',\
                'L1_LFV-MU5VF',\
                'L1_MJJ-500-NFF',\
                'L1_MU5VF_J40',\
                'L1_MU8F_eTAU20M',\
                #'L1_MU5VF_J20',\
                #'L1_MU5VF_J30p0ETA49_2J20p0ETA49',\
                'L1_eTAU30M_2eTAU20M_jJ55_2jJ50_3jJ30',\
                'L1_eTAU30M_2eTAU20M_jXE70',\
                'L1_eTAU60',\
                'L1_XE35',
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


def enhancedBiasMenuSequence(flags):

    reco = enhancedBiasReco(flags)
    selAcc = SelectionCA("enhancedBiasSequence") 
    selAcc.mergeReco(reco)
    selAcc.addHypoAlgo(CompFactory.L1InfoHypo("EnhancedBiasHypo"))

    return MenuSequenceCA(flags,
                          selAcc,
                          HypoToolGen = EnhancedBiasHypoToolGen)



def enahncedBiasSequence_Cfg(flags):
    if isComponentAccumulatorCfg():
        return enhancedBiasMenuSequence(flags)
    else:
        return menuSequenceCAToGlobalWrapper(enhancedBiasMenuSequence, flags)


class EnhancedBiasChainConfiguration(ChainConfigurationBase):
    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self, chainDict)


    def assembleChainImpl(self, flags):
        chainSteps = []
        log.debug("Assembling chain for %s", self.chainName)

        chainSteps.append( self.getStep(flags, 1, "EnhancedBias", [enahncedBiasSequence_Cfg]) )

        return self.buildChain(chainSteps)
