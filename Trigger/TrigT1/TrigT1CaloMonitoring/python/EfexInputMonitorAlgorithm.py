#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def EfexInputMonitoringConfig(flags):
    '''Function to configure LVL1 EfexInput algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from LArBadChannelTool.LArBadChannelConfig import LArMaskedSCCfg

    result.merge(LArMaskedSCCfg(flags))

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.EfexInputMonitorAlgorithm,'EfexInputMonAlg')

    helper.defineHistogram('LBNString,Error;h_summary',title='EfexInput Monitoring summary;LBN;Error',
                              fillGroup="errors",
                              type='TH2I',
                              xlabels=[""],
                              ybins=1,ymin=0,ymax=1,ylabels=[""],
                              opt=['kCanRebin','kAlwaysCreate'],merge="merge")

    helper.defineHistogram('LBNString,TowerEta,TowerCount;h_dataTowers_ecal',title='DataTowers ECAL Average;LBN;Eta',
                           fillGroup="ecal",
                           type='TProfile2D',
                           xlabels=[""],
                           ybins=50,ymin=-2.5,ymax=2.5,
                           opt=['kCanRebin','kAlwaysCreate'],merge="merge")
    helper.defineHistogram('LBNString,TowerEta,TowerCount;h_dataTowers_hcal',title='DataTowers HCAL Average;LBN;Eta',
                           fillGroup="hcal",
                           type='TProfile2D',
                           xlabels=[""],
                           ybins=50,ymin=-2.5,ymax=2.5,
                           opt=['kCanRebin','kAlwaysCreate'],merge="merge")
    helper.defineTree('Error,EventNumber,TowerId,TowerEta,TowerPhi,TowerEmstatus,TowerHadstatus,TowerSlot,TowerCount,RefTowerCount,SlotSCID,timeSince,timeUntil;errors',
                                           "error/string:eventNumber/l:id/I:eta/F:phi/F:em_status/i:had_status/i:slot/I:count/I:ref_count/I:scid/string:timeSince/I:timeUntil/I",
                                           title="errors tree;Error",fillGroup="errors")


    result.merge(helper.result())
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

    EfexInputMonitorCfg = EfexInputMonitoringConfig(flags)
    cfg.merge(EfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
