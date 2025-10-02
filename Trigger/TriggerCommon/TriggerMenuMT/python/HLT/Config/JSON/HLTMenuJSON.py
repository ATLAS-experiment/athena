# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import re
import json
from functools import cache
from TrigConfigSvc.TrigConfigSvcCfg import getHLTMenuFileName
from AthenaCommon.CFElements import getSequenceChildren
from AthenaCommon.Logging import logging
__log = logging.getLogger( __name__ )

# remove prescale suffixes
def __getMenuBaseName(menuName):
    pattern = re.compile(r'_v\d+|DC14')
    patternPos = pattern.search(menuName)
    if patternPos:
        menuName=menuName[:patternPos.end()]
    else:
        __log.info('Can\'t find pattern to shorten menu name, either non-existent in name or not implemented.')
    return menuName  

def __getStepsDataFromAlgSequence(HLTAllSteps):
    """ Generates a list where the index corresponds to a Step number and the stored object is a list of Sequencers making up the Step 
    """
    stepsData = []
    if HLTAllSteps is not None:
        for HLTStep in HLTAllSteps.Members:
            if "_reco" not in HLTStep.getName(): # Avoid the pre-step Filter execution
                for Step in getSequenceChildren( HLTStep ):
                    for View in getSequenceChildren( Step ):
                        for Reco in getSequenceChildren( View ):
                            if "_reco" in Reco.getName() and HLTStep.getName() not in stepsData:
                                stepsData.append( HLTStep.Members )
                                break
                continue

            stepsData.append( HLTStep.Members )
    else:
        __log.warn( "No HLTAllSteps sequencer, will not export per-Step data for chains.")
    return stepsData

@cache  # called frequently
def __getFilterChains(filterAlg):
    if hasattr(filterAlg, "Chains"):
        # Format: leg000_CHAIN or CHAIN
        return set(ch[7:] if ch.startswith('leg') else ch for ch in filterAlg.Chains)
    else:
        return set()

def __getChainSequencers(stepsData, chainName):
    """ Finds the Filter which is responsible for this Chain in each Step.
        Return a list of the per-Step name() of the Sequencer which is unlocked by the Chain's Filter in the Step.
    """
    sequencers = []
    for counter, step in enumerate(stepsData, 1):
        mySequencer = None        
        for sequencer in step:     
            try:
                sequencerFilter = sequencer.Members[0] # Always the first child in the step
            except (AttributeError, IndexError):
                continue  # empty steps
            if chainName in __getFilterChains(sequencerFilter):
                if mySequencer is not None:
                    __log.error( "Multiple Filters found (corresponding Sequencers %s, %s) for %s in Step %i!",
                        mySequencer.getName(), sequencer.getName(), chainName, counter)
                mySequencer = sequencer

        sequencers.append(mySequencer.getName() if mySequencer else "")
    # drop trailing empty names
    while len(sequencers) != 0 and sequencers[-1] == "":
        del sequencers[-1]
    return sequencers

def __getSequencerAlgs(stepsData):
    """ For each Sequencer in each Step, return a flat list of the full name of all Algorithms under the Sequencer
    """
    from AthenaCommon.CFElements import findAllAlgorithms
    sequencerAlgs = {}
    for step in stepsData:
        for sequencer in step:
            sequencerAlgs[ sequencer.getName() ] = list(map(lambda x: x.getFullJobOptName(), findAllAlgorithms(sequencer)))
    return sorted(sequencerAlgs.items(), key=lambda t: t[0])

def generateJSON(flags, chainDicts, HLTAllSteps):
    """ Generates JSON given the ChainProps and sequences
    """
    # Menu dictionary that is used to create the JSON content
    menuDict = {"filetype": "hltmenu",
                "name": __getMenuBaseName(flags.Trigger.triggerMenuSetup),
                "chains": {},
                "streams": {},
                "sequencers": {}}

    # List of steps data for sequencers
    stepsData = __getStepsDataFromAlgSequence(HLTAllSteps)
    from TriggerMenuMT.HLT.Menu import StreamInfo
    for chain in chainDicts:
        # Prepare information for stream list and fill separate dictionary
        chainStreamTags = []
        for streamName in chain["stream"]:
            streamTag = StreamInfo.getStreamTag(streamName)
            # Stream needs to have been defined in StreamInfo.py otherwise is not added to JSON
            if streamTag is None:
                __log.error('Stream %s does not have StreamTags defined excluding from JSON', streamName)
                continue
            # Add stream to the chain
            chainStreamTags.append(streamName)
            # If not already listed, add stream details to stream dictionary
            if streamName not in menuDict["streams"]:
                menuDict["streams"][streamName] = {
                    "name": streamName,
                    "type": streamTag.type(),
                    "obeyLB": streamTag.obeysLumiBlock(),
                    "forceFullEventBuilding": streamTag.forceFullEventBuilding()
                }

        # Find L1 Threshold information for current chain
        l1Thresholds  = []
        
        [ l1Thresholds.append(p['L1threshold']) for p in chain['chainParts'] ]

        # Now have all information to write the chain to the menu dictionary
        chainName = chain["chainName"]
        menuDict["chains"][chainName] = {
            "counter": chain["chainCounter"],
            "nameHash": chain["chainNameHash"],
            "legMultiplicities": chain["chainMultiplicities"],
            "l1item": chain["L1item"],
            "l1thresholds": l1Thresholds,
            "groups": chain["groups"],
            "streams": chainStreamTags,
            "sequencers": __getChainSequencers(stepsData, chainName)
        }

    # All algorithms executed by a given Sequencer
    menuDict["sequencers"].update( __getSequencerAlgs(stepsData) )

    __validateJSON(menuDict)

    # Menu dictionary now completed, write to JSON
    fileName = getHLTMenuFileName( flags)
    __log.info( "Writing HLT Menu JSON to %s", fileName )
    with open( fileName, 'w' ) as fp:
        json.dump( menuDict, fp, indent=4, sort_keys=False )


def __validateJSON(menuDict):
    """ Runs some validation checks which may pick up on issues with the menu
    """
    pass
    #__validateGlobalAlgs(menuDict) # To be enabled once the current offenders are fixed

def __validateGlobalAlgs(menuDict):
    """ Check that global algs only go into one Step
    """
    algToStep = {}
    import re
    from AthenaConfiguration.ComponentFactory import CompFactory
    inError = False
    for seqName, seqeuncer in menuDict["sequencers"].items():
        stepNumber = int(re.search(r'\d+', seqName).group()) # Obtain first number from string
        fullEventMode = False
        for alg in seqeuncer:
            if isinstance(alg, CompFactory.EventViewCreatorAlgorithm):
                fullEventMode = False
            elif isinstance(alg, CompFactory.InputMakerForRoI):
                fullEventMode = True
            if not fullEventMode:
                continue
            if alg in algToStep and algToStep[alg] != stepNumber:
                __log.error("{} is a full-event context alg, it should only be running in one Step, however it is in both Steps {} and {}".format(alg, stepNumber, algToStep[alg]))
                inError = True
            else:
                algToStep[alg] = stepNumber
    if inError:
        raise Exception("[validateJSON] Problems detected in validateGlobalAlgs().")    
