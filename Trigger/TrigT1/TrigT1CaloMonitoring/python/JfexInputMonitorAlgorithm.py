#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JfexInputMonitoringConfig(flags):
    '''Function to configure LVL1 JfexInput algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JfexInputMonitorAlgorithm,'JfexInputMonAlg')
    
    
    #Algorithms needed for the monitoring
    if flags.Input.Format==Format.BS:

        #jfex emulated input: EmulatedTowers
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
        result.merge(jFexEmulatedTowersCfg(flags))    
    

    # get any algorithms
    JfexInputMonAlg = helper.alg

    # add any steering
    groupName = 'JfexInputMonitor' # the monitoring group name is also used for the package name
    JfexInputMonAlg.Grouphist = groupName

    
    from math import pi
    
    x_phi = []
    for i in range(67):
        phi = (-pi- pi/32) + pi/32*i 
        x_phi.append(phi)
    x_phi = sorted(x_phi)

    
    eta_phi_bins = {
        'xbins': 100, 'xmin': -5, 'xmax': 5,
        'ybins': x_phi
    }

    helper.defineTree('LBNString,Error,EventNumber,TowerId,TowerSource,TowerEta,TowerPhi,TowerCount,RefTowerCount,TowerSat,RefTowerSat,timeSince,timeUntil;errors',
                      "lbnString/string:error/string:eventNumber/l:id/i:source/i:eta/F:phi/F:count/i:ref_count/i:sat/I:ref_sat/I:timeSince/I:timeUntil/I",
                      title="errors tree;LBN;Error",fillGroup="errors")
    helper.defineHistogram('TowerEta,TowerPhi;h_saturated', title='jFex Saturated DataTowers; #eta; #phi',
                           type='TH2F',fillGroup=groupName,cutmask="TowerSaturated",**eta_phi_bins)
    helper.defineHistogram('TowerEta,TowerPhi;h_invalidCodes', title='jFex DataTower Invalid Et codes (4095); #eta; #phi',
                           type='TH2F',fillGroup=groupName,cutmask="TowerInvalid",**eta_phi_bins)
    helper.defineHistogram('TowerEta,TowerPhi;h_emptyCodes', title='jFex DataTower Empty Et codes (0); #eta; #phi',
                           type='TH2F',fillGroup=groupName,cutmask="TowerEmpty",**eta_phi_bins)

    result.merge(helper.result())
    return result

