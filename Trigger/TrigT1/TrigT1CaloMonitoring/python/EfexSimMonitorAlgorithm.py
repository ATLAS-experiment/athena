#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
    result.merge( LArMaskedSCCfg(flags, reloadEveryEvent = flags.Common.isOnline and flags.DQ.doMonitoring) )

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.EfexSimMonitorAlgorithm,'EfexSimMonAlg')

    # treat every event as not being fexInput if not decoding fex inputs
    if not flags.Trigger.L1.doCaloInputs: helper.alg.eFexTowerContainer=""

    helper.defineDQAlgorithm("L1CaloMismatchRate",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_GreaterThan_Threshold","BinThreshold":"0.9"}, # counts bins with value>0.9
                             thresholdConfig={"NBins":[0,10]}, # warn if any high rate, error if more than 10 bins anywhere.
                             )

    helper.defineHistogram('EventType,Signature,tobMismatched;h_simSummary',title='Sim-HW Mismatches (percentage);Event Type;Signature',
                           fillGroup="mismatches",
                            path='Expert/Sim/detail', # place summary plot in the detail path in Expert audience
                            hanConfig={"display":"SetPalette(87),Draw=COLZTEXT"},
                            type='TProfile2D',
                            xlabels=["DataTowers","EmulatedTowers"],
                            ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                            opt=['kCanRebin','kAlwaysCreate'],merge="merge")
    helper.defineHistogram('LBN,Signature;h_mismatched_SimReady',
                           fillGroup="mismatches",
                           paths=['Shifter/Sim'],
                           hanConfig={"algorithm":"Histogram_Empty","description":"Number of events with a mismatch, per LB (x-axis), per signature (y-axis) for signatures that are deemed simulation-ready","display":"SetPalette(55)"},
                           type='TH2I', cutmask='SimulationReadyMismatch',
                           title='Mismatched Simulation-Ready Events;LB;Signature;Events',
                           xbins=1,xmin=0,xmax=1,
                           ylabels=["gJ","gLJ","jJ","jTAU","jEM","jXE","jTE","eTAU","eEM"],
                           opt=['kAddBinsDynamically','kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineHistogram('LBN,Signature,tobMismatched;h_mismatched_SimReadyRate',
                           fillGroup="mismatches",
                           paths=['Expert/Sim'],
                           hanConfig={"algorithm":"L1CaloMismatchRate","description":"Mismatch rate, per LB (x-axis), per signature (y-axis) for signatures that are deemed simulation-ready - should not be high rate (see DQ algo)","display":"SetPalette(55)"},
                           type='TProfile2D', cutmask='SimulationReady',
                           title='Mismatched Rate for Simulation-Ready Signatures;LB;Signature;Event Rate (%)',
                           xbins=1,xmin=0,xmax=1,
                           ylabels=["gJ","gLJ","jJ","jTAU","jEM","jXE","jTE","eTAU","eEM"],
                           opt=['kAddBinsDynamically','kCanRebin','kAlwaysCreate'],merge='merge')
    # when there are mismatches, would be useful to know where they occurred (might be a single module gone bad)
    # so register a location-vs-lbn histogram
    for sig in ["eEM","eTAU"]:
        helper.defineHistogram("LBN,locIdx;h_"+sig+"_mismatches_posLbnMap", title = "Mismatched " + sig + " [DataTower evts];LB;Position (Module:Proc:Eta:Phi);TOBs",
                           fillGroup = sig + "_mismatches", cutmask='SimulationReady',
                           path = "Expert/Sim",
                           hanConfig={
                               "algorithm":"Histogram_Empty",
                               "display":"SetPalette(87)",
                               "description":"Location of mismatched " + sig + " TOBs in simulation-ready events. Use this plot to identify any localized eFEX issues. N.B. this plot is only created if there are mismatches."},
                           type="TH2I",
                           xbins=1,xmin=0,xmax=1,
                           ybins=1,ymin=0,ymax=1,
                               opt=['kAddBinsDynamically'])
        helper.defineHistogram("LBN,locIdx;h_"+sig+"_mismatchesEmulated_posLbnMap", title = "Mismatched " + sig + " [EmulatedTower evts];LB;Position (Module:Proc:Eta:Phi);TOBs",
                               fillGroup = sig + "_mismatches", cutmask='IsEmulatedTowers',
                               path = "Expert/Sim",
                               hanConfig={
                                   "algorithm":"Histogram_Empty",
                                   "display":"SetPalette(87)",
                                   "description":"Location of mismatched " + sig + " TOBs in events with EmulatedTower simput. Discuss mismatches with expert, they may be caused by LATOME readout issues if there are LAr Mismatches in Input/eFEX folder. N.B. this plot is only created if there are mismatches."},
                               type="TH2I",
                               xbins=1,xmin=0,xmax=1,
                               ybins=1,ymin=0,ymax=1,
                               opt=['kAddBinsDynamically'])
    helper.defineHistogram('LBNString,Signature;h_mismatched_DataTowerEvts',
                           fillGroup="mismatches",
                           type='TH2I', cutmask='IsDataTowers',
                           title='Mismatched DataTower Events;LB:FirstEvtNum;Signature;Events',
                           xbins=1,xmin=0,xmax=1,
                           ybins=1,ymin=0,ymax=1,
                           opt=['kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineHistogram('LBN,Signature;h_mismatched_EmulatedTowerEvts',
                           fillGroup="mismatches",
                           type='TH2I', cutmask='IsEmulatedTowers',
                           title='Mismatched EmulatedTower Events;LB;Signature;Events',
                           xbins=1,xmin=0,xmax=1,
                           ybins=1,ymin=0,ymax=1,
                           opt=['kCanRebin','kAlwaysCreate','kAddBinsDynamically'],merge='merge')
    helper.defineTree('LBN,SignatureEvtType,LBNString,EventNumber,EventType,timeSince,timeUntil,dataEtas,dataPhis,dataWord0s,simEtas,simPhis,simWord0s;mismatched',
                      "lbn/l:Signature/string:lbnString/string:eventNumber/l:EventType/string:timeSince/I:timeUntil/I:dataEtas/vector<float>:dataPhis/vector<float>:dataWord0s/vector<unsigned int>:simEtas/vector<float>:simPhis/vector<float>:simWord0s/vector<unsigned int>",
                      title="mismatched (including events with LATOME readout and OTF masking issues);LBN;Signature",
                      fillGroup="mismatches")


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

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    EfexSimMonitorCfg = EfexSimMonitoringConfig(flags)
    cfg.merge(EfexSimMonitorCfg)

    cfg.run()

