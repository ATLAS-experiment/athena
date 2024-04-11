#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JfexInputMonitoringConfig(flags):
    '''Function to configure LVL1 JfexInput algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(flags,CompFactory.JfexInputMonitorAlgorithm,'JfexInputMonAlg')
    
    
    #Algorithms needed for the monitoring
    if flags.Input.Format==Format.BS:
        
        #Decorator for the DataTowers
        from L1CaloFEXAlgos.L1CaloFEXAlgosConfig import L1CalojFEXDecoratorCfg
        result.merge(L1CalojFEXDecoratorCfg(flags,ExtraInfo = False))
        
        #jfex emulated input: EmulatedTowers
        from L1CaloFEXAlgos.FexEmulatedTowersConfig import jFexEmulatedTowersCfg
        result.merge(jFexEmulatedTowersCfg(flags))    
    

    # get any algorithms
    JfexInputMonAlg = helper.alg

    # add any steering
    groupName = 'JfexInputMonitor' # the monitoring group name is also used for the package name
    JfexInputMonAlg.Grouphist = groupName

    #mainDir = 'L1Calo'
    trigPath = 'Developer/JfexInput/'
    Calosource_names = ["Barrel","Tile","EMEC","HEC","FCAL1","FCAL2","FCAL3"]
    FPGA_names = ["U1","U2","U4","U3"]
    
    from math import pi
    
    x_phi = []
    for i in range(67):
        phi = (-pi- pi/32) + pi/32*i 
        x_phi.append(phi)
    x_phi = sorted(x_phi)
    
    phi_bins = {
        'xbins': x_phi
    }
    
    eta_phi_bins = {
        'xbins': 100, 'xmin': -5, 'xmax': 5,
        'ybins': x_phi
    }
        
    helper.defineHistogram('genLocation,genType;h_jFEX_Errors', path="Developer/JfexInput", type='TH2I',
                                   fillGroup=groupName+"Gen",
                            title='jFEX generic monitoring for shifters;Location;Type',
                            xbins=4, xmin=0, xmax=4, xlabels=["Sim_DataTowers","Sim_EmulatedTowers","Input_Mismatch","Input_Invalids"],
                            ybins=4, ymin=0, ymax=4, ylabels=["TOB", "global TOB", "EM layer", "HAD layer" ],
                            opt=['kCanRebin'])
                            
    #JfexInputMonAlg.jFEXMonTool = myGenericGroup

    helper.defineHistogram('EventType,Decision;h_summary',title='JfexInput Monitoring summary;Event Type;Decision',
                            fillGroup=groupName,
                              type='TH2I',path=trigPath,
                              xbins=1,xmin=0,xmax=1,xlabels=["Normal"],
                              ybins=1,ymin=0,ymax=1,ylabels=["AllOk"],
                              opt=['kCanRebin'],merge="merge")

    
    decorGroup = groupName+"_decorated"
    decorAllGroup = groupName+"_decorated_all"
    emulatedGroup = groupName+"_emulated"

    for i in range(7):
        fillGroup = groupName+"_details_"+str(i)   
        
        if(i == 1):
            helper.defineHistogram('DataEt_Tile,EmulatedEt_Tile;h_SumSCell_vs_Data_'+Calosource_names[i], title='Data Et vs Tile Et ('+ Calosource_names[i]+'); Data Et [GeV]; Tile Et [GeV]',
                                   fillGroup=fillGroup,
                            type='TH2F',path=trigPath+"expert/", xbins=200,xmin=0,xmax=100,ybins=200,ymin=0,ymax=100)
                            
        else:    
            helper.defineHistogram('DataEt,EmulatedEt;h_SumSCell_vs_Data_'+Calosource_names[i], title='Data vs SCell Sum Et ('+ Calosource_names[i]+'); Data Et [MeV]; SCell Sum Et [MeV]',
                                   fillGroup=fillGroup,
                            type='TH2F',path=trigPath+"expert/", xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
        

    helper.defineHistogram('region,type;h_DataErrors', title='jFEX Data mismatches per region; Region; Type',
                            type='TH2F',path=trigPath, xbins=7,xmin=0,xmax=7,ybins=2,ymin=0,ymax=2,xlabels=Calosource_names,ylabels=["Invalid codes","Data mismatch"])

    helper.defineHistogram('TowerEtaInvalid,TowerPhiInvalid;h_2Dmap_InvalidCodes', title='jFex DataTower Invalid Et codes (4095); #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)

    helper.defineHistogram('TowerEtaEmpty,TowerPhiEmpty;h_2Dmap_EmptyCodes', title='jFex DataTower Empty Et codes (0); #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    helper.defineHistogram('TowerEta,TowerPhi;h_2Dmap_MismatchedEts', title='jFex DataTower mismatches (no invalid codes); #eta; #phi',
                           fillGroup=decorGroup,
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    helper.defineHistogram('DataEt,SCellSum;h_SumSCell_vs_Data_Mismatched', title='Data vs SCell Sum Et (unmatching); Data Et [MeV]; SCell Sum Et [MeV]',
                               fillGroup=decorGroup,
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
    helper.defineHistogram('DataEt,SCellSum;h_SumSCell_vs_Data_all', title='Data vs SCell Sum Et (all Towers); Data Et [MeV]; SCell Sum Et [MeV]',
                                  fillGroup=decorAllGroup,
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
    helper.defineHistogram('DataEt,frac_SCellSum;h_frac_SumSCell_vs_Data', title='Data vs (Et_{SCell}-Et_{Data})/Et_{Data} (no invalid codes); Data Et [MeV]; (Et_{SCell}-Et_{Data})/Et_{Data}',
                               fillGroup=decorGroup,
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=100,ymin=-20,ymax=20)
                            
    helper.defineHistogram('TowerEta,TowerPhi;h_2Dmap_DataVsEmulated', title='Input Data vs Emulated data; #eta; #phi',
                                  fillGroup=emulatedGroup,
                            type='TH2F',path=trigPath, **eta_phi_bins)
    helper.defineHistogram('TowerEtaDeco,TowerPhiDeco;h_2Dmap_EmulatedVsDecorated', title='Emulated vs Decorated data; #eta; #phi',
                                  fillGroup=emulatedGroup,
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    # histograms of jFex tower variables (content of the container)
    helper.defineHistogram('NJfexTowers;h_nJfexTowers', title='Number of jFex towers',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=2000,xmin=0,xmax=20000)

    helper.defineHistogram('TowerEta;h_TowerEta', title='jFex Tower Eta',
                            fillGroup=groupName,type='TH1F', path=trigPath+"Content/", xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('TowerPhi;h_TowerPhi', title='jFex Tower Phi',
                            fillGroup=groupName,type='TH1F', path=trigPath+"Content/", **phi_bins)

    helper.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='jFex Tower Eta vs Phi',
                            fillGroup=groupName,type='TH2F',path=trigPath+"Content/", **eta_phi_bins)

    helper.defineHistogram('TowerGlobalEta;h_TowerGlobalEta', title='jFex Tower Global Eta',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('TowerGlobalPhi;h_TowerGlobalPhi', title='jFex Tower Global Phi',
                            fillGroup=groupName,type='TH1F', path=trigPath+"Content/", xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('TowerGlobalEta,TowerGlobalPhi;h_TowerGlobalEtaPhiMap', title='jFex Tower Global Eta vs Phi',
                            fillGroup=groupName,type='TH2I',path=trigPath+"Content/", xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)

    helper.defineHistogram('TowerModule;h_TowerModule', title='jFex Tower Module Number',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=6,xmin=0,xmax=6)
  
    helper.defineHistogram('TowerFpga;h_TowerFpga', title='jFex Tower FPGA Number',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4,xmin=0,xmax=4,xlabels=FPGA_names)

    helper.defineHistogram('TowerChannel;h_TowerChannel', title='jFex Tower Channel Number',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=60,xmin=0,xmax=60)

    helper.defineHistogram('TowerDataID;h_TowerDataID', title='jFex Tower Data ID',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=16,xmin=0,xmax=16)

    helper.defineHistogram('TowerSimulationID;h_TowerSimulationID', title='jFex Tower Simulation ID',
                            fillGroup=groupName,type='TH1F', path=trigPath+"Content/", xbins=1000,xmin=0,xmax=1500000.0)

    helper.defineHistogram('TowerCalosource;h_TowerCalosource', title='jFex Tower Calo Source',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=7,xmin=0,xmax=7,xlabels=Calosource_names)

    helper.defineHistogram('TowerEtcount_barrel;h_TowerEtcount_barrel', title='jFex Tower Et Barrel',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_tile;h_TowerEtcount_tile', title='jFex Tower Et Tile',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_emec;h_TowerEtcount_emec', title='jFex Tower Et EMEC',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_hec;h_TowerEtcount_hec', title='jFex Tower Et HEC',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_fcal1;h_TowerEtcount_fcal1', title='jFex Tower Et FCAL1',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_fcal2;h_TowerEtcount_fcal2', title='jFex Tower Et FCAL2',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerEtcount_fcal3;h_TowerEtcount_fcal3', title='jFex Tower Et FCAL3',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    helper.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='jFex Tower Saturation FLag',
                            fillGroup=groupName,type='TH1I', path=trigPath+"Content/", xbins=2,xmin=0,xmax=2)

  
    acc = helper.result()
    result.merge(acc)
    return result    

if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m TrigT1CaloMonitoring.JfexInputMonitoringAlgorithm',
                                   description="""Used to run jFEX Monitoring\n\n
                                   Example: python -m TrigT1CaloMonitoring.JfexInputMonitoringAlgorithm --filesInput file.root.\n
                                   Overwrite inputs using standard athena opts --filesInput, evtMax etc. see athena --help""")
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
    args = parser.parse_args()


    flags = initConfigFlags()
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Output.HISTFileName = 'jFexInputData_Monitoring.root'
    
    flags.Exec.MaxEvents = args.evtMax
    flags.Exec.SkipEvents = args.skipEvents    

    flags.lock()
    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JfexInputMonitorCfg = JfexInputMonitoringConfig(flags)
    cfg.merge(JfexInputMonitorCfg)
    cfg.run()
