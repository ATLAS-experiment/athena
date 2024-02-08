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
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFComponents import CFSequence, RoRSequenceFilterNode, PassFilterNode, CFSequenceCA
from TriggerMenuMT.HLT.Config.ControlFlow.MenuComponentsNaming import CFNaming
from TriggerMenuMT.HLT.Config.Validation.CFValidation import testHLTTree

from AthenaCommon.CFElements import parOR, seqAND, getSequenceChildren, isSequence, compName, findSubSequence,findAlgorithm
from AthenaCommon.AlgSequence import AlgSequence, dumpSequence
from AthenaCommon.Configurable import ConfigurableCABehavior
from AthenaCommon.Logging import logging

from AthenaConfiguration.ComponentAccumulator import conf2toConfigurable, appendCAtoAthena
from AthenaConfiguration.ComponentFactory import CompFactory

from DecisionHandling.DecisionHandlingConfig import TriggerSummaryAlg
from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection
from TriggerJobOpts.TriggerConfig import collectHypos, collectFilters, collectViewMakers, collectDecisionObjects, \
     triggerMonitoringCfg, triggerSummaryCfg, collectHypoDecisionObjects
from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import getTrigNavSlimmingMTOnlineConfig
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.GenerateMenuMT_newJO import isCAMenu 

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

def makeHLTTree(flags, newJO=False, hltMenuConfig = None):
    """ Creates the full HLT tree, main function called from GenerateMenu.py"""

    # Check if hltMenuConfig exits, if yes, derive information from this
    if hltMenuConfig is None:
        raise Exception("[makeHLTTree] hltMenuConfig is set to None, please check!")

    # get topSequnece
    topSequence = AlgSequence()

    # find main HLT top sequence (already set up in runHLT_standalone)
    hltSeeding = findAlgorithm(topSequence, "HLTSeeding")

    # add the HLT steps Node
    steps = seqAND("HLTAllSteps")
    hltTop = findSubSequence(topSequence, "HLTTop")
    hltTop += steps

    hltEndSeq = parOR("HLTEndSeq")
    hltTop += hltEndSeq

    hltFinalizeSeq = seqAND("HLTFinalizeSeq")

    log.debug("[makeHLTTree] will now make the DF and CF tree from chains")

    # make DF and CF tree from chains
    finalDecisions, acc = decisionTreeFromChains(flags, steps, hltMenuConfig.configsList(), hltMenuConfig.dictsList(), newJO)

    successful_scan = sequenceScanner( steps )

    if not successful_scan:
        raise Exception("[makeHLTTree] At least one sequence is expected in more than one step. Check error messages and fix!")    

    flatDecisions=[]
    for step in finalDecisions:
        flatDecisions.extend (step)

    summary = makeSummary(flags, "Final", flatDecisions)
    hltEndSeq += summary

    log.debug("[makeHLTTree] created the final summary tree")
    # TODO - check we are not running things twice. Once here and once in TriggerConfig.py
    

    # Collections required to configure the algs below
    hypos = collectHypos(steps)
    filters = collectFilters(steps)
    viewMakers = collectViewMakers(steps)

    viewMakerMap = {compName(vm):vm for vm in viewMakers}
    for vmname, vm in viewMakerMap.items():
        log.debug(f"{vmname} InputMakerOutputDecisions: {vm.InputMakerOutputDecisions}")
        if vmname.endswith("_probe"):
            try:
                log.debug(f"Setting InputCachedViews on {vmname} to read decisions from tag leg {vmname[:-6]}: {vm.InputMakerOutputDecisions}")
                vm.InputCachedViews = viewMakerMap[vmname[:-6]].InputMakerOutputDecisions
            except KeyError: # We may be using a probe leg that has different reco from the tag
                log.debug(f"Tag leg does not match probe: '{vmname[:-6]}', will not use cached views")

    with ConfigurableCABehavior():
        summaryAcc, summaryAlg = triggerSummaryCfg( flags, hypos )

    # Schedule the DecisionSummaryMakerAlg
    hltFinalizeSeq += conf2toConfigurable( summaryAlg )
    appendCAtoAthena( summaryAcc )

    # Add end-of-event sequences executed conditionally on the DecisionSummaryMakerAlg filter status
    acceptedEventChainDicts = [cd for cd in hltMenuConfig.dictsList() \
                               if 'Calib' in cd['signatures'] \
                               and 'acceptedevts' in cd['chainParts'][0]['purpose']]
    if flags.Trigger.enableEndOfEventProcessing and acceptedEventChainDicts:
        from TrigGenericAlgs.TrigGenericAlgsConfig import EndOfEventROIConfirmerAlgCfg, EndOfEventFilterAlgCfg
        endOfEventRoIMaker = conf2toConfigurable(EndOfEventROIConfirmerAlgCfg('EndOfEventROIConfirmerAlg'))
        hltFinalizeSeq += endOfEventRoIMaker
        acceptedEventTopSeq = parOR("acceptedEventTopSeq")
        acceptedEventTopSeq.IgnoreFilterPassed=True
        hltFinalizeSeq += conf2toConfigurable(acceptedEventTopSeq)
        for acceptedEventChainDict in acceptedEventChainDicts:
            # Common config for each chain
            prescaleChain = acceptedEventChainDict['chainName']
            seqLabel = prescaleChain.replace('HLT_acceptedevts','')
            acceptedEventSeq = seqAND('acceptedEventSeq'+seqLabel)
            endOfEventFilterAlg = EndOfEventFilterAlgCfg('EndOfEventFilterAlg'+seqLabel, chainName=prescaleChain)
            acceptedEventSeq += conf2toConfigurable(endOfEventFilterAlg)
            # Now add chain-specific end-of-event sequences executed conditionally on the prescale
            purposes = acceptedEventChainDict['chainParts'][0]['purpose']

            # The LAr Noise Burst end-of-event sequence
            if 'larnoiseburst' in purposes:
                # Add stream filter to EndOfEventFilterAlg
                # Only accept events going to streams that already do full calo reco
                # CosmicCalo explicitly requested [ATR-26096]
                endOfEventFilterAlg.StreamFilter = ['Main','VBFDelayed','TLA','DarkJetPEBTLA','FTagPEBTLA','CosmicCalo']

                from TriggerMenuMT.HLT.CalibCosmicMon.CalibChainConfiguration import getLArNoiseBurstRecoCfg
                from ..MenuComponents import algorithmCAToGlobalWrapper
                recoSeq = algorithmCAToGlobalWrapper(getLArNoiseBurstRecoCfg,flags)
                acceptedEventSeq += recoSeq
            elif any(purpose.startswith("met") for purpose in purposes):
                from TriggerMenuMT.HLT.MET.EndOfEvent import getMETRecoSequences
                algorithms, rois, streams = getMETRecoSequences(flags, purposes)
                endOfEventFilterAlg.StreamFilter = streams
                endOfEventRoIMaker.RoIs = [x for x in rois if x not in endOfEventRoIMaker.RoIs]
                acceptedEventSeq += algorithms
            # elif ... add other end of event sequences (with the corresponding chain) here if needed

            acceptedEventTopSeq += conf2toConfigurable(acceptedEventSeq)
    
    # More collections required to configure the algs below
    decObj = collectDecisionObjects( hypos, filters, hltSeeding, summaryAlg )
    decObjHypoOut = collectHypoDecisionObjects(hypos, inputs=False, outputs=True)

    with ConfigurableCABehavior():
        monAcc, monAlg = triggerMonitoringCfg( flags, hypos, filters, hltSeeding )

    hltEndSeq += conf2toConfigurable( monAlg )
    appendCAtoAthena( monAcc )

    from TrigCostMonitor.TrigCostMonitorConfig import TrigCostMonitorFinalizeCfg
    costAlg = TrigCostMonitorFinalizeCfg(flags)
    if costAlg: # None if Cost Monitoring is turned off
        hltFinalizeSeq += conf2toConfigurable( costAlg )

    # Finally, we create the EDM output
    from TriggerJobOpts.TriggerConfig import triggerMergeViewsCfg, triggerEDMGapFillerCfg
    with ConfigurableCABehavior():
        edmAcc = ComponentAccumulator()
        # The order is important: 1) view merging, 2) gap filling
        edmAcc.merge( triggerMergeViewsCfg(flags, viewMakers) )
        # For RDO output, we run the GapFiller. Otherwise it is done in Reco.
        edmSet = ['AOD','ESD'] if flags.Output.doWriteRDO else []
        edmAcc.merge( triggerEDMGapFillerCfg(flags, edmSet, decObj, decObjHypoOut) )
        edmAlg = edmAcc.popEventAlgo("EDMCreatorAlg")
    appendCAtoAthena( edmAcc )
    hltFinalizeSeq += conf2toConfigurable(edmAlg)

    if flags.Trigger.doOnlineNavigationCompactification:
        onlineSlimAlg = getTrigNavSlimmingMTOnlineConfig(flags)
        hltFinalizeSeq += conf2toConfigurable(onlineSlimAlg)

    hltEndSeq += hltFinalizeSeq

    # Test the configuration
    testHLTTree( hltTop )

    def debugDecisions(hypos, summary, mon, summaryAlg):
        """ set DEBUG flag to the hypos of all the sequences (used to debug), other debug functions can be added here """
        from GaudiKernel.Constants import DEBUG # noqa: ATL900
        for step, stepHypos in sorted(hypos.items()):
            for hypo in stepHypos:
                hypo.OutputLevel=DEBUG   # noqa:  ATL900
        summary.OutputLevel = DEBUG # noqa: ATL900
        mon.OutputLevel = DEBUG # noqa: ATL900
        summaryAlg.OutputLevel = DEBUG # noqa: ATL900


    # Switch on  DEBUG ouput in some algorithms
    #debugDecisions(hypos, summary, monAlg, summaryAlg)

    
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
        name = compName(seq)                
        match=re.search('^Step([0-9]+)_filter',name)
        if match:
            stepIndex = match.group(1)
            log.debug("sequenceScanner: This is another step: %s %s", name, stepIndex)
        inViewSequence = ""
        inView = False
        for c in getSequenceChildren( seq ):
            if isSequence(c):
                # Detect whether this is the view sequence pointed to
                # by the EV creator alg, or if it is in such a sequence
                inView = c.getName()==inViewSequence or childInView
                stepIndex = _mapSequencesInSteps(c, stepIndex, childInView=inView)
                _seqMapInStep[compName(c)].add((stepIndex,inView))
                log.verbose("sequenceScanner: Child %s of sequence %s is in view? %s --> '%s'", compName(c), name, inView, inViewSequence)
            else:
                if isinstance(c, CompFactory.EventViewCreatorAlgorithm):
                    inViewSequence = c.ViewNodeName
                    log.verbose("sequenceScanner: EventViewCreatorAlg %s is child of sequence %s with ViewNodeName %s", compName(c), name, c.ViewNodeName)
        log.debug("sequenceScanner: Sequence %s is in view? %s --> '%s'", name, inView, inViewSequence)
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
   

def decisionTreeFromChains(flags, HLTNode, chains, allDicts, newJO):
    """ Creates the decision tree, given the starting node and the chains containing the sequences  """
    log.info("[decisionTreeFromChains] Run decisionTreeFromChains on %s", HLTNode.getName())    
    HLTNodeName = HLTNode.getName()
    with ConfigurableCABehavior():
        acc = ComponentAccumulator()

    if len(chains) == 0:
        log.info("[decisionTreeFromChains] Configuring empty decisionTree")
        if isCAMenu():
            acc.addSequence(HLTNode)
        return ([], acc)
    
    (dfAcc, finalDecisions, CFseq_list) = createDataFlow(flags, chains, allDicts)
    acc.merge(dfAcc)
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
    with ConfigurableCABehavior():
        acc = ComponentAccumulator()

    # find tot nsteps
    chainWithMaxSteps = max(chains, key = lambda chain: len(chain.steps))
    NSTEPS = len(chainWithMaxSteps.steps)
    log.info("[createDataFlow] creating DF for %d chains and total %d steps", len(chains), NSTEPS)

    # initialize arrays for monitor
    finalDecisions = [ [] for n in range(NSTEPS) ]
    CFseqList = [ [] for n in range(NSTEPS) ]

    # loop over chains
    for chain in chains:
        log.debug("\n Configuring chain %s with %d steps: \n   - %s ", chain.name,len(chain.steps),'\n   - '.join(map(str, [{step.name:step.multiplicity} for step in chain.steps])))

        lastCFseq = None
        lastDecisions = []
        for nstep, chainStep in enumerate( chain.steps ):
            log.debug("\n************* Start connecting step %d %s for chain %s", nstep+1, chainStep.name, chain.name)           
            if nstep == 0:
                if chainStep.stepDicts:
                    filterInput = [ mapThresholdToL1DecisionCollection(p["chainParts"][0]["L1threshold"]) for p in chainStep.stepDicts]
                else:
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

            foundCFSeq = [cfseq for cfseq in CFseqList[nstep] if filterName == cfseq.filter.Alg.getName()]
            # TODO: Check sequence consistency if skipping, to avoid issues like https://its.cern.ch/jira/browse/ATR-28617
            log.debug("Found %d CF sequences with filter name %s", len(foundCFSeq), filterName)
            if not foundCFSeq:
                sequenceFilter = buildFilter(filterName, filterInput, chainStep.isEmpty)
                if isCAMenu():
                    CFseq = CFSequenceCA( chainStep = chainStep, filterAlg = sequenceFilter)
                else:
                    CFseq = CFSequence( ChainStep = chainStep, FilterAlg = sequenceFilter)
                CFseq.connect(filterOutput)
                CFseqList[nstep].append(CFseq)
                lastCFseq = CFseq
            else:                
                if len(foundCFSeq) > 1:
                    log.error("Found more than one sequence containing filter %s", filterName)
                
                lastCFseq = foundCFSeq[0]
                if isCAMenu():
                    # skip re-merging
                    if flags.Trigger.fastMenuGeneration:
                        for menuseq in chainStep.sequences:
                            menuseq.ca.wasMerged()
                            if menuseq.globalRecoCA:
                                menuseq.globalRecoCA.wasMerged()
                    else:
                        lastCFseq.mergeStepSequences(chainStep)

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
                log.debug("Adding chain %s to input %s of %s", leg, finput,compName(sequenceFilter.Alg))
                sequenceFilter.addChain(leg, finput)
                
            log.debug("Now Filter has chains: %s", sequenceFilter.getChains())
            log.debug("Now Filter has chains/input: %s", sequenceFilter.getChainsPerInput())

            if lastCFseq.step.combo is not None:
                lastCFseq.step.combo.addChain( [d for d in allDicts if d['chainName'] == chain.name ][0])
                log.debug("Added chains to ComboHypo: %s",lastCFseq.step.combo.getChains())
            else:
                log.debug("Combo not implemented if it's empty step")

            # add HypoTools to this step (cumulating all same steps)
            hyposAcc = lastCFseq.createHypoTools(flags,chain.name,chainStep)
            if hyposAcc:
                assert len(hyposAcc.getEventAlgos()) == 0, 'Hypo CA contains algorithms'
                acc.merge(hyposAcc)

            if len(chain.steps) == nstep+1:
                log.debug("Adding finalDecisions for chain %s at step %d:", chain.name, nstep+1)
                for dec in lastDecisions:
                    finalDecisions[nstep].append(dec)
                    log.debug(dec)
                    
        #end of loop over steps
        log.debug("\n Built CD for chain %s with %d steps: \n   - %s ", chain.name,len(chain.steps),'\n   - '.join(map(str, [{step.name:step.multiplicity} for step in chain.steps])))
    #end of loop over chains

    log.debug("End of createDataFlow for %d chains and total %d steps", len(chains), NSTEPS)
    return (acc, finalDecisions, CFseqList)

def createControlFlow(flags, HLTNode, CFseqList):
    """ Creates Control Flow Tree starting from the CFSequences"""    
    HLTNodeName = HLTNode.getName()    
    log.debug("[createControlFlow] on node %s with %d CFsequences",HLTNodeName, len(CFseqList))
    with ConfigurableCABehavior():
        acc = ComponentAccumulator()
    if isCAMenu():
        acc.addSequence(HLTNode)
    for nstep, sequences in enumerate(CFseqList):
        stepSequenceName =  CFNaming.stepName(nstep)
        log.debug("\n******** Create CF Tree %s with %d AthSequencers", stepSequenceName, len(sequences))

        # create filter node
        log.debug("[createControlFlow] Create filter step %s with %d filters", stepSequenceName, len(CFseqList[nstep])) 
        stepCFFilter = parOR(stepSequenceName + CFNaming.FILTER_POSTFIX)
        if isCAMenu():
            acc.addSequence(stepCFFilter, parentName=HLTNodeName)
        else:
            HLTNode += stepCFFilter
        filter_list = []
        # add the filter to the node
        for cseq in sequences:
            filterAlg = cseq.filter.Alg 
            if filterAlg.getName() not in filter_list:
                log.debug("[createControlFlow] Add  %s to filter node %s", filterAlg.getName(), stepSequenceName)
                filter_list.append(filterAlg.getName())   
                if isCAMenu():
                    stepCFFilter.Members += [filterAlg]
                else:
                    stepCFFilter += filterAlg
    
        # create reco step node
        log.debug("[createControlFlow] Create reco step %s with %d sequences", stepSequenceName, len(CFseqList))
        stepCFReco = parOR(stepSequenceName + CFNaming.RECO_POSTFIX)  
        if isCAMenu():
            acc.addSequence(stepCFReco, parentName = HLTNodeName)
        else:
            HLTNode += stepCFReco
        # add the sequences to the reco node
        addedEmtpy = False
        for cseq in sequences:
            if  cseq.empty and addedEmtpy:
                if isCAMenu():
                    cseq.ca.wasMerged()
                continue
            if  cseq.empty: # adding Empty only once to avoid merging multiple times the PassSequence
                addedEmtpy= True
            log.debug(" *** Create CF Tree for CFSequence %s", cseq.step.name)
            if isCAMenu():
                acc.merge(cseq.ca, sequenceName=stepCFReco.getName())
            else:
                filterAlg = cseq.filter.Alg  
                if len(cseq.step.sequences) == 0:  
                    stepCFReco += filterAlg                 
                else:
                    seqAndWithFilter = seqAND(cseq.step.name)  
                    stepReco = parOR(cseq.step.name + CFNaming.RECO_POSTFIX)                                 
                    seqAndWithFilter += [filterAlg, stepReco]
                    recoSeqSet = set()
                    hypoSet = set()
                    for menuseq in cseq.step.sequences:
                        menuseq.addToSequencer(recoSeqSet, hypoSet)
  
                    stepReco += sorted(list(recoSeqSet), key = lambda t: t.getName())
                    seqAndWithFilter += sorted(list(hypoSet), key = lambda t: t.getName()) 
                    if cseq.step.combo is not None:         
                        seqAndWithFilter += cseq.step.combo.Alg
            
                    stepCFReco += seqAndWithFilter

        # add the monitor summary
        stepDecisions = []
        for CFseq in CFseqList[nstep]:
            stepDecisions.extend(CFseq.decisions)

        summary = makeSummary( flags, stepSequenceName, stepDecisions )
        if isCAMenu():
            acc.addEventAlgo([summary],sequenceName = HLTNode.getName())            
        else:
            HLTNode += summary

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
    log.debug("Filter Done: %s", compName(sfilter.Alg))

    
    return (sfilter)


