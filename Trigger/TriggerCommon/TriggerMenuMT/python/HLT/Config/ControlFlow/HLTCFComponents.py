# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.MenuComponents import AlgNode, HypoAlgNode
from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import isComboHypoAlg
from TriggerMenuMT.HLT.Config.MenuComponents import EmptyMenuSequence
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.CFElements import findAlgorithmByPredicate, parOR, seqAND
from functools import lru_cache

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

RoRSeqFilter = CompFactory.RoRSeqFilter
PassFilter   = CompFactory.PassFilter


class SequenceFilterNode(AlgNode):
    """Node for any kind of sequence filter"""
    def __init__(self, Alg, inputProp, outputProp):
        AlgNode.__init__(self,  Alg, inputProp, outputProp)

    def addChain(self, name, input_name):
        return

    def getChains(self):
        return []

    def getChainsPerInput(self):
        return [[]]

    def __repr__(self):
        return "SequenceFilter::%s  [%s] -> [%s], chains=%s"%(self.Alg.name,' '.join(map(str, self.getInputList())),' '.join(map(str, self.getOutputList())), self.getChains())


class RoRSequenceFilterNode(SequenceFilterNode):
    def __init__(self, name): 
        Alg= RoRSeqFilter(name)            
        SequenceFilterNode.__init__(self,  Alg, 'Input', 'Output')

    def addChain(self, name, input_name):
        input_index = self.readInputList().index(input_name)
        if len(self.Alg.ChainsPerInput) == input_index:
            self.Alg.ChainsPerInput.append([name])
        elif len(self.Alg.ChainsPerInput) > input_index:
            self.Alg.ChainsPerInput[input_index].append(name)
        else:
            log.error("Error: why requiring input %i when size is %i ?" , input_index , len(self.Alg.ChainsPerInput))
            raise RuntimeError("Error: why requiring input %i when size is %i " , input_index , len(self.Alg.ChainsPerInput))
            
        return self.Alg.Chains.append(name) # still neded?
        
    def getChains(self):
        return self.Alg.Chains

    def getChainsPerInput(self):
        return self.Alg.ChainsPerInput



class PassFilterNode(SequenceFilterNode):
    """ PassFilter is a Filter node without inputs/outputs, so OutputProp=InputProp=empty"""
    def __init__(self, name):        
        Alg=CompFactory.AthSequencer( "PassSequence" )
        Alg.IgnoreFilterPassed=True   # always pass     
        SequenceFilterNode.__init__(self,  Alg, '', '')

    def addOutput(self, name):
        self.outputs.append(str(name)) 

    def addInput(self, name):
        self.inputs.append(str(name)) 

    def getOutputList(self):
        return self.outputs

    def getInputList(self):
        return self.inputs

def isPassSequence(alg):
    return isinstance(alg, PassFilterNode)  

#########################################################
# CFSequence class
#########################################################
class CFSequence(object):
    """Class to describe the flow of decisions through ChainStep + filter with their connections (input, output)
    A Filter can have more than one input/output if used in different chains, so this class stores and manages all of them (when doing the connect)
    """
    def __init__(self, ChainStep, FilterAlg):
        log.debug(" *** Create CFSequence %s with Filter %s", ChainStep.name, FilterAlg.Alg.getName()) 
        self.filterNode = FilterAlg
        self.step = ChainStep

        self.ca = ComponentAccumulator()
        #empty step: add the PassSequence, one instance only is appended to the tree
        seqAndWithFilter = FilterAlg.Alg if ChainStep.isEmpty else seqAND(ChainStep.name)        
        self.ca.addSequence(seqAndWithFilter)
        self.seq = seqAndWithFilter
        if not ChainStep.isEmpty: 
            self.ca.addEventAlgo(FilterAlg.Alg, sequenceName=seqAndWithFilter.getName())
            self.stepReco = parOR(ChainStep.name + CFNaming.RECO_POSTFIX)  
            # all reco algorithms from all the sequences in a parallel sequence                            
            self.ca.addSequence(self.stepReco, parentName=seqAndWithFilter.getName())
            log.debug("created parOR %s inside seqAND %s  ", self.stepReco.getName(), seqAndWithFilter.getName())
            self.mergeStepSequences(ChainStep)
            # merge the Hypoalg (before the Combo)
            for menuseq in ChainStep.sequences:
                if not isinstance(menuseq, EmptyMenuSequence):
                    self.ca.merge(menuseq.hypoAcc, sequenceName=seqAndWithFilter.getName())   

        self.connectCombo()
        self.setDecisions()        
        if self.step.combo is not None:   
            self.ca.merge(self.step.combo.acc, sequenceName=seqAndWithFilter.getName())   
        log.debug("CFSequence.__init: created %s ",self)    


    def setDecisions(self):
        """ Set the output decision of this CFSequence as the hypo outputdecision; In case of combo, takes the Combo outputs"""
        self.decisions=[]
        # empty steps:
        if self.step.combo is None:
            self.decisions.extend(self.filterNode.getOutputList())
        else:
            self.decisions.extend(self.step.combo.getOutputList())   
        log.debug("CFSequence: set out decisions: %s", self.decisions)
    

    def connectCombo(self):
        """ connect Combo to Hypos"""
        if self.step.combo is None:
            log.debug("CFSequence.connectCombo: no Combo found")
            return

        for seq in self.step.sequences:            
            combo_input=seq.getOutputList()[0]
            self.step.combo.addInput(combo_input)
            inputs = self.step.combo.readInputList()
            legindex = inputs.index(combo_input)
            log.debug("CFSequence.connectCombo: adding input to  %s: %s",  self.step.combo.Alg.getName(), combo_input)
            # inputs are the output decisions of the hypos of the sequences
            combo_output=CFNaming.comboHypoOutputName (self.step.combo.Alg.getName(), legindex)            
            self.step.combo.addOutput(combo_output)
            log.debug("CFSequence.connectCombo: adding output to  %s: %s",  self.step.combo.Alg.getName(), combo_output)
    
    def mergeStepSequences(self, chainStep):
        for menuseq in chainStep.sequences:
            try:
                self.ca.merge(menuseq.ca, sequenceName=self.stepReco.getName())
            except Exception as e:
                log.error(f'Failed to merge into {self.stepReco.getName()}')
                raise e

    @lru_cache(None)
    def findComboHypoAlg(self):
        return findAlgorithmByPredicate(self.seq, lambda alg: alg.name == self.step.Alg.name and isComboHypoAlg(alg))

    def __repr__(self):
        return "--- CFSequence ---\n + Filter: %s \n + decisions: %s\n +  %s \n"%(\
                    self.filterNode.Alg.name, self.decisions, self.step)




class CFGroup(object):
    """Class to store the Step + its Filter (CFSequence) plus the chains and dictionaries of the legs using that step   """
    def __init__(self, ChainStep, FilterAlg):
        self.stepDicts = []  # will become a list of lists
        self.chains = []
        self.comboToolConfs = []
        self.createCFSequence(ChainStep, FilterAlg)
        log.debug("CFGroup.__init: created for %s ",ChainStep.name)
    
    def createCFSequence(self, ChainStep, FilterAlg):
        '''This creates the CAs for the menu sequences, if fastMenu style, and the CFSequence'''
        log.debug("CFGroup.creating CFSEquence")
        self.sequenceCA = CFSequence(ChainStep, FilterAlg)
        return self.sequenceCA
    
    def addStepLeg(self, newstep, chainName):
        self.stepDicts.append(newstep.stepDicts) # one dict per leg
        self.chains.append(chainName)
        self.comboToolConfs.append(newstep.comboToolConfs)
    

    def connect(self, connections):
        """Connect filter to ChainStep (and all its sequences) through these connections (which are sets of filter outputs)
        if a ChainStep contains the same sequence multiple times (for multi-leg chains),
        the filter is connected only once (to avoid multiple DH links)
        """
        if log.isEnabledFor(logging.DEBUG):
            log.debug("CFGroup: connect Filter %s with %d menuSequences of step %s, using %d connections", self.sequenceCA.filterNode.Alg.name, len(self.sequenceCA.step.sequences), self.sequenceCA.step.name, len(connections))
            log.debug("   --- sequences: ")
            for seq in self.sequenceCA.step.sequences:
                log.debug(seq)

        if len(connections) == 0:
            log.error("No filter outputs are set!")

        if len(self.sequenceCA.step.sequences):
            # check whether the number of filter outputs are the same as the number of sequences in the step
            if len(connections) != len(self.sequenceCA.step.sequences):
                log.error("CFGroup: Found %d connections and %d MenuSequences in Step %s", len(connections), len(self.sequenceCA.step.sequences), self.sequenceCA.step.name)
                raise Exception("[CFGroup] Connections and sequences do not match, this must be fixed!")
            
            for nseq, seq in enumerate(self.sequenceCA.step.sequences):
                filter_out = connections[nseq]
                log.debug("CFGroup: Found input %s to sequence::%s from Filter::%s", filter_out, seq.name, self.sequenceCA.filterNode.Alg.name)
                seq.connectToFilter( filter_out )               
        else:
          log.debug("This CFGroup has no sequences: outputs are the Filter outputs, which are %d", len(self.sequenceCA.decisions))

    def createHypoTools(self, flags):
        """ set and create HypoTools accumulated on the self.step from an input step configuration
        """        
        if self.sequenceCA.step.combo is None:
            return

        log.debug("CFGroup.createHypoTools for Step %s", self.sequenceCA.step.name)
        for sdict in self.stepDicts:
            for seq, onePartChainDict in zip(self.sequenceCA.step.sequences, sdict):
                log.debug('    seq: %s, onePartChainDict:', seq.name)
                log.debug('    %s', onePartChainDict)
                if not isinstance(seq, EmptyMenuSequence):
                    hypoToolConf=seq.getHypoToolConf()
                    if hypoToolConf is None: # avoid empty sequences
                        log.error("HypoToolConf not found ", seq.name)
                    hypoToolConf.setConf( onePartChainDict )
                    hypo = HypoAlgNode(Alg = self.sequenceCA.ca.getEventAlgo(seq.hypo.Alg.getName()))
                    hypoToolAcc = hypo.addHypoTool(flags, hypoToolConf) #this creates the HypoTools
                    if isinstance(hypoToolAcc, ComponentAccumulator):                   
                        self.sequenceCA.ca.merge(hypoToolAcc)
                   
        for chain,conf in zip(self.chains, self.comboToolConfs):
            chainDict = HLTMenuConfig.getChainDictFromChainName(chain)
            self.sequenceCA.step.combo.createComboHypoTools(flags, chainDict, conf)
