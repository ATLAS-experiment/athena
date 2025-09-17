# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFTools import (NoHypoToolCreated, 
                                                             algColor, 
                                                             isHypoBase,
                                                             isInputMakerBase)
from AthenaCommon.CFElements import parOR, seqAND, findAlgorithmByPredicate
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching

from collections.abc import MutableSequence
import functools
import re

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )
# Pool of mutable ComboHypo instances (FIXME: ATR-29181)
_ComboHypoPool = dict()
_CustomComboHypoAllowed = set()

class Node(object):
    """base class representing one Alg + inputs + outputs, to be used to connect """
    """stores all the inputs, even if repeated (self.inputs)"""
    def __init__(self, Alg):
        self.name = ("%sNode")%( Alg.getName() )
        self.Alg=Alg
        self.inputs=[]
        self.outputs=[]

    def addOutput(self, name):
        self.outputs.append(str(name))

    def addInput(self, name):
        self.inputs.append(str(name)) 

    def getOutputList(self):
        return self.outputs

    def getInputList(self):
        return self.inputs

    def __repr__(self):
        return "Node::%s  [%s] -> [%s]"%(self.Alg.getName(), ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))


class AlgNode(Node):
    """Node class that represent an algorithm: sets R/W handles (as unique input/output) and properties as parameters  """
    """Automatically de-duplicates input ReadHandles upon repeated calls to addInput."""
    def __init__(self, Alg, inputProp, outputProp):
        Node.__init__(self, Alg)
        self.outputProp = outputProp
        self.inputProp = inputProp

    def setPar(self, propname, value):
        cval = getattr( self.Alg, propname)
        if isinstance(cval, MutableSequence):
            cval.append(value)
            return setattr(self.Alg, propname, cval)
        else:
            return setattr(self.Alg, propname, value)

    def addOutput(self, name):
        outputs = self.readOutputList()
        if name in outputs:
            log.debug("Output DH not added in %s: %s already set!", self.Alg.getName(), name)
        else:
            if self.outputProp != '':
                self.setPar(self.outputProp, name)
            else:
                log.debug("no outputProp set for output of %s", self.Alg.getName())
        Node.addOutput(self, name)

    def readOutputList(self):
        cval = getattr(self.Alg, self.outputProp)
        return (cval if isinstance(cval, MutableSequence) else
                ([str(cval)] if cval else []))

    def addInput(self, name):
        inputs = self.readInputList()        
        if name in inputs:
            log.debug("Input DH not added in %s: %s already set!", self.Alg.getName(), name)
        else:
            if self.inputProp != '':
                self.setPar(self.inputProp, name)
            else:
                log.debug("no InputProp set for input of %s", self.Alg.getName())
        Node.addInput(self, name)
        return len(self.readInputList())

    def readInputList(self):
        cval = getattr(self.Alg, self.inputProp)
        return (cval if isinstance(cval, MutableSequence) else
                ([str(cval)] if cval else []))

    def __repr__(self):
        return "Alg::%s  [%s] -> [%s]"%(self.Alg.getName(), ' '.join(map(str, self.getInputList())), ' '.join(map(str, self.getOutputList())))


class HypoToolConf:
    """ Class to group info on hypotools for ChainDict"""
    def __init__(self, hypoToolGen):
        self.hypoToolGen = hypoToolGen
        self.name=hypoToolGen.__name__

    def setConf( self, chainDict):
        if type(chainDict) is not dict:
            raise RuntimeError("Configuring hypo with %s, not good anymore, use chainDict" % str(chainDict) )
        self.chainDict = chainDict

    def create(self, flags):
        """creates instance of the hypo tool"""
        return self.hypoToolGen( flags, self.chainDict )

    def confAndCreate(self, flags, chainDict):
        """sets the configuration and creates instance of the hypo tool"""
        self.setConf(chainDict)
        return self.create(flags)


class HypoAlgNode(AlgNode):
    """AlgNode for HypoAlgs"""
    initialOutput= 'StoreGateSvc+UNSPECIFIED_OUTPUT'
    def __init__(self, Alg):
        assert isHypoBase(Alg), "Error in creating HypoAlgNode from Alg "  + Alg.name
        AlgNode.__init__(self, Alg, 'HypoInputDecisions', 'HypoOutputDecisions')
        self.previous=[]

    def addOutput(self, name):
        outputs = self.readOutputList()
        if name in outputs:
            log.debug("Output DH not added in %s: %s already set!", self.name, name)
        elif self.initialOutput in outputs:
            AlgNode.addOutput(self, name)
        else:
            log.error("Hypo %s has already %s as configured output: you may want to duplicate the Hypo!",
                      self.name, outputs[0])

    def addHypoTool (self, flags, hypoToolConf):
        log.debug("Adding HypoTool %s for chain %s to %s", hypoToolConf.name, hypoToolConf.chainDict['chainName'], self.Alg.getName())        
        try:
            result = hypoToolConf.create(flags)
            if isinstance(result, ComponentAccumulator):
                tool = result.popPrivateTools()
                assert not isinstance(tool, list), "Can not handle list of tools"
                self.Alg.HypoTools.append(tool)
                return result
            else:
                self.Alg.HypoTools = self.Alg.HypoTools + [result]  # see ATEAM-773

        except NoHypoToolCreated as e:
            log.debug("%s returned empty tool: %s", hypoToolConf.name, e)
        return None
    
    def setPreviousDecision(self,prev):
        self.previous.append(prev)
        return self.addInput(prev)

    def __repr__(self):
        return "HypoAlg::%s  [%s] -> [%s], previous = [%s], HypoTools=[%s]" % \
            (self.Alg.name,' '.join(map(str, self.getInputList())),
             ' '.join(map(str, self.getOutputList())),
             ' '.join(map(str, self.previous)),
             ' '.join([t.getName() for t in self.Alg.HypoTools]))


class InputMakerNode(AlgNode):
    """AlgNode for InputMaker Algs"""
    def __init__(self, Alg):
        assert isInputMakerBase(Alg), "Error in creating InputMakerNode from Alg "  + Alg.name
        AlgNode.__init__(self,  Alg, 'InputMakerInputDecisions', 'InputMakerOutputDecisions')
        input_maker_output = CFNaming.inputMakerOutName(self.Alg.name)
        self.addOutput(input_maker_output)


class ComboHypoNode(AlgNode):
    """AlgNode for Combo HypoAlgs"""
    def __init__(self, name, comboHypoCfg):
        self.comboHypoCfg = comboHypoCfg
        self.acc = self.create( name )        
        thealgs= self.acc.getEventAlgos()
        if thealgs is None:
            log.error("ComboHypoNode: Combo alg %s not found", name)
        if len(thealgs) != 1:
            log.error("ComboHypoNode: Combo alg %s len is %d",name, len(thealgs))
        Alg=thealgs[0]

        log.debug("ComboHypoNode init: Alg %s", name)
        AlgNode.__init__(self,  Alg, 'HypoInputDecisions', 'HypoOutputDecisions')

    def __del__(self):
        self.acc.wasMerged()

    def create (self, name):
        log.debug("ComboHypoNode.create %s",name)
        return self.comboHypoCfg(name=name)

    """
    AlgNode automatically de-duplicates input ReadHandles upon repeated calls to addInput.
    Node instead stores all the inputs, even if repeated (self.inputs)
    This function maps from the raw number of times that addInput was called to the de-duplicated index of the handle.
    E.g. a step processing chains such as HLT_e5_mu6 would return [0,1]
    E.g. a step processing chains such as HLT_e5_e6 would return [0,0]
    E.g. a step processing chains such as HLT_e5_mu6_mu7 would return [0,1,1]
    These data are needed to configure the step's ComboHypo
    """
    def mapRawInputsToInputsIndex(self):
        mapping = []        
        theInputs = self.readInputList() #only unique inputs    
        for rawInput in self.inputs: # all inputs
            mapping.append( theInputs.index(rawInput) )
        return mapping


    def addChain(self, chainDict):        
        chainName = chainDict['chainName']
        chainMult = chainDict['chainMultiplicities']        
        legsToInputCollections = self.mapRawInputsToInputsIndex()                
        if len(chainMult) != len(legsToInputCollections):
            log.error("ComboHypoNode for Alg:{} with addChain for:{} Chain multiplicity:{} Per leg input collection index:{}."
                .format(self.Alg.name, chainName, tuple(chainMult), tuple(legsToInputCollections)))
            log.error("The size of the multiplicies vector must be the same size as the per leg input collection vector.")
            log.error("The ComboHypo needs to know which input DecisionContainers contain the DecisionObjects to be used for each leg.")
            log.error("Check why ComboHypoNode.addInput(...) was not called exactly once per leg.")
            raise Exception("[createDataFlow] Error in ComboHypoNode.addChain. Cannot proceed.")

        if chainName in self.Alg.MultiplicitiesMap:
            log.error("ComboAlg %s has already been configured for chain %s", self.Alg.name, chainName)
            raise Exception("[createDataFlow] Error in ComboHypoNode.addChain. Cannot proceed.")
        else:
            self.Alg.MultiplicitiesMap[chainName] = chainMult
            self.Alg.LegToInputCollectionMap[chainName] = legsToInputCollections


    def getChains(self):
        return self.Alg.MultiplicitiesMap.keys()


    def createComboHypoTools(self, flags, chainDict, comboToolConfs):
         """Create the ComboHypoTools and add them to the main alg"""
         if not len(comboToolConfs):
             return
         confs = [ HypoToolConf( tool ) for tool in comboToolConfs ]
         log.debug("ComboHypoNode.createComboHypoTools for chain %s, Alg %s with %d tools", chainDict["chainName"],self.Alg.getName(), len(comboToolConfs))        
         for conf in confs:
             log.debug("ComboHypoNode.createComboHypoTools adding %s", conf)
             tools = self.Alg.ComboHypoTools
             self.Alg.ComboHypoTools = tools + [ conf.confAndCreate( flags, chainDict ) ]
 

##########################################################
# Now sequences and chains
##########################################################

class EmptyMenuSequence:
    """Class to emulate reco sequences with no Hypo"""
    """It contains an InputMaker and and empty seqAND used for merging"""
    """It contains empty function to follow the same MenuSequence behaviour"""
    def __init__(self, the_name):
        log.debug("Made EmptySequence %s", the_name)
        self._name = the_name

        # isEmptyStep causes the IM to try at runtime to merge by feature by default
        # (i.e for empty steps appended after a leg has finised). But if this failes then it will
        # merge by initial ROI instead (i.e. for empy steps prepended before a leg has started)
        makerAlg = CompFactory.InputMakerForRoI(f"IM{the_name}",
                                                isEmptyStep = True,
                                                RoIsLink = 'initialRoI')

        self._maker       = InputMakerNode( Alg = makerAlg )
        self._sequence    = Node( Alg = seqAND(the_name, [makerAlg]))

        self.ca = ComponentAccumulator()
        self.ca.addSequence(seqAND(the_name))
        self.ca.addEventAlgo(makerAlg, sequenceName=the_name)

    def __del__(self):
        self.ca.wasMerged()

    @property
    def sequence(self):
        return self._sequence

    @property
    def maker(self):
        # Input makers are added during DataFlow building (connectToFilter) when a chain
        # uses this sequence in another step. So we need to make sure to update the
        # algorithm when accessed.
        self._maker.Alg = self.ca.getEventAlgo(self._maker.Alg.name)
        return self._maker

    @property
    def name(self):
        return self._name

    def getOutputList(self):
        return self.maker.readOutputList() # Only one since it's merged

    def connectToFilter(self, outfilter):
        """Connect filter to the InputMaker"""
        self.maker.addInput(outfilter)

    def getHypoToolConf(self):
        return None                     
        
    def buildDFDot(self, cfseq_algs, all_hypos, last_step_hypo_nodes, file):
        cfseq_algs.append(self.maker)
        cfseq_algs.append(self.sequence )
        file.write("    %s[fillcolor=%s]\n"%(self.maker.Alg.getName(), algColor(self.maker.Alg)))
        file.write("    %s[fillcolor=%s]\n"%(self.sequence.Alg.getName(), algColor(self.sequence.Alg)))
        return cfseq_algs, all_hypos, last_step_hypo_nodes

    def __repr__(self):
        return "MenuSequence::%s \n Hypo::%s \n Maker::%s \n Sequence::%s \n HypoTool::%s\n"\
            %(self.name, "Empty", self.maker.Alg.getName(), self.sequence.Alg.getName(), "None")

def createEmptyMenuSequenceCfg(flags, name):
    """ creates the generator function named as the empty sequence"""
    def create_sequence(flags, name):  
        return EmptyMenuSequence(name)
    # this allows to create the function with the same name as the sequence   
    create_sequence.__name__ = name
    globals()[name] = create_sequence
    return globals()[name]


def isEmptySequenceCfg(o):
    return 'Empty' in o.func.__name__

class MenuSequence:
    """Class to group reco sequences with the Hypo.
    By construction it has one Hypo only, which gives the name to this class object"""

    def __init__(self, flags, selectionCA, HypoToolGen):
        self.ca = selectionCA
        # separate the HypoCA to be merged later
        self.hypoAcc  = selectionCA.hypoAcc

        sequence = self.ca.topSequence()
        self._sequence = Node(Alg=sequence)

        #  get the InputMaker
        inputMaker = [ a for a in self.ca.getEventAlgos() if isInputMakerBase(a)]
        assert len(inputMaker) == 1, f"{len(inputMaker)} input makers in the ComponentAccumulator"
        inputMaker = inputMaker[0]
        assert inputMaker.name.startswith("IM"), f"Input maker {inputMaker.name} name needs to start with 'IM'"
        self._maker = InputMakerNode( Alg = inputMaker )
        input_maker_output = self.maker.readOutputList()[0] # only one since it's merged
        

        # get the HypoAlg
        hypoAlg = selectionCA.hypoAcc.getEventAlgos()
        assert len(hypoAlg) == 1, f"{len(hypoAlg)} hypo algs in the ComponentAccumulator"
        hypoAlg = hypoAlg[0]
        hypoAlg.RuntimeValidation = flags.Trigger.doRuntimeNaviVal

        self._name = CFNaming.menuSequenceName(hypoAlg.name)
        self._hypo = HypoAlgNode( Alg = hypoAlg )
        self._hypo.addOutput( CFNaming.hypoAlgOutName(hypoAlg.name) )
        self._hypo.setPreviousDecision( input_maker_output )
        self._hypoToolConf = HypoToolConf( HypoToolGen )

        # Connect InputMaker output to ROBPrefetchingAlg(s) if there is any
        if ROBPrefetching.StepRoI in flags.Trigger.ROBPrefetchingOptions:
            for child in sequence.Members:
                if ( isinstance(child, CompFactory.ROBPrefetchingAlg) and
                     input_maker_output not in child.ROBPrefetchingInputDecisions ):
                    child.ROBPrefetchingInputDecisions.append(input_maker_output)

        log.debug("connecting InputMaker and HypoAlg, adding: InputMaker::%s.output=%s",
                  self.maker.Alg.name, input_maker_output)
        log.debug("HypoAlg::%s.HypoInputDecisions=%s, HypoAlg::%s.HypoOutputDecisions=%s",
                  self.hypo.Alg.name, self.hypo.readInputList()[0],
                  self.hypo.Alg.name, self.hypo.readOutputList()[0])

    def __del__(self):
        self.ca.wasMerged()
        self.hypoAcc.wasMerged()

    @property
    def name(self):
        return self._name

    @property
    def sequence(self):
        return self._sequence

    @property
    def maker(self):
        # Input makers are added during DataFlow building (connectToFilter) when a chain
        # uses this sequence in another step. So we need to make sure to update the
        # algorithm when accessed.
        self._maker.Alg = self.ca.getEventAlgo(self._maker.Alg.name)
        return self._maker

    @property
    def hypo(self):
        return self._hypo

    def getOutputList(self):
        return [self._hypo.readOutputList()[0]]

    def connectToFilter(self, outfilter):
        """Connect filter to the InputMaker"""
        log.debug("connectToFilter: connecting %s to inputs of %s", outfilter, self.maker.Alg.name)
        self.maker.addInput(outfilter)
          
    def getHypoToolConf(self) :
        return self._hypoToolConf

            
    def buildDFDot(self, cfseq_algs, all_hypos, last_step_hypo_nodes, file):
        cfseq_algs.append(self.maker)
        cfseq_algs.append(self.sequence)
        file.write("    %s[fillcolor=%s]\n"%(self.maker.Alg.getName(), algColor(self.maker.Alg)))
        file.write("    %s[fillcolor=%s]\n"%(self.sequence.Alg.getName(), algColor(self.sequence.Alg)))    
        cfseq_algs.append(self._hypo)
        file.write("    %s[color=%s]\n"%(self._hypo.Alg.getName(), algColor(self._hypo.Alg)))
        all_hypos.append(self._hypo)
        return cfseq_algs, all_hypos, last_step_hypo_nodes

    def __repr__(self):    
        hyponame = self._hypo.Alg.name
        hypotool = self._hypoToolConf.name
        return "MenuSequence::%s \n Hypo::%s \n Maker::%s \n Sequence::%s \n HypoTool::%s\n"\
          %(self.name, hyponame, self.maker.Alg.name, self.sequence.Alg.name, hypotool)


class Chain(object):
    """Basic class to define the trigger menu """
    __slots__ ='name','steps','nSteps','alignmentGroups','L1decisions', 'topoMap'
    def __init__(self, name, ChainSteps, L1decisions, nSteps = None, alignmentGroups = None, topoMap=None):
 
        """
        Construct the Chain from the steps
        Out of all arguments the ChainSteps & L1Thresholds are most relevant, the chain name is used in debug messages
        """
        
        # default mutable values must be initialized to None
        if nSteps is None:  nSteps = []  
        if alignmentGroups is None:  alignmentGroups = []

        self.name   = name
        self.steps  = ChainSteps
        self.nSteps = nSteps
        self.alignmentGroups = alignmentGroups
       
        
        # The chain holds a map of topo ComboHypoTool configurators
        # This is needed to allow placement of the ComboHypoTool in the right position
        # for multi-leg chains (defaults to last step)
        # Format is {"[step name]" : ([topo config function], [topo descriptor string]), ...}
        # Here, the topo descriptor string would usually be the chain name expression that
        # configures the topo
        self.topoMap = {}
        if topoMap:
            self.topoMap.update(topoMap)

        # L1decisions are used to set the seed type (EM, MU,JET), removing the actual threshold
        # in practice it is the HLTSeeding Decision output
        self.L1decisions = L1decisions 
        log.debug("[Chain.__init__] Made Chain %s with seeds: %s ", name, self.L1decisions)

    def append_bjet_steps(self,new_steps):
        assert len(self.nSteps) == 1, "[Chain.append_bjet_steps] appending already-merged step lists - chain object will be broken. This should only be used to append Bjets to jets!"
        self.steps = self.steps + new_steps
        self.nSteps = [len(self.steps)]

    def append_step_to_jet(self,new_steps):
        assert len(self.nSteps) == 1, "[Chain.append_step_to_jet] appending already-merged step lists - chain object will be broken. This is used either for appending Beamspot algorithms to jets!"
        self.steps = self.steps + new_steps
        self.nSteps = [len(self.steps)]


    def numberAllSteps(self):
        if len(self.steps)==0:
            return
        else:
            for stepID,step in enumerate(self.steps):
                step_name = step.name
                if re.search('^Step[0-9]_',step_name):
                    step_name = step_name[6:]
                elif re.search('^Step[0-9]{2}_', step_name):
                    step_name = step_name[7:]   
                step.name = 'Step%d_'%(stepID+1)+step_name
                # also modify the empty sequence names to follow the step name change
                for iseq, seq in enumerate(step.sequenceGens):
                    if isEmptySequenceCfg(seq): 
                        name = seq.func.__name__ 
                        if re.search('Seq[0-9]_',name):
                            newname = re.sub('Seq[0-9]_', 'Seq%d_'%(stepID+1), name)
                            #replace the empty sequence        
                            thisEmpty = createEmptyMenuSequenceCfg(flags=None, name=newname)                
                            step.sequenceGens[iseq]=functools.partial(thisEmpty, flags=None, name=newname)
        return


    def insertEmptySteps(self, empty_step_name, n_new_steps, start_position):
        #start position indexed from 0. if start position is 3 and length is 2, it works like:
        # [old1,old2,old3,old4,old5,old6] ==> [old1,old2,old3,empty1,empty2,old4,old5,old6]

        if len(self.steps) == 0 :
            log.error("I can't insert empty steps because the chain doesn't have any steps yet!")

        if len(self.steps) < start_position :
            log.error("I can't insert empty steps at step %d because the chain doesn't have that many steps!", start_position)

        
        chain_steps_pre_split = self.steps[:start_position]
        chain_steps_post_split = self.steps[start_position:]

        next_step_name = ''
        prev_step_name = ''
        # copy the same dictionary as the last step, which else?
        prev_chain_dict = []
        if start_position == 0:
            next_step_name = chain_steps_post_split[0].name
            if re.search('^Step[0-9]_',next_step_name):
                next_step_name = next_step_name[6:]
            elif re.search('^Step[0-9]{2}_', next_step_name):
                next_step_name = next_step_name[7:]

            prev_step_name = 'empty_'+str(len(self.L1decisions))+'L1in'
            prev_chain_dict = chain_steps_post_split[0].stepDicts
        else:
            if len(chain_steps_post_split) == 0:
                log.error("Adding empty steps to the end of a chain (%s)- why would you do this?",self.name)
            else:
                prev_step_name = chain_steps_pre_split[-1].name
                next_step_name = chain_steps_post_split[0].name
            prev_chain_dict = chain_steps_pre_split[-1].stepDicts


        steps_to_add = []
        for stepID in range(1,n_new_steps+1):
            new_step_name =  prev_step_name+'_'+empty_step_name+'%d_'%stepID+next_step_name

            log.debug("Adding empty step %s", new_step_name)
            steps_to_add += [ChainStep(new_step_name, chainDicts=prev_chain_dict, isEmpty=True)]
        
        self.steps = chain_steps_pre_split + steps_to_add + chain_steps_post_split

        return

    def checkNumberOfLegs(self):
        """ return 0 if the chain has unexpected number of step legs """
        if len(self.steps) == 0: # skip if it's noAlg chains            
            return 1

        mult=[step.nLegs for step in self.steps] # one nLegs per step
        not_empty_mult = [m for m in mult if m!=0]
        # cannot accept chains with all empty steps 
        if len(not_empty_mult) == 0: 
            log.error("checkNumberOfLegs: Chain %s has all steps with nLegs =0: what to do?", self.name)
            return 0

        # cannot accept chains with steps with different number of legs
        if not_empty_mult.count(not_empty_mult[0]) != len(not_empty_mult):
            log.error("checkNumberOfLegs: Chain %s has steps with differnt number of legs: %s", self.name, ' '.join(mult))
            return 0

        # check that the chain number of legs is the same as the number of L1 seeds
        if not_empty_mult[0] != len(self.L1decisions):
            log.error("checkNumberOfLegs: Chain %s has %i legs per step, and %d L1Decisions", self.name, mult, len(self.L1decisions))            
            return 0
        return not_empty_mult[0]
    
    
    # Receives a pair with the topo config function and an identifier string,
    # optionally also a target step name
    # The string is needed to rename the step after addition of the ComboHypoTool
    def addTopo(self,topoPair,step="last"):
        stepname = "last step" if step=="last" else step.name
        log.debug("Adding topo configurator %s for %s to %s", topoPair[0].__qualname__, topoPair[1], "step " + stepname)
        self.topoMap[step] = topoPair

    def __str__(self):
        return "\n-*- Chain %s -*- \n + Seeds: %s, Steps: %s, AlignmentGroups: %s "%(\
                    self.name, ' '.join(map(str, self.L1decisions)), self.nSteps, self.alignmentGroups)     

    def __repr__(self):
        return "\n-*- Chain %s -*- \n + Seeds: %s, Steps: %s, AlignmentGroups: %s \n + Steps: \n %s \n"%(\
                    self.name, ' '.join(map(str, self.L1decisions)), self.nSteps, self.alignmentGroups, '\n '.join(map(str, self.steps)))       
        


class ChainStep(object):
    """ Class to describe one step of a chain; 
    a step is described by a list of ChainDicts and a list of sequence generators;
    there is one leg per ChainDict;
    a step can have one leg (single) or more legs (combined);
    not-empty steps have one sequence per leg;
    empty steps have zero sequences, while chainDict len is not zero; 
    legID is taken from the ChainDict;
    """
    
    def __init__(self, name,  SequenceGens = None, chainDicts = None, comboHypoCfg = None, comboToolConfs = None, isEmpty = False, createsGhostLegs = False):

        # default mutable values must be initialized to None
        if SequenceGens is None:  SequenceGens = []
        if comboHypoCfg is None: comboHypoCfg = functools.partial(ComboHypoCfg)
        if comboToolConfs is None: comboToolConfs = []
        assert chainDicts is not None,"Error building a ChainStep without chainDicts"

        self.name = name
        self.sequences = []
        self.sequenceGens = SequenceGens 
        if not isinstance(comboHypoCfg, functools.partial):             
            raise RuntimeError("[ChainStep] Tried to configure a ChainStep %s with ComboHypo %s that is not a function" % (name, comboHypoCfg) ) 
        
        self.comboHypoCfg = comboHypoCfg
        self.comboToolConfs = list(comboToolConfs)       
        self.stepDicts = chainDicts # one dict per leg        
        self.nLegs = len(self.stepDicts) # cannot be zero
        self.isEmpty = isEmpty                
        
        # sanity check on inputs, excluding empty steps 
        if not self.isEmpty:                     
            log.debug("Building step %s for chain %s: n.sequences=%d, nLegs=%i", name, chainDicts[0]['chainName'], len (self.sequenceGens) , self.nLegs )             
            if len (self.sequenceGens)  != self.nLegs: 
                log.error("[ChainStep] SequenceGens: %s",self.sequenceGens)
                log.error("[ChainStep] stepDicts: %s",self.stepDicts)
                log.error("[ChainStep] n.legs: %i",self.nLegs)
                raise RuntimeError("[ChainStep] Tried to configure a ChainStep %s with %i legs and %i sequences. These lists must have the same size" % (name, self.nLegs, len (self.sequenceGens) ) )
                        
           
        for iseq, seq in enumerate(self.sequenceGens):              
            if not isinstance(seq, functools.partial):
                log.error("[ChainStep] %s SequenceGens verification failed, sequence %d is not partial function, likely ChainBase.getStep function was not used", self.name, iseq)
                log.error("[ChainStep] It rather seems to be of type %s trying to print it", type(seq))
                raise RuntimeError("Sequence is not packaged in a tuple, see error message above" ) 
                                                 
        self.onlyJets  = False
        sig_set = None
        if 'signature' in chainDicts[0]:             
            sig_set = set([step['signature'] for step in chainDicts])
            if len(sig_set) == 1 and ('Jet' in sig_set or 'Bjet' in sig_set):
                self.onlyJets = True
            if len(sig_set) == 2 and ('Jet' in sig_set and 'Bjet' in sig_set):
                self.onlyJets = True

        
        
        if not self.isEmpty:
            self.setChainPartIndices()
        self.makeCombo()

    def createSequences(self):
        """ creation of this step sequences with instantiation of the CAs"""        
        log.debug("createSequences: creating %d sequences for step %s", len(self.sequenceGens), self.name)
        for seq in self.sequenceGens:
            log.debug("createSequences: creating sequence %s", seq.func.__name__)
            self.sequences.append(seq()) # create the sequences         
            
    
    #Heather updated for full jet chain dicts
    def setChainPartIndices(self):    
        leg_counter = 0
        lists_of_chainPartNames = []
        for step_dict in self.stepDicts:
            if len(lists_of_chainPartNames) == 0:
                lists_of_chainPartNames += [[cp['chainPartName'] for cp in step_dict['chainParts']]]
            else:
                new_list_of_chainPartNames = [cp['chainPartName'] for cp in step_dict['chainParts']]
                if new_list_of_chainPartNames == lists_of_chainPartNames[-1]:
                    leg_counter -= len(new_list_of_chainPartNames)
            for chainPart in step_dict['chainParts']:
                chainPart['chainPartIndex'] =  leg_counter
                leg_counter += 1
        return


    def addComboHypoTools(self, tool):
        #this function does not add tools, it just adds one tool. do not pass it a list!
        self.comboToolConfs.append(tool)

    def getComboHypoFncName(self):
        return self.comboHypoCfg.func.__name__ 



    def makeCombo(self):
        """ Configure the Combo Hypo Alg and generate the corresponding function, without instantiation which is done in createSequences() """ 
        self.combo = None
        if self.isEmpty:
            return        
        comboNameFromStep = CFNaming.comboHypoName(self.name) # name expected from the step name
        funcName = self.getComboHypoFncName() # name of the function generator
        key = hash((comboNameFromStep, funcName))
        if key not in _ComboHypoPool:            
            tmpCombo = ComboHypoNode(comboNameFromStep, self.comboHypoCfg) 
            CHname = tmpCombo.name[:-4]   # remove 'Node'
            # exceptions for BLS chains that re-use the same custom CH in differnt steps
            # this breaks the run-one-CH-per-step, but the BLS CH are able to handle decisions internally
            if comboNameFromStep != CHname:
                log.debug("Created ComboHypo with name %s, expected from the step is instead %s. This is accepted only for allowed custom ComboHypos", CHname, comboNameFromStep)
                _CustomComboHypoAllowed.add(CHname)
                key = hash((CHname, funcName))
            _ComboHypoPool[key] = tmpCombo
        self.combo = _ComboHypoPool[key] 
        log.debug("Created combo %s with name %s, step comboName %s, key %s", funcName, self.combo.name, comboNameFromStep,key)


    def createComboHypoTools(self, flags, chainName):
        chainDict = HLTMenuConfig.getChainDictFromChainName(chainName)
        self.combo.createComboHypoTools(flags, chainDict, self.comboToolConfs)
            
    def getChainLegs(self):
        """ This is extrapolating the chain legs from the step dictionaries"""       
        legs = [part['chainName'] for part in self.stepDicts]
        return legs

    def getChainNames(self):  
        if self.combo is not None:   
            return list(self.combo.getChains())
        return self.getChainLegs()

    def __repr__(self):
        if len(self.sequenceGens) == 0:        
            return "\n--- ChainStep %s ---\n is Empty, ChainDict = %s "%(self.name,  ' '.join(map(str, [dic['chainName'] for dic in self.stepDicts])) )
        
        repr_string= "\n--- ChainStep %s ---\n , nLegs = %s  ChainDict = %s \n + MenuSequenceGens = %s "%\
          (self.name,  self.nLegs, 
             ' '.join(map(str, [dic['chainName'] for dic in self.stepDicts])),
             ' '.join(map(str, [seq.func.__name__ for seq in self.sequenceGens]) ))
             
        if self.combo is not None:
            repr_string += "\n + ComboHypo = %s" % self.combo.Alg.name
            if len(self.comboToolConfs)>0:
                repr_string +=",  ComboHypoTools = %s" %(' '.join(map(str, [tool.__name__ for tool in self.comboToolConfs]))) 
        repr_string += "\n"       
        return repr_string


class InEventRecoCA( ComponentAccumulator ):
    """ Class to handle in-event reco """
    def __init__(self, name, inputMaker=None, **inputMakerArgs):
        super( InEventRecoCA, self ).__init__()
        self.name = name
        self.recoSeq = None

        if inputMaker:
            assert len(inputMakerArgs) == 0, "No support for explicitly passed input maker and and input maker arguments at the same time" 
            self.inputMakerAlg = inputMaker
        else:
            assert 'name' not in inputMakerArgs, "The name of input maker is predefined by the name of sequence"
            args = {'name': "IM"+name,
                    'RoIsLink' : 'initialRoI',
                    'RoIs' : f'{name}RoIs',
                    'RoITool': CompFactory.ViewCreatorInitialROITool(),
                    'mergeUsingFeature': False}
            args.update(**inputMakerArgs)
            self.inputMakerAlg = CompFactory.InputMakerForRoI(**args)
                
    def addRecoSequence(self):
        if self.recoSeq is None:
            self.recoSeq = parOR( self.name )
            self.addSequence( self.recoSeq )

    def mergeReco( self, ca ):
        """ Merged CA moving reconstruction algorithms into the right sequence """ 
        self.addRecoSequence()       
        return self.merge( ca, sequenceName=self.recoSeq.name )

    def addRecoAlgo( self, algo ):
        """ Place algorithm in the correct reconstruction sequence """
        self.addRecoSequence()  
        return self.addEventAlgo( algo, sequenceName=self.recoSeq.name )

    def inputMaker( self ):
        return self.inputMakerAlg



class InViewRecoCA(ComponentAccumulator):
    """ Class to handle in-view reco, sets up the View maker if not provided and exposes InputMaker so that more inputs to it can be added in the process of assembling the menu """
    def __init__(self, name, viewMaker=None, isProbe=False, **viewMakerArgs):
        super( InViewRecoCA, self ).__init__()
        self.name = name +"_probe" if isProbe else name
        def updateHandle(baseTool, probeTool, handleName):
            if hasattr(baseTool, handleName) and getattr(baseTool, handleName).Path!="StoreGateSvc+":
                setattr(probeTool, handleName, getattr(probeTool, handleName).Path + "_probe")

        if len(viewMakerArgs) != 0:
            assert viewMaker is None, "No support for explicitly passed view maker and args for EventViewCreatorAlgorithm" 

        if viewMaker:
            assert len(viewMakerArgs) == 0, "No support for explicitly passed view maker and args for EventViewCreatorAlgorithm" 
            if isProbe:
                self.viewMakerAlg = viewMaker.__class__(viewMaker.getName()+'_probe', **viewMaker._properties)
                self.viewMakerAlg.Views = viewMaker.Views+'_probe'
                roiTool = self.viewMakerAlg.RoITool.__class.__(self.viewMakerAlg.RoITool.getName()+'_probe', **self.viewMakerAlg.RoITool._properties)
                log.debug(f"InViewRecoCA: Setting InputCachedViews on {self.viewMaker.getName()} to read decisions from tag leg {viewMaker.getName()}: {viewMaker.InputMakerOutputDecisions}")
                self.viewMakerAlg.InputCachedViews = viewMaker.InputMakerOutputDecisions
                updateHandle(viewMakerArgs['RoITool'], roiTool, "RoisWriteHandleKey")
                if hasattr(viewMakerArgs['RoITool'], "RoiCreator"):
                    updateHandle(viewMakerArgs['RoITool'], roiTool, "ExtraPrefetchRoIsKey")
                    updateHandle(viewMakerArgs['RoITool'].RoiCreator, roiTool.RoiCreator, "RoisWriteHandleKey")

                self.viewMakerAlg.RoITool = roiTool
            else:
                self.viewMakerAlg = viewMaker
        else:
            assert 'name' not in viewMakerArgs, "The name of view maker is predefined by the name of sequence"
            assert 'Views' not in viewMakerArgs, "The Views is predefined by the name of sequence"
            assert 'ViewsNodeName' not in viewMakerArgs, "The ViewsNodeName is predefined by the name of sequence"
            if 'RoITool' in viewMakerArgs:
                roiTool = viewMakerArgs['RoITool']
            else:
                roiTool = CompFactory.ViewCreatorInitialROITool()


            args = {'name': f'IM_{self.name}', 
                    'ViewFallThrough'   : True,
                    'RoIsLink'          : 'initialRoI',
                    'RoITool'           : roiTool,
                    'InViewRoIs'        : f'{name}RoIs',
                    'Views'             : f'{name}Views'+'_probe' if isProbe else f'{name}Views',
                    'ViewNodeName'      : f'{name}InViews'+'_probe' if isProbe else f'{name}InViews',
                    'RequireParentView' : False,
                    'mergeUsingFeature' : False }
            args.update(**viewMakerArgs)
            self.viewMakerAlg = CompFactory.EventViewCreatorAlgorithm(**args)
            if isProbe:
                updateHandle(args['RoITool'], roiTool, "RoisWriteHandleKey")
                if hasattr(args['RoITool'], "RoiCreator"):
                    updateHandle(args['RoITool'], roiTool, "ExtraPrefetchRoIsKey")
                    updateHandle(args['RoITool'].RoiCreator, roiTool.RoiCreator, "RoisWriteHandleKey")
        self.viewsSeq = parOR( self.viewMakerAlg.ViewNodeName )
        self.addSequence( self.viewsSeq )

    def mergeReco( self, ca ):
        """ Merge CA moving reconstruction algorithms into the right sequence """
        return self.merge( ca, sequenceName=self.viewsSeq.name )


    def addRecoAlgo( self, algo ):
        """ Place algorithm in the correct reconstruction sequence """
        return self.addEventAlgo( algo, sequenceName=self.viewsSeq.name )


    def inputMaker( self ):
        return self.viewMakerAlg


class SelectionCA(ComponentAccumulator):
    """ CA component for MenuSequence sequence """
    def __init__(self, name, isProbe=False):
        self.name = name+"_probe" if isProbe else name        
        self.isProbe=isProbe
        super( SelectionCA, self ).__init__()   

        self.stepViewSequence = seqAND(self.name)
        self.hypoAcc = ComponentAccumulator()
        
    def wasMerged(self):
        super( SelectionCA, self ).wasMerged()
        self.hypoAcc.wasMerged()

    def mergeReco(self, recoCA, robPrefetchCA=None, upSequenceCA=None):        
        ''' upSequenceCA is the user CA to run before the recoCA'''
        ca=ComponentAccumulator()
        ca.addSequence(self.stepViewSequence)
        if upSequenceCA:
            ca.merge(upSequenceCA, sequenceName=self.stepViewSequence.name)
        ca.addEventAlgo(recoCA.inputMaker(), sequenceName=self.stepViewSequence.name)
        if robPrefetchCA:
            ca.merge(robPrefetchCA, self.stepViewSequence.name)
        ca.merge(recoCA, sequenceName=self.stepViewSequence.name)
        self.merge(ca)        
        
    def mergeHypo(self, other):
        """To be used when the hypo alg configuration comes with auxiliary tools/services""" 
        self.hypoAcc.merge(other)       

    def addHypoAlgo(self, algo):
        """To be used when the hypo alg configuration does not require auxiliary tools/services"""        
        if self.isProbe:
            newname = algo.getName()+'_probe'
            algo.name=newname
        self.hypoAcc.addEventAlgo(algo)

    def hypo(self):
        """Access hypo algo (or throws)"""
        h = findAlgorithmByPredicate(self.stepViewSequence, lambda alg: "HypoInputDecisions" in alg._descriptors ) # can't use isHypo
        assert h is not None, "No hypo in SeelectionCA {}".format(self.name)
        return h

    def inputMaker(self):
        """Access Input Maker (or throws)"""
        im = findAlgorithmByPredicate(self.stepViewSequence, lambda alg: "InputMakerInputDecisions" in alg._descriptors )
        assert im is not None, "No input maker in SeelectionCA {}".format(self.name)
        return im

    def topSequence(self):
        return self.stepViewSequence
