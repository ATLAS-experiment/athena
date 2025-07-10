#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration



def MuonTriggerCountConfig(helper):
    
    from AthenaConfiguration.ComponentFactory import CompFactory

    GroupName = 'TriggerCount'

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    from .MuonMatchingToolConfig import MuonMatchingToolConfig
    monAlg = helper.addAlgorithm(CompFactory.MuonTriggerCount,'MuonTriggerCount',
                                 MuonSelectionTool = helper.result().popToolsAndMerge(MuonSelectionToolCfg(helper.flags, MuQuality=1)),
                                 MuonMatchingTool = helper.result().popToolsAndMerge(MuonMatchingToolConfig(helper.flags)))

    ### monitoring groups
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess = getHLTMonitoringAccess(helper.flags)
    monitoredChains = moniAccess.monitoredChains(signatures="muonMon",monLevels=["shifter","t0","val"])

    # if mon groups not found fall back to hard-coded trigger monitoring list
    if len(monitoredChains) == 0:
        # HLT_mu6_L1MU6 is test chain for small statistics, so it will be removed.
        monitoredChains = ['HLT_mu6_L1MU5VF', 'HLT_mu24_ivarmedium_L1MU14FCH', 'HLT_mu50_L1MU14FCH', 'HLT_mu60_0eta105_msonly_L1MU14FCH', 'HLT_2mu14_L12MU8F', 'HLT_mu22_mu8noL1_L1MU14FCH']
    else:
        monitoredChains.sort()

    monAlg.MonitoredChains = monitoredChains

    monAlg.Group = GroupName

    histGroup = helper.addGroup(monAlg, GroupName, 'HLT/MuonMon/TriggerCount')

    histGroup.defineHistogram('TriggerCount;Monitoring_Chain',
                              title='Monitoring Chain Count;;Events',
                              type='TH1I',path='', xlabels=monitoredChains)
