# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from TrigBphysHypo.TrigMultiTrkComboHypoMonitoringConfig import TrigMultiTrkComboHypoMonitoring, TrigMultiTrkComboHypoToolMonitoring
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Config.MenuComponents import algorithmCAToGlobalWrapper
from AthenaConfiguration.ComponentFactory import isComponentAccumulatorCfg
from AthenaCommon.Logging import logging
log = logging.getLogger('TrigMultiTrkComboHypoConfig')

trigMultiTrkComboHypoToolDict = {
    'bJpsimumu'  : { 'massRange' : (2500.,  4300.), 'chi2' : 20. },
    'bJpsi'      : { 'massRange' : (2500.,  4300.), 'chi2' : 20. },
    'bJpsimutrk' : { 'massRange' : (2600.,  3600.), 'chi2' : 20. },
    'bUpsimumu'  : { 'massRange' : (8000., 12000.), 'chi2' : 20. },
    'bUpsi'      : { 'massRange' : (8000., 12000.), 'chi2' : 20. },
    'bDimu'      : { 'massRange' : (1500., 14000.), 'chi2' : 20. },
    'bDimu2700'  : { 'massRange' : ( 100.,  2700.), 'chi2' : 20. },
    'bDimu6000'  : { 'massRange' : ( 100.,  6000.), 'chi2' : 20. },
    'bBmumu'     : { 'massRange' : (4000.,  8500.), 'chi2' : 20. },
    'bPhi'       : { 'massRange' : ( 940.,  1100.), 'chi2' : 10. },
    'bTau'       : { 'massRange' : (   0.,  2700.), 'chi2' : 50. },
    'b3mu'       : { 'massRange' : ( 100., 10000.), 'chi2' : 30., 'nTrk' : 3, 'charge' : 1 },
    'bBeeM6000'  : { 'massRange' : ( 100.,  6000.), 'chi2' : 20. },
    'b0dRAB12vtx20'         : { 'massRange' : ( 0., 999999999.), 'chi2' : 20., 'deltaRMax' : 1.2 },
    'b0dRAB207invmAB22vtx20': { 'massRange' : ( 7000., 22000.), 'chi2' : 20., 'deltaRMax' : 2.0 },
    'b0dRAB127invmAB22vtx20': { 'massRange' : ( 7000., 22000.), 'chi2' : 20., 'deltaRMax' : 1.2 },
    'b7invmAB22vtx20'  : { 'massRange' : (  7000., 22000.), 'chi2' : 20., 'charge' : -1 },
    'b7invmAB9vtx20'   : { 'massRange' : (  7000.,  9000.), 'chi2' : 20., 'charge' : -1 },
    'b11invmAB60vtx20' : { 'massRange' : ( 11000., 60000.), 'chi2' : 20., 'charge' : -1 },
    'b11invmAB24vtx20' : { 'massRange' : ( 11000., 24000.), 'chi2' : 20., 'charge' : -1 },
    'b24invmAB60vtx20' : { 'massRange' : ( 24000., 60000.), 'chi2' : 20., 'charge' : -1 }
}


def StreamerDimuL2ComboHypoCfg(flags, name):
    log.debug('DimuL2ComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : True,
        "trigSequenceName" : 'Dimu',
        "trigLevel" : 'L2',
        "TrackCollectionKey" : 'HLT_IDTrack_Muon_FTF'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def StreamerDimuL2IOComboHypoCfg(flags, name):
    log.debug('DimuL2IOComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : True,
        "trigSequenceName" : 'Dimu',
        "trigLevel" : 'L2IO'   }
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def StreamerDimuL2MTComboHypoCfg(flags, name):
    log.debug('DimuL2MTComboHypoCfg.name = %s ', name)
    kwargs = { "isStreamer" : True,
        "trigSequenceName" : 'Dimu',
        "trigLevel" : 'L2MT' }
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def DimuEFComboHypoCfg(flags, name):
    log.debug('DimuEFComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : False,
        "trigSequenceName" : 'Dimu',
        "trigLevel" : 'EF',
        "checkMultiplicity" : True,
        "TrigBphysCollectionKey" : 'HLT_DimuEF'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def StreamerDimuEFComboHypoCfg(flags, name):
    log.debug('StreamerDimuEFComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : True,
        "trigSequenceName" : 'StreamerDimu',
        "nTracks" : [ 2 ],
        "chi2" : 20.,
        "massRange" : [ (100., 6000.) ],
        "trackPtThresholds" : [ [ 100., 100. ] ],
        "trigLevel" : 'EF'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def DiElecPrecisionGSFComboHypoCfg(flags, name):
    log.debug('DiElecPrecisionGSFComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : False,
        "trigSequenceName" : 'DiElecPrecisionGSF',
        "trigLevel" : 'EF',
        "doElectrons" : True,
        "TrigBphysCollectionKey" : 'HLT_DiElecPrecisionGSF'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def NoMuonDiElecPrecisionGSFComboHypoCfg(flags, name):
    log.debug('NoMuonDiElecPrecisionGSFComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : False,
        "trigSequenceName" : 'NoMuonDiElecPrecisionGSF',
        "trigLevel" : 'EF',
        "doElectrons" : True,
        "TrigBphysCollectionKey" : 'HLT_NoMuonDiElecPrecisionGSF' }
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def BmutrkComboHypoCfg(flags, name):
    log.debug('BmutrkComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : False,
        "trigSequenceName" : 'Bmutrk',
        "trigLevel" : 'EF',
        "TrackCollectionKey" : 'HLT_IDTrack_Bmumux_IDTrig',
        "isMuTrkMode" : True,
        "chi2" : 20.,
        "nTracks" : [ 2 ],
        "totalCharge" : [ 0 ],
        "massRange" : [ (2500., 4400.) ],
        "trackPtThresholds" : [ [ 10000., 2000. ] ],
        "TrigBphysCollectionKey" : 'HLT_Bmutrk'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def DrellYanComboHypoCfg(flags, name):
    log.debug('DrellYanComboHypoCfg.name = %s ', name)
    kwargs = {"isStreamer" : False,
        "trigSequenceName" : 'DrellYan',
        "trigLevel" : 'EF',
        "nTracks" : [ 2 ],
        "totalCharge" : [ -1 ],
        "massRange" : [ (7000., 60000.) ],
        "trackPtThresholds" : [ [ 100., 100. ] ],
        "chi2" : 20.,
        "combineInputDecisionCollections" : True,
        "applyOverlapRemoval" : False,
        "useLeptonMomentum" : True,
        "TrigBphysCollectionKey" : 'HLT_DrellYan'}
    if not isComponentAccumulatorCfg():
        return algorithmCAToGlobalWrapper(ConfigurationComboHypo, flags, **kwargs)[0]
    else:
        return ConfigurationComboHypo(flags, **kwargs)

def TrigMultiTrkComboHypoToolFromDict(flags, chainDict):
    return ConfigurationComboHypoTool(flags, chainDict)


def ConfigurationComboHypo(flags, trigSequenceName = 'Dimu', **kwargs):

   kwargs.setdefault("isStreamer", False)
   kwargs.setdefault("trigLevel", 'L2')
   kwargs.setdefault("TrackCollectionKey", '')
   kwargs.setdefault("TrigBphysCollectionKey", 'TrigBphysContainer')
   kwargs.setdefault("CheckMultiplicityMap", False)
   kwargs.setdefault("doElectrons", False)
   trigLevel = kwargs['trigLevel']
   acc = ComponentAccumulator()
   trigLevelDict = {'L2':0, 'L2IO':1, 'L2MT':2, 'EF':3}
   try:
       value = trigLevelDict[trigLevel]
       log.debug('TrigMultiTrkComboHypo.trigLevel = %s ', value)
   except KeyError:
       raise Exception('TrigMultiTrkComboHypo.trigLevel should be L2, L2IO, L2MT or EF, but %s provided.', trigLevel)

   baseName = 'Streamer'+trigSequenceName+trigLevel if kwargs['isStreamer'] else trigSequenceName+trigLevel

   from TrigBphysHypo.TrigBPhyCommonConfig import TrigBPHY_TrkVKalVrtFitterCfg
   from InDetConfig.InDetConversionFinderToolsConfig import BPHY_VertexPointEstimatorCfg
   if kwargs["doElectrons"]:
       kwargs.setdefault("nTracks", [ 2 ])
       kwargs.setdefault("trackPtThresholds", [ [ -1., -1. ] ])
       kwargs.setdefault("massRange",   [ (100., 20000.) ])
       kwargs.setdefault("mergedElectronChains", [ 'BPH-0DR3-EM7J15', 'HLT_e5_lhvloose_bBeeM6000' ])
       kwargs.setdefault("caloClusterEtThreshold", -1.)
       alg = CompFactory.TrigMultiTrkComboHypo(
         name = baseName+'ComboHypo',
         VertexFitter = acc.popToolsAndMerge(TrigBPHY_TrkVKalVrtFitterCfg(flags, baseName)),
         VertexPointEstimator = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, 'VertexPointEstimator_'+baseName)),
         MonTool = TrigMultiTrkComboHypoMonitoring(flags, 'TrigMultiTrkComboHypoMonitoring_'+baseName),
         **kwargs)
       acc.addEventAlgo(alg, primary=True)
       return acc
   else:
       kwargs.setdefault("nTracks", [ 2, 3 ])
       kwargs.setdefault("trackPtThresholds", [ [ 3650., 3650. ], [ 3650., 3650., 3650. ] ])
       kwargs.setdefault("massRange", [ (100., 20000.), (0., 11000.) ])
       alg = CompFactory.TrigMultiTrkComboHypo(
           name = baseName+'ComboHypo',
           VertexFitter = acc.popToolsAndMerge(TrigBPHY_TrkVKalVrtFitterCfg(flags, baseName)),
           VertexPointEstimator = acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, 'VertexPointEstimator_'+baseName)),
           MonTool = TrigMultiTrkComboHypoMonitoring(flags, 'TrigMultiTrkComboHypoMonitoring_'+baseName),
           **kwargs)

       acc.addEventAlgo(alg, primary=True)
       return acc

def ConfigurationComboHypoTool(flags, chainDict):

   tool = CompFactory.TrigMultiTrkComboHypoTool(chainDict['chainName'])

   try:
       topo = chainDict['topo'][0]
       value = trigMultiTrkComboHypoToolDict[topo]
       tool.massRange = value['massRange']
       tool.chi2 = value['chi2']
       tool.nTrk = value['nTrk'] if 'nTrk' in value else 2
       tool.totalCharge = value['charge'] if 'charge' in value else 0
       if 'deltaRMin' in value:
          tool.deltaRMin = value['deltaRMin']
       if 'deltaRMax' in value:
          tool.deltaRMax = value['deltaRMax']
   except LookupError:
       raise Exception('TrigMultiTrkComboHypo misconfigured for \'%s\': topo \'%s\' is not supported.', chainDict['chainName'], topo)

   if 'nocut' in chainDict['topo']:
       tool.AcceptAll = True

   if 'noos' in chainDict['topo']:
       tool.totalCharge = -100 # negative number to indicate no charge cut

   if 'Lxy0' in chainDict['topo']:
       tool.LxyCut = 0.0

   electronMultiplicity = [int(chainPart['multiplicity']) for chainPart in chainDict['chainParts'] if chainPart['signature']=='Electron']
   if len(electronMultiplicity) == 1 and electronMultiplicity[0] == 1:
       tool.isMergedElectronChain = True

   if 'bJpsimutrk' in chainDict['topo']:
       tool.isMuonTrkPEB = True
       tool.totalCharge = 0
       tool.trackPtThresholds = [-1., 2000.] if 'lowpt' in chainDict['topo'] else  [-1., 3000.]

   if 'bTau' in chainDict['topo']:
       tool.nTrk = sum(int(chainPart['multiplicity']) for chainPart in chainDict['chainParts'])
       tool.totalCharge = 1 if tool.nTrk == 3 else -1

   signatures = chainDict['signatures']
   tool.isCombinedChain = (signatures.count(signatures[0]) != len(signatures))
   tool.legMultiplicities = chainDict['chainMultiplicities']

   monGroups = ['bphysMon:online']
   if any(group in monGroups for group in chainDict['monGroups']):
        tool.MonTool = TrigMultiTrkComboHypoToolMonitoring(flags, 'MonTool')
   return tool

