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

    helper.defineHistogram('LBN,ErrorAndLocation;h_summary',title='EfexInput Monitoring summary;LBN;Error:TowerID',
                              path="Expert/Inputs/eFEX/detail",
                              hanConfig={"description":"TowerID format: '[E]EPPMMF', where E=eta index, P=phi index,M=module,F=fpga"},
                              fillGroup="errors",
                              type='TH2I',
                              xbins=1,xmin=0,xmax=1,
                              ybins=1,ymin=0,ymax=1,
                              opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")

    import math
    helper.defineHistogram('TowerEta,TowerPhi;h_errors',title='EfexInput Errors (BadEMStatus,BadHadStatus);#eta;#phi',
                           path="Expert/Inputs/eFEX",
                           hanConfig={"algorithm":"Histogram_Empty","description":"Locations of any non-zero em or hadronic status flags. Check <a href='./detail/h_summary'>detail/h_summary</a> for more detail if there are entries"},
                           fillGroup="errors",cutmask='IsMonReady',
                           type='TH2I',
                           xbins=50,xmin=-2.5,xmax=2.5,
                           ybins=64,ymin=-math.pi,ymax=math.pi)

    helper.defineDQAlgorithm("Efex_ecal_hot_etaThiMapFilled",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_Equal_Threshold","BinThreshold":"0."},
                             thresholdConfig={"NBins":[0,64*50]}, # currently there is no known deadspot in ecal, so that is warning. Error if empty
                             )
    helper.defineDQAlgorithm("Efex_hcal_hot_etaThiMapFilled",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_Equal_Threshold","BinThreshold":"0."},
                             thresholdConfig={"NBins":[50,64*50]}, # currently there are disabled drawers (24 dead towers). Error if empty
                             )

    for layer in ["ecal","hcal"]:
        helper.defineHistogram(f'TowerEta,TowerPhi;h_dataTowers_{layer}_hot_EtaPhiMap',title=f'{layer.upper()} SuperCells >= 500MeV;#eta;#phi',
                               cutmask="AboveCut",
                               path="Expert/Inputs/eFEX",
                               hanConfig={"algorithm":f"Efex_{layer}_hot_etaThiMapFilled","description":f"Check <a href='./detail/h_dataTowers_{layer}_hot_posVsLBN'>detail plot</a> to get timeseries for each location"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=50,xmin=-2.5,xmax=2.5,
                               ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

        helper.defineHistogram(f'LBN,binNumber;h_dataTowers_{layer}_hot_posVsLBN',title=f'{layer.upper()} SuperCells >= 500MeV;LB;50(y-1)+x',
                           path="Expert/Inputs/eFEX/detail",
                           cutmask="AboveCut",
                           hanConfig={"description":f"x and y correspond to axis bin numbers on <a href='../h_dataTowers_{layer}_hot_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                           fillGroup=layer,
                           type='TH2I',
                           xbins=1,xmin=0,xmax=10,
                           ybins=64*50,ymin=0.5,ymax=64*50+0.5,
                           opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")
        helper.defineHistogram(f'TowerEta,TowerPhi;h_dataTowers_{layer}_cold_EtaPhiMap',title=f'{layer.upper()} SuperCells <= -500MeV;#eta;#phi',
                               cutmask="BelowCut",
                               path="Expert/Inputs/eFEX",
                               hanConfig={"description":f"Check <a href='./detail/h_dataTowers_{layer}_cold_posVsLBN'>detail plot</a> to get timeseries for each location"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=50,xmin=-2.5,xmax=2.5,
                               ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

        helper.defineHistogram(f'LBN,binNumber;h_dataTowers_{layer}_cold_posVsLBN',title=f'{layer.upper()} SuperCells <= -500MeV;LB;50(y-1)+x',
                               path="Expert/Inputs/eFEX/detail",
                               cutmask="BelowCut",
                               hanConfig={"description":f"x and y correspond to axis bin numbers on <a href='../h_dataTowers_{layer}_cold_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=1,xmin=0,xmax=10,
                               ybins=64*50,ymin=0.5,ymax=64*50+0.5,
                               opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")

    helper.defineTree('LBNString,Error,EventNumber,TowerId,TowerEta,TowerPhi,TowerEmstatus,TowerHadstatus,TowerSlot,TowerCount,RefTowerCount,SlotSCID,timeSince,timeUntil;errors',
                                           "lbnString/string:error/string:eventNumber/l:id/I:eta/F:phi/F:em_status/i:had_status/i:slot/I:count/I:ref_count/I:scid/string:timeSince/I:timeUntil/I",
                                           title="errors tree;LBN;Error",fillGroup="errors")


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
