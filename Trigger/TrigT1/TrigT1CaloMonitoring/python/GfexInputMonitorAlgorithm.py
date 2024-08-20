#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def GfexInputMonitoringConfig(flags):
    '''Function to configure LVL1 GfexInput algorithm in the monitoring system.'''

    import math 
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

    # mainDir = 'L1Calo'
    trigPath = 'Developer/GfexInput/'

    # See if the file contains xTOBs else use TOBs
    #hasXtobs = True if "L1_eFexTower" in inputFlags.Input.Collections else False
    #if not hasXtobs:
    #    EfexInputMonAlg.eFexTowerContainer = "L1_eEMRoI"

    #tobStr = "TOB"


    # histograms of gFex tower variables
    helper.defineHistogram('NGfexTowers;h_nGfexTowers', title='Number of gFex towers in each event with Et > 10 GeV; gTowers per event',
                                   fillGroup = "highEtgTowers", type='TH1I', path=trigPath, xbins=100,xmin=0,xmax=100)

    helper.defineHistogram('TowerEta,TowerPhi,TowerEt;h_TowerHeatMap', title='gFex Tower Average Et Distribution (gTowerEt > 10 GeV) ;#eta;#phi;averageEt (count x 50 MeV)',
                            fillGroup = "highEtgTowers",type='TProfile2D',path=trigPath, xbins=100,xmin=-5.0,xmax=5.0,ybins=66,ymin=-math.pi,ymax=math.pi)


    helper.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='gFex Tower Eta vs Phi with gTower Et < 10 GeV ;#eta;#phi',
                            fillGroup = "lowEtgTowers", type='TH2F',path=trigPath, xbins=100,xmin=-5.0,xmax=5.0,ybins=66,ymin=-math.pi,ymax=math.pi)

    
    helper.defineHistogram('TowerFpga;h_TowerFpga', title='gFex Tower (gTowerEt > 10 GeV ) FPGA Number;Number of FPGA',
                            fillGroup = "highEtgTowes", type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4.0)

    helper.defineHistogram('TowerEt;h_TowerEt', title='gFex Tower Et ; Et (count x 50 MeV)',
                            fillGroup = "gTowers", type='TH1I', path=trigPath, xbins= 1000, xmin=-500.0, xmax=500.0)

    helper.defineHistogram('MaxEt;h_MaxTowerEt', title='gFex Max Tower Et for saturation=true; Et (count x 50 MeV)',
                            fillGroup = "gTowers", type='TH1F', path=trigPath, xbins=350,xmin=-50.0,xmax=300.0)
    
    helper.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='gFex Tower Saturation FLag',
                            fillGroup = "gTowers", type='TH1F', path=trigPath, xbins=3,xmin=0,xmax=3.0)

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
