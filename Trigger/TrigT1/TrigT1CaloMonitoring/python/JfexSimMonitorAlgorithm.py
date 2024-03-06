#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JfexSimMonitoringConfig(flags, UseOfflineCopy = True):
    '''Function to configure LVL1 Efex simulation comparison algorithm in the monitoring system.'''

    # use L1Calo's special MonitoringCfgHelper
    from AthenaConfiguration.ComponentFactory import CompFactory
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JfexSimMonitorAlgorithm,'JfexSimMonAlg')
    JfexSimMonAlg = helper.alg

    # # add any steering
    # groupName = "JfexSimMonitor" # the monitoring group name is also used for the package name
    # JfexSimMonAlg.Grouphist = groupName
    #
    # mainDir = 'L1Calo'
    # trigPath = 'JfexSim/'
    #
    doXtobs = False
    if doXtobs:
        JfexSimMonAlg.jFexSRJetRoIContainer = "L1_jFexSRJetxRoI"
        JfexSimMonAlg.jFexLRJetRoIContainer = "L1_jFexLRJetxRoI"
        JfexSimMonAlg.jFexTauRoIContainer   = "L1_jFexTauxRoI"
        JfexSimMonAlg.jFexFwdElRoIContainer = "L1_jFexFwdElxRoI"
        JfexSimMonAlg.jFexMETRoIContainer   = "L1_jFexMETxRoI"
        JfexSimMonAlg.jFexSumETRoIContainer = "L1_jFexSumETxRoI"
        
        JfexSimMonAlg.jFexSRJetRoISimContainer = "L1_jFexSRJetxRoISim"
        JfexSimMonAlg.jFexLRJetRoISimContainer = "L1_jFexLRJetxRoISim"
        JfexSimMonAlg.jFexTauRoISimContainer   = "L1_jFexTauxRoISim"
        JfexSimMonAlg.jFexFwdElRoISimContainer = "L1_jFexFwdElxRoISim"
        JfexSimMonAlg.jFexMETRoISimContainer   = "L1_jFexMETxRoISim"
        JfexSimMonAlg.jFexSumETRoISimContainer = "L1_jFexSumETxRoISim"


    helper.defineHistogram('EventType,Signature,tobMismatched;h_simSummary',title='Sim-HW Mismatches (percentage);Event Type;Signature',
                           fillGroup="mismatches",
                           path='Expert/Sim/detail', # place summary plot in the detail path in Expert audience
                           hanConfig={"display":"SetPalette(87),Draw=COLZTEXT"},
                           type='TProfile2D', # would like TProfile2D but binning with labels doesn't work
                           xlabels=["DataTowers","EmulatedTowers"],
                           ymin=0,ymax=len(L1CaloMonitorCfgHelper.SIGNATURES),ylabels=L1CaloMonitorCfgHelper.SIGNATURES,
                           opt=['kCanRebin','kAlwaysCreate'],merge="merge")
    #
    # # add monitoring algorithm to group, with group name and main directory
    # myGroup = helper.addGroup(JfexSimMonAlg, groupName, mainDir)
    #
    #
    #
    #
    # myGenericGroup = helper.addGroup(None, groupName+"Gen", mainDir)
    #
    # myGenericGroup.defineHistogram('genLocation,genType;h_jFEX_Errors', path=None, type='TH2I',
    #                         title='jFEX generic monitoring for shifters;Location;Type',
    #                         xbins=4, xmin=0, xmax=4, xlabels=["Sim_DataTowers","Sim_EmulatedTowers","Input_Mismatch","Input_Invalids"],
    #                         ybins=4, ymin=0, ymax=4, ylabels=["TOB", "global TOB", "EM layer", "HAD layer" ],
    #                         opt=['kCanRebin'])
    #
    # JfexSimMonAlg.jFEXMonTool = myGenericGroup
    #
    #
    #
    # Input_items = ["EmulatedTowers","DataTowers"]
    # TOB_items = ["jJ","jLJ","jTau","jEM","jXE", "jTE"]
    # FPGA_names = ["U1","U2","U4","U3"]
    # Modules_names = ["jFEX 0","jFEX 1","jFEX 2","jFEX 3","jFEX 4","jFEX 5"]
    #
    # from ROOT import TMath
    #
    # x_phi = []
    # for i in range(67):
    #     phi = (-TMath.Pi()- TMath.Pi()/32) + TMath.Pi()/32*i
    #     x_phi.append(phi)
    # x_phi = sorted(x_phi)
    #
    # eta_phi_bins = {
    #     'xbins': 100, 'xmin': -5, 'xmax': 5,
    #     'ybins': x_phi
    # }
    #
    # myGroup.defineHistogram('item,input;h_ErrorTOBs', title="Errors in each TOB item depending on the input data; TOB item; Input Tower",
    #                         type='TH2I',path=trigPath, xbins=6,xmin=0,xmax=6,ybins=2,ymin=0,ymax=2,xlabels=TOB_items,ylabels=Input_items)
    #
    # for tower in Input_items:
    #     for item in TOB_items:
    #
    #
    #         if item == "jTE" or item == "jXE":
    #             groupname = groupName+"_SimEqData_"+item+"_"+tower
    #             SimEqDataGroup   = helper.addGroup(JfexSimMonAlg, groupname, mainDir)
    #
    #             groupname = groupName+"_SimDiffData_"+item+"_"+tower
    #             SimDiffDataGroup = helper.addGroup(JfexSimMonAlg, groupname, mainDir)
    #
    #             SimEqDataGroup.defineHistogram('jfex,fpga;h_DetectorMap_'+tower, title=item+" jFex module vs FPGA - "+tower+"; jFEX module; FPGA",
    #                                             type='TH2I',path=trigPath+item+'/matched', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)
    #
    #             SimDiffDataGroup.defineHistogram('jfex,fpga;h_DetectorMap_'+tower, title=item+" jFex module vs FPGA - "+tower+"; jFEX module; FPGA",
    #                                             type='TH2I',path=trigPath+item+'/unmatched', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)
    #
    #             SimDiffDataGroup.defineHistogram('LB;h_LBerror_'+tower, title=item+" LB for the mismatch - "+tower+"; LB; Counts",
    #                                             type='TH1I',path=trigPath+item+'/unmatched', xbins=1,xmin=0,xmax=0,opt=['kCanRebin'])
    #
    #         else:
    #
    #             groupname = groupName+"_SimEqData_"+item+"_"+tower
    #             SimEqDataGroup = helper.addGroup(JfexSimMonAlg, groupname, mainDir)
    #
    #             groupname = groupName+"_SimNoData_"+item+"_"+tower
    #             SimNoDataGroup = helper.addGroup(JfexSimMonAlg, groupname, mainDir)
    #
    #             groupname = groupName+"_DataNoSim_"+item+"_"+tower
    #             DataNoSimGroup = helper.addGroup(JfexSimMonAlg, groupname, mainDir)
    #
    #             SimEqDataGroup.defineHistogram('eta,phi;h_EtaPhiMap_'+tower, title="jFex "+item+" #eta vs #phi matched in Simulation - "+tower+";#eta;#phi",
    #                                             type='TH2F',path=trigPath+item+'/matched', **eta_phi_bins)
    #
    #             SimNoDataGroup.defineHistogram('eta,phi;h_EtaPhiMap_'+tower, title=item+" in Simulation but not in Data - "+tower+";#eta;#phi",
    #                                             type='TH2F',path=trigPath+item+'/unmatched/SimNoData', **eta_phi_bins)
    #
    #             SimNoDataGroup.defineHistogram('jfex,fpga;h_DetectorMap_'+tower, title=item+" jFex module vs FPGA - "+tower+"; jFEX module; FPGA",
    #                                             type='TH2I',path=trigPath+item+'/unmatched/SimNoData', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)
    #
    #             SimNoDataGroup.defineHistogram('LB;h_LBerror_'+tower, title=item+" LB for the mismatch - "+tower+"; LB; Counts",
    #                                             type='TH1I',path=trigPath+item+'/unmatched/SimNoData', xbins=1,xmin=0,xmax=0,opt=['kCanRebin'])
    #
    #             DataNoSimGroup.defineHistogram('eta,phi;h_EtaPhiMap_'+tower, title=item+" in Simulation but not in Data - "+tower+";#eta;#phi",
    #                                             type='TH2F',path=trigPath+item+'/unmatched/DataNoSim', **eta_phi_bins)
    #
    #             DataNoSimGroup.defineHistogram('jfex,fpga;h_DetectorMap_'+tower, title=item+" jFex module vs FPGA - "+tower+"; jFEX module; FPGA",
    #                                             type='TH2I',path=trigPath+item+'/unmatched/DataNoSim', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)
    #
    #             DataNoSimGroup.defineHistogram('LB;h_LBerror_'+tower, title=item+" LB for the mismatch - "+tower+"; LB; Counts",
    #                                             type='TH1I',path=trigPath+item+'/unmatched/DataNoSim', xbins=1,xmin=0,xmax=0,opt=['kCanRebin'])
    #

    return helper.result()


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m TrigT1CaloMonitoring.JfexSimMonitorAlgorithm',
                                   description="""Used to run jFEX Monitoring\n\n
                                   Example: python -m TrigT1CaloMonitoring.JfexSimMonitorAlgorithm --filesInput file.root.\n
                                   Overwrite inputs using standard athena opts --filesInput, evtMax etc. see athena --help""")
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
    args = parser.parse_args()


    flags = initConfigFlags()
    flags.Trigger.triggerConfig='DB'
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Output.HISTFileName = 'jFexSimData_Monitoring.root'
    
    flags.Exec.MaxEvents = args.evtMax
    flags.Exec.SkipEvents = args.skipEvents    
    
    flags.lock()

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False
    
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    
    JfexSimMonitorCfg = JfexSimMonitoringConfig(flags)
    cfg.merge(JfexSimMonitorCfg)
    
    from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
    JfexInputMonitorCfg = JfexInputMonitoringConfig(flags)
    cfg.merge(JfexInputMonitorCfg)    

    cfg.run()
