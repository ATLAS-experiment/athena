#  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

import re
import math

from AthenaCommon.Logging import logging
from AthenaConfiguration.Enums import LHCPeriod
log = logging.getLogger('TrigMuonEfficiencyMonConfig.py')

def regex(pat):
    if 'cached_regex' not in globals():
        global cached_regex
        cached_regex = {}
    if pat not in cached_regex:
        cached_regex.update({pat:re.compile(pat)})
    return cached_regex[pat]

def get_singlemu_chain_closest_to(chainList, ref_hlt_pt, ref_hlt_type, ref_l1_pt):
    # find the "closest" HLT muon chain with respect to the chain you would use for the tag
    # in tag&probe; "close" means "mostly as good as the given reference HLT/L1 chain",
    # which implies pt thresholds should be similar and isolation, if pt thresholds are the
    # same, should be required
    chain_data = []
    for chainName in chainList:
      # regexp to match ordinary single-muon HLT chain names
      match = regex('HLT_mu([0-9]+)(?:_([a-zA-Z_]+))?_(L1MU([0-9]+)[A-Z_]+)').match(chainName)
      if match:
        hlt_threshold = float(match.group(1))
        hlt_type = match.group(2) # None, ivarmedium, barrel only...
        #level1_item = match.group(3) # not used so not assigned
        level1_threshold = float(match.group(4))
        if hlt_type is None or hlt_type == 'ivarmedium': # we restrict ourselves to ordinary cases
          chain_data.append((chainName, hlt_type, hlt_threshold, level1_threshold))

    # we determine automatically the HLT chain to choose, based on these criteria (in order of priority):
    # 1) how far the HLT pt cut is wrt the ideal chain we'd want (based on abs(pt-ptref) := delta_hlt)
    # 2) we prefer the chain with the lowest pt cut, if two are available with the same delta_hlt (i.e. if we want 24, we'll take 23 instead of 26 GeV)
    # 3) we prefer the isolated version of the trigger (higher tag purity)
    # 4+5) we prefer the chain with the lowest L1 item (again as close as possible to the one we'd want)
    # note that the check for 3) is performed in this way as "sorted" uses ascending order (so 0 comes before 1)
    chain_data_sorted = sorted(chain_data, key=lambda tup: (abs(tup[2]-ref_hlt_pt), tup[2]-ref_hlt_pt, tup[1]!=ref_hlt_type, abs(tup[3]-ref_l1_pt), tup[3]-ref_l1_pt))
    chainList_sorted = [x[0] for x in chain_data_sorted]
    return chainList_sorted


##
# @brief A special configuration for ttbar samples, where the cut on m_mumu is loosened to improve the acceptance.
# This is useless for data samples because the ttbar process has very small contribution.
def TrigMuonEfficiencyMonTTbarConfig(helper):
    
    from AthenaConfiguration.ComponentFactory import CompFactory

    ### monitorig groups
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess = getHLTMonitoringAccess(helper.flags)
    Chains = moniAccess.monitoredChains(signatures="muonMon",monLevels=["shifter","t0","val"])
    MonitoredChains = [c for c in Chains if 'HLT_mu' in c] # this makes sure we don't consider 2mu, 3mu, 4mu chains
  
    # if mon groups not found fall back to hard-coded trigger monitoring list
    if len(MonitoredChains) == 0:
        # HLT_mu6_L1MU6 is test chain for small statistics, so it will be removed.
        MonitoredChains = ['HLT_mu6_L1MU5VF', 'HLT_mu24_ivarmedium_L1MU14FCH', 'HLT_mu50_L1MU14FCH', 'HLT_mu60_0eta105_msonly_L1MU14FCH', 'HLT_mu14_L1MU8F', 'HLT_mu22_mu8noL1_L1MU14FCH', 'HLT_mu6_mu6noL1_L1MU5VF']

    ### determine what's the HLT chain to be used to select events and for the tag muon
    singlemu_chains_sorted = get_singlemu_chain_closest_to(MonitoredChains, 24, 'ivarmedium', 14)
    if not singlemu_chains_sorted:
        log.warning('No suitable single-muon trigger chain found as tag for ttbar tag&probe')
        return  
    else:
        tagandprobe_chain = singlemu_chains_sorted[0]
        log.info(f'Using {tagandprobe_chain} as tag and event trigger in ttbar tag&probe')        

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    for chain in MonitoredChains:
        monAlg = helper.addAlgorithm(CompFactory.TrigMuonEfficiencyMon,'TrigMuEff_ttbar_'+chain,
                                     MuonSelectionTool = helper.result().popToolsAndMerge(MuonSelectionToolCfg(helper.flags, MuQuality=1)))

        monAlg.EventTrigger = tagandprobe_chain
        monAlg.TagTrigger = tagandprobe_chain
        monAlg.Method = 'TTbarTagAndProbe'
        monAlg.MonitoredChains = [chain]
        threshold, level1 = regex('HLT_mu([0-9]+).*_(L1MU[A-Za-z0-9_]+)').match(chain).groups()
        monAlg.L1Seeds = [regex('L1MU').sub('L1_MU', level1)]
        monAlg.Thresholds = [float(threshold)]
        monAlg.Group = 'Eff_ttbar_'+chain
        
        GroupName = 'Eff_ttbar_'+chain
        histGroup = helper.addGroup(monAlg, GroupName, 'HLT/MuonMon/Efficiency/ttbar/'+chain)

        PlotConfig(monAlg, chain)
        defineEfficiencyHistograms(monAlg, histGroup, GroupName, chain, helper.flags)

    return


def TrigMuonEfficiencyMonZTPConfig(helper):

    from AthenaConfiguration.ComponentFactory import CompFactory

    ### monitorig groups
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess = getHLTMonitoringAccess(helper.flags)
    Chains = moniAccess.monitoredChains(signatures="muonMon",monLevels=["shifter","t0","val"])
    MonitoredChains = [c for c in Chains if 'HLT_mu' in c] # this makes sure we don't consider 2mu, 3mu, 4mu chains
          
    # if mon groups not found fall back to hard-coded trigger monitoring list
    if len(MonitoredChains) == 0:
        # HLT_mu6_L1MU6 is test chain for small statistics, so it will be removed.
        MonitoredChains = ['HLT_mu6_L1MU5VF', 'HLT_mu24_ivarmedium_L1MU14FCH', 'HLT_mu50_L1MU14FCH', 'HLT_mu60_0eta105_msonly_L1MU14FCH', 'HLT_mu14_L1MU8F', 'HLT_mu22_mu8noL1_L1MU14FCH', 'HLT_mu6_mu6noL1_L1MU5VF']

    ### determine what's the HLT chain to be used to select events and for the tag muon
    singlemu_chains_sorted = get_singlemu_chain_closest_to(MonitoredChains, 24, 'ivarmedium', 14)
    if not singlemu_chains_sorted:
        log.warning('No suitable single-muon trigger chain found as tag for Z tag&probe.')
        return 
    else:
        tagandprobe_chain = singlemu_chains_sorted[0]
        log.info(f'Using {tagandprobe_chain} as tag and event trigger in Z tag&probe')


    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    for chain in MonitoredChains:
        monAlg = helper.addAlgorithm(CompFactory.TrigMuonEfficiencyMon,'TrigMuEff_ZTP_'+chain,
                                     MuonSelectionTool = helper.result().popToolsAndMerge(MuonSelectionToolCfg(helper.flags, MuQuality=1)))

        monAlg.EventTrigger = tagandprobe_chain
        monAlg.TagTrigger = tagandprobe_chain
        monAlg.Method = 'ZTagAndProbe'
        monAlg.MonitoredChains = [chain]
        threshold, level1 = regex('HLT_mu([0-9]+).*_(L1MU[A-Za-z0-9_]+)').match(chain).groups()
        monAlg.L1Seeds = [regex('L1MU').sub('L1_MU', level1)]
        monAlg.Thresholds = [float(threshold)]
        monAlg.Group = 'Eff_ZTP_'+chain
        
        GroupName = 'Eff_ZTP_'+chain
        histGroup = helper.addGroup(monAlg, GroupName, 'HLT/MuonMon/Efficiency/ZTP/'+chain)

        PlotConfig(monAlg, chain)
        defineEfficiencyHistograms(monAlg, histGroup, GroupName, chain, helper.flags)

    return



def PlotConfig(monAlg, chain):
     from xAODMuon.xAODMuonEnums import xAODMuonEnums
     if "msonly" in chain:
         monAlg.MuonType = xAODMuonEnums.MuonStandAlone
     else:
         monAlg.MuonType = xAODMuonEnums.Combined

     if "msonly" in chain:
         monAlg.doL2CB = False
         monAlg.doEFCB = False
     if "ivar" not in chain:
         monAlg.doEFIso = False

     if "0eta105" in chain:
         monAlg.BarrelOnly = True
         
     if "noL1" in chain:
         monAlg.doEFSAFS = True
         monAlg.doEFCBFS = True
     else:
         monAlg.doEFSAFS = False
         monAlg.doEFCBFS = False


def defineEfficiencyHistograms(monAlg, histGroup, GroupName, chain, flags):

    def defineEachStepHistograms(xvariable, xlabel, xbins, xmin, xmax):
        histGroup.defineHistogram(GroupName+'_'+xvariable+';'+xvariable,
                                  title='All offline combined muon '+chain+';'+xlabel+';Events',
                                  type='TH1F',path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
        histGroup.defineHistogram(GroupName+'_L1pass,'+GroupName+'_'+xvariable+';EffL1MU_'+xvariable+'_wrt_Probe',
                                  title='L1MU Efficiency '+chain+';'+xlabel+';Efficiency',
                                  type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
        if monAlg.doL2SA:
            histGroup.defineHistogram(GroupName+'_L2SApass,'+GroupName+'_'+xvariable+';EffL2SA_'+xvariable+'_wrt_Upstream',
                                      title='L2MuonSA Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L1pass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
        
            histGroup.defineHistogram(GroupName+'_L2SApass,'+GroupName+'_'+xvariable+';EffL2SA_'+xvariable+'_wrt_offlineCB',
                                      title='L2MuonSA Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
        
        if monAlg.doL2CB: 
            histGroup.defineHistogram(GroupName+'_L2CBpass,'+GroupName+'_'+xvariable+';EffL2CB_'+xvariable+'_wrt_Upstream',
                                      title='L2muComb Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2SApass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_L2CBpass,'+GroupName+'_'+xvariable+';EffL2CB_'+xvariable+'_wrt_offlineCB',
                                      title='L2muComb Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
        if monAlg.doEFSA:
            histGroup.defineHistogram(GroupName+'_EFSApass,'+GroupName+'_'+xvariable+';EffEFSA_'+xvariable+'_wrt_Upstream',
                                      title='EFSA Muon Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2CBpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFSApass,'+GroupName+'_'+xvariable+';EffEFSA_'+xvariable+'_wrt_offlineCB',
                                      title='EFSA Muon Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFSApass,'+GroupName+'_'+xvariable+';EffEFSA_'+xvariable+'_wrt_offlineCB_passedL2SA',
                                      title='EFSA Muon Efficiency passed L2SA '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2SApass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
        if monAlg.doEFCB:
            histGroup.defineHistogram(GroupName+'_EFCBpass,'+GroupName+'_'+xvariable+';EffEFCB_'+xvariable+'_wrt_Upstream',
                                      title='EFCB Muon Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_EFSApass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFCBpass,'+GroupName+'_'+xvariable+';EffEFCB_'+xvariable+'_wrt_offlineCB',
                                      title='EFCB Muon Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFCBpass,'+GroupName+'_'+xvariable+';EffEFCB_'+xvariable+'_wrt_offlineCB_passedL2CB',
                                      title='EFCB Muon Efficiency passed L2CB '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2CBpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)

        if monAlg.doEFSAFS:
            histGroup.defineHistogram(GroupName+'_EFSAFSpass,'+GroupName+'_'+xvariable+';EffEFSAFS_'+xvariable+'_wrt_Upstream',
                                      title='EFSAFS Muon Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_EFCBpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFSAFSpass,'+GroupName+'_'+xvariable+';EffEFSAFS_'+xvariable+'_wrt_offlineCB',
                                      title='EFSAFS Muon Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFSAFSpass,'+GroupName+'_'+xvariable+';EffEFSAFS_'+xvariable+'_wrt_offlineCB_passedL2SA',
                                      title='EFSAFS Muon Efficiency passed L2SA '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2SApass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
        if monAlg.doEFCBFS:
            histGroup.defineHistogram(GroupName+'_EFCBFSpass,'+GroupName+'_'+xvariable+';EffEFCBFS_'+xvariable+'_wrt_Upstream',
                                      title='EFCBFS Muon Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_EFSAFSpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFCBFSpass,'+GroupName+'_'+xvariable+';EffEFCBFS_'+xvariable+'_wrt_offlineCB',
                                      title='EFCBFS Muon Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFCBFSpass,'+GroupName+'_'+xvariable+';EffEFCBFS_'+xvariable+'_wrt_offlineCB_passedL2CB',
                                      title='EFCBFS Muon Efficiency passed L2CB '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_L2CBpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
    
        if monAlg.doEFIso:
            histGroup.defineHistogram(GroupName+'_EFIsopass,'+GroupName+'_'+xvariable+';EffEFIso_'+xvariable+'_wrt_Upstream',
                                      title='EFIso Muon Efficiency '+chain+' wrt Upstream;'+xlabel+';Efficiency',
                                      cutmask=GroupName+'_EFCBpass',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)
    
            histGroup.defineHistogram(GroupName+'_EFIsopass,'+GroupName+'_'+xvariable+';EffEFIso_'+xvariable+'_wrt_offlineCB',
                                      title='EFIso Muon Efficiency '+chain+' wrt offlineCB;'+xlabel+';Efficiency',
                                      type='TEfficiency', path='',xbins=xbins,xmin=xmin,xmax=xmax)


    defineEachStepHistograms('muPt', 'p_{T} [GeV]', 50, 0.0, 100.)
    defineEachStepHistograms('muEta', '#eta', 30, -3.0, 3.0)
    defineEachStepHistograms('muPhi', '#phi', 30, -math.pi, math.pi)
    if flags.GeoModel.Run >= LHCPeriod.Run4:
        defineEachStepHistograms('averageMu', 'average pileup', 4, 100., 220.)
    else:
        defineEachStepHistograms('averageMu', 'average pileup', 4, 0., 80.)


    histGroup.defineHistogram(GroupName+'_invmass;invmass',
                              title='invariant mass of tag & probe muon '+chain+';inv mass [GeV];Events',
                              type='TH1F',path='',xbins=40,xmin=0.,xmax=200.)
    
