#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def GfexInputMonitoringConfig(flags):
    '''Function to configure LVL1 GfexInput algorithm in the monitoring system.'''

    
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.GfexInputMonitorAlgorithm,'GfexInputMonAlg')

    # add any steering
    groupName = 'GfexInputMonitor' # the monitoring group name is also used for the package name
    helper.alg.PackageName = groupName


    # path to the expert area for gFex Monitoring Plots
    trigPath = 'Expert/Inputs/gFEX'

    
    import numpy as np
    eta_bins= [-4.9, -4.1,-3.5,-3.25,-3.2,-3.1,-2.9,-2.7,-2.5]
    for eta in np.arange (-2.2,2.2,0.2):
            eta_bins.append(eta)
    eta_bins+= [2.2,2.5,2.7,2.9,3.1,3.25,3.3,3.5,4.1,4.9]

    helper.defineDQAlgorithm("Gfex_Tower_etaPhiMapFilled",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_Equal_Threshold","BinThreshold":"0."},
                             thresholdConfig={"NBins":[0,32*40]}, # 0 bins expected to be empty
                             )


    helper.defineTree('LBNString,Error,EventNumber,TowerId,TowerEta,TowerPhi,TowerEt,RefTowerEt,TowerSaturationflag,RefTowerSat;errors',
                      "lbnString/string:error/string:eventNumber/l:id/i:eta/F:phi/F:et/I:ref_et/I:sat/B:ref_sat/B",
                      title="errors tree;LBN;Error",fillGroup="errors")

    # histograms of gFex tower variables
    helper.defineHistogram('LBN,NGfexTowers;h_LBN_vs_nGfexTowers', title='Number of gFex towers in each event with Et > 10 GeV;LBN; gTowers per event; Number of events',
                           fillGroup = "highEtgTowers",
                           type='TH2D',
                           path=f'{trigPath}/detail',
                           hanConfig={"description":"The y value gives the number of gTowers with Et > 10 GeV; the z value gives the number of events"},
                           xbins=1,xmin=0,xmax=1,
                           ybins=1,ymin=0,ymax=1,
                           opt=['kAddBinsDynamically']
                           )

    helper.defineHistogram('TowerEta,TowerPhi;h_HotTower_EtaPhiMap', title='gFex Tower Eta vs Phi (gTowerEt > 2 GeV) ;#eta;#phi; Number of gTowers',
                           fillGroup = "highEtgTowers",
                           type='TH2F',
                           path=trigPath,
                           hanConfig={
                            "algorithm": "Gfex_Tower_etaPhiMapFilled",
                            "description":"Look for hot spots with unusual high statistics; Check <a href='./detail/h_HotTowers_posVsLBN'>detail plot</a> to get timeseries for each location", "display":"SetPalette(55),Draw=COL1Z"},
                           xbins=eta_bins, ybins=32,ymin=-3.2,ymax=3.2)

                          

    helper.defineHistogram('LBN,binNumber;h_HotTowers_posVsLBN',title='gFex Towers with Et > 2GeV;LB;32(x-1)+y',
                           path=f"{trigPath}/detail",
                           fillGroup = "highEtgTowers",
                           hanConfig={"description":"x and y correspond to axis bin numbers on <a href='../h_HotTower_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                           type='TH2I',
                           xbins=1,xmin=0,xmax=10,
                           ybins=40*32,ymin=0.5,ymax=40*32+0.5,
                           opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")

 #   helper.defineHistogram('TowerEta,TowerPhi,TowerEt;h_TowerHeatMapHotTowers', title='gFex Tower Average Et Distribution (gTowerEt > 2 GeV) ;#eta;#phi;averageEt (count x 50 MeV)',
 #                          fillGroup = "highEtgTowers",
 #                          type='TProfile2D',
 #                          path=trigPath,
 #                          hanConfig={"description":"The histogram is an average Et distrribution og the gTowers in the eta-phi space. The z-axis gives the average Et."},
 #                          **eta_phi_bins)


    helper.defineHistogram('TowerEta,TowerPhi;h_ColdTower_EtaPhiMap', title='gFex Tower Eta vs Phi (gTower Et < - 2 GeV) ;#eta;#phi;Number of gTowers',
                           fillGroup = "lowEtgTowers",
                           type='TH2F',
                           path=trigPath,
                           hanConfig={
                            "algorithm": "Gfex_Tower_etaPhiMapFilled",
                            "description":"Look for cold spots with unusual high statistics; Check <a href='./detail/h_ColdTowers_posVsLBN'>detail plot</a> to get timeseries for each location ", "display":"SetPalette(55),Draw=COL1Z"},
                           xbins=eta_bins, ybins=32,ymin=-3.2,ymax=3.2)

    helper.defineHistogram('LBN,binNumber;h_ColdTowers_posVsLBN',title='gFex Towers with Et < -2GeV;LB;32(x-1)+y',
                           path=f"{trigPath}/detail",
                           fillGroup = "lowEtgTowers",
                           hanConfig={"description":"x and y correspond to axis bin numbers on <a href='../h_ColdTower_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                           type='TH2I',
                           xbins=1,xmin=0,xmax=10,
                           ybins=40*32,ymin=0.5,ymax=40*32+0.5,
                           opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")

#    helper.defineHistogram('TowerEta,TowerPhi,TowerEt;h_TowerHeatMapColdTowers', title='gFex Tower Average Et Distribution (gTower Et < - 2 GeV) ;#eta;#phi;averageEt (count x 50 MeV)',
#                           fillGroup = "lowEtgTowers",
#                           type='TProfile2D',
#                           path=trigPath,
#                           hanConfig={"description":"The histogram is an average Et distrribution og the gTowers in the eta-phi space. This includes only those gTowers with Et < 10 GeV. The z-axis gives the average Et."},
#                           **eta_phi_bins)                           

    helper.defineHistogram('TowerEt;h_TowerEt', title='gFex Tower Et ; Et (count x 50 MeV)',
                        fillGroup = "gTowers",
                        type='TH1I',
                        path='Developer/gFexInput',
                        hanConfig={"description":""},
                        xbins= 100, xmin=-50.0, xmax=100.0)
    

    helper.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='gFex Tower Saturation FLag',
                            fillGroup = "gTowers",
                            type='TH1F',
                            path='Developer/gFexInput',
                            xbins=3,xmin=0.0,xmax=3.0)


    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    GfexInputMonitorCfg = GfexInputMonitoringConfig(flags)
    cfg.merge(GfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
