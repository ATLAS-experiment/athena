#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# note: string DQ algo parameters must have leading/trailing ' char, to distinguish from other parts of algo config
# these initial values were determined from run 486894 in 2024
# these should be reviewed at an L1Calo meeting in 2025, prior to data taking
# commenting out for start of 2025 - will remove this comment once new set of known anomalies established
knownAnomalies_hotHcal = {
    #     "KnownDead":"\"1,29;12,57;15,57;18,24;18,41;18,50;19,41;2,29;20,41;21,41;22,41;23,29;23,41;24,41;25,36;25,41;26,18;26,38;27,18;28,18;29,18;32,18;33,18;41,21;41,54;42,21;42,43;47,6;7,62;8,54;9,41;9,51;9,52;9,53;9,54;9,56\"",
    "KnownDead":"\"1,29;13,36;18,24;18,50;2,29;23,29;25,36;26,38;30,15;32,46;36,39;41,21;41,54;42,21;42,43;47,6;7,62;8,54;9,41;9,51;9,52;9,53;9,54;9,56;26,10;27,10;28,10;29,10;30,10;31,10;32,10;33,10;18,3;19,3;20,3;21,3;22,3;23,3;24,3;25,3\"",
    "KnownWarm":"\"26,3;27,3;28,3;29,3;30,3;31,3;32,3;33,3;34,3\"", # noisy tile drawer
    "KnownHot":"\"15,49\"", # one hotspot in LAr HCal?
    "KnownCold":"\"\""
    # Below is the HI Config
    #"KnownCold":"\"31,58\"",
    #"KnownHot":"\"15,49;11,27;6,33;22,47;26,20;3,5;40,24\"",
    #"KnownDead":"\"9,17;9,18;9,19;9,20;9,21;9,22;9,23;9,24;5,33;6,33;8,23;3,34;43,59;44,61;45,41;46,24\""
}

knownAnomalies_coldHcal = {
    #"KnownDead":"\"1,29;2,29;41,21;41,54;42,21;42,43;47,6;7,62;8,54;9,41;9,51;9,52;9,53;9,54;9,56\"",
    "KnownDead":knownAnomalies_hotHcal["KnownDead"],
    "KnownCold":"\"1,30;2,30\"",
    "KnownHot":"\"6,33\""
    #"KnownWarm":"\"5,33\"" # one slightly-frequent coldspot
}
knownAnomalies_hotEcal = {
    "KnownHot":"\"2,15;2,16;2,47;2,48;2,55;2,56;47,35;47,36;48,35;48,36;47,47;47,48;48,47;48,48;4,5;13,48\"",
    "KnownCold":"\"19,18;48,8;8,33;9,19;9,20;9,33;9,34\"",
    "KnownWarm":"\"2,15;2,47;2,55;2,56;47,35;47,36;47,47;47,48;48,47;50,44\""
    # Below is HI Config
    # "KnownCold":"\"8,33;9,33;8,34;9,34;10,18;9,19;9,20;10,20;20,48;49,41;49,42;19,18;19,9;3,17;48,8;8,11;8,40;26,14\"",
    # "KnownWarm":"\"22,13;22,14;23,13:23,14;22,47;22,48;23,47;23,48\"",
    # "KnownHot":"\"4,5;13,48\"" # from HI Running
}
knownAnomalies_coldEcal = {
    #     "KnownHot":"\"1,15;2,15;47,47;48,36\"",
    "KnownHot":"\"1,15;1,16;2,15;2,16;1,47;1,48;2,47;2,48;1,55;1,56;2,55;2,56;22,47;22,48;23,47;23,48;22,13;22,14;23,13;23,14;50,43;50,44;47,35;47,36;48,35;48,36;47,47;47,48;48,47;48,48\"",
    "KnownCold":"\"8,33;8,34;49,41;49,42;10,20;9,34\""
    #     "KnownWarm":"\"1,47;1,48;1,49;1,55;1,56;2,16;2,47;2,48;2,55;2,56;22,14;23,14;47,35;47,36;47,48;48,35;48,47;48,48;49,51;50,43;50,44\""
}

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
    helper.defineHistogram('TowerEta,TowerPhi;h_errors',title='EfexInput Errors;#eta;#phi',
                           path="Expert/Inputs/eFEX",
                           hanConfig={"algorithm":"Histogram_Empty","description":"Locations of any non-zero em or hadronic status flags. Check <a href='./detail/h_summary'>detail/h_summary</a> for more detail if there are entries"},
                           fillGroup="errors",cutmask='IsMonReady',
                           type='TH2I',
                           xbins=50,xmin=-2.5,xmax=2.5,
                           ybins=64,ymin=-math.pi,ymax=math.pi)





    commonAlgConfig = {"libname":"libdqm_summaries.so",
                       "name":"L1Calo_BinsDiffFromStripMedian",
                       "PublishDetail":32}
    hotCuts = {"ColdCut":-3.5,"WarmCut":9,"HotCut":20} # when looking at frequency of hot deposits, use these cuts
    coldCuts = {"ColdCut":-4,"WarmCut":5,"HotCut":10} # lower thresholds for looking at negative energy deposits, as RMS is usually smaller

    commonThresholdConfig = {
        "NWrongKnown":[0,100], # warn of any corrections that are needed for the known anomalies lists
        "NDead":[0,2], # warn on any new dead spots, error if more than a couple
        "NHot":[0,5],  # warn on any new hot spots, error if more than a couple
        "NCold":[0,5],  # warn on any new cold spots, error if more than a couple
        "NWarm":[30,40],  # warn on more than 30 warm spots, error if more than 40
        "NDeadStrip":[0,0], # no dead strips - exception to this will be in cold hcal, where tile cannot be negative
        "NConsecUnlikelyStrip":[2,5], # warn if more than 2 consecutive strips deemed unlikely
    }

    helper.defineDQAlgorithm("Efex_ecal_hot_etaPhiMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotEcal,
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_hcal_hot_etaPhiMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotHcal,
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_ecal_hot_etaPhiLBMapOutliers_Shifter",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotEcal|{"NBinsY":64,"LiveMode":1},
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_hcal_hot_etaPhiLBMapOutliers_Shifter",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotHcal|{"NBinsY":64,"LiveMode":1},
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_ecal_hot_etaPhiLBMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotEcal|{"NBinsY":64,"LiveMode":0},
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_hcal_hot_etaPhiLBMapOutliers",
                             hanConfig=commonAlgConfig|hotCuts|knownAnomalies_hotHcal|{"NBinsY":64,"LiveMode":0},
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_ecal_cold_etaPhiMapOutliers",
                             hanConfig=commonAlgConfig|coldCuts|knownAnomalies_coldEcal,
                             thresholdConfig=commonThresholdConfig
                             )

    helper.defineDQAlgorithm("Efex_hcal_cold_etaPhiMapOutliers",
                             hanConfig=commonAlgConfig|coldCuts|knownAnomalies_coldHcal,
                             thresholdConfig=commonThresholdConfig|{"NDeadStrip":[30,30]} # negatives not possible in tile (-1.5->1.5)
                             )

    for layer in ["ecal","hcal"]:
        helper.defineHistogram(f'TowerEta,TowerPhi;h_dataTowers_{layer}_hot_EtaPhiMap',title=f'{layer.upper()} SuperCells >= 500MeV;#eta;#phi',
                               cutmask="AboveCut",
                               paths=["Expert/Inputs/eFEX/detail","Shifter/Inputs/eFEX"],
                               hanConfig={"algorithm":f"Efex_{layer}_hot_etaPhiMapOutliers","description":f"Check <a href='./detail/h_dataTowers_{layer}_hot_posVsLBN'>detail plot</a> to get timeseries for each location"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=50,xmin=-2.5,xmax=2.5,
                               ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

        helper.defineHistogram(f'LBN,binNumber;h_dataTowers_{layer}_hot_posVsLBN',title=f'{layer.upper()} SuperCells >= 500MeV;LB;64(x-1)+y',
                           paths=["Expert/Inputs/eFEX/detail","Shifter/Inputs/eFEX"],
                           cutmask="AboveCut",
                           hanConfig={"Expert/algorithm":f"Efex_{layer}_hot_etaPhiLBMapOutliers","Shifter/algorithm":f"Efex_{layer}_hot_etaPhiLBMapOutliers_Shifter",
                                      "description":f"x and y correspond to axis bin numbers on <a href='../h_dataTowers_{layer}_hot_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                           fillGroup=layer,
                           type='TH2I',
                           xbins=1,xmin=0,xmax=10,
                           ybins=64*50,ymin=0.5,ymax=64*50+0.5,
                           opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")
        helper.defineHistogram(f'TowerEta,TowerPhi;h_dataTowers_{layer}_cold_EtaPhiMap',title=f'{layer.upper()} SuperCells <= -500MeV;#eta;#phi',
                               cutmask="BelowCut",
                               path="Expert/Inputs/eFEX/detail",
                               hanConfig={"algorithm":f"Efex_{layer}_cold_etaPhiMapOutliers","description":f"Check <a href='./detail/h_dataTowers_{layer}_cold_posVsLBN'>detail plot</a> to get timeseries for each location"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=50,xmin=-2.5,xmax=2.5,
                               ybins=64,ymin=-math.pi,ymax=math.pi,opt=['kAlwaysCreate'])

        helper.defineHistogram(f'LBN,binNumber;h_dataTowers_{layer}_cold_posVsLBN',title=f'{layer.upper()} SuperCells <= -500MeV;LB;64(x-1)+y',
                               path="Expert/Inputs/eFEX/detail",
                               cutmask="BelowCut",
                               hanConfig={"description":f"x and y correspond to axis bin numbers on <a href='../h_dataTowers_{layer}_cold_EtaPhiMap'/>eta-phi plot</a>. Use this plot to check if hotspot/coldspots affected whole or part of run: turn on Projection X1 to see 1D hist of individual locations"},
                               fillGroup=layer,
                               type='TH2I',
                               xbins=1,xmin=0,xmax=10,
                               ybins=64*50,ymin=0.5,ymax=64*50+0.5,
                               opt=['kAddBinsDynamically','kAlwaysCreate'],merge="merge")

    helper.defineTree('LBN,Error,LBNString,EventNumber,TowerId,TowerEta,TowerPhi,TowerEmstatus,TowerHadstatus,TowerSlot,TowerCount,RefTowerCount,SlotSCID,timeSince,timeUntil;errors',
                                           "lbn/I:error/string:lbnString/string:eventNumber/l:id/I:eta/F:phi/F:em_status/i:had_status/i:slot/I:count/I:ref_count/I:scid/string:timeSince/I:timeUntil/I",
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
