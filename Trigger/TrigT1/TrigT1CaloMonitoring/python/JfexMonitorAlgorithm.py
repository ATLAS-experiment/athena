#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JfexMonitoringConfig(flags):
    '''Function to configure LVL1 Jfex algorithm in the monitoring system.'''

    import math 
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JfexMonitorAlgorithm,'JfexMonAlg')
    JfexMonAlg = helper.alg

    # add any steering
    groupName = 'JfexMonitor' # the monitoring group name is also used for the package name
    JfexMonAlg.Grouphist = groupName
    
    # mainDir = 'L1Calo'
    trigPath = 'Developer/Jfex/'


    # define jfex histograms
    
    FPGA_names = ["U1","U2","U4","U3"]
    Modules_names = ["jFEX 0","jFEX 1","jFEX 2","jFEX 3","jFEX 4","jFEX 5"]
    
    from ROOT import TMath
    
    x_phi = []
    for i in range(67):
        phi = (-TMath.Pi()- TMath.Pi()/32) + TMath.Pi()/32*i 
        x_phi.append(phi)
    x_phi = sorted(x_phi)
    
    phi_bins = {
        'xbins': x_phi
    }
    
    eta_phi_bins = {
        'xbins': 100, 'xmin': -5, 'xmax': 5,
        'ybins': x_phi
    }

    
    ######  jJ  ######  
    helper.defineHistogram('jJ_jFexNumber;h_jJ_jFexNumber', title='jFex SRJet Module;Module number;Counts',
                           fillGroup=groupName,
                            type='TH1I', path=trigPath+'jJ/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jJ_fpgaNumber;h_jJ_fpgaNumber', title='jFex SRJet FPGA;FPGA number;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jJ/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jJ_jFexNumber,jJ_fpgaNumber;h_jJ_DetectorMap', title="jFex SRJet module vs FPGA; jFEX module; FPGA",
                            fillGroup=groupName,
                            type='TH2I',path=trigPath+'jJ/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jJ_Et;h_jJ_Et', title='jFex SRJet Transverse Energy;tobEt [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jJ/', xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jJ_Eta;h_jJ_Eta', title='jFex SRJet #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jJ/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jJ_Phi;h_jJ_Phi', title='jFex SRJet #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jJ/',**phi_bins)

    helper.defineHistogram('jJ_Eta,jJ_Phi;h_jJ_EtaPhiMap', title="jFex SRJet #eta vs #phi;#eta;#phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jJ/', **eta_phi_bins)

    helper.defineHistogram('jJ_GlobalEta;h_jJ_GlobalEta', title='jFex SRJet Global #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jJ/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jJ_GlobalPhi;h_jJ_GlobalPhi', title='jFex SRJet Global #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jJ/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jJ_GlobalEta,jJ_GlobalPhi;h_jJ_GlobalEtaPhiMap', title="jFex SRJet Global #eta vs #phi;(int) #eta;(int) #phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jJ/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)                            
                            
    ######  jLJ  ######  
    helper.defineHistogram('jLJ_jFexNumber;h_jLJ_jFexNumber', title='jFex LRJet Module;Module number;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jLJ/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jLJ_fpgaNumber;h_jLJ_fpgaNumber', title='jFex LRJet FPGA;FPGA number;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jLJ/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jLJ_jFexNumber,jLJ_fpgaNumber;h_jLJ_DetectorMap', title="jFex LRJet module vs FPGA; jFEX module; FPGA",
                            fillGroup=groupName,
                            type='TH2I',path=trigPath+'jLJ/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jLJ_Et;h_jLJ_Et', title='jFex LRJet Transverse Energy;tobEt [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jLJ/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jLJ_Eta;h_jLJ_Eta', title='jFex LRJet #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jLJ/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jLJ_Phi;h_jLJ_Phi', title='jFex LRJet #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jLJ/',**phi_bins)

    helper.defineHistogram('jLJ_Eta,jLJ_Phi;h_jLJ_EtaPhiMap', title="jFEX LRJet #eta vs #phi;#eta;#phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jLJ/', **eta_phi_bins)

    helper.defineHistogram('jLJ_GlobalEta;h_jLJ_GlobalEta', title='jFex LRJet Global #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jLJ/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jLJ_GlobalPhi;h_jLJ_GlobalPhi', title='jFex LRJet Global #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jLJ/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jLJ_GlobalEta,jLJ_GlobalPhi;h_jLJ_GlobalEtaPhiMap', title="jFex LRJet #eta vs #phi;(int) #eta; (int) #phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jLJ/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)                                  
    ######  jTau  ######   
    helper.defineHistogram('jTau_jFexNumber;h_jTau_jFexNumber', title='jFex Tau Module;Module number;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jTau/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jTau_fpgaNumber;h_jTau_fpgaNumber', title='jFex Tau FPGA;FPGA number;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTau/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jTau_jFexNumber,jTau_fpgaNumber;h_jTau_DetectorMap', title="jFex Tau module vs FPGA; jFEX module; FPGA",
                            fillGroup=groupName,
                            type='TH2I',path=trigPath+'jTau/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jTau_Et;h_jTau_Et', title='jFex Tau Transverse Energy;tobEt [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jTau/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jTau_Iso;h_jTau_Iso', title='jFex Tau Isolation;tobIso [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jTau/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jTau_Eta;h_jTau_Eta', title='jFex Tau #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTau/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jTau_Phi;h_jTau_Phi', title='jFex Tau #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTau/',**phi_bins)

    helper.defineHistogram('jTau_Eta,jTau_Phi;h_jTau_EtaPhiMap', title="jFex Tau #eta vs #phi;#eta;#phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jTau/', **eta_phi_bins)

    helper.defineHistogram('jTau_GlobalEta;h_jTau_GlobalEta', title='jFex Tau Global #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTau/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jTau_GlobalPhi;h_jTau_GlobalPhi', title='jFex Tau Global #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTau/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jTau_GlobalEta,jTau_GlobalPhi;h_jTau_GlobalEtaPhiMap', title="jFex Tau Global #eta vs #phi;(int) #eta; (int) #phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jTau/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)                               
    ######  jEM  ######  
    helper.defineHistogram('jEM_jFexNumber;h_jEM_jFexNumber', title='jFex EM Module;Module number;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jEM/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jEM_fpgaNumber;h_jEM_fpgaNumber', title='jFex EM FPGA;FPGA number;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jEM/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jEM_jFexNumber,jEM_fpgaNumber;h_jTau_DetectorMap', title="jFex EM module vs FPGA; jFEX module; FPGA",
                            fillGroup=groupName,
                            type='TH2I',path=trigPath+'jEM/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jEM_Et;h_jEM_Et', title='jFex EM Transverse Energy;tobEt [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jEM/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jEM_Eta;h_jEM_Eta', title='jFex EM #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jEM/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jEM_Phi;h_jEM_Phi', title='jFex EM #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jEM/',**phi_bins)

    em_labels = ['None','loose','medium','tight']
    helper.defineHistogram('jEM_Iso;h_jEM_Iso', title='jFex EM Isolation;tobIso;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_f1;h_jEM_f1', title='jFex EM Frac1;EM Frac1;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_f2;h_jEM_f2', title='jFex EM Frac2;EM Frac2;Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_Eta,jEM_Phi;h_jEM_EtaPhiMap', title="jFex EM #eta vs #phi;#eta;#phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jEM/',  **eta_phi_bins) 

    helper.defineHistogram('jEM_GlobalEta;h_jEM_GlobalEta', title='jFex EM Global #eta;#eta;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jEM/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jEM_GlobalPhi;h_jEM_GlobalPhi', title='jFex EM Global #phi;#phi;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jEM/',xbins=67,xmin=-1,xmax=65)
                            
    helper.defineHistogram('jEM_GlobalEta,jEM_GlobalPhi;h_jEM_GlobalEtaPhiMap', title="jFex EM Global #eta vs #phi;(int) #eta; (int) #phi",
                            fillGroup=groupName,
                            type='TH2F',path=trigPath+'jEM/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)                                
    ######  jXE  ######                          
    helper.defineHistogram('jXE_X;h_jXE_X', title='jFex MET X component (tobEx);tobEx [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jXE/',xbins=1000,xmin=-500,xmax=500)
    
    helper.defineHistogram('jXE_Y;h_jXE_Y', title='jFex MET Y component (tobEy);tobEy [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jXE/',xbins=1000,xmin=-500,xmax=500)

    helper.defineHistogram('jXE_MET;h_jXE_MET', title='jFex MET ;Total jXE [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jXE/',xbins=500,xmin=0,xmax=1000)

    helper.defineHistogram('jXE_phi;h_jXE_phi', title='jFex MET phi ;#phi=atan(Ey/Ex) values;Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jXE/',xbins=66,xmin=-math.pi,xmax=math.pi)

                            
    ######  jTE  ######                          
    helper.defineHistogram('jTE_low;h_jTE_low', title='jFex SumEt low #eta;tob Et_lower [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jTE/',xbins=700,xmin=0,xmax=700)
    
    helper.defineHistogram('jTE_high;h_jTE_high', title='jFex SumEt high #eta;tob Et_upper [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1I', path=trigPath+'jTE/',xbins=700,xmin=0,xmax=700)

    helper.defineHistogram('jTE_SumEt;h_jTE_SumEt', title='jFex SumEt total ;Total jTE [200 MeV Scale];Counts',
                            fillGroup=groupName,
                            type='TH1F', path=trigPath+'jTE/',xbins=1000,xmin=0,xmax=4000)


    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m TrigT1CaloMonitoring.JfexMonitorAlgorithm',
                                   description="""Used to run jFEX Monitoring\n\n
                                   Example: python -m TrigT1CaloMonitoring.JfexMonitorAlgorithm --filesInput file.root --skipEvents 0 --evtMax 100""")
                                   
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
    args = parser.parse_args()


    flags = initConfigFlags()
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Output.HISTFileName = 'jFexTOB_Monitoring.root'
    
    flags.Exec.MaxEvents = args.evtMax
    flags.Exec.SkipEvents = args.skipEvents    

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JfexMonitorCfg = JfexMonitoringConfig(flags)
    cfg.merge(JfexMonitorCfg)
    
    cfg.run()
