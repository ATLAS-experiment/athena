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
    helper.defineHistogram('NGfexTowers;h_nGfexTowers', title='Number of gFex towers',
                            fillGroup = groupName, type='TH1I', path=trigPath, xbins=500,xmin=0,xmax=5000)

    helper.defineHistogram('TowerEta;h_TowerEta', title='gFex Tower Eta',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('TowerPhi;h_TowerPhi', title='gFex Tower Phi',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=66,xmin=-math.pi,xmax=math.pi)

    helper.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='gFex Tower Eta vs Phi',
                            fillGroup = groupName, type='TH2F',path=trigPath, xbins=100,xmin=-5.0,xmax=5.0,ybins=66,ymin=-math.pi,ymax=math.pi)

    helper.defineHistogram('TowerEtaindex;h_TowerEtaindex', title='gFex Tower Eta Index',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=50,xmin=0.0,xmax=35.0)

    helper.defineHistogram('TowerPhiindex;h_TowerPhiindex', title='gFex Tower Phi Index',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=64,xmin=0.0,xmax=32.0)

    helper.defineHistogram('TowerEtaindex,TowerPhiindex;h_TowerEtaPhiMapindex', title='gFex Tower Eta vs Phi index',
                            fillGroup = groupName, type='TH2F',path=trigPath, xbins=50,xmin=0.0,xmax=35.0,ybins=64,ymin=0,ymax=32.0)

    helper.defineHistogram('TowerFpga;h_TowerFpga', title='gFex Tower FPGA Number',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4.0)

    helper.defineHistogram('TowerEt;h_TowerEt', title='gFex Tower Et',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=1000,xmin=0,xmax=1000.0)

    helper.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='gFex Tower Saturation FLag',
                            fillGroup = groupName, type='TH1F', path=trigPath, xbins=2,xmin=0,xmax=2.0)
    
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

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

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
