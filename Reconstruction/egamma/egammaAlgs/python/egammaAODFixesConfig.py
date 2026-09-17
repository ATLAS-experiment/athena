# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import LHCPeriod
from RecJobTransforms.AODFixHelper import releaseInRange
from AthenaCommon.Logging import logging
from PathResolver import PathResolver

def getfunc():
    from inspect import currentframe, getframeinfo
    caller = currentframe().f_back
    func_name = getframeinfo(caller)[2]
    caller = caller.f_back
    func = caller.f_locals.get(
            func_name, caller.f_globals.get(
                func_name))

    return func

@AccumulatorCache
def FixFromAMITag(flags):
    doFixFromAMITags = []


    msg=logging.getLogger("listOfRecoReleases")
    from PyUtils.AMITagHelperConfig import inputAMITags
    listOfTags = inputAMITags(flags)
    listOfRecoTags = []
    nr = 0
    nf = 0
    for ie,e in enumerate(listOfTags):
        # no idea why it can return an empty string...
        msg.debug('tag %s at position %i',e,ie)
        if not len(e):
            continue
        if e[0] == 'r':
            nr += 1
            listOfRecoTags.append(e)
        if e[0] == 'f':
            nf += 1
            listOfRecoTags.append(e)

    msg.info('List of tags %s %s %s',' '.join(listOfTags),', of reco tags ',' '.join(listOfRecoTags))
    # use case : a DRAW file was produced at T0 (f-tag), and recoed in reprocessing (r-tag)
    if nf >= 1 and nr >= 1:
        msg.info('Both r- and f-tags are present, will remove the f ones')
    listOfRecoTags = [ e for e in listOfRecoTags if e[0] == 'r' ]
    listOfRecoTags_noMerge = [ ]
    has_been_merged = False
    

    if len(listOfRecoTags) == 0:
        doFixFromAMITags.append((True,True))
        return doFixFromAMITags


    filename_merging = PathResolver.FindCalibFile("egammaAlgs/Merging_reco_tag.txt") ##list of the reconstruction tag that are merging tag
    for e in listOfRecoTags:
        msg.info('Testing %s',e)
        with open(filename_merging) as f:
            for line in f:
                if e == line.strip():
                    msg.info('it is a merging tag; skip')
                    has_been_merged = True
                    continue
        if not  has_been_merged:
            listOfRecoTags_noMerge.append(e)

    if len(listOfRecoTags_noMerge) == 0:
        doFixFromAMITags.append((True,True))
        return doFixFromAMITags

    
    msg.info('remaining tag after removing merge tag %s',listOfRecoTags_noMerge)
    
    filename_timingTag = PathResolver.FindCalibFile("egammaAlgs/Timing_fix_reco_tag.txt") ## List of reconstruction tag that belong in the range where the timing fix should not be applied (Athena-23.0.0 to Athena-23.0.11) 
    filename_AmbiguityTag = PathResolver.FindCalibFile("egammaAlgs/Ambiguity_fix_reco_tag.txt") ## List of reconstruction tag that belong in the range wher the ambiguity link should be applied (Athena-24.0.0 to Athena-24.0.83)
    
    doFix_timing = True
    doFix_amb = True
    for e in listOfRecoTags_noMerge:
        with open(filename_timingTag) as f:
            for line in f:
                if e == line.strip():
                    doFix_timing = False and doFix_timing
                else:
                    doFix_timing = True and doFix_timing
        with open(filename_AmbiguityTag) as f:
            for line in f:
                if e == line.strip():
                    doFix_amb = True and doFix_amb
                else:
                    doFix_amb = False and doFix_amb

    doFixFromAMITags.append((doFix_timing,doFix_amb))
        
    return  doFixFromAMITags,listOfRecoTags_noMerge


def doFixTime(flags,relNum = None):
    return releaseInRange(flags,"Athena-23.0.12","Athena-23.0.200",relNum) or \
      releaseInRange(flags,"Athena-24.0.0","Athena-24.0.200",relNum) or \
      releaseInRange(flags,"Athena-25.0.0","Athena-25.0.200",relNum)

def runAODFix(flags, correctCluster = True, checkRelMerge = True):

    msg=logging.getLogger("GetDecisionToRunAODFix")

    ALToFix = "egammaAmbiguityLinksFix" not in flags.Input.AODFixesDone
    TimeToFix = "egammatopoIsoFix" not in flags.Input.AODFixesDone

    if flags.GeoModel.Run >= LHCPeriod.Run3:
        doFix_meta = doFixTime(flags) and TimeToFix
    else:
        doFix_meta=False
    doAmbiguityFix_meta = releaseInRange(flags,"Athena-24.0.0","Athena-24.0.83") and ALToFix

    doFix = doFix_meta or doAmbiguityFix_meta
    doFix_time = doFix_meta
    doAmbiguityFix = doAmbiguityFix_meta


    
    if checkRelMerge:
        doFixFromAMITags = []
        doFixFromAMITags, inputReleaseFromAMITags = FixFromAMITag(flags)
        msg.info('doFix from AMI tags = %s',doFixFromAMITags)
        for ie, e in enumerate(doFixFromAMITags):
            if e[0] != doFix_time or e[1] != doAmbiguityFix:
                msg.warning('Inconsistent information from AMI reco tag release %s and input release %s', \
                            inputReleaseFromAMITags[ie],flags.Input.Release)
                if ie ==0:
                    msg.warning('if no flag , will use the info collected from AMI')
                    doFix_time = e[0] and TimeToFix
                    doAmbiguityFix = e[1] and ALToFIx
                    doFix = doFix_time or doAmbiguityFix    

    fixes = set()
    if doFix:
        if doAmbiguityFix:
            fixes.add('egammaAmbiguityLinksFix')
        # no timing cut in HI reco, so not these fixes
        if doFix_time and not flags.Reco.EnableHI:
            fixes.add('egammatopoIsoFix')
            if correctCluster:
                fixes.add('egClusterL2_3Fix')
        else:
            doFix = doAmbiguityFix

    return doFix, fixes

def egammaAODFixesCfg(flags, correctCluster = True):

    msg=logging.getLogger("egammaAODFixes")
    #first check if we need to apply this AODFix
    doFix, fixes = runAODFix(flags, correctCluster)
    msg.info('Decision for egamma AOD fix = %s',doFix)
    if not doFix:
        return None
    else: 
        if fixes:
            msg.info('Will apply fixes = %s', ', '.join(fixes))
        else:
            msg.info('Range is ok but there are no fix to apply')
            return None

    # I do this because there are in fact two AOD fixes here:
    # one for ambiguity links, one for timing issue (topoetcone + cluster fixes)
    # sometimes, the ambiguity link fix is not needed, need to now this
    getfunc().__name__ = " ".join(fixes)

    result=ComponentAccumulator()

    # No fix if nothing relevant in input
    hasElectrons = "Electrons" in flags.Input.Collections
    hasPhotons = "Photons" in flags.Input.Collections
    if not hasElectrons and not hasPhotons:
        return None

    # TO BE FIXED
    # if only one collection is present, this will crash

    #Use the AddressRemappingSvc to rename the input object
    from SGComps.AddressRemappingConfig import InputRenameCfg
    if hasElectrons:
        result.merge(InputRenameCfg("xAOD::ElectronContainer", "Electrons", "old_Electrons"))
        result.merge(InputRenameCfg("xAOD::ElectronAuxContainer", "ElectronsAux.", "old_ElectronsAux."))
    if hasPhotons:
        result.merge(InputRenameCfg("xAOD::PhotonContainer", "Photons", "old_Photons"))
        result.merge(InputRenameCfg("xAOD::PhotonAuxContainer", "PhotonsAux.", "old_PhotonsAux."))

    kwargs = dict()
    kwargs['CorrectCluster'] = correctCluster
    kwargs['FixAmbiguityLinks'] = 'egammaAmbiguityLinksFix' in fixes
    if correctCluster:
          # First some detector config
          # TO BE UNDERSTOOD : why is this explicitely needed here (without it : ERROR SG::ExcNoCondCont: Can't retrieve CondCont from ReadCondHandle for key ConditionStore+LArBadChannel. Can't retrieve.)
          from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
          result.merge(LArBadChannelCfg(flags))
          #from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
          #result.merge( TileBadChannelsCondAlgCfg(flags, **kwargs) )
          from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
          result.popToolsAndMerge( CaloBadChanToolCfg(flags) )
          from LArGeoAlgsNV.LArGMConfig import LArGMCfg
          result.merge(LArGMCfg(flags))
          # then the AODFix itself
          result.merge(InputRenameCfg("xAOD::CaloClusterContainer", "egammaClusters", "old_egammaClusters"))
          result.merge(InputRenameCfg("xAOD::CaloClusterAuxContainer", "egammaClustersAux.", "old_egammaClustersAux."))
          result.merge(InputRenameCfg("CaloClusterCellLinkContainer", "egammaClusters_links", "old_egammaClusters_links"))
          kwargs['CaloDetDescrManager'] = 'CaloDetDescrManager'
          from egammaTools.egammaSwToolConfig import egammaSwToolCfg
          kwargs['ClusterCorrectionTool'] =  result.popToolsAndMerge(egammaSwToolCfg(flags))
          from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
          kwargs['MVACalibSvc'] = result.getPrimaryAndMerge(egammaMVASvcCfg(flags))
          kwargs['EGammaClustersOutputName'] = flags.Egamma.Keys.Output.CaloClusters
          kwargs['EGammaClustersInputName'] = f'old_{flags.Egamma.Keys.Output.CaloClusters}'

          kwargs['IsoLeakCorrectionTool'] = CompFactory.CP.IsolationCorrectionTool(
                LogLogFitForLeakage = True)

    #Re-run the required algorithms
    result.addEventAlgo(CompFactory.egammaAODFixes(**kwargs))

    return result


   
