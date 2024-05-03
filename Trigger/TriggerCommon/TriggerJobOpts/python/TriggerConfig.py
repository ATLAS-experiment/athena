# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import re
from collections import OrderedDict, defaultdict
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format, MetadataCategory
from AthenaCommon.CFElements import seqAND, seqOR, parOR, flatAlgorithmSequences, getSequenceChildren, isSequence
from AthenaCommon.Logging import logging

__log = logging.getLogger('TriggerConfig')


def __isCombo(alg):
    return hasattr( alg, "MultiplicitiesMap" )

def __stepNumber(stepName):
    """extract step number frmo strings like Step2... -> 2"""
    return int(stepName.split('_')[0].replace("Step",""))


def collectHypos( steps ):
    """
    Method iterating over the CF and picking all the Hypothesis algorithms

    Returned is a map with the step name and list of all instances of hypos in that step.
    Input is top HLT sequencer.
    """
    __log.info("Collecting hypos from steps")
    hypos = defaultdict( list )

    for stepSeq in steps.Members:
        if not isSequence( stepSeq ):
            continue

        if "filter" in stepSeq.getName():
            __log.debug("Skipping filtering steps %s", stepSeq.getName() )
            continue

        __log.debug( "collecting hypos from step %s", stepSeq.getName() )
        for seq,algs in flatAlgorithmSequences(stepSeq).items():
            for alg in sorted(algs, key=lambda t: str(t.getName())):
                if isSequence( alg ):
                    continue
                # will replace by function once dependencies are sorted
                if hasattr(alg, 'HypoInputDecisions'):
                    __log.debug("found hypo %s in %s", alg.getName(), stepSeq.getName())
                    if __isCombo( alg ) and len(alg.ComboHypoTools):
                        __log.debug( "    with %d comboHypoTools: %s", len(alg.ComboHypoTools), ' '.join(map(str, [tool.getName() for  tool in alg.ComboHypoTools])))
                    hypos[stepSeq.getName()].append( alg )
                else:
                    __log.verbose("Not a hypo %s", alg.getName())

    return OrderedDict(hypos)

def __decisionsFromHypo( hypo ):
    """ return all chains served by this hypo and the keys of produced decision object """
    from TrigCompositeUtils.TrigCompositeUtils import isLegId
    __log.debug("Hypo type %s is combo %r", hypo.getName(), __isCombo(hypo))    
    if __isCombo(hypo):
        return [key for key in list(hypo.MultiplicitiesMap.keys()) if not isLegId(key)], hypo.HypoOutputDecisions
    else: # regular hypos
        return [ t.getName() for t in hypo.HypoTools if not isLegId(t.getName())], [str(hypo.HypoOutputDecisions)]


def collectViewMakers( steps ):
    """ collect all view maker algorithms in the configuration """
    makers = [] # map with name, instance and encompasing recoSequence
    for stepSeq in getSequenceChildren( steps ):
        for recoSeq in getSequenceChildren( stepSeq ):
            if not isSequence( recoSeq ):
                continue
            algsInSeq = flatAlgorithmSequences( recoSeq )
            for seq,algs in algsInSeq.items():
                for alg in algs:
                    if "EventViewCreator" in alg.getFullJobOptName(): # TODO base it on checking types of write handles once available
                        if alg not in makers:
                            makers.append(alg)
    __log.debug("Found ViewMakers: %s", ' '.join([ maker.getName() for maker in makers ]))
    return makers



def collectFilters( steps ):
    """
    Similarly to collectHypos but works for filter algorithms

    The logic is simpler as all filters are grouped in step filter sequences
    Returns map: step name -> list of all filters of that step
    """
    __log.info("Collecting filters")
    filters = defaultdict( list )

    for stepSeq in steps.Members:
        if "filter" in stepSeq.getName():
            filters[stepSeq.getName()] = stepSeq.Members
            __log.debug("Found Filters in Step %s : %s", stepSeq.getName(), stepSeq.Members)

    return filters


def collectHLTSeedingDecisionObjects(hltSeeding):
    decisionObjects = set()
    decisionObjects.update([ str(d.Decisions) for d in hltSeeding.RoIBRoIUnpackers + hltSeeding.xAODRoIUnpackers ])
    decisionObjects.update([ str(d.DecisionsProbe) for d in hltSeeding.RoIBRoIUnpackers + hltSeeding.xAODRoIUnpackers ])
    from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection
    decisionObjects.add( mapThresholdToL1DecisionCollection("FSNOSEED") ) # Include also Full Scan
    decisionObjects.discard('') # Unpackers which do not use the PROBE container have an empty string for their WriteHandleKey
    __log.info("Collecting %i decision objects from HLTSeeding instance", len(decisionObjects))
    return decisionObjects

def collectHypoDecisionObjects(hypos, inputs = True, outputs = True):
    decisionObjects = set()
    for step, stepHypos in sorted(hypos.items()):
        for hypoAlg in stepHypos:
            __log.debug( "Hypo %s with input %s and output %s ",
                         hypoAlg.getName(), hypoAlg.HypoInputDecisions, hypoAlg.HypoOutputDecisions )
            if isinstance( hypoAlg.HypoInputDecisions, list):
                if inputs:
                    [ decisionObjects.add( str(d) ) for d in hypoAlg.HypoInputDecisions ]
                if outputs:
                    [ decisionObjects.add( str(d) ) for d in hypoAlg.HypoOutputDecisions ]
            else:
                if inputs:
                    decisionObjects.add( str(hypoAlg.HypoInputDecisions) )
                if outputs:
                    decisionObjects.add( str(hypoAlg.HypoOutputDecisions) )
    __log.info("Collecting %i decision objects from hypos", len(decisionObjects))
    return sorted(decisionObjects)

def collectFilterDecisionObjects(filters, inputs = True, outputs = True):
    decisionObjects = set()
    for step, stepFilters in filters.items():
        for filt in stepFilters:
            if inputs and hasattr( filt, "Input" ):
                decisionObjects.update( str(i) for i in filt.Input )
            if outputs and hasattr( filt, "Output" ):
                decisionObjects.update( str(o) for o in filt.Output )
    __log.info("Collecting %i decision objects from filters", len(decisionObjects))
    return decisionObjects

def collectHLTSummaryDecisionObjects(hltSummary):
    decisionObjects = set()
    decisionObjects.add( str(hltSummary.DecisionsSummaryKey) )
    __log.info("Collecting %i decision objects from hltSummary", len(decisionObjects))
    return decisionObjects

def collectDecisionObjects(  hypos, filters, hltSeeding, hltSummary ):
    """
    Returns the set of all decision objects of HLT
    """
    decObjL1 = collectHLTSeedingDecisionObjects(hltSeeding)
    decObjHypo = collectHypoDecisionObjects(hypos, inputs = True, outputs = True)
    decObjFilter = collectFilterDecisionObjects(filters, inputs = True, outputs = True)
    # InputMaker are not needed explicitly as the Filter Outputs = InputMaker Inputs
    # and InputMaker Outputs = Hypo Inputs
    # Therefore we implicitly collect all navigaiton I/O of all InputMakers
    decObjSummary = collectHLTSummaryDecisionObjects(hltSummary)
    decisionObjects = set()
    decisionObjects.update(decObjL1)
    decisionObjects.update(decObjHypo)
    decisionObjects.update(decObjFilter)
    decisionObjects.update(decObjSummary)
    return list(sorted(decisionObjects))

def triggerSummaryCfg(flags, hypos):
    """
    Configures an algorithm(s) that should be run after the selection process
    Returns: ca, algorithm
    """
    acc = ComponentAccumulator()
    from TrigOutputHandling.TrigOutputHandlingConfig import DecisionSummaryMakerAlgCfg
    decisionSummaryAlg = DecisionSummaryMakerAlgCfg(flags)
    chainToLastCollection = OrderedDict() # keys are chain names, values are lists of collections


    # sort steps according to the step number i.e. strings Step1 Step2 ... Step10 Step11 rather than
    # alphabetic order Step10 Step11 Step1 Step2
    for stepName, stepHypos in sorted( hypos.items(), key=lambda x : __stepNumber(x[0]) ):
        # While filling the chainToLastCollection dict we will replace the content intentionally
        # i.e. the chain has typically multiple steps and we want the collections from the last one
        # In a step the chain is handled by hypo or hypo followed by the combo,
        # in second case we want out of the later.
        # For that we process first ComboHypo and then regular Hypos 
        # (TODO, review this whn config is symmetrised by addition of ComboHypos always)
        orderedStepHypos = sorted(stepHypos, key=lambda hypo: not __isCombo(hypo))

        chainToCollectionInStep = OrderedDict()
        for hypo in orderedStepHypos:
            hypoChains, hypoOutputKeys = __decisionsFromHypo( hypo )
            for chain in hypoChains:
                if chain not in chainToCollectionInStep:
                    chainToCollectionInStep[chain] = hypoOutputKeys                    
        chainToLastCollection.update( chainToCollectionInStep )

    from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
    from HLTSeeding.HLTSeedingConfig import mapThresholdToL1DecisionCollection
    if len(HLTMenuConfig.dicts()) == 0:
        __log.warning("No HLT menu, chains w/o algorithms are not handled")
    else:
        for chainName, chainDict in HLTMenuConfig.dicts().items():
            if chainName not in chainToLastCollection:
                __log.debug("The chain %s is not mentioned in any step", chainName)
                # TODO once sequences available in the menu we need to crosscheck it here
                assert len(chainDict['chainParts'])  == 1, "Chains w/o the steps can not have multiple parts in chainDict, it makes no sense: %s"%chainName
                chainToLastCollection[chainName] = [ mapThresholdToL1DecisionCollection( chainDict['chainParts'][0]['L1threshold'] ) ]

    for c, cont in chainToLastCollection.items():
        __log.debug("Final decision of chain  %s will be read from %d %s", c, len(cont), str(cont))
    # Flatten all the collections preserving the order
    collectionsWithFinalDecisions = []
    for chain, collections in chainToLastCollection.items():
        for c in collections:
            if c not in collectionsWithFinalDecisions:
                collectionsWithFinalDecisions.append(c)
    __log.debug("Final keys %s", collectionsWithFinalDecisions)
    decisionSummaryAlg.FinalDecisionKeys = collectionsWithFinalDecisions
    decisionSummaryAlg.FinalStepDecisions = dict(chainToLastCollection)
    decisionSummaryAlg.DecisionsSummaryKey = "HLTNav_Summary" # Output
    decisionSummaryAlg.SetFilterStatus = flags.Trigger.writeBS
    return acc, decisionSummaryAlg


def triggerMonitoringCfg(flags, hypos, filters, hltSeeding):
    """
    Configures components needed for monitoring chains
    """
    acc = ComponentAccumulator()
    TrigSignatureMoni, DecisionCollectorTool=CompFactory.getComps("TrigSignatureMoni","DecisionCollectorTool",)
    mon = TrigSignatureMoni()
    mon.L1Decisions = "HLTSeedingSummary"
    mon.FinalDecisionKey = "HLTNav_Summary" # Input
    if len(hypos) == 0:
        __log.warning("Menu is not configured")
        return acc, mon

    # lambda sort because we have strings Step1 Step2 ... Step10 Step11 and python sorts that
    # to Step10 Step11 Step1 Step2
    stepCounter = 1
    for stepName, stepHypos in sorted( hypos.items(), key=lambda x : __stepNumber(x[0])):
        assert __stepNumber(stepName) == stepCounter, "There are steps that have no hypos, decisions counting is not going to work"
        stepCounter += 1
        stepDecisionKeys = []
        stepFeatureDecisionKeys = []
        for hypo in stepHypos:
            hypoChains, hypoOutputKeys  = __decisionsFromHypo( hypo )
            if __isCombo(hypo):                
                stepDecisionKeys.extend( hypoOutputKeys )
            else:
                stepFeatureDecisionKeys.extend( hypoOutputKeys )

        dcEventTool = DecisionCollectorTool( "EventDecisionCollector" + stepName, Decisions=list(OrderedDict.fromkeys(stepDecisionKeys)))
        dcFeatureTool = DecisionCollectorTool( "FeatureDecisionCollector" + stepName, Decisions=list(OrderedDict.fromkeys(stepFeatureDecisionKeys)))
        __log.debug( "The step monitoring decisions in %s %s", dcEventTool.getName(), dcEventTool.Decisions)
        __log.debug( "The step monitoring decisions in %s %s", dcFeatureTool.getName(), dcFeatureTool.Decisions)
        mon.DecisionCollectorTools += [ dcEventTool ]
        mon.FeatureCollectorTools  += [ dcFeatureTool ]

    # Configure additional chain error monitoring for athenaHLT/online:
    if flags.Trigger.Online.isPartition:
        from TrigServices.TrigServicesConfig import TrigServicesCfg
        onlineServicesAcc = TrigServicesCfg(flags)
        hltEventLoopMgr = onlineServicesAcc.getPrimary()

        hltEventLoopMgr.TrigErrorMonTool.AlgToChainTool = CompFactory.TrigCompositeUtils.AlgToChainTool()
        hltEventLoopMgr.TrigErrorMonTool.MonTool.defineHistogram(
            'ErrorChainName,ErrorCode', path='EXPERT', type='TH2I',
            title='Error StatusCodes per chain;Chain name;StatusCode',
            xbins=1, xmin=0, xmax=1, ybins=1, ymin=0, ymax=1)

        acc.merge(onlineServicesAcc)

    mon.L1Decisions  = hltSeeding.HLTSeedingSummaryKey

    from DecisionHandling.DecisionHandlingConfig import setupFilterMonitoring
    [ [ setupFilterMonitoring( flags, alg ) for alg in algs ]  for algs in list(filters.values()) ]

    return acc, mon



def triggerOutputCfg(flags, hypos):
    # Following cases are considered:
    # 1) Running in partition or athenaHLT - configure BS output written by the HLT framework
    # 2) Running offline athena and writing BS - configure BS output written by OutputStream alg
    # 3) Running offline athena with POOL output - configure POOL output written by OutputStream alg
    onlineWriteBS = False
    offlineWriteBS = False
    writePOOL = False

    if flags.Trigger.writeBS:
        if flags.Trigger.Online.isPartition:
            onlineWriteBS = True
        else:
            offlineWriteBS = True
    if flags.Output.doWriteRDO or flags.Output.doWriteESD or flags.Output.doWriteAOD:
        writePOOL = True

    # Consistency checks
    if offlineWriteBS and not flags.Output.doWriteBS:
        __log.error('flags.Trigger.writeBS is True but flags.Output.doWriteBS is False')
        return None, ''
    if writePOOL and onlineWriteBS:
        __log.error("POOL HLT output writing is configured online")
        return None, ''
    if writePOOL and offlineWriteBS:
        __log.error("Writing HLT output to both BS and POOL in one job is not supported at the moment")
        return None, ''

    # Determine EDM set name
    edmSet = ''
    if writePOOL:
        edmSet = flags.Trigger.AODEDMSet if flags.Output.doWriteAOD else flags.Trigger.ESDEDMSet
    elif onlineWriteBS or offlineWriteBS:
        edmSet = 'BS'

    # Create the configuration
    if onlineWriteBS:
        __log.info("Configuring online ByteStream HLT output")
        acc = triggerBSOutputCfg(flags, hypos)
    elif offlineWriteBS:
        __log.info("Configuring offline ByteStream HLT output")
        acc = triggerBSOutputCfg(flags, hypos, offline=True)
    elif writePOOL:
        __log.info("Configuring POOL HLT output")
        acc = triggerPOOLOutputCfg(flags)
    else:
        __log.info("No HLT output writing is configured")
        acc = ComponentAccumulator()

    return acc, edmSet


def triggerBSOutputCfg(flags, hypos, offline=False):
    """
    Returns CA with algorithms and/or tools required to do the serialisation

    decObj - list of all navigation objects
    decObjHypoOut - list of decisions produced by hypos
    hypos - the {stepName: hypoList} dictionary with all hypo algorithms - used to find the PEB decision keys
    offline - if true CA contains algorithms that need to be merged to output stream sequence,
              if false the CA contains a tool that needs to be added to HltEventLoopMgr
    """
    from TrigEDMConfig import DataScoutingInfo
    from TrigEDMConfig.TriggerEDM import getRun3BSList
    from TrigEDMConfig.TriggerEDMDefs import allowTruncation
    from TrigOutputHandling.TrigOutputHandlingConfig import TriggerEDMSerialiserToolCfg, StreamTagMakerToolCfg, TriggerBitsMakerToolCfg

    # Get list of all output collections for ByteStream (including DataScouting)
    collectionsToBS = getRun3BSList(["BS"] + DataScoutingInfo.getAllDataScoutingIdentifiers())

    # Tool serialising EDM objects to fill the HLT result
    serialiser = TriggerEDMSerialiserToolCfg(flags)

    for typekey, bsfragments, props in collectionsToBS:
        # Translate fragment names like BS, CostMonDS to ROB fragment IDs 0 (full result), 1, ... (DS results)
        moduleIDs = sorted(DataScoutingInfo.getFullHLTResultID() if f == 'BS' else
                           DataScoutingInfo.getDataScoutingResultID(f)
                           for f in bsfragments)

        __log.debug('adding to serialiser list: %s, modules: %s', typekey, moduleIDs)
        serialiser.addCollection(typekey, moduleIDs, allowTruncation = allowTruncation in props)

    # Tools adding stream tags and trigger bits to HLT result
    stmaker = StreamTagMakerToolCfg()
    bitsmaker = TriggerBitsMakerToolCfg()

    # Map hypo decisions producing PEBInfo to StreamTagMakerTool.PEBDecisionKeys
    PEBKeys = []
    for hypoList in hypos.values():
        for hypo in hypoList:
            if hypo.getFullJobOptName().startswith('PEBInfoWriterAlg/'):
                PEBKeys.append(str(hypo.HypoOutputDecisions))

    PEBKeys = sorted(set(PEBKeys))
    __log.debug('Setting StreamTagMakerTool.PEBDecisionKeys = %s', PEBKeys)
    stmaker.PEBDecisionKeys = PEBKeys

    acc = ComponentAccumulator()

    if offline:
        # Create HLT result maker and alg
        from TrigOutputHandling.TrigOutputHandlingConfig import HLTResultMTMakerCfg
        HLTResultMTMakerAlg=CompFactory.HLTResultMTMakerAlg
        hltResultMakerTool = HLTResultMTMakerCfg(flags)
        hltResultMakerTool.StreamTagMaker = stmaker
        hltResultMakerTool.MakerTools = [bitsmaker, serialiser]
        hltResultMakerAlg = HLTResultMTMakerAlg()
        hltResultMakerAlg.ResultMaker = hltResultMakerTool

        # Provide ByteStreamMetaData from input, required by the result maker tool
        if flags.Input.Format is Format.BS:
            from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamReadCfg
            readBSAcc = ByteStreamReadCfg(flags)
            readBSAcc.getEventAlgo('SGInputLoader').Load.add(
                ('ByteStreamMetadataContainer', 'InputMetaDataStore+ByteStreamMetadata'))
            acc.merge(readBSAcc)
        else:
            # No BS metadata (thus no DetectorMask) in POOL files, need to disable the checks using it
            hltResultMakerAlg.ResultMaker.ExtraROBs = []
            hltResultMakerAlg.ResultMaker.ExtraSubDets = []

        # Transfer trigger bits to xTrigDecision which is read by offline BS writing ByteStreamCnvSvc
        decmaker = CompFactory.getComp("TrigDec::TrigDecisionMakerMT")("TrigDecMakerMT")

        # Schedule the insertion of L1 prescales into the conditions store
        # Required for writing L1 trigger bits to xTrigDecision
        from TrigConfigSvc.TrigConfigSvcCfg import L1PrescaleCondAlgCfg
        acc.merge(L1PrescaleCondAlgCfg(flags))

        # Create OutputStream alg
        from TriggerJobOpts.TriggerByteStreamConfig import ByteStreamWriteCfg
        writingOutputs = ["HLT::HLTResultMT#HLTResultMT"]
        writingInputs = [("HLT::HLTResultMT", "HLTResultMT"),
                         ("xAOD::TrigDecision", "xTrigDecision")]

        # Rewrite LVL1 result if LVL1 simulation is enabled
        if flags.Trigger.doLVL1:
            if flags.Trigger.enableL1MuonPhase1 or flags.Trigger.enableL1CaloPhase1:
                writingOutputs += ['xAOD::TrigCompositeContainer#L1TriggerResult']
                writingInputs += [('xAOD::TrigCompositeContainer', 'StoreGateSvc+L1TriggerResult')]
            if flags.Trigger.enableL1CaloLegacy or not flags.Trigger.enableL1MuonPhase1:
                writingOutputs += ['ROIB::RoIBResult#RoIBResult']
                writingInputs += [('ROIB::RoIBResult', 'StoreGateSvc+RoIBResult')]

            from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamEncoderCfg
            acc.merge(L1TriggerByteStreamEncoderCfg(flags))

        writingAcc = ByteStreamWriteCfg(flags, type_names=writingOutputs, extra_inputs=writingInputs)
        writingAcc.addEventAlgo(hltResultMakerAlg)
        writingAcc.addEventAlgo(decmaker)
        acc.merge(writingAcc)

    else:
        from TrigServices.TrigServicesConfig import TrigServicesCfg
        onlineServicesAcc = TrigServicesCfg(flags)
        hltEventLoopMgr = onlineServicesAcc.getPrimary()
        hltEventLoopMgr.ResultMaker.StreamTagMaker = stmaker
        hltEventLoopMgr.ResultMaker.MakerTools = [serialiser, bitsmaker]
        onlineServicesAcc.getEventAlgo('SGInputLoader').Load.add(
            ('ByteStreamMetadataContainer', 'InputMetaDataStore+ByteStreamMetadata'))
        acc.merge(onlineServicesAcc)
    return acc


def triggerPOOLOutputCfg(flags):
    # Get the list of output collections from TriggerEDM
    acc = ComponentAccumulator()

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList

    # Produce trigger bits
    bitsmaker = CompFactory.TriggerBitsMakerTool()
    decmaker = CompFactory.TrigDec.TrigDecisionMakerMT("TrigDecMakerMT", BitsMakerTool = bitsmaker)
    acc.addEventAlgo( decmaker )

    # Export trigger metadata during the trigger execution when running with POOL output.
    from .TriggerRecoConfig import TriggerMetadataWriterCfg
    metadataAcc, metadataOutputs = TriggerMetadataWriterCfg(flags)
    acc.merge( metadataAcc )

    # Produce xAOD L1 RoIs from RoIBResult
    from AnalysisTriggerAlgs.AnalysisTriggerAlgsConfig import RoIBResultToxAODCfg
    xRoIBResultAcc, xRoIBResultOutputs = RoIBResultToxAODCfg(flags)
    acc.merge(xRoIBResultAcc)
    # Ensure outputs are produced before streamAlg runs


    # Create OutputStream
    for doit, outputType, edmSet in [( flags.Output.doWriteRDO, 'RDO', flags.Trigger.ESDEDMSet), # not a mistake, RDO content is meant to be as ESD
                                     ( flags.Output.doWriteESD, 'ESD', flags.Trigger.ESDEDMSet), 
                                     ( flags.Output.doWriteAOD, 'AOD', flags.Trigger.AODEDMSet)]:
        if not doit: continue

        edmList = getTriggerEDMList(edmSet, flags.Trigger.EDMVersion, flags.Trigger.ExtraEDMList)

        # Build the output ItemList
        itemsToRecord = []
        for edmType, edmKeys in edmList.items():
            itemsToRecord.extend([edmType+'#'+collKey for collKey in edmKeys])

        # Add EventInfo
        itemsToRecord.append('xAOD::EventInfo#EventInfo')
        itemsToRecord.append('xAOD::EventAuxInfo#EventInfoAux.')


        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg, outputStreamName
        acc.merge(OutputStreamCfg(flags, outputType, ItemList=itemsToRecord,
                                  disableEventTag=True, takeItemsFromInput=(outputType == 'RDO')))
        from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
        acc.merge(SetupMetaDataForStreamCfg(flags, outputType,
                                            createMetadata=[MetadataCategory.TriggerMenuMetaData]))

        alg = acc.getEventAlgo(outputStreamName(outputType))
        # Ensure OutputStream runs after TrigDecisionMakerMT and xAODMenuWriterMT
        alg.ExtraInputs |= {
            ("xAOD::TrigDecision", str(decmaker.TrigDecisionKey)),
            ("xAOD::TrigConfKeys", metadataOutputs)} | set(xRoIBResultOutputs)

    return acc


def triggerMergeViewsCfg( flags, viewMakers ):
    """Configure the view merging algorithm"""

    from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3, InViews

    acc = ComponentAccumulator()
    mergingTool = CompFactory.HLTEDMCreator("ViewsMergingTool")
    alg = CompFactory.HLTEDMCreatorAlg("EDMCreatorAlg",
                                       OutputTools = [mergingTool])

    # configure views merging
    needMerging = [x for x in TriggerHLTListRun3 if len(x) >= 4 and
                   any(isinstance(v, InViews) for v in x[3])]
    __log.info("These collections need merging: %s", " ".join([ c[0] for c in needMerging ]))

    for coll in needMerging:
        collType, collName = coll[0].split("#")
        collType = collType.split(":")[-1]
        possibleViews = [ str(v) for v in coll[3] if isinstance(v, InViews) ]
        for viewsColl in possibleViews:
            attrView = getattr(mergingTool, collType+"Views", [])
            attrInView = getattr(mergingTool, collType+"InViews", [])
            attrName = getattr(mergingTool, collType, [])
            attrView.append( viewsColl )
            attrInView.append( collName )
            attrName.append( collName )

            setattr(mergingTool, collType+"Views", attrView )
            setattr(mergingTool, collType+"InViews", attrInView )
            setattr(mergingTool, collType, attrName )
            producer = [ maker for maker in viewMakers if maker.Views == viewsColl ]
            if len(producer) == 0:
                __log.warning("The producer of %s for %s not in the menu, its outputs won't ever make it out of the HLT", viewsColl, coll)
                continue
            if len(producer) > 1:
                for pr in producer[1:]:
                    if pr != producer[0]:
                        __log.error("Several View making algorithms produce the same output collection %s: %s", viewsColl, ' '.join([p.getName() for p in producer ]))
                        continue

    acc.addEventAlgo(alg)
    return acc


def triggerEDMGapFillerCfg( flags, edmSet, decObj=[], decObjHypoOut=[], extraInputs=[], extraOutputs=[] ):
    """Configure the EDM gap filler"""

    from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3, Alias

    # Ignore the following collections in the GapFiller. List of regular expressions
    # that are fully matched against the EDM entry ("type#key").
    ignore = [
        # GapFiller always creates Aux stores
        ".*AuxContainer#.*", ".*AuxInfo#.*",
    ]
    if flags.Trigger.doHLT:
        # Online, these collections will be created after the EDMCreator runs
        ignore += ["xAOD::TrigCompositeContainer#HLTNav_Summary_OnlineSlimmed",
                   "xAOD::TrigCompositeContainer#HLT_RuntimeMetadata"]

    acc = ComponentAccumulator()
    tool = CompFactory.HLTEDMCreator(f"GapFiller_{'' if edmSet==['BS'] else '_'+'_'.join(edmSet)}")
    alg = CompFactory.HLTEDMCreatorAlg("EDMCreatorAlg",
                                       OutputTools = [tool])
    alg.ExtraInputs = set(extraInputs)
    alg.ExtraOutputs = set(extraOutputs)

    if len(edmSet) != 0:
        groupedByType = defaultdict( list )
        re_ignore = [re.compile(x) for x in ignore]

        # scan the EDM
        for el in TriggerHLTListRun3:
            if not any([ outputType in el[1].split() for outputType in edmSet ]):
                continue
            if any(ign.fullmatch(el[0]) for ign in re_ignore):
                continue
            collType, collName = el[0].split("#")
            if len(el) >= 4: # see if there is an alias
                aliases = [ str(a) for a in el[3] if isinstance(a, Alias) ]
                if len(aliases) == 1:
                    __log.debug("GapFiller configuration found an aliased type '%s' for '%s'", aliases[0], collType)
                    collType = aliases[0]
                elif len(aliases) > 1:
                    __log.error("GapFiller configuration found inconsistent '%s' (too many aliases?)", aliases)

            groupedByType[collType].append( collName )

        for collType, collNameList in groupedByType.items():
            propName = collType.split(":")[-1]
            if hasattr( tool, propName ):
                setattr( tool, propName, collNameList )
                __log.debug("GapFiller will create EDM collection type '%s' for '%s'", collType, collNameList)
            else:
                __log.debug("EDM collections of type %s are not going to be added to StoreGate, if not created by the HLT", collType )

    if decObj or decObjHypoOut:
        __log.debug("GapFiller is ensuring the creation of all the decision object collections")
        __log.debug("'%s'", decObj)
        # Gap filler is also used to perform re-mapping of the HypoAlg outputs which is a sub-set of decObj
        tool.FixLinks = list(decObjHypoOut)
        # Append and hence confirm all TrigComposite collections
        tool.TrigCompositeContainer += list(decObj)

    acc.addEventAlgo(alg, primary=True)
    return acc


def triggerRunCfg( flags, menu=None ):
    """
    top of the trigger config (for real triggering online or on MC)
    Returns: ca only
    """
    acc = ComponentAccumulator()

    # L1ConfigSvc needed for HLTSeeding
    from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg

    acc.merge( L1ConfigSvcCfg(flags) )

    acc.addSequence( seqOR( "HLTTop") )

    # HLTPreSeq only used for CostMon so far, skip if CostMon disabled
    if flags.Trigger.CostMonitoring.doCostMonitoring:
        acc.addSequence( parOR("HLTPreSeq"), parentName="HLTTop" )

    from TrigCostMonitor.TrigCostMonitorConfig import TrigCostMonitorCfg
    acc.merge( TrigCostMonitorCfg( flags ), sequenceName="HLTPreSeq" )


    acc.addSequence( parOR("HLTBeginSeq"), parentName="HLTTop" )
    # bit of a hack as for "legacy" type JO a seq name for cache creators has to be given,
    # in newJO realm the seqName will be removed as a comp fragment shoudl be unaware of where it will be attached
 
    if flags.Trigger.doCFEmulationTest:
        from TriggerMenuMT.CFtest.EmuStepProcessingConfig import emulateHLTSeedingCfg
        hltSeedingAcc = emulateHLTSeedingCfg(flags)
    else:
        acc.merge( triggerIDCCacheCreatorsCfg( flags, seqName="AthAlgSeq" ), sequenceName="HLTBeginSeq" )
        from HLTSeeding.HLTSeedingConfig import HLTSeedingCfg
        hltSeedingAcc = HLTSeedingCfg( flags )
    # TODO, once moved to newJO the algorithm can be added to hltSeedingAcc and merging will be sufficient here
    acc.merge( hltSeedingAcc,  sequenceName="HLTBeginSeq" )

    # detour to the menu here, (missing now, instead a temporary hack)
    if menu:
        menuAcc = menu( flags )
        HLTSteps = menuAcc.getSequence( "HLTAllSteps" )
        __log.info( "Configured menu with %d steps", len(HLTSteps.Members))
        acc.merge( menuAcc, sequenceName="HLTTop")

    # collect hypothesis algorithms from all sequence
    hypos = collectHypos( HLTSteps )
    filters = collectFilters( HLTSteps )
    acc.addSequence( parOR("HLTEndSeq"), parentName="HLTTop" )
    acc.addSequence( seqAND("HLTFinalizeSeq"), parentName="HLTEndSeq" )

    nfilters = sum(len(v) for v in filters.values())
    nhypos = sum(len(v) for v in hypos.values())
    __log.info( "Algorithms counting: Number of Filter algorithms: %d  -  Number of Hypo algoirthms: %d", nfilters , nhypos)

    summaryAcc, summaryAlg = triggerSummaryCfg( flags, hypos )
    acc.merge( summaryAcc, sequenceName="HLTFinalizeSeq" )
    if flags.Trigger.doCFEmulationTest:
        summaryAlg.Prescaler=CompFactory.PrescalingEmulationTool()
    acc.addEventAlgo( summaryAlg, sequenceName="HLTFinalizeSeq" )
    acc.merge( triggerEndOfEventCfg(flags), sequenceName="HLTFinalizeSeq" )

    #once menu is included we should configure monitoring here as below
    hltSeedingAlg = hltSeedingAcc.getEventAlgo("HLTSeeding")

    monitoringAcc, monitoringAlg = triggerMonitoringCfg( flags, hypos, filters, hltSeedingAlg )
    acc.merge( monitoringAcc, sequenceName="HLTEndSeq" )
    acc.addEventAlgo( monitoringAlg, sequenceName="HLTEndSeq" )

    decObj = collectDecisionObjects( hypos, filters, hltSeedingAlg, summaryAlg )
    decObjHypoOut = collectHypoDecisionObjects(hypos, inputs=False, outputs=True)
    __log.info( "Number of decision objects found in HLT CF %d of which %d are the outputs of hypos",
                len(decObj), len(decObjHypoOut) )
    __log.debug( decObj )

    # configure components need to normalise output before writing out
    viewMakers = collectViewMakers( HLTSteps )

    # Add HLT Navigation to EDM list
    from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3, addHLTNavigationToEDMList
    __log.info( "Number of EDM items before adding navigation: %d", len(TriggerHLTListRun3))
    addHLTNavigationToEDMList(flags, TriggerHLTListRun3, decObj, decObjHypoOut)
    __log.info( "Number of EDM items after adding navigation: %d", len(TriggerHLTListRun3))

    # Add Extra to EDM list
    if flags.Trigger.ExtraEDMList:
        from TrigEDMConfig.TriggerEDMRun3 import addExtraCollectionsToEDMList
        __log.info( "Adding extra collections to EDM: %s", str(flags.Trigger.ExtraEDMList))
        addExtraCollectionsToEDMList(TriggerHLTListRun3, flags.Trigger.ExtraEDMList)

    # Configure output writing
    outputAcc, edmSet = triggerOutputCfg( flags, hypos )
    acc.merge( outputAcc, sequenceName="HLTTop" )

    # Cost monitoring should be finished between acceptedEventTopSeq and EDMCreator
    from TrigCostMonitor.TrigCostMonitorConfig import TrigCostMonitorFinalizeCfg
    costFinalizeAlg = TrigCostMonitorFinalizeCfg(flags)
    if costFinalizeAlg: # None if Cost Monitoring is turned off
        acc.addEventAlgo(costFinalizeAlg, sequenceName="HLTFinalizeSeq" )

    if edmSet:
        # The order is important: 1) view merging, 2) gap filling
        acc.merge( triggerMergeViewsCfg(flags, viewMakers), sequenceName="HLTFinalizeSeq" )
        # For BS output, the EDM gap-filling is done in Reco. Online we only ensure a
        # consistent set of decision objects (TrigCompositeContainer):
        acc.merge( triggerEDMGapFillerCfg(flags, [edmSet] if edmSet!='BS' else [],
                                          decObj, decObjHypoOut), sequenceName="HLTFinalizeSeq" )

        if flags.Trigger.doOnlineNavigationCompactification:
            from TrigNavSlimmingMT.TrigNavSlimmingMTConfig import getTrigNavSlimmingMTOnlineConfig
            onlineSlimAlg = getTrigNavSlimmingMTOnlineConfig(flags)
            acc.addEventAlgo( onlineSlimAlg, sequenceName="HLTFinalizeSeq" )

    # Cleanup menu config to allow garbage collection (ATR-28855)
    from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
    HLTMenuConfig.destroy()

    return acc


def triggerIDCCacheCreatorsCfg(flags, seqName = None):
    """
    Configures IDC cache loading
    Returns: CA
    """
    acc = ComponentAccumulator(seqName)

    if flags.Trigger.doMuon:
        from MuonConfig.MuonBytestreamDecodeConfig import MuonCacheCfg
        acc.merge( MuonCacheCfg(flags), sequenceName = seqName )

        from MuonConfig.MuonRdoDecodeConfig import MuonPrdCacheCfg
        acc.merge( MuonPrdCacheCfg(flags), sequenceName = seqName )

    if flags.Trigger.doID:
        from TrigInDetConfig.TrigInDetConfig import InDetIDCCacheCreatorCfg
        acc.merge( InDetIDCCacheCreatorCfg(flags), sequenceName = seqName )

    return acc


def triggerEndOfEventCfg(flags):
    from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig

    acceptedEventChainDicts = [cd for cd in HLTMenuConfig.dictsList() \
                               if 'Calib' in cd['signatures'] \
                               and 'acceptedevts' in cd['chainParts'][0]['purpose']]

    acc = ComponentAccumulator()

    # If no relevant chains or just not desired, can shortcut and return empty CA
    if not (flags.Trigger.enableEndOfEventProcessing and acceptedEventChainDicts):
        return acc

    # Add end-of-event sequences executed conditionally on the DecisionSummaryMakerAlg filter status
    endOfEventRoIMaker = CompFactory.EndOfEventROIConfirmerAlg('EndOfEventROIConfirmerAlg')
    acc.addEventAlgo( endOfEventRoIMaker )
    acc.addSequence( parOR("acceptedEventTopSeq") )
    acc.getSequence("acceptedEventTopSeq").IgnoreFilterPassed=True

    # Define the alg configuration by purpose
    def EndOfEventSeqCfg(flags, prescaleChain, purposes):
        acc = ComponentAccumulator()
        seqLabel = prescaleChain.replace('HLT_acceptedevts','')
        seqName = 'acceptedEventSeq'+seqLabel
        acc.addSequence( seqAND(seqName) )

        endOfEventFilterAlg = CompFactory.EndOfEventFilterAlg('EndOfEventFilterAlg'+seqLabel, ChainName=prescaleChain)
        acc.addEventAlgo(endOfEventFilterAlg, sequenceName=seqName)
        # The LAr Noise Burst end-of-event sequence
        if 'larnoiseburst' in purposes:
            # Add stream filter to EndOfEventFilterAlg
            # Only accept events going to streams that already do full calo reco
            # CosmicCalo explicitly requested [ATR-26096]
            endOfEventFilterAlg.StreamFilter = ['Main','VBFDelayed','TLA','DarkJetPEBTLA','FTagPEBTLA','CosmicCalo']

            from TriggerMenuMT.HLT.CalibCosmicMon.CalibChainConfiguration import getLArNoiseBurstRecoCfg
            acc.merge(getLArNoiseBurstRecoCfg(flags), sequenceName=seqName)
        elif any(purpose.startswith("met") for purpose in purposes):
            from TriggerMenuMT.HLT.MET.EndOfEvent import getMETRecoSequences
            metcfg, rois, streams = getMETRecoSequences(flags, purposes)
            endOfEventFilterAlg.StreamFilter = streams
            endOfEventRoIMaker.RoIs = [x for x in rois if x not in endOfEventRoIMaker.RoIs]
            acc.merge(metcfg, sequenceName=seqName)

        return acc
            

    for acceptedEventChainDict in acceptedEventChainDicts:
        # Common config for each chain
        prescaleChain = acceptedEventChainDict['chainName']
        # Now add chain-specific end-of-event sequences executed conditionally on the prescale
        purposes = acceptedEventChainDict['chainParts'][0]['purpose']

        acc.merge(EndOfEventSeqCfg(flags, prescaleChain, purposes), sequenceName="acceptedEventTopSeq")

    return acc

def triggerPostRunCfg(flags):
    """
    Configures components needed for processing trigger information in RAW/ESD step
    Returns: ca only
    """
    acc = ComponentAccumulator()
    # configure in order BS decodnig, EDM gap filling, insertion of trigger metadata to ESD

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Trigger.forceEnableAllChains = True
    flags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1",]
    from AthenaConfiguration.TestDefaults import defaultGeometryTags
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.lock()

    def testMenu(flags):
        menuCA = ComponentAccumulator()
        menuCA.addSequence( seqAND("HLTAllSteps") )
        return menuCA

    acc = triggerRunCfg( flags, menu = testMenu )

    f=open("TriggerRunConf.pkl","wb")
    acc.store(f)
    f.close()
