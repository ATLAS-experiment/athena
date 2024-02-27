# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
        self.resetInput()
        self.resetOutput() ## why do we need this in CA mode??

    def addChain(self, name, input_name):
        input_index = self.readInputList().index(input_name)
        chains_in_input = self.getPar("ChainsPerInput")
        if len(chains_in_input) == input_index:
            chains_in_input.append([name])
        elif len(chains_in_input) > input_index:
            chains_in_input[input_index].append(name)
        else:
            log.error("Error: why requiring input %i when size is %i ?" , input_index , len(chains_in_input))
            raise RuntimeError("Error: why requiring input %i when size is %i " , input_index , len(chains_in_input))
            
        self.Alg.ChainsPerInput= chains_in_input
        return self.setPar("Chains", name) # still neded?
        
    def getChains(self):
        return self.getPar("Chains")

    def getChainsPerInput(self):
        return self.getPar("ChainsPerInput")



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
        self.empty= ChainStep.isEmpty
        self.filter = FilterAlg
        self.step = ChainStep
        self.combo = ChainStep.combo  #copy this instance
        self.connectCombo()
        self.setDecisions()                
        log.debug("CFSequence.__init: created %s ",self)

    def setDecisions(self):
        """ Set the output decision of this CFSequence as the hypo outputdecision; In case of combo, takes the Combo outputs"""
        self.decisions=[]
        # empty steps:
        if self.combo is None:
            self.decisions.extend(self.filter.getOutputList())
        else:
            self.decisions.extend(self.combo.getOutputList())            
        log.debug("CFSequence: set out decisions: %s", self.decisions)


    def connect(self, connections):
        """Connect filter to ChainStep (and all its sequences) through these connections (which are sets of filter outputs)
        if a ChainStep contains the same sequence multiple times (for multi-object chains),
        the filter is connected only once (to avoid multiple DH links)
        """
        if log.isEnabledFor(logging.DEBUG):
            log.debug("CFSequence: connect Filter %s with %d menuSequences of step %s, using %d connections", self.filter.Alg.name, len(self.step.sequences), self.step.name, len(connections))
            log.debug("   --- sequences: ")
            for seq in self.step.sequences:
                log.debug(seq)

        if len(connections) == 0:
            log.error("No filter outputs are set!")

        if len(self.step.sequences):
            # check whether the number of filter outputs are the same as the number of sequences in the step
            if len(connections) != len(self.step.sequences):
                log.error("Found %d connections and %d MenuSequences in Step %s", len(connections), len(self.step.sequences), self.step.name)
                raise Exception("[CFSequence] Connections and sequences do not match, this must be fixed!")
            nseq=0
            for seq in self.step.sequences:
                filter_out = connections[nseq]
                log.debug("CFSequence: Found input %s to sequence::%s from Filter::%s", filter_out, seq.name, self.filter.Alg.name)
                seq.connectToFilter( filter_out )
                nseq+=1
        else:
          log.debug("This CFSequence has no sequences: outputs are the Filter outputs, which are %d", len(self.decisions))


    def connectCombo(self):
        """ connect Combo to Hypos"""
        if self.combo is None:
            return

        for seq in self.step.sequences:            
            combo_input=seq.getOutputList()[0]
            self.combo.addInput(combo_input)
            inputs = self.combo.readInputList()
            legindex = inputs.index(combo_input)
            log.debug("CFSequence.connectCombo: adding input to  %s: %s",  self.combo.Alg.getName(), combo_input)
            # inputs are the output decisions of the hypos of the sequences
            combo_output=CFNaming.comboHypoOutputName (self.combo.Alg.getName(), legindex)            
            self.combo.addOutput(combo_output)
            log.debug("CFSequence.connectCombo: adding output to  %s: %s",  self.combo.Alg.getName(), combo_output)

    
    def createHypoTools(self, flags, chain, newstep):
        """ set and create HypoTools accumulated on the self.step from an input step configuration
        """
        if self.combo is None:
            return
        
        acc = ComponentAccumulator()

        assert len(newstep.sequences) == len(self.step.sequences), f'Trying to add HypoTools from new step {newstep.name}, which differ in number of sequences'
        assert len(self.step.sequences) == len(newstep.stepDicts), f'The number of sequences of step {self.step.name} ({len(self.step.sequences)}) differ from the number of dictionaries in the chain {len(newstep.stepDicts)}'
 
        log.debug("createHypoTools for Step %s", newstep.name)
        log.debug('from chain %s with step mult= %d', chain, sum(newstep.multiplicity))
        log.debug("N(seq)=%d, N(chainDicts)=%d", len(newstep.sequences), len(newstep.stepDicts))
        
        for seq, myseq, onePartChainDict in zip(newstep.sequences, self.step.sequences, newstep.stepDicts):
            log.debug('    seq: %s, onePartChainDict:', seq.name)
            log.debug('    %s', onePartChainDict)
            hypoToolConf=seq.getHypoToolConf()
            if hypoToolConf is not None: # avoid empty sequences
                hypoToolConf.setConf( onePartChainDict )
                hypoAcc = myseq.hypo.addHypoTool(flags, hypoToolConf) #this creates the HypoTools
                if isinstance(hypoAcc, ComponentAccumulator):
                    acc.merge(hypoAcc)

        chainDict = HLTMenuConfig.getChainDictFromChainName(chain)
        self.combo.createComboHypoTools(flags, chainDict, newstep.comboToolConfs)
        return acc
    
    def __repr__(self):
        return "--- CFSequence ---\n + Filter: %s \n + decisions: %s\n +  %s \n"%(\
                    self.filter.Alg.name, self.decisions, self.step)


class CFSequenceCA(CFSequence):
    """Class to describe the flow of decisions through ChainStep + filter with their connections (input, output)
    A Filter can have more than one input/output if used in different chains, so this class stores and manages all of them (when doing the connect)
    """
    def __init__(self, chainStep, filterAlg):
        log.debug(" *** Create CFSequence %s with Filter %s", chainStep.name, filterAlg.Alg.getName())
        
        self.empty= chainStep.isEmpty
        self.ca = ComponentAccumulator()
        #empty step: add the PassSequence, one instance only is appended to the tree
        seqAndWithFilter = filterAlg.Alg if self.empty else seqAND(chainStep.name)        
        self.ca.addSequence(seqAndWithFilter)
        self.seq = seqAndWithFilter
        if not self.empty: 
            self.ca.addEventAlgo(filterAlg.Alg, sequenceName=seqAndWithFilter.getName())
            self.stepReco = parOR(chainStep.name + CFNaming.RECO_POSTFIX)  # all reco algorithms from all the sequences in a parallel sequence                            
            self.ca.addSequence(self.stepReco, parentName=seqAndWithFilter.getName())
            log.debug("created parOR %s inside seqAND %s  ", self.stepReco.getName(), seqAndWithFilter.getName())
            self.mergeStepSequences(chainStep)
            
            
        CFSequence.__init__(self, chainStep, filterAlg)
        if not self.empty: 
        # merge the Hypoalg (before the Combo)
            for menuseq in chainStep.sequences:
                if not isinstance(menuseq, EmptyMenuSequence):
                    self.ca.merge(menuseq.hypoAcc, sequenceName=seqAndWithFilter.getName())   

        if self.combo is not None:    
            self.ca.merge(self.step.combo.acc, sequenceName=seqAndWithFilter.getName())          

    def mergeStepSequences(self, chainStep):
        for menuseq in chainStep.sequences:
            try:
                self.ca.merge(menuseq.ca, sequenceName=self.stepReco.getName())
            except Exception as e:
                log.error(f'Failed to merge into {self.stepReco.getName()}')
                raise e
            if menuseq.globalRecoCA:
                self.ca.merge(menuseq.globalRecoCA)

    @lru_cache(None)
    def findComboHypoAlg(self):
        return findAlgorithmByPredicate(self.seq, lambda alg: alg.name == self.step.Alg.name and isComboHypoAlg(alg))


    def createHypoTools(self, flags, chain, newstep):
        """ set and create HypoTools accumulated on the self.step from an input step configuration
        """
        
        if self.step.combo is None:
            return

        assert len(newstep.sequences) == len(self.step.sequences), f'Trying to add HypoTools from new step {newstep.name}, which differ in number of sequences'
        assert len(self.step.sequences) == len(newstep.stepDicts), f'The number of sequences of step {self.step.name} ({len(self.step.sequences)}) differ from the number of dictionaries in the chain {len(newstep.stepDicts)}'
 
        log.debug("createHypoTools for Step %s", newstep.name)
        log.debug('from chain %s with step mult= %d', chain, sum(newstep.multiplicity))
        log.debug("N(seq)=%d, N(chainDicts)=%d", len(newstep.sequences), len(newstep.stepDicts))
        
        for seq, myseq, onePartChainDict in zip(newstep.sequences, self.step.sequences, newstep.stepDicts):
            log.debug('    seq: %s, onePartChainDict:', seq.name)
            log.debug('    %s', onePartChainDict)
            hypoToolConf=seq.getHypoToolConf()
            if hypoToolConf is not None: # avoid empty sequences
                hypoToolConf.setConf( onePartChainDict )
                hypo = HypoAlgNode(Alg = self.ca.getEventAlgo(myseq.hypo.Alg.getName()))
                hypoToolAcc = hypo.addHypoTool(flags, hypoToolConf) #this creates the HypoTools
                if isinstance(hypoToolAcc, ComponentAccumulator):                   
                    self.ca.merge(hypoToolAcc)
                   

        chainDict = HLTMenuConfig.getChainDictFromChainName(chain)
        self.combo.createComboHypoTools(flags, chainDict, newstep.comboToolConfs)      
