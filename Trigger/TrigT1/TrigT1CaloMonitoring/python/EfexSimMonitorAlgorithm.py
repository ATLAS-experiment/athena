#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def EfexSimMonitoringConfig(flags):
    '''Function to configure LVL1 Efex simulation comparison algorithm in the monitoring system.'''


    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # uncomment if you want to see all the flags
    #flags.dump() # print all the configs

    # sim monitoring requires knowing how close to a LAr masking the event is, add MaskedSCCondAlg
    from LArBadChannelTool.LArBadChannelConfig import LArMaskedSCCfg
    result.merge( LArMaskedSCCfg(flags) )

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.EfexSimMonitorAlgorithm,'EfexSimMonAlg')

    # treat every event as not being fexInput if not decoding fex inputs
    if not flags.Trigger.L1.doCaloInputs: helper.alg.eFexTowerContainer=""

    helper.defineHistogram('EventType,Signature,tobMismatched;h_simSummary',title='Sim-HW Mismatches (percentage);Event Type;Signature',
                           fillGroup="mismatches",
                            path='Expert/Sim/detail', # place summary plot in the detail path in Expert audience
                            hanConfig={"display":"SetPalette(87),Draw=COLZTEXT"},
                            type='TProfile2D',
                            xlabels=["DataTowers","EmulatedTowers"],
                            ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                            opt=['kCanRebin','kAlwaysCreate'],merge="merge")
    helper.defineHistogram('LBNString,Signature;h_mismatched_DataTowerEvts',
                           fillGroup="mismatches",
                           paths=['Shifter/Sim','Expert/Sim'],
                           hanConfig={"algorithm":"Histogram_Empty"},
                           type='TH2I', cutmask='IsDataTowers',
                           title='Mismatched DataTower Events;LB:FirstEvtNum;Signature;Events',
                           xlabels=[""],
                           ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                           opt=['kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineHistogram('LBNString,Signature;h_mismatched_EmulatedTowerEvts',
                           fillGroup="mismatches",
                           type='TH2I', cutmask='IsEmulatedTowers',
                           title='Mismatched EmulatedTower Events;LB:FirstEvtNum;Signature;Events',
                           xlabels=[""],
                           ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                           opt=['kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineTree('LBNString,LBN,EventNumber,fexReadout,timeSince,timeUntil,tobType,dataEtas,dataPhis,dataWord0s,simEtas,simPhis,simWord0s;mismatched',
                      "lbnString/string:lbn/l:eventNumber/l:fexReadout/i:timeSince/I:timeUntil/I:tobType/i:dataEtas/vector<float>:dataPhis/vector<float>:dataWord0s/vector<unsigned int>:simEtas/vector<float>:simPhis/vector<float>:simWord0s/vector<unsigned int>",
                      title="mismatched;LBN",fillGroup="mismatches")


    result.merge(helper.result())
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    import glob

    # MCs processed adding L1_eEMRoI
    inputs = glob.glob('/eos/user/t/thompson/ATLAS/LVL1_mon/MC_ESD/l1calo.361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W.eFex_gFex_2022-01-13T2101.root')
    
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    flags.Exec.MaxEvents=10

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    EfexSimMonitorCfg = EfexSimMonitoringConfig(flags)
    cfg.merge(EfexSimMonitorCfg)

    cfg.run()

