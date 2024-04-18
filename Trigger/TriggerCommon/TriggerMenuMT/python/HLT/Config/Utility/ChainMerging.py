# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from TriggerMenuMT.HLT.Config.Utility.MenuAlignmentTools import get_alignment_group_ordering as getAlignmentGroupOrdering
from TriggerMenuMT.HLT.Config.MenuComponents import Chain, ChainStep, EmptyMenuSequence, EmptyMenuSequenceCfg, isEmptySequenceCfg

from AthenaCommon.Logging import logging
from DecisionHandling.DecisionHandlingConfig import ComboHypoCfg
from TrigCompositeUtils.TrigCompositeUtils import legName
import functools
from copy import deepcopy
import re

log = logging.getLogger( __name__ )

def mergeChainDefs(listOfChainDefs, chainDict, perSig_lengthOfChainConfigs = None):
    #chainDefList is a list of Chain() objects
    #one for each part in the chain
    
    strategy = chainDict["mergingStrategy"]
    offset = chainDict["mergingOffset"]
    log.debug("[mergeChainDefs] %s: Combine by using %s merging", chainDict['chainName'], strategy)

    leg_numbering = []

    if 'Bjet' in chainDict['signatures'] and 'Jet' in chainDict['signatures']:#and chainDict['Signature'] == 'Bjet':
        leg_numbering = [it for it,s in enumerate(chainDict['signatures'])]# if s != 'Jet']

    if strategy=="parallel":
        return mergeParallel(listOfChainDefs,  offset)
    elif strategy=="serial":
        return mergeSerial(listOfChainDefs)

    elif strategy=="auto":
        ordering = getAlignmentGroupOrdering()
        merging_dict = {}
        for ich,cConfig in enumerate(listOfChainDefs):
            chain_ag = cConfig.alignmentGroups[0]
            if chain_ag not in ordering:
                log.error("[mergeChainDefs] Alignment group %s can't be auto-merged because it's not in the grouping list!",chain_ag)
            if chain_ag in merging_dict:
                merging_dict[chain_ag] += [ich]
            else:
                merging_dict[chain_ag] = [ich]
                
        tmp_merged = []
        tmp_merged_ordering = []
        for ag in merging_dict:
            if len(merging_dict[ag]) > 1:
                log.debug("[mergeChainDefs] parallel merging")
                new_chain_defs, perSig_lengthOfChainConfigs = mergeParallel(list( listOfChainDefs[i] for i in merging_dict[ag] ), offset, leg_numbering, perSig_lengthOfChainConfigs)
                tmp_merged += [new_chain_defs]
                tmp_merged_ordering += [ordering.index(ag)]
            else:
                log.debug("[mergeChainDefs] don't need to parallel merge")
                tmp_merged += [listOfChainDefs[merging_dict[ag][0]]]
                tmp_merged_ordering += [ordering.index(ag)]
        
        #reset the ordering to index from zero (padding comes later!)
        merged_ordering = [-1]*len(tmp_merged_ordering)
        copy_ordering = tmp_merged_ordering.copy()
        tmp_val = 0
        while len(copy_ordering) > 0:
            min_index = tmp_merged_ordering.index(min(copy_ordering))
            copy_ordering.pop(copy_ordering.index(min(copy_ordering)))
            merged_ordering[min_index] = tmp_val
            tmp_val += 1
            
        # only serial merge if necessary
        if len(tmp_merged) == 1:            
            if perSig_lengthOfChainConfigs is None:
                log.debug("[mergeChainDefs] tmp merged has length 1, returning 0th element")
                return tmp_merged[0]
            else:
                log.debug("[mergeChainDefs] tmp merged has length 1, returning 0th element and perSig list")
                return tmp_merged[0], perSig_lengthOfChainConfigs

        if perSig_lengthOfChainConfigs is None:
            log.debug("[mergeChainDefs] serial merging first")
            return mergeSerial(tmp_merged, merged_ordering) #shouldn't need to modify it here!  
        else:
            log.debug("[mergeChainDefs] returning mergeSerial result and perSig_lengthOfChainConfigs %s",perSig_lengthOfChainConfigs)
            return mergeSerial(tmp_merged, merged_ordering), perSig_lengthOfChainConfigs #shouldn't need to modify it here!  
        
    else:
        log.error("[mergeChainDefs] Merging failed for %s. Merging strategy '%s' not known.", (listOfChainDefs, strategy))
        return -1



def check_leg_lengths(perSig_lengthOfChainConfigs):
    if not perSig_lengthOfChainConfigs: #default is None
        return "", -1
    leg_length_dict = {}
    for leg_lengths, leg_grps in perSig_lengthOfChainConfigs:
        for grp, length in zip(leg_grps,leg_lengths):
            if grp in leg_length_dict:
                leg_length_dict[grp] += [length]
            else:
                leg_length_dict[grp] = [length]
    found_mismatch = False
    max_length = -1
    mismatched_ag = ""
    log.debug("[check_leg_lengths] leg lengths: %s",leg_length_dict)
    for grp,lengths in leg_length_dict.items():
        if len(set(lengths)) > 1: #a mismatch! 
            log.debug("[check_leg_lengths] found mismatch for %s given %s", grp, lengths)
            if found_mismatch:
                log.error("[check_leg_lengths] Second mismatch in the same chain! I don't know how to deal with this, please resolve. Chain leg lengths: %s",perSig_lengthOfChainConfigs)
                log.error("[check_leg_lengths] Second mismatch in the same chain! lengths,grp: %s,%s",lengths, grp)
                raise Exception("[are_lengths_mismatched] Cannot merge chain, exiting.")
            found_mismatch = True
            max_length = max(lengths)
            mismatched_ag = grp
            
    return mismatched_ag, max_length

    
def mergeParallel(chainDefList, offset, leg_numbering = [], perSig_lengthOfChainConfigs = None):
    
    if offset != -1:
        log.error("[mergeParallel] Offset for parallel merging not implemented.")
        raise Exception("[mergeParallel] Cannot merge this chain, exiting.")

    allSteps = []
    allStepsMult = []
    nSteps = []
    chainName = ''
    l1Decisions = []
    alignmentGroups = []
    vertical_alignment_groups = []

    for iConfig, cConfig in enumerate(chainDefList):
        if chainName == '':
            chainName = cConfig.name
        elif chainName != cConfig.name:
            log.error("[mergeParallel] Something is wrong with the combined chain name: cConfig.name = %s while chainName = %s", cConfig.name, chainName)
            raise Exception("[mergeParallel] Cannot merge this chain, exiting.")

        if len(set(cConfig.alignmentGroups)) == 1:
            alignmentGroups.append(cConfig.alignmentGroups[0])
        elif len(cConfig.alignmentGroups) > 1:
            log.debug("[mergeParallel] Parallel merging an already merged chain with different alignment groups? This is odd! %s",cConfig.alignmentGroups)
            log.debug("...let's look at the config: %s", perSig_lengthOfChainConfigs)
            # if the length the matching group in the pre-merged part is shorter than the full one,
            # we need to patch it up to the full length by adding empty steps so that when
            # we merge, the longer leg doesn't merge onto the second alignment group 
            align_grp_to_lengthen, max_length = check_leg_lengths(perSig_lengthOfChainConfigs)
            if max_length > -1:
                current_leg_ag_length = -1
                index_modified_leg = -1
                leg_lengths, leg_ags = perSig_lengthOfChainConfigs[iConfig]  
                for ileg, (length, ag) in enumerate(zip(leg_lengths, leg_ags)):
                    if ag == align_grp_to_lengthen:
                        current_leg_ag_length = length
                        index_modified_leg = ileg
                        log.debug("[mergeParallel] ileg %s, length %s, ag %s: ",ileg, length, ag)
                        break 
                        # it's already merged so even if there is more than one in this chain
                        # they had better be the same length already
                    
                n_new_steps = max_length - current_leg_ag_length
                
                previous_step_dicts = cConfig.steps[current_leg_ag_length-1].stepDicts
                for i in range(1,n_new_steps+1):
                    step_mult = []
                    sigNames = []

                    for ileg,stepDict in enumerate(previous_step_dicts):
                        is_fs_string = 'FS' if isFullScanRoI(cConfig.L1decisions[ileg]) else ''
                        sigNames += [stepDict['chainParts'][0]['signature'] + is_fs_string]

                    seqMultName = '_'.join([sigName for sigName in sigNames])
                    seqStepName = 'Empty' + align_grp_to_lengthen + 'Align' + str(current_leg_ag_length+i) + '_' + seqMultName
                    seqNames = [getEmptySeqName(previous_step_dicts[iSeq]['signature'], current_leg_ag_length+i, align_grp_to_lengthen) for iSeq in range(len(sigNames))]

                    emptySequences = build_empty_sequences(previous_step_dicts, step_mult, 'mergeParallel', cConfig.L1decisions, seqNames, chainName)

                    cConfig.steps.insert(current_leg_ag_length + i - 1, #-1 to go to indexed from zero
                                        ChainStep( seqStepName, Sequences=emptySequences,
                                                  multiplicity = step_mult, chainDicts=previous_step_dicts,
                                                  isEmpty = True)
                                        )
                                 
                                 
                # edited the lengths, so need to update the leg length dict the code we did so!
                perSig_lengthOfChainConfigs[iConfig][0][index_modified_leg] = max_length
        else: 
            log.debug("[mergeParallel] Alignment groups are empty for this combined chain")

        allSteps.append(cConfig.steps)
        allStepsMult.append(len(cConfig.steps[0].multiplicity))
        nSteps.append(len(cConfig.steps))
        l1Decisions.extend(cConfig.L1decisions)
            
    # Use zip_longest_parallel so that we get None in case one chain has more steps than the other
    orderedSteps = list(zip_longest_parallel(allSteps, allStepsMult))
  
    if perSig_lengthOfChainConfigs is not None and len(perSig_lengthOfChainConfigs) > 0:
      in_chain_ag_lengths = {}
      ag_ordering = getAlignmentGroupOrdering()
      for ag in ag_ordering:
        for ag_lengths,sig_ags in perSig_lengthOfChainConfigs:
            for ag_length, sig_ag in zip(ag_lengths, sig_ags):
                if (sig_ag in in_chain_ag_lengths and in_chain_ag_lengths[sig_ag] < ag_length) or sig_ag not in in_chain_ag_lengths:
                    in_chain_ag_lengths[sig_ag] = ag_length
      for ag, ag_length in in_chain_ag_lengths.items():
          vertical_alignment_groups += [ag]*ag_length
    else:
        #it's all one alignment group in this case
        vertical_alignment_groups = [alignmentGroups[0]]*len(orderedSteps)            


    log.debug("[mergeParallel] alignment groups horizontal: %s", alignmentGroups)
    log.debug("[mergeParallel] alignment groups vertical: %s", vertical_alignment_groups)
    
    combChainSteps =[]
    log.debug("[mergeParallel] len(orderedSteps): %d", len(orderedSteps))
    for chain_index in range(len(chainDefList)):
        log.debug('[mergeParallel] Chain object to merge (i.e. chainDef) %s', chainDefList[chain_index])

    for step_index, (steps, step_ag) in enumerate(zip(orderedSteps,vertical_alignment_groups)):
        mySteps = list(steps)
        log.debug("[mergeParallel] Merging step counter %d", step_index+1)

        combStep = makeCombinedStep(mySteps, step_index+1, chainDefList, orderedSteps, combChainSteps, leg_numbering, step_ag)
        combChainSteps.append(combStep)
                                  
    combinedChainDef = Chain(chainName, ChainSteps=combChainSteps, L1decisions=l1Decisions, 
                                nSteps = nSteps, alignmentGroups = alignmentGroups)

    log.debug("[mergeParallel] Parallel merged chain %s with these steps:", chainName)
    for step in combinedChainDef.steps:
        log.debug('\n   %s', step)

    return combinedChainDef, perSig_lengthOfChainConfigs


def getEmptySeqName(stepName, step_number, alignGroup):
    #remove redundant instances of StepN
    if re.search('^Step[0-9]_',stepName):
        stepName = stepName[6:]
    elif re.search('^Step[0-9]{2}_', stepName):
        stepName = stepName[7:]    

    seqName = 'Empty'+ alignGroup +'Seq'+str(step_number)+ '_'+ stepName
    return seqName

    

def isFullScanRoI(inputL1Nav):
    fsRoIList = ['HLTNav_L1FSNOSEED','HLTNav_L1MET','HLTNav_L1J']
    if inputL1Nav in fsRoIList:
        return True
    else:
        return False

def noPrecedingStepsPreMerge(newsteps,chain_index,ileg):
    for step in newsteps:
        seq = step[chain_index].sequences[ileg]
        if isinstance(seq, EmptyMenuSequence):
            continue
        else:
            #if there's a non-empty sequence in a step before, there is clearly a
            #preceding step in this chain.
            return False
    return True

def noPrecedingStepsPostMerge(newsteps, ileg):
    for step in newsteps:
        seq = step.sequences[ileg]
        if isinstance(seq, EmptyMenuSequence):
            continue
        else:
            #if there's a non-empty sequence in a step before, there is clearly a
            #preceding step in this chain.
            return False
    return True
        
def getCurrentAG(chainStep):
    
    filled_seq_ag = []
    for iseq,seq in enumerate(chainStep.sequenceFunctions):
        # In the case of dummy configs, they are all empty
        if isEmptySequenceCfg(seq):
            continue
        else:
            # get the alignment group of the leg that is running a non-empty sequence
            # if we double-serial merge enough this will have to be recursive. Throw an error here for now
            # if the length is greater than one. I don't think this will ever come up
            if len(set(cp['alignmentGroup'] for cp in chainStep.stepDicts[iseq]['chainParts'])) > 1:
                log.error("[getCurrentAG] The leg has more than one chainPart (%s). Either the alignmentGroup property is bad or this is an unimplemented situation.",chainStep.stepDicts[iseq]['chainParts'])
                raise Exception("[getCurrentAG] Not sure what is happening here, but I don't know what to do.")
            filled_seq_ag += [chainStep.stepDicts[iseq]['chainParts'][0]['alignmentGroup']]

    if len(filled_seq_ag) == 0:
        log.error("[getCurrentAG] No non-empty sequences were found in %s", chainStep.sequenceFunctions)
        log.error("[getCurrentAG] The chainstep is %s", chainStep)
        raise Exception("[getCurrentAG] Cannot find the current alignment group for this chain")        
    elif len(set(filled_seq_ag)) > 1:
        log.error("[getCurrentAG] Found more than one alignment group for this step %s", filled_seq_ag)
        raise Exception("[getCurrentAG] Cannot find the current alignment group for this chain")
    else:
        return filled_seq_ag[0]

def serial_zip(allSteps, chainName, chainDefList, legOrdering):

    #note: allSteps and chainDefList do not have the legs in the same order
    #the legOrdering is a mapping between the two:
    # chainDefList[legOrdering[0]] <=> allSteps[0]

    legs_per_part = [len(chainDefList[stepPlacement].steps[0].multiplicity) for stepPlacement in legOrdering]
    n_parts = len(allSteps)
    log.debug('[serial_zip] configuring chain with %d parts with multiplicities %s', n_parts, legs_per_part)
    log.debug('[serial_zip]     and leg ordering %s', legOrdering)
    newsteps = []

    #per-part (horizontal) iteration by alignment ordering
    #i.e. if we run muon then electron, allSteps[0] = muon steps, allSteps[1] = electron steps
    #leg ordering tells us where it was ordered in the chain name, so e_mu in this case would
    #have legOrdering = [1,0]
    for chain_index, (chainSteps, stepPlacement) in enumerate(zip(allSteps, legOrdering)): 

        for step_index, step in enumerate(chainSteps):  #serial step iteration
            if step_index == 0:
                prev_ag_step_index = step_index
                previousAG = getCurrentAG(step)
            log.debug('[serial_zip] chain_index: %s step_index: %s, alignment group: %s', chain_index, step_index, previousAG)
            # create list of correct length (chainSteps in parallel)
            stepList = [None]*n_parts

            # put the step from the current sub-chain into the right place
            stepList[stepPlacement] = step
            log.debug('[serial_zip] Put step: %s', step.name)

            # all other chain parts' steps should contain an empty sequence
            for chain_index2, (nLegs, stepPlacement2) in enumerate(zip(legs_per_part, legOrdering)): #more per-leg iteration
                emptyStep = stepList[stepPlacement2]
                if emptyStep is None:
                    if chain_index2 == chain_index:
                        log.error("chain_index2 = chain_index, but the stepList still has none!")
                        raise Exception("[serial_zip] duplicating existing leg, why has this happened??")

                    #this WILL NOT work for jets!
                    step_mult = []
                    emptyChainDicts = []
                    if chain_index2 < chain_index:
                        emptyChainDicts = allSteps[chain_index2][-1].stepDicts
                    else:
                        emptyChainDicts = allSteps[chain_index2][0].stepDicts

                    log.debug("[serial_zip] nLegs: %s, len(emptyChainDicts): %s, len(L1decisions): %s", nLegs, len(emptyChainDicts), len(chainDefList[stepPlacement2].L1decisions))
                    sigNames = []
                    for ileg,(emptyChainDict,_) in enumerate(zip(emptyChainDicts,chainDefList[stepPlacement2].L1decisions)):
                        if isFullScanRoI(chainDefList[stepPlacement2].L1decisions[ileg]):
                            sigNames +=[emptyChainDict['chainParts'][0]['signature']+'FS']
                        else:
                            sigNames +=[emptyChainDict['chainParts'][0]['signature']]

                    seqMultName = '_'.join([sigName for sigName in sigNames])
                    currentAG = ''
                    
                    #now we need to know what alignment group this step is in to properly name the empty sequence
                    if len(set(chainDefList[stepPlacement].alignmentGroups)) == 1:
                        currentAG = chainDefList[stepPlacement].alignmentGroups[0]
                        ag_step_index = step_index+1
                    else:
                        # this happens if one of the bits to serial merge is already serial merged.
                        currentAG = getCurrentAG(step)
                        if currentAG == previousAG:
                            ag_step_index = prev_ag_step_index + 1
                            prev_ag_step_index = ag_step_index
                        else:
                            ag_step_index = 1
                            previousAG = currentAG
                            prev_ag_step_index = 1
                     
                    seqStepName = 'Empty' + currentAG +'Align'+str(ag_step_index)+'_'+seqMultName

                    seqNames = [getEmptySeqName(emptyChainDicts[iSeq]['signature'], ag_step_index, currentAG) for iSeq in range(nLegs)]

                    log.verbose("[serial_zip] step name for this leg: %s", seqStepName)
                    log.verbose("[serial_zip] created empty sequence(s): %s", seqNames)
                    log.verbose("[serial_zip] L1decisions %s ", chainDefList[stepPlacement2].L1decisions)
                        
                    emptySequences = build_empty_sequences(emptyChainDicts, step_mult, 'serial_zip', chainDefList[stepPlacement2].L1decisions, seqNames, chainName)

                    stepList[stepPlacement2] = ChainStep( seqStepName, Sequences=emptySequences,
                                                          multiplicity = step_mult, chainDicts=emptyChainDicts,
                                                          isEmpty = True)

            newsteps.append(stepList)
    log.debug('After serial_zip')
    for s in newsteps:
        log.debug( ', '.join(map(str, [step.name for step in s]) ) )
    return newsteps


def mergeSerial(chainDefList, chainDefListOrdering):
    allSteps = []
    legOrdering = []
    nSteps = []
    chainName = ''
    l1Decisions = []
    alignmentGroups = []
    log.debug('[mergeSerial] Merge chainDefList:')
    log.debug(chainDefList)
    log.debug('[mergeSerial]  wth ordering %s:',chainDefListOrdering)
        
    for ic,cOrder in enumerate(chainDefListOrdering):

        #put these in order of alignment
        cConfig = chainDefList[chainDefListOrdering.index(ic)] 
        leg_order  = chainDefListOrdering.index(ic) #but keep track of where it came from
        
        if chainName == '':
            chainName = cConfig.name
        elif chainName != cConfig.name:
            log.error("[mergeSerial] Something is wrong with the combined chain name: cConfig.name = %s while chainName = %s", cConfig.name, chainName)
            raise Exception("[mergeSerial] Cannot merge this chain, exiting.")

        #allSteps is ordered such that the first entry in the list is what we want to *run* first 
        allSteps.append(cConfig.steps)
        legOrdering.append(leg_order)
        nSteps.extend(cConfig.nSteps)
        l1Decisions.extend(chainDefList[ic].L1decisions) # do not invert L1Decisions because they follow the order of the sequences inside teh steps
        alignmentGroups.extend(cConfig.alignmentGroups)

    serialSteps = serial_zip(allSteps, chainName, chainDefList, legOrdering)
    combChainSteps =[]
    for chain_index in range(len(chainDefList)):
        log.debug('[mergeSerial] Chain object to merge (i.e. chainDef) %s', chainDefList[chain_index])
    for step_index, steps in enumerate(serialSteps):
        mySteps = list(steps)
        combStep = makeCombinedStep(mySteps, step_index+1, chainDefList)
        combChainSteps.append(combStep)
                        
    combinedChainDef = Chain(chainName, ChainSteps=combChainSteps, L1decisions=l1Decisions, 
                               nSteps = nSteps, alignmentGroups = alignmentGroups)

    log.debug("[mergeSerial] Serial merged chain %s with number of steps/leg %s with these steps:", chainName, combinedChainDef.nSteps)
    for step in combinedChainDef.steps:
        log.debug('   %s', step)

    return combinedChainDef

def checkStepContent(parallel_steps):
    """
    return True if any step contains a real Sequence
    """
    for step in parallel_steps:
        if step is None or step.isEmpty:
            continue
        for seq in step.sequenceFunctions:
            if not isEmptySequenceCfg(seq):
                return True    
    return False   

def makeCombinedStep(parallel_steps, stepNumber, chainDefList, allSteps = [], currentChainSteps = [], leg_numbering = [], alignment_group = ""):
    stepName = 'merged' #we will renumber all steps after chains are aligned #Step' + str(stepNumber)
    stepSeq = []
    stepMult = []
    log.verbose("[makeCombinedStep] steps %s ", parallel_steps)
    stepDicts = []
    comboHypoTools = []
    comboHypo = None
    
    leg_counter = 0
    currentStepName = ''
    # if *all* the steps we're trying to merge are either empty sequences or empty steps
    # we need to create a single empty step instead. 
    hasNonEmptyStep = checkStepContent(parallel_steps)
  
    if not hasNonEmptyStep:
        
        if len(parallel_steps)>=len(chainDefList) and all(step is None for step in parallel_steps[len(chainDefList):]):
            # We need to remove manually here the None steps exceeding the len of chainDefList. The right solution
            # would be to make sure that these cases don't happen upstream, but I am not confident enough with this
            # code to make such a large (and dangerous) change. But it would be nice to do that in future if possible..
            parallel_steps=parallel_steps[:len(chainDefList)]
            log.debug("[makeCombinedStep] removing empty steps exceeding chainDefList size. The new steps are now %s ", parallel_steps)

        for chain_index, step in enumerate(parallel_steps):
            # every step is empty but some might have empty sequences and some might not
            if step is None or len(step.sequenceFunctions) == 0:

                new_stepDicts = deepcopy(chainDefList[chain_index].steps[-1].stepDicts)
                currentStepName = 'Empty' + chainDefList[chain_index].alignmentGroups[0]+'Align'+str(stepNumber)+'_'+new_stepDicts[0]['chainParts'][0]['multiplicity']+new_stepDicts[0]['signature']
                log.debug('[makeCombinedStep] step has no sequences, making empty step %s', currentStepName)

                # we need a chain dict here, use the one corresponding to this leg of the chain
                for new_stepDict in new_stepDicts:
                    oldLegName = new_stepDict['chainName']
                    if re.search('^leg[0-9]{3}_',oldLegName):
                        oldLegName = oldLegName[7:]
                    new_stepDict['chainName'] = legName(oldLegName,leg_counter)
                    log.debug("[makeCombinedStep] stepDict naming old: %s, new: %s", oldLegName, new_stepDict['chainName'])
                    stepDicts.append(new_stepDict)
                    leg_counter += 1

            else:
                # Standard step with empty sequence(s)
                currentStepName = step.name
                #remove redundant instances of StepN_ and merged_ (happens when merging already merged chains)
                
                if re.search('^Step[0-9]_',currentStepName):
                    currentStepName = currentStepName[6:]
                elif re.search('^Step[0-9]{2}_', currentStepName):
                    currentStepName = currentStepName[7:]                
                if re.search('^merged_',currentStepName):
                    currentStepName = currentStepName[7:]

                # update the chain dict list for the combined step with the chain dict from this step
                log.debug('[makeCombinedStep] adding step dictionaries %s',step.stepDicts)

                for new_stepDict in deepcopy(step.stepDicts):
                    oldLegName = new_stepDict['chainName']
                    if re.search('^leg[0-9]{3}_',oldLegName):
                        oldLegName = oldLegName[7:]
                    if len(leg_numbering) > 0:
                        leg_counter = leg_numbering[chain_index]
                    new_stepDict['chainName'] = legName(oldLegName,leg_counter)
                    log.debug("[makeCombinedStep] stepDict naming old: %s, new: %s", oldLegName, new_stepDict['chainName'])
                    stepDicts.append(new_stepDict)
                    leg_counter += 1

            stepName += '_' + currentStepName

        theChainStep = ChainStep(stepName, Sequences=[], multiplicity=[], chainDicts=stepDicts, comboHypoCfg=ComboHypoCfg) 
        log.debug("[makeCombinedStep] Merged empty step: \n %s", theChainStep)
        return theChainStep

    for chain_index, step in enumerate(parallel_steps): #this is a horizontal merge!

        if step is None or (hasNonEmptyStep and len(step.sequenceFunctions) == 0):
            # this happens for merging chains with different numbers of steps, we need to "pad" out with empty sequences to propogate the decisions
            # all other chain parts' steps should contain an empty sequence

            if chain_index+1 > len(chainDefList): 
                chain_index-=chain_index
                                                
            if alignment_group == "":
                alignment_group = chainDefList[0].alignmentGroups[0]

            new_stepDict = deepcopy(chainDefList[chain_index].steps[-1].stepDicts[-1])
            seqName = getEmptySeqName(new_stepDict['signature'], stepNumber, alignment_group)

            if isFullScanRoI(chainDefList[chain_index].L1decisions[0]):
                stepSeq.append(functools.partial(EmptyMenuSequenceCfg, None, name=seqName+"FS"))
                currentStepName = 'Empty' + alignment_group +'Align'+str(stepNumber)+'_'+new_stepDict['chainParts'][0]['multiplicity']+new_stepDict['signature']+'FS'
            else:
                stepSeq.append(functools.partial(EmptyMenuSequenceCfg, None, name=seqName))
                currentStepName = 'Empty' + alignment_group +'Align'+str(stepNumber)+'_'+new_stepDict['chainParts'][0]['multiplicity']+new_stepDict['signature']

            log.debug("[makeCombinedStep]  chain_index: %s, step name: %s,  empty sequence name: %s", chain_index, currentStepName, seqName)

            #stepNumber is indexed from 1, need the previous step indexed from 0, so do - 2
            prev_step_mult = -1
            if stepNumber > 1 and len(currentChainSteps[stepNumber-2].multiplicity) >0:
                prev_step_mult = int(currentChainSteps[stepNumber-2].multiplicity[chain_index])
            else:
                #get the step multiplicity from the step dict. This should be 
                prev_step_mult = int(new_stepDict['chainParts'][0]['multiplicity'])
            stepMult.append(prev_step_mult)
            # we need a chain dict here, use the one corresponding to this leg of the chain
            oldLegName = new_stepDict['chainName']
            if re.search('^leg[0-9]{3}_',oldLegName):
                oldLegName = oldLegName[7:]
            new_stepDict['chainName'] = legName(oldLegName,leg_counter)
            stepDicts.append(new_stepDict)
            leg_counter += 1
        else:
            # Standard step, append it to the combined step
            log.debug("[makeCombinedStep]  step %s, multiplicity  = %s", step.name, str(step.multiplicity))
            if len(step.sequenceFunctions):                
                log.debug("[makeCombinedStep]    with sequences = %s", ' '.join(map(str, [seq.func.__name__ for seq in step.sequenceFunctions])))

            # this function only works if the input chains are single-object chains (one menu seuqnce)
            if len(step.sequenceFunctions) > 1:
                log.debug("[makeCombinedStep] combining in an already combined chain")

            if ( comboHypo is None or
                 (hasattr(step.comboHypoCfg, '__name__') and step.comboHypoCfg.__name__ != "ComboHypoCfg") ):
                comboHypo = step.comboHypoCfg
            currentStepName = step.name
            #remove redundant instances of StepN_ and merged_ (happens when merging already merged chains)
            if re.search('^Step[0-9]_',currentStepName):
                currentStepName = currentStepName[6:]
            elif re.search('^Step[0-9]{2}_', currentStepName):
                currentStepName = currentStepName[7:]    
            if re.search('^merged_',currentStepName):
                currentStepName = currentStepName[7:]
            stepSeq.extend(step.sequenceFunctions)
            # set the multiplicity of all the legs 
            if len(step.multiplicity) == 0:
                stepMult.append(0)
            else:
                stepMult.extend(step.multiplicity)
            comboHypoTools.extend(step.comboToolConfs)
            # update the chain dict list for the combined step with the chain dict from this step
            log.debug('[makeCombinedStep] adding step dictionaries %s',step.stepDicts)
            log.debug('[makeCombinedStep] my leg_numbering is: %s, for chain_index %s',leg_numbering, chain_index) 
            for new_stepDict in deepcopy(step.stepDicts):
                oldLegName = new_stepDict['chainName']
                if re.search('^leg[0-9]{3}_',oldLegName):
                    oldLegName = oldLegName[7:]
                if len(leg_numbering) > 0:
                    leg_counter = leg_numbering[chain_index]
                new_stepDict['chainName'] = legName(oldLegName,leg_counter)
                log.debug("[makeCombinedStep] stepDict naming old: %s, new: %s", oldLegName, new_stepDict['chainName'])
                stepDicts.append(new_stepDict)
                leg_counter += 1


        # the step naming for combined chains needs to be revisted!!
        stepName += '_' + currentStepName
        log.debug('[makeCombinedStep] current step name %s',stepName)
        # for merged steps, we need to update the name to add the leg name
    
    comboHypoTools = list(set(comboHypoTools))
    theChainStep = ChainStep(stepName, Sequences=stepSeq, multiplicity=stepMult, chainDicts=stepDicts, comboHypoCfg=comboHypo, comboToolConfs=comboHypoTools) 
    log.debug("[makeCombinedStep] Merged step: \n %s", theChainStep)
  
    
    return theChainStep

# modified version of zip_longest('ABCD', 'xy', fillvalue='-') --> Ax By C- D-, which takes into account the multiplicity of the steps
def zip_longest_parallel(AllSteps, multiplicity, fillvalue=None):
    from itertools import repeat
    
    iterators = [iter(it) for it in AllSteps]
    inactives =set()
    if len(iterators)==0:
        return
    while True:
        values = []
        for i, it in enumerate(iterators): #Here we loop over the different chain parts
            try:
                value = next(it)
            except StopIteration:
                if i not in inactives:
                    #We want to add the inactive iterator to the list of inactives iterators
                    inactives.add(i)
                if len(inactives)>=len(iterators):
                    #We want to exit the while True if we reached the end of all iterators.
                    return
                iterators[i] = repeat(fillvalue, int(multiplicity[i]))
                value = fillvalue
            values.append(value)
            if int(multiplicity[i]) > 1 and value == fillvalue:
                values.extend([fillvalue]*int(multiplicity[i]-1))

        yield tuple(values)


def build_empty_sequences(emptyChainDicts, step_mult, caller, L1decisions, seqNames, chainName):
    emptySequences = []
    for ileg in range(len(L1decisions)):                        
        if isFullScanRoI(L1decisions[ileg]):
            log.debug("[%s] adding FS empty sequence", caller)
            emptySequences += [functools.partial(EmptyMenuSequenceCfg, None, name=seqNames[ileg]+"FS")]
        else:
            log.debug("[%s] adding non-FS empty sequence", caller)
            emptySequences += [functools.partial(EmptyMenuSequenceCfg, None, name=seqNames[ileg])]
            
    log.verbose("[%s] emptyChainDicts %s", caller, emptyChainDicts)
    log.debug("[%s] %s has number of empty sequences %d and empty legs in stepDicts %d",
              caller, chainName, len(emptySequences), len(emptyChainDicts))
    if len(emptySequences) != len(emptyChainDicts):
        log.error("[%s] %s has a different number of empty sequences/legs %d than stepDicts %d",
                  caller, chainName, len(emptySequences), len(emptyChainDicts))

        raise Exception(f"[{caller}] Cannot create this chain step, exiting.")

    for sd in emptyChainDicts:
        if sd['signature'] == 'Jet' or sd['signature'] == 'Bjet':
            step_mult += [1]
        elif len(sd['chainParts']) != 1:
            log.error("[%s] %s has chainParts has length != 1 within a leg! chain dictionary for this step: \n %s",
                      caller, chainName, sd)
            raise Exception(f"[{caller}] Cannot create this chain step, exiting.")
        else:
            step_mult += [int(sd['chainParts'][0]['multiplicity'])]

    if len(emptySequences) != len(step_mult):
        log.error("[%s] %s has a different number of empty sequences/legs %d than multiplicities %d",
                  caller, chainName, len(emptySequences), len(step_mult))
        raise Exception(f"[{caller}] Cannot create this chain step, exiting.")

    log.verbose('[%s] step multiplicity %s',caller, step_mult)


    return emptySequences
