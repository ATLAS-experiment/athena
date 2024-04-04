# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
    ------ Documentation on HLT Tree creation -----

++ Filter creation strategy

++ Connections between InputMaker/HypoAlg/Filter

++ Seeds

++ Combined chain strategy

- The combined chains use duplicates of the single-object-HypoAlg, called HypoAlgName_for_stepName.
  These duplicates are connected to a dedicated ComboHypoAlg (added by the framework), able to count object multiplicity
     -- This is needed for two reasons:
           -- the HypoAlg is designed to have only one input TC (that is already for the single object)
           -- otherwise the HypoAlg would be equipped with differnt HypoTools with the same name (see for example e3_e8)
     -- If the combined chain is symmetric (with multiplicity >1), the Hypo is duplicated only once,
        equipped with a HypoTool configured as single object and followed by one ComboHypoAlg


"""

from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFDot import stepCF_DataFlow_to_dot, stepCF_ControlFlow_to_dot, all_DataFlow_to_dot
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFComponents import RoRSequenceFilterNode, PassFilterNode, CFSequenceCA
from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming

from AthenaCommon.CFElements import parOR, seqAND, isSequence
from AthenaCommon.AlgSequence import  dumpSequence
from AthenaCommon.Logging import logging

from AthenaConfiguration.ComponentFactory import CompFactory

from DecisionHandling.DecisionHandlingConfig import TriggerSummaryAlg
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from builtins import map, range, str, zip
from collections import OrderedDict, defaultdict
import re

log = logging.getLogger( __name__ )

#### Functions to create the CF tree from CF configuration objects
def makeSummary(flags, name, flatDecisions):
    """ Returns a TriggerSummaryAlg connected to given decisions"""

    summary = TriggerSummaryAlg( flags, CFNaming.stepSummaryName(name) )
    summary.InputDecision = "HLTSeedingSummary"
    summary.FinalDecisions = list(OrderedDict.fromkeys(flatDecisions))
    return summary


def createStepRecoNode(name, seq_list, dump=False):
    """ Elementary HLT reco step, contianing all sequences of the step """

    log.debug("Create reco step %s with %d sequences", name, len(seq_list))
    stepCF = parOR(name + CFNaming.RECO_POSTFIX)
    for seq in seq_list:
        stepCF += createCFTree(seq)

    if dump:
        dumpSequence (stepCF, indent=0)
    return stepCF


def createStepFilterNode(name, seq_list, dump=False):
    """ Elementary HLT filter step: OR node containing all Filters of the sequences. The node gates execution of next reco step """

    log.debug("Create filter step %s with %d filters", name, len(seq_list))
    stepCF = parOR(name + CFNaming.FILTER_POSTFIX)
    filter_list=[]
    for seq in seq_list:
        filterAlg = seq.filter.Alg        
        log.debug("createStepFilterNode: Add  %s to filter node %s", filterAlg.getName(), name)
        if filterAlg not in filter_list:
            filter_list.append(filterAlg)

    stepCF = parOR(name + CFNaming.FILTER_POSTFIX, subs=filter_list)
    if dump:
        dumpSequence (stepCF, indent=0)
    return stepCF


def createCFTree(CFseq):
    """ Creates AthSequencer nodes with sequences attached """

    log.debug(" *** Create CF Tree for CFSequence %s", CFseq.step.name)
    filterAlg = CFseq.filter.Alg
    
    #empty step: add the PassSequence, one instance only is appended to the tree
    if len(CFseq.step.sequences)==0:  
        seqAndWithFilter=filterAlg       
        return seqAndWithFilter

    stepReco = parOR(CFseq.step.name + CFNaming.RECO_POSTFIX)  # all reco algorithms from all the sequences in a parallel sequence
    seqAndWithFilter = seqAND(CFseq.step.name, [filterAlg, stepReco])

    recoSeqSet=set()
    hypoSet=set()
    for menuseq in CFseq.step.sequences:
        menuseq.addToSequencer(recoSeqSet,hypoSet)
  
    stepReco   += sorted(list(recoSeqSet), key=lambda t: t.getName())
    seqAndWithFilter += sorted(list(hypoSet), key=lambda t: t.getName()) 
    if CFseq.step.combo is not None:         
        seqAndWithFilter += CFseq.step.combo.Alg

    return seqAndWithFilter


#######################################
## CORE of Decision Handling
#######################################
    
def matrixDisplay( allCFSeq ):

    def __getHyposOfStep( step ):
        if len(step.sequences):
            step.getChainNames()           
        return []
   
    # fill dictionary to cumulate chains on same sequences, in steps (dict with composite keys)
    mx = defaultdict(list)

    for stepNumber,cfseq_list in enumerate(allCFSeq, 1):
        for cfseq in cfseq_list:
            chains = __getHyposOfStep(cfseq.step)
            for seq in cfseq.step.sequences:
                if seq.name == "Empty":
                    mx[stepNumber, "Empty"].extend(chains)
                else:
                    mx[stepNumber, seq.sequence.Alg.getName()].extend(chains)

    # sort dictionary by fist key=step
    sorted_mx = OrderedDict(sorted( list(mx.items()), key= lambda k: k[0]))

    log.debug( "" )
    log.debug( "="*90 )
    log.debug( "Cumulative Summary of steps")
    log.debug( "="*90 )
    for (step, seq), chains in list(sorted_mx.items()):
        log.debug( "(step, sequence)  ==> (%d, %s) is in chains: ",  step, seq)
        for chain in chains:
            log.debug( "              %s",chain)

    log.debug( "="*90 )


def sequenceScanner( HLTNode ):
    """ Checks the alignement of sequences and steps in the tree"""
    # +-- AthSequencer/HLTAllSteps
    #   +-- AthSequencer/Step1_filter
    #   +-- AthSequencer/Step1_reco
    
    _seqMapInStep = defaultdict(set)
    _status = True
    def _mapSequencesInSteps(seq, stepIndex, childInView):
        """ Recursively finds the steps in which sequences are used"""
        if not isSequence(seq):
            return stepIndex
        match=re.search('^Step([0-9]+)_filter',seq.name)
        if match:
            stepIndex = match.group(1)
            log.debug("sequenceScanner: This is another step: %s %s", seq.name, stepIndex)
        inViewSequence = ""
        inView = False
        for c in seq.Members:
            if isSequence(c):
                # Detect whether this is the view sequence pointed to
                # by the EV creator alg, or if it is in such a sequence
                inView = c.getName()==inViewSequence or childInView
                stepIndex = _mapSequencesInSteps(c, stepIndex, childInView=inView)
                _seqMapInStep[c.name].add((stepIndex,inView))
                log.verbose("sequenceScanner: Child %s of sequence %s is in view? %s --> '%s'", c.name, seq.name, inView, inViewSequence)
            else:
                if isinstance(c, CompFactory.EventViewCreatorAlgorithm):
                    inViewSequence = c.ViewNodeName
                    log.verbose("sequenceScanner: EventViewCreatorAlg %s is child of sequence %s with ViewNodeName %s", c.name, seq.name, c.ViewNodeName)
        log.debug("sequenceScanner: Sequence %s is in view? %s --> '%s'", seq.name, inView, inViewSequence)
        return stepIndex

    # do the job:
    final_step=_mapSequencesInSteps(HLTNode, 0, childInView=False)

    for alg, steps in _seqMapInStep.items():
        if 'PassSequence' in alg or 'HLTCaloClusterMakerFSRecoSequence' in alg \
            or 'HLTCaloCellMakerFSRecoSequence' in alg: # do not count PassSequences, which is used many times. Harcoding HLTCaloClusterMakerFSRecoSequence and HLTCaloCellMakerFSRecoSequence when using FullScanTopoCluster building for photon triggers with RoI='' (also for Jets and MET) following discussion in ATR-24722. To be fixed
            continue
        # Sequences in views can be in multiple steps
        nonViewSteps = sum([0 if isInViews else 1 for (stepIndex,isInViews) in steps])
        if nonViewSteps > 1:
            steplist = [stepIndex for stepIndex,inViewSequence in steps]
            log.error("sequenceScanner: Sequence %s is expected outside of a view in more than one step: %s", alg, steplist)
            match=re.search('Step([0-9]+)',alg)
            if match:
                candidateStep=match.group(1)
                log.error("sequenceScanner:         ---> candidate good step is %s", candidateStep)
            _status=False
            raise RuntimeError(f"Duplicated event-scope sequence {alg} in steps {steplist}")

    log.debug("sequenceScanner: scanned %s steps with status %d", final_step, _status)
    return _status
   

def decisionTreeFromChains(flags, HLTNode, chains, allDicts):
    """ Creates the decision tree, given the starting node and the chains containing the sequences  """
    log.info("[decisionTreeFromChains] Run decisionTreeFromChains on %s", HLTNode.getName())    
    HLTNodeName = HLTNode.getName()
    acc = ComponentAccumulator()

    if len(chains) == 0:
        log.info("[decisionTreeFromChains] Configuring empty decisionTree")
        acc.addSequence(HLTNode)
        return ([], acc)
    
    ( finalDecisions, CFseq_list) = createDataFlow(flags, chains, allDicts)    
    cfAcc = createControlFlow(flags, HLTNode, CFseq_list)
    acc.merge(cfAcc)

    # create dot graphs
    log.debug("finalDecisions: %s", finalDecisions)

    if flags.Trigger.generateMenuDiagnostics:
        all_DataFlow_to_dot(HLTNodeName, CFseq_list)
    
    # matrix display
    # uncomment for serious debugging
    # matrixDisplay( CFseq_list )

    return (finalDecisions,acc)


def createDataFlow(flags, chains, allDicts):
    """ Creates the filters and connect them to the menu sequences"""
   
    # find tot nsteps
    chainWithMaxSteps = max(chains, key = lambda chain: len(chain.steps))
    NSTEPS = len(chainWithMaxSteps.steps)
    log.info("[createDataFlow] creating DF for %d chains and total %d steps", len(chains), NSTEPS)

    # initialize arrays for monitor
    finalDecisions = [ [] for n in range(NSTEPS) ]
    CFseqList = [ [] for n in range(NSTEPS) ]
    CFSeqByFilterName = [ {} for n in range(NSTEPS) ] # CFSeqeunces keyed by filter name (speedup)

    # loop over chains
    for chain in chains:
        log.debug("\n Configuring chain %s with %d steps: \n   - %s ", chain.name,len(chain.steps),'\n   - '.join(map(str, [{step.name:step.multiplicity} for step in chain.steps])))

        lastCFseq = None
        lastDecisions = []
        for nstep, chainStep in enumerate( chain.steps ):
            chainStep.createSequences(flags.Trigger.fastMenuGeneration)
            log.debug("\n************* Start connecting step %d %s for chain %s", nstep+1, chainStep.name, chain.name)           
            if nstep == 0:             
                filterInput = chain.L1decisions
            else:
                filterInput = lastDecisions
            if len(filterInput) == 0 :
                log.error("[createDataFlow] Filter for step %s has %d inputs! At least one is expected", chainStep.name, len(filterInput))
                raise Exception("[createDataFlow] Cannot proceed, exiting.")
            
            log.debug("Set Filter input: %s while setting the chain: %s", filterInput, chain.name)

            # make one filter per step:
            sequenceFilter = None            
            filterName = CFNaming.filterName(chainStep.name)
            if chainStep.isEmpty:
                filterOutput = filterInput
            else:
                filterOutput = [CFNaming.filterOutName(filterName, inputName) for inputName in filterInput ]

            # TODO: Check sequence consistency if skipping, to avoid issues like https://its.cern.ch/jira/browse/ATR-28617
            foundCFseq = CFSeqByFilterName[nstep].get(filterName, None)
            log.debug("%s CF sequences with filter name %s",  "Not found" if foundCFseq is None else "Found", filterName)
            if foundCFseq is None:
                sequenceFilter = buildFilter(filterName, filterInput, chainStep.isEmpty)
                CFseq = CFSequenceCA( chainStep = chainStep, filterAlg = sequenceFilter)
                CFseq.connect(filterOutput)
                CFSeqByFilterName[nstep][CFseq.filter.Alg.getName()] = CFseq
                CFseqList[nstep].append(CFseq)
                lastCFseq = CFseq
            else:                
                lastCFseq = foundCFseq
                
                # skip re-merging
                if flags.Trigger.fastMenuGeneration:
                    for menuseq in chainStep.sequences:
                        menuseq.ca.wasMerged()
                        if menuseq.globalRecoCA:
                            menuseq.globalRecoCA.wasMerged()

                sequenceFilter = lastCFseq.filter
                if len(list(set(sequenceFilter.getInputList()).intersection(filterInput))) != len(list(set(filterInput))):
                    [ sequenceFilter.addInput(inputName) for inputName in filterInput ]
                    [ sequenceFilter.addOutput(outputName) for outputName in  filterOutput ]
                    lastCFseq.connect(filterOutput)

            lastDecisions = lastCFseq.decisions
                                            
            # add chains to the filter:
            chainLegs = chainStep.getChainLegs()
            if len(chainLegs) != len(filterInput): 
                log.error("[createDataFlow] lengths of chainlegs = %s differ from inputs = %s", str(chainLegs), str(filterInput))
                raise Exception("[createDataFlow] Cannot proceed, exiting.")
            for finput, leg in zip(filterInput, chainLegs):
                log.debug("Adding chain %s to input %s of %s", leg, finput, sequenceFilter.Alg.name)
                sequenceFilter.addChain(leg, finput)
                
            log.debug("Now Filter has chains: %s", sequenceFilter.getChains())
            log.debug("Now Filter has chains/input: %s", sequenceFilter.getChainsPerInput())

            if lastCFseq.step.combo is not None:
                lastCFseq.step.combo.addChain( [d for d in allDicts if d['chainName'] == chain.name ][0])
                log.debug("Added chains to ComboHypo: %s",lastCFseq.step.combo.getChains())
            else:
                log.debug("Combo not implemented if it's empty step")

            # add HypoTools to this step (cumulating all same steps)
            lastCFseq.createHypoTools(flags,chain.name,chainStep)            

            if len(chain.steps) == nstep+1:
                log.debug("Adding finalDecisions for chain %s at step %d:", chain.name, nstep+1)
                for dec in lastDecisions:
                    finalDecisions[nstep].append(dec)
                    log.debug(dec)
                    
        #end of loop over steps
        log.debug("\n Built CD for chain %s with %d steps: \n   - %s ", chain.name,len(chain.steps),'\n   - '.join(map(str, [{step.name:step.multiplicity} for step in chain.steps])))
    #end of loop over chains

    log.debug("End of createDataFlow for %d chains and total %d steps", len(chains), NSTEPS)
    return (finalDecisions, CFseqList)


def createControlFlow(flags, HLTNode, CFseqList):
    """ Creates Control Flow Tree starting from the CFSequences"""    
    HLTNodeName = HLTNode.getName()    
    log.debug("[createControlFlow] on node %s with %d CFsequences",HLTNodeName, len(CFseqList))

    acc = ComponentAccumulator()
    acc.addSequence(HLTNode)

    for nstep, sequences in enumerate(CFseqList):
        stepSequenceName =  CFNaming.stepName(nstep)
        log.debug("\n******** Create CF Tree %s with %d AthSequencers", stepSequenceName, len(sequences))

        # create filter node
        log.debug("[createControlFlow] Create filter step %s with %d filters", stepSequenceName, len(CFseqList[nstep])) 
        stepCFFilter = parOR(stepSequenceName + CFNaming.FILTER_POSTFIX)
        acc.addSequence(stepCFFilter, parentName=HLTNodeName)

        filter_list = []
        # add the filter to the node
        for cseq in sequences:
            filterAlg = cseq.filter.Alg 
            if filterAlg.getName() not in filter_list:
                log.debug("[createControlFlow] Add  %s to filter node %s", filterAlg.getName(), stepSequenceName)
                filter_list.append(filterAlg.getName())   
                stepCFFilter.Members += [filterAlg]

        # create reco step node
        log.debug("[createControlFlow] Create reco step %s with %d sequences", stepSequenceName, len(CFseqList))
        stepCFReco = parOR(stepSequenceName + CFNaming.RECO_POSTFIX)  
        acc.addSequence(stepCFReco, parentName = HLTNodeName)

        # add the sequences to the reco node
        addedEmtpy = False
        for cseq in sequences:
            if  cseq.empty and addedEmtpy:
                cseq.ca.wasMerged()
                continue
            if  cseq.empty: # adding Empty only once to avoid merging multiple times the PassSequence
                addedEmtpy= True
            log.debug(" *** Create CF Tree for CFSequence %s", cseq.step.name)
            acc.merge(cseq.ca, sequenceName=stepCFReco.getName())

        # add the monitor summary
        stepDecisions = []
        for CFseq in CFseqList[nstep]:
            stepDecisions.extend(CFseq.decisions)

        summary = makeSummary( flags, stepSequenceName, stepDecisions )
        acc.addEventAlgo([summary],sequenceName = HLTNode.getName())

        if flags.Trigger.generateMenuDiagnostics:
            log.debug("Now Draw Menu Diagnostic dot graphs...")
            stepCF_DataFlow_to_dot( stepCFReco.getName(), CFseqList[nstep] )
            stepCF_ControlFlow_to_dot( stepCFReco )
       
        log.debug("************* End of step %d, %s", nstep+1, stepSequenceName)

    return acc





def buildFilter(filter_name,  filter_input, empty):
    """
     Build the FILTER
     one filter per previous sequence at the start of the sequence: always create a new one
     if the previous hypo has more than one output, try to get all of them
     one filter per previous sequence: 1 input/previous seq, 1 output/next seq
    """
    if empty:
        log.debug("Calling PassFilterNode %s", filter_name)
        sfilter = PassFilterNode(name = filter_name)
        for i in filter_input:
            sfilter.addInput(i)
            sfilter.addOutput(i)
    else:
        sfilter = RoRSequenceFilterNode(name = filter_name)        
        for i in filter_input:
            sfilter.addInput(i)
            sfilter.addOutput(CFNaming.filterOutName(filter_name, i))

    log.debug("Added inputs to filter: %s", sfilter.getInputList())
    log.debug("Added outputs to filter: %s", sfilter.getOutputList())
    log.debug("Filter Done: %s", sfilter.Alg.name)

    
    return (sfilter)


