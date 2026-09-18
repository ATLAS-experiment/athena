# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.Enums import LHCPeriod
from RecJobTransforms.AODFixHelper import releaseInRange
from AthenaCommon.Logging import logging

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
def listOfRecoReleases(flags):
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

    if len(listOfRecoTags) == 0:
        return []

    import pyAMI.client
    import pyAMI.atlas.api as AtlasAPI

    client = pyAMI.client.Client('atlas')

    relList = set()
    for e in listOfRecoTags:
        msg.info('Testing %s',e)
        d = AtlasAPI.get_ami_tag(client,e)
        if len(d):
            msg.info('List length %d',len(d))
            if 'transformation' in d[0] and d[0]['transformation'].find('Merge') >= 0:
                   msg.info('it is a merging tag; skip')
                   continue
            if 'cacheName' in d[0]: relList.add('Athena-'+d[0]['cacheName'])

    return list(relList)

def doFixTime(flags,relNum = None):
    return releaseInRange(flags,"Athena-23.0.12","Athena-23.0.200",relNum) or \
      releaseInRange(flags,"Athena-24.0.0","Athena-24.0.200",relNum) or \
      releaseInRange(flags,"Athena-25.0.0","Athena-25.0.200",relNum)

def runAODFix(flags, correctCluster = True, checkRelWithAMI = False):

    msg=logging.getLogger("GetDecisionToRunAODFix")

    ALToFix = "egammaAmbiguityLinksFix" not in flags.Input.AODFixesDone
    TimeToFix = "egammatopoIsoFix" not in flags.Input.AODFixesDone

    if flags.GeoModel.Run >= LHCPeriod.Run3:
        doFix_meta = doFixTime(flags) and TimeToFix
    else:
        doFix_meta=False
    doAmbiguityFix_meta = releaseInRange(flags,"Athena-24.0.0","Athena-24.0.83") and ALToFix

    doFix = doFix_meta
    doAmbiguityFix = doAmbiguityFix_meta

    if checkRelWithAMI:
        doFixFromAMITags = []
        inputReleaseFromAMITags = listOfRecoReleases(flags)
        if len(inputReleaseFromAMITags) == 0:
            pass
        msg.info('Release list %s',' '.join(inputReleaseFromAMITags))
        for e in inputReleaseFromAMITags:
            doF = doFixTime(flags,e) and TimeToFix
            doAF = releaseInRange(flags,"Athena-24.0.0","Athena-24.0.83",e) and ALToFix
            doFixFromAMITags.append((doF,doAF))
        msg.info('doFix from AMI tags = %s',doFixFromAMITags)

        for ie,e in enumerate(doFixFromAMITags):
            if e[0] != doFix or e[1] != doAmbiguityFix:
                msg.warning('Inconsistent information from AMI reco tag release %s and input release %s', \
                            inputReleaseFromAMITags[ie],flags.Input.Release)
                if ie == 0:
                    msg.warning('Will use the release number first in the list')
                    doFix = e[0]
                    doAmbiguityFix = e[1]

    fixes = set()
    if doFix:
        if doAmbiguityFix:
            fixes.add('egammaAmbiguityLinksFix')
        # no timing cut in HI reco, so not these fixes
        if not flags.Reco.EnableHI:
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


   
