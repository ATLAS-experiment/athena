#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def GfexSimMonitoringConfig(flags, UseOfflineCopy = True):
    '''Function to configure LVL1 Gfex simulation comparison algorithm in the monitoring system.'''

    # use L1Calo's special MonitoringCfgHelper
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.GfexSimMonitorAlgorithm,'GfexSimMonAlg')

    helper.defineDQAlgorithm("L1CaloMismatchRate",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_GreaterThan_Threshold","BinThreshold":"0.9"}, # counts bins with value>0.9
                             thresholdConfig={"NBins":[0,10]}, # warn if any high rate, error if more than 10 bins anywhere.
                             )

    helper.defineHistogram('LBN,Signature;h_mismatched_SimReady',
                           fillGroup="mismatches",
                           paths=['Shifter/Sim'],
                           hanConfig={"algorithm":"Histogram_Empty","description":"Number of events with a mismatch, per LB (x-axis), per signature (y-axis) for signatures that are deemed simulation-ready","display":"SetPalette(55)"},
                           type='TH2I', cutmask='SimulationReadyMismatch',
                           title='Mismatched Simulation-Ready Events;LB;Signature;Events',
                           xbins=1,xmin=0,xmax=1,
                           ylabels=["gJ","gLJ","jJ","jTAU","jXE","jTE","eTAU","eEM"],
                           opt=['kAddBinsDynamically','kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineHistogram('LBN,Signature,tobMismatched;h_mismatched_SimReadyRate',
                           fillGroup="mismatches",
                           paths=['Expert/Sim'],
                           hanConfig={"algorithm":"L1CaloMismatchRate","description":"Mismatch rate, per LB (x-axis), per signature (y-axis) for signatures that are deemed simulation-ready - should not be high rate (see DQ algo)","display":"SetPalette(55)"},
                           type='TProfile2D', cutmask='SimulationReady',
                           title='Mismatched Rate for Simulation-Ready Signatures;LB;Signature;Event Rate (%)',
                           xbins=1,xmin=0,xmax=1,
                           ylabels=["gJ","gLJ","jJ","jTAU","jXE","jTE","eTAU","eEM"],
                           opt=['kAddBinsDynamically','kCanRebin','kAlwaysCreate'],merge='merge')
    helper.defineHistogram('EventType,Signature,tobMismatched;h_simSummary',title='Sim-HW Mismatches (percentage);Event Type;Signature',
                           fillGroup="mismatches",
                           path='Expert/Sim/detail', # place summary plot in the detail path in Expert audience
                           hanConfig={"display":"SetPalette(87),Draw=COLZTEXT"},
                           type='TProfile2D',
                           xlabels=["DataTowers","EmulatedTowers"],
                           ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                           opt=['kCanRebin','kAlwaysCreate'],merge="merge")

    for sig in ["gJ","gLJ"]:
        helper.defineHistogram("LBN,locIdx;h_"+sig+"_mismatches_posLbnMap", title = "Mismatched " + sig + " [DataTower evts];LB;Position (iEta:iPhi);TOBs",
                               fillGroup = "mismatches"+ sig,
                               type="TH2I",
                               path="Expert/Sim/detail",
                               hanConfig={
                                   "display":"SetPalette(87)",
                                   "description":"Location of mismatched " + sig + " TOBs. Use this plot to identify any localized gFEX issues. N.B. this plot is only created if there are mismatches."},
                               xbins=1,xmin=0,xmax=1,
                               ybins=1,ymin=0,ymax=1,
                               opt=['kAddBinsDynamically'])


    helper.defineTree('LBN,Signature,LBNString,EventNumber,dataEtas,dataPhis,dataWord0s,simEtas,simPhis,simWord0s;mismatched',
                      "lbn/l:Signature/string:lbnString/string:eventNumber/l:dataEtas/vector<float>:dataPhis/vector<float>:dataWord0s/vector<unsigned int>:simEtas/vector<float>:simPhis/vector<float>:simWord0s/vector<unsigned int>",
                      title="mismatched;LBN;Signature",fillGroup="mismatches")

    return helper.result()


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    import argparse
    parser = argparse.ArgumentParser(prog='python -m TrigT1CaloMonitoring.GfexSimMonitorAlgorithm',
                                     description="""Used to run gFEX Monitoring\n\n
                                   Example: python -m TrigT1CaloMonitoring.GfexSimMonitorAlgorithm --filesInput file.root.\n
                                   Overwrite inputs using standard athena opts --filesInput, evtMax etc. see athena --help""")
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
    args = parser.parse_args()


    flags = initConfigFlags()
    flags.Trigger.triggerConfig='DB'
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Output.HISTFileName = 'gFexSimData_Monitoring.root'

    flags.Exec.MaxEvents = args.evtMax
    flags.Exec.SkipEvents = args.skipEvents

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    GfexSimMonitorCfg = GfexSimMonitoringConfig(flags)
    cfg.merge(GfexSimMonitorCfg)

    from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
    GfexInputMonitorCfg = GfexInputMonitoringConfig(flags)
    cfg.merge(GfexInputMonitorCfg)

    cfg.run()
