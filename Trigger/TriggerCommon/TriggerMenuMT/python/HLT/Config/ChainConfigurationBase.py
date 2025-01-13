# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

import abc
import inspect
import functools
from TriggerMenuMT.HLT.Config.MenuComponents import Chain, ChainStep
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection


#----------------------------------------------------------------
# Base class to configure chain
#----------------------------------------------------------------
class ChainConfigurationBase(metaclass=abc.ABCMeta):

    def __init__(self, chainDict):

        # Consider using translation from  dict["chainName"] to dict.chainName (less typing),
        # though need to be able to access list of dictionaries as value of chainPart as well
        # dict = type("dict", (object,), dict)

        self.dict = {}
        self.dict.update(chainDict)

        self.chainName = self.dict['chainName']
        self.chainL1Item = self.dict['L1item']

        # check dictionary contains only one chain part
        if len( self.dict['chainParts'] ) != 1:
            raise RuntimeError( "Passed chain dictionary with %i chainParts to ChainConfigurationBase, but this constructor only supports chains with a single part" % len( self.dict['chainParts'] ) )

        self.chainPart = self.dict['chainParts'][0]
        self.L1Threshold = self.chainPart['L1threshold'] # now threshold is always there
        self.mult = int(self.chainPart['multiplicity'])
        self.chainPartName = self.chainPart['chainPartName']
        self.chainPartNameNoMult = ''
        self.chainPartNameNoMultwL1 = ''

        self.chainPartNameNoMult = self.chainPartName[1:] if self.mult > 1 else self.chainPartName
        self.chainPartNameNoMultwL1 += "_"+self.chainL1Item

    def getStep(self, flags, stepName, sequenceCfgArray, comboHypoCfg=ComboHypoCfg, comboTools=[], **stepArgs):
        log.debug("Configuring step %s with %d chainParts", stepName, len(self.dict['chainParts']))
        
        # do not generate Menu Sequences, just store the functions that can do that
        seqArray = [functools.partial(gen, flags, **stepArgs) for gen in sequenceCfgArray]          

        if (len(seqArray)>0):                                
            if inspect.signature(comboHypoCfg).parameters and all(inspect.signature(comboTool).parameters for comboTool in comboTools):                
                # Bind flags to comboHypo generator if needed
                if 'flags' in inspect.signature(comboHypoCfg).parameters:
                    comboHypoCfg = functools.partial(comboHypoCfg, flags)
                # Be carefull: custom ComboHpos are passed as partial functions, others no
                # TODO: make uniform way to pass ComboHypo to the step (all partial functions)
                return ChainStep(stepName, seqArray, 
                                 [self.dict], comboHypoCfg = comboHypoCfg, comboToolConfs = comboTools)

        # if not returned any step
        raise RuntimeError("[getStep] No sequences generated for step %s!", stepName)

    def getEmptyStep(self, stepName):
        log.debug('Configuring empty step %s', stepName)
        return ChainStep(stepName, chainDicts=[self.dict], isEmpty=True)
 
    def buildChain(self, chainSteps):
    
        alignmentGroups = []
        if isinstance(self.chainPart, dict):
            alignmentGroups = [self.chainPart['alignmentGroup']]
        elif isinstance(self.chainPart, list):
            
            alignmentGroups = [cp['alignmentGroup'] for cp in self.chainPart]
            testAlignGrps = list(set(alignmentGroups))
            if not(len(testAlignGrps) == 1 and testAlignGrps[0] == 'JetMET'):
                log.error("ChainConfigurationBase.buildChain(): number of chainParts does not correspond chainSteps")    
                log.error('ChainConfigurationBase.buildChain() chainPart: %s',self.chainPart)
                log.error("ChainConfigurationBase.buildChain() alignmentGroups: %s", alignmentGroups)
                log.error("ChainConfigurationBase.buildChain() chainName: %s", self.chainName)
                log.error("ChainConfigurationBase.buildChain() chainSteps: %s", chainSteps)               
            else:
                alignmentGroups = testAlignGrps            
   
        else:
            log.error("ChainConfigurationBase.buildChain(): chainPart is not a list or dict, not sure what to do here! %s	", self.chainPart)
              
        L1decision = mapThresholdToL1DecisionCollection(self.L1Threshold) 
        myChain = Chain(name = self.chainName,
                        ChainSteps = chainSteps,
                        L1decisions = [L1decision],
                        nSteps = [len(chainSteps)], # not true for combined chains
                        alignmentGroups = alignmentGroups
                         )

        return myChain

    @abc.abstractmethod
    def assembleChainImpl(self, flags):
        return

    def assembleChain(self, flags):
        return self.assembleChainImpl(flags)
