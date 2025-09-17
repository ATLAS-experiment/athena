# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TrigPartialEventBuilding.TrigPartialEventBuildingConfig import getRegSelTools
from AthenaCommon.Logging import logging
_log = logging.getLogger( __name__ )


def TimeBurnerCfg(flags, name="TimeBurner", **kwargs):
    return CompFactory.TimeBurner(name, **kwargs)

def TimeBurnerHypoToolGen(flags, chainDict):
    # Dummy HypoTool (it is not even called by TimeBurner)
    return CompFactory.TrigGenericHypoTool(chainDict['chainName'],
                                           PassString = "")

def EndOfEventROIConfirmerAlgCfg(name):
    return CompFactory.EndOfEventROIConfirmerAlg(name)

def EndOfEventFilterAlgCfg(name, chainName):
    return CompFactory.EndOfEventFilterAlg(name, ChainName=chainName)

def TrigEventInfoRecorderAlgCfg(flags, name, trigEventInfoKey, decoratePFlowInfo, decorateEMTopoInfo, renounceAll=False, primaryVertexInputName="HLT_IDVertex_FS", RhoKey_PFlow = 'HLT_Kt4EMPFlowEventShape', RhoKey_EMTopo = 'HLT_Kt4EMTopoEventShape'):
    acc = ComponentAccumulator()
    alg = CompFactory.TrigEventInfoRecorderAlg(name,
          trigEventInfoKey = trigEventInfoKey,
          decoratePFlowInfo = decoratePFlowInfo,
          decorateEMTopoInfo = decorateEMTopoInfo,
          renounceAll = renounceAll,
          primaryVertexInputName = primaryVertexInputName,
          RhoKey_PFlow = RhoKey_PFlow,
          RhoKey_EMTopo = RhoKey_EMTopo,
          )
    acc.addEventAlgo(alg, primary=True)
    return acc


def L1CorrelationMonitoringCfg(flags):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    mon = GenericMonitoringTool(flags, 'MonTool')

    maxBC = 6              # fixed plotting half-range ±6 (CTP limit is 15)
    nbin = 2*maxBC + 1
    # 1-D delta BC histogram
    mon.defineHistogram(
        'BeforeAfterFlag',        # ← variable list, positional
        path  = 'EXPERT',
        type  = 'TH1F',
        title = 'L1 correlation; delta BC (relative to L1A); Events',
        xbins = nbin,
        xmin  = -maxBC-0.5,
        xmax  =  maxBC+0.5)
    
    mon.defineHistogram(
        'BeforeAfterFlag,otherType',
        path  = 'EXPERT',
        type  = 'TH2F',
        title = 'delta BC vs Other-BC type; delta BC (relative to L1A); Other-BC type',
        xbins = nbin, xmin = -maxBC-0.5, xmax = maxBC+0.5,
        ybins = 8,    ymin = -0.5,      ymax = 7.5)

    # 2-D type matrix (same as before – just remove var=)
    mon.defineHistogram(
        'l1Accept,otherType',
        path  = 'EXPERT',
        type  = 'TH2F',
        title = 'Type matrix ; L1A type ; Other-BC type',
        xbins = 8, xmin = -0.5, xmax = 7.5,
        ybins = 8, ymin = -0.5, ymax = 7.5)
    
    mon.defineHistogram(
        'BeforeAfterFlag,l1Accept',
        path  = 'EXPERT',
        type  = 'TH2F',
        title = 'delta BC vs L1A type; delta BC (relative); L1A type',
        xbins = nbin, xmin = -maxBC-0.5, xmax = maxBC+0.5,
        ybins = 8,    ymin = -0.5,      ymax = 7.5)
    
    # BeforeOffset:	Distance in BCs to the first earlier fired BC
    mon.defineHistogram(
        'BeforeOffset',
        path  = 'EXPERT',
        type  = 'TH1F',
        title = 'Offset of earlier BC trigger relative to L1A; delta BC before; Events',
        xbins = maxBC,
        xmin  = -0.5,
        xmax  = maxBC + 0.5
    )

    # AfterOffset:	Distance in BCs to the first later fired BC
    mon.defineHistogram(
        'AfterOffset',
        path  = 'EXPERT',
        type  = 'TH1F',
        title = 'Offset of later BC trigger relative to L1A; delta BC after; Events',
        xbins = maxBC,
        xmin  = -0.5,
        xmax  = maxBC + 0.5
    )

    # OtherTypeBefore:	Trigger type (1–7) of the earlier BC
    mon.defineHistogram(
        'OtherTypeBefore',
        path  = 'EXPERT',
        type  = 'TH1F',
        title = 'Trigger type of the earlier neighbor BC; Type code; Events',
        xbins = 8,
        xmin  = -0.5,
        xmax  = 7.5
    )

    # OtherTypeAfter:	Trigger type (1–7) of the later BC
    mon.defineHistogram(
        'OtherTypeAfter',
        path  = 'EXPERT',
        type  = 'TH1F',
        title = 'Trigger type of the later neighbor BC; Type code; Events',
        xbins = 8,
        xmin  = -0.5,
        xmax  = 7.5
    )

    # Correlation between early/late BC types
    mon.defineHistogram(
        'OtherTypeBefore,OtherTypeAfter',
        path  = 'EXPERT',
        type  = 'TH2F',
        title = 'Trigger type before vs after; Earlier BC type; Later BC type',
        xbins = 8, xmin = -0.5, xmax = 7.5,
        ybins = 8, ymin = -0.5, ymax = 7.5
    )

    # Time structure of symmetric/asymmetric cases
    mon.defineHistogram(
        'BeforeOffset,AfterOffset',
        path  = 'EXPERT',
        type  = 'TH2F',
        title = 'Offset before vs after; delta BC before; delta BC after',
        xbins = maxBC, xmin = -0.5, xmax = maxBC + 0.5,
        ybins = maxBC, ymin = -0.5, ymax = maxBC + 0.5
    )

    # delta BC × CTPID occupancy maps (TBP)
    mon.defineHistogram(
        'DeltaBCAll,CTPIDAll;MistimeMap_TBP',
        path='EXPERT', type='TH2F',
        title='TBP occupancy vs delta BC and CTPID;delta BC (relative to L1A);CTPID',
        xbins=2*maxBC+1, xmin=-maxBC-0.5, xmax=maxBC+0.5,
        ybins=512,      ymin=-0.5,        ymax=511.5
    )

    # TAP
    mon.defineHistogram(
        'DeltaBCAll_TAP,CTPIDAll_TAP;MistimeMap_TAP',
        path='EXPERT', type='TH2F',
        title='TAP occupancy vs delta BC and CTPID;delta BC (relative to L1A);CTPID',
        xbins=2*maxBC+1, xmin=-maxBC-0.5, xmax=maxBC+0.5,
        ybins=512,      ymin=-0.5,        ymax=511.5
    )

    # 512×512 pair maps: CTPID(BC=0) vs CTPID(BC=±1, ±2)
    mon.defineHistogram(
        'CTPID0tbp_m2,CTPIDtbp_m2;PairMap_TBP_DeltaBCm2',
        path='EXPERT', type='TH2F',
        title='TBP: CTPID at BC 0 vs CTPID at BC=-2;CTPID at BC=0;CTPID at BC=-2',
        xbins=512, xmin=-0.5, xmax=511.5,
        ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tbp_m1,CTPIDtbp_m1;PairMap_TBP_DeltaBCm1',
        path='EXPERT', type='TH2F',
        title='TBP: CTPID at BC 0 vs CTPID at BC=-1;CTPID at BC=0;CTPID at BC=-1',
        xbins=512, xmin=-0.5, xmax=511.5,
        ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tbp_p1,CTPIDtbp_p1;PairMap_TBP_DeltaBCp1',
        path='EXPERT', type='TH2F',
        title='TBP: CTPID at BC 0 vs CTPID at BC=+1;CTPID at BC=0;CTPID at BC=+1',
        xbins=512, xmin=-0.5, xmax=511.5,
        ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tbp_p2,CTPIDtbp_p2;PairMap_TBP_DeltaBCp2',
        path='EXPERT', type='TH2F',
        title='TBP: CTPID at BC 0 vs CTPID at BC=+2;CTPID at BC=0;CTPID at BC=+2',
        xbins=512, xmin=-0.5, xmax=511.5,
        ybins=512, ymin=-0.5, ymax=511.5
    )
    # 512×512 TAP pair maps
    mon.defineHistogram(
        'CTPID0tap_m2,CTPIDtap_m2;PairMap_TAP_DeltaBCm2',
        path='EXPERT', type='TH2F',
        title='TAP: CTPID at BC 0 vs CTPID at BC=-2;CTPID at BC=0;CTPID at BC=-2',
        xbins=512, xmin=-0.5, xmax=511.5, ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tap_m1,CTPIDtap_m1;PairMap_TAP_DeltaBCm1',
        path='EXPERT', type='TH2F',
        title='TAP: CTPID at BC 0 vs CTPID at BC=-1;CTPID at BC=0;CTPID at BC=-1',
        xbins=512, xmin=-0.5, xmax=511.5, ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tap_p1,CTPIDtap_p1;PairMap_TAP_DeltaBCp1',
        path='EXPERT', type='TH2F',
        title='TAP: CTPID at BC 0 vs CTPID at BC=+1;CTPID at BC=0;CTPID at BC=+1',
        xbins=512, xmin=-0.5, xmax=511.5, ybins=512, ymin=-0.5, ymax=511.5
    )
    mon.defineHistogram(
        'CTPID0tap_p2,CTPIDtap_p2;PairMap_TAP_DeltaBCp2',
        path='EXPERT', type='TH2F',
        title='TAP: CTPID at BC 0 vs CTPID at BC=+2;CTPID at BC=0;CTPID at BC=+2',
        xbins=512, xmin=-0.5, xmax=511.5, ybins=512, ymin=-0.5, ymax=511.5
    )


    return mon

def L1CorrelationAlgCfg(flags, name='L1CorrelationAlg', **kw):
    """
    Factory for L1CorrelationAlg.

    The C++ algorithm scans the full recorded window per event (from l1a_idx).
    We book monitoring histograms with a fixed range (no job option).
    """

    # Build and attach the monitoring tool
    kw.setdefault('MonTool', L1CorrelationMonitoringCfg(flags))
    
    return CompFactory.L1CorrelationAlg(name, **kw)


def ROBPrefetchingAlgCfg(flags, name, regSelDets=[], **kwargs):
    acc = ComponentAccumulator()
    alg = CompFactory.ROBPrefetchingAlg(name, **kwargs)
    alg.RegionSelectorTools = acc.popToolsAndMerge(getRegSelTools(flags, regSelDets))
    acc.addEventAlgo(alg, primary=True)

    return acc

def ROBPrefetchingAlgCfg_Si(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Si_'+nameSuffix, ['Pixel', 'SCT'], **kwargs)

def ROBPrefetchingAlgCfg_Calo(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Calo_'+nameSuffix, ['TTEM', 'TTHEC', 'FCALEM', 'FCALHAD', 'TILE'], **kwargs)

def ROBPrefetchingAlgCfg_Muon(flags, nameSuffix, **kwargs):
    return ROBPrefetchingAlgCfg(flags, 'ROBPrefetchingAlg_Muon_'+nameSuffix, ['MDT', 'RPC', 'TGC', 'CSC', 'MM', 'sTGC'], **kwargs)
   
def getChainsForPrefetching(CF_list):
    from TrigConfHLTUtils.HLTUtils import string2hash
    from AthenaCommon.CFElements import getSequenceChildren, isSequence
    from collections import defaultdict
    def sequenceAlgs(seq):
        algs = []        
        for alg in getSequenceChildren(seq):            
            if isSequence(alg):
                algs.extend(sequenceAlgs(alg))
            elif alg.getName().startswith('IMEmpty'):
                # skip empty probe step in tag&probe chains
                continue
            else:
                algs.append(alg.getName())
      
        return algs

    def firstNonEmptyStepAlgs(CF_list): 
        algsMap = defaultdict(list) # {chainLegName, algsInFirstNonEmptyStep}
        firstSequence = []

        for cfseq_per_step in CF_list:
            for cfseq in cfseq_per_step: # all the steps with same step number
                step = cfseq.sequenceCA.step                
                foundFirstSequence = False
                for ileg, menuSeq in enumerate(step.sequences): 
                    seqA = sequenceAlgs(menuSeq.sequence.Alg)                  
                    if seqA: # found not empty sequence in this step
                        for stepD, chain in zip(cfseq.stepDicts, cfseq.chains): # loop over chains
                            if chain in firstSequence:
                                continue                            
                            legName = stepD[ileg]['chainName']
                            if legName not in algsMap: # add only the first time
                                algsMap[legName] = seqA                               
                                firstSequence.append(chain)
                                foundFirstSequence = True

                    if foundFirstSequence:
                        break
                                    
        return algsMap

    detGroupIdentifierAlgs = {
        'Si' : ['PixelRawDataProvider','SCTRawDataProvider'],
        'Calo' : ['HLTCaloCellMaker','FastCaloL2EgammaAlg'],
        'Muon' : ['RpcRawDataProvider','TgcRawDataProvider','MdtRawDataProvider','sTgcRawDataProvider','MMRawDataProvider']
    }

    def algsToDetGroup(algs):
        groups = []
        for group,idAlgs in detGroupIdentifierAlgs.items():            
            if any([ida in algName for algName in algs for ida in idAlgs]):
                groups.append(group)
        if len(groups)>1:
            raise RuntimeError(f'Multiple detector groups: {groups:s} matched to the list of algs: {algs:s}')
        return groups[0] if groups else None

    chainFilterMap = { # {DetGroup, list of chain leg hashes}
        'Si': [],
        'Calo': [],
        'Muon': []
    }
    
    
    algsMap = firstNonEmptyStepAlgs(CF_list)
    for legName,algs in algsMap.items():
        det = algsToDetGroup(algs)
        if not det:
            continue
        _log.debug("%s initialRoI will prefetch %s", legName, det)
        chainFilterMap[det].append(string2hash(legName))
    return chainFilterMap

# Legacy searches for the algs through the top level sequence
def configurePrefetchingInitialRoI(flags, chains):
    from AthenaCommon.AlgSequence import AlgSequence
    from AthenaCommon.CFElements import findSubSequence, findAlgorithm
    chainFilterMap = getChainsForPrefetching(chains)

    hltBeginSeq = findSubSequence(AlgSequence(), 'HLTBeginSeq')
    for det,chainFilter in chainFilterMap.items():
        prefetchAlg = findAlgorithm(hltBeginSeq, f'ROBPrefetchingAlg_{det}_initialRoI')
        if not chainFilter:
            # Empty filter means unconditional prefetching
            # - prevent this by adding a non-existent hash to the list which effectively disables the prefetching alg
            _log.info('No chains matched to %s - forcing ChainFilter=[0] to disable this alg\'s prefetching', prefetchAlg.getName())
            chainFilter = [0]

        prefetchAlg.ChainFilter = chainFilter

# In CA we have no access to the HLTBeginSeq in here
# Instead generate the prefetching alg configs and append the ChainFilter,
# to be merged into the global config.
def prefetchingInitialRoIConfig(flags, CFseq_list):
    chainFilterMap = getChainsForPrefetching(CFseq_list)
    configurators = {
        'Si':   ROBPrefetchingAlgCfg_Si,
        'Calo': ROBPrefetchingAlgCfg_Calo,
        'Muon': ROBPrefetchingAlgCfg_Muon,
    }

    prefetchCfg = ComponentAccumulator()

    for det,chainFilter in chainFilterMap.items():
        if not chainFilter:
            # Empty filter means unconditional prefetching
            # - prevent this by adding a non-existent hash to the list which effectively disables the prefetching alg
            _log.info('No chains matched to ROBPrefetchingAlg_%s_initialRoI - forcing ChainFilter=[0] to disable this alg\'s prefetching', det)
            chainFilter = [0]
        prefetchAlg = configurators[det](flags, 'initialRoI', ChainFilter=chainFilter)
        prefetchCfg.merge(prefetchAlg)

    return prefetchCfg


