# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from RecJobTransforms.AODFixHelper import releaseInRange

def getfunc():
    from inspect import currentframe, getframeinfo
    caller = currentframe().f_back
    func_name = getframeinfo(caller)[2]
    caller = caller.f_back
    func = caller.f_locals.get(
            func_name, caller.f_globals.get(
                func_name))

    return func

def runAODFix(flags, correctCluster = True):
    doFix = releaseInRange(flags,"Athena-24.0.0","Athena-24.0.200") or \
      releaseInRange(flags,"Athena-25.0.0","Athena-25.0.200")

    doAmbiguityFix = releaseInRange(flags,"Athena-24.0.0","Athena-24.0.83")

    name = ''
    if doAmbiguityFix:
        name += 'egammaAmbiguityLinksFix'
    name += ' egammatopoIsoFix'
    if correctCluster:
        name += ' egClusterL2_3Fix'
    return doFix, name

def egammaAODFixesCfg(flags, correctCluster = True):
    
    #first check if we need to apply this AODFix
    doFix, name = runAODFix(flags, correctCluster)
    if not doFix:
        return None
    getfunc().__name__ = name

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
    kwargs['FixAmbiguityLinks'] = (name.find('egammaAmbiguityLinksFix') >= 0)
    if correctCluster:
          # TO BE UNDERSTOOD : why is this explicitely needed here (without it : ERROR SG::ExcNoCondCont: Can't retrieve CondCont from ReadCondHandle for key ConditionStore+LArBadChannel. Can't retrieve.)
          from LArBadChannelTool.LArBadChannelConfig import LArBadChannelCfg
          result.merge(LArBadChannelCfg(flags))
          #from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
          #result.merge( TileBadChannelsCondAlgCfg(flags, **kwargs) )
          from CaloBadChannelTool.CaloBadChanToolConfig import CaloBadChanToolCfg
          result.popToolsAndMerge( CaloBadChanToolCfg(flags) )
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


   
