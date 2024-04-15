#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#


def MistimedStreamMonitorConfig(flags):
    '''Function to configure LVL1 Mistimed Stream algorithm in the monitoring system.'''
    import math 

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'MistimedStreamMonitorCfg')

    # get any algorithms
    MistimedMonAlg = helper.addAlgorithm(CompFactory.MistimedStreamMonitorAlgorithm,'MistimedStreamMonitorAlg')
    
    # import tools
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    helper.result().merge(DetDescrCnvSvcCfg(flags))

    from TrigT1CaloCondSvc.L1CaloCondConfig import L1CaloCondAlgCfg 
    helper.result().merge(L1CaloCondAlgCfg(flags,Physics=True, Calib1=False, Calib2=False)) 
    
    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    helper.result().getPrimaryAndMerge(getxAODConfigSvc(flags))

    # add any steering
    groupName = 'MistimedStreamMonitor' # the monitoring group name is also used for the package name
    MistimedMonAlg.PackageName = groupName

    # Histogram paths
    mainDir = 'L1Calo'
    trigPath = 'MistimedStream'

    # Trigger tower plots: eta-phi granularity
    etabins = [-4.9,-4.475,-4.050,-3.625,-3.2,-3.1,-2.9,
                     -2.7,-2.5,-2.4,-2.3,-2.2,-2.1,-2.0,-1.9,
                     -1.8,-1.7,-1.6,-1.5,-1.4,-1.3,-1.2,-1.1,
                     -1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
                     -0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,
                     0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,
                     1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.7,2.9,
                     3.1,3.2,3.625,4.050,4.475,4.9]


                            

    phibins = 64
    phimin = 0

    
    NumberOfGlobalErrors=11
    globalStatus_xlabels = [
        "All",
        "unsuitable readout",
        "HLT_mistimemonj400",
        "L1_Trigger",
        "<= 4 bad peak TT",
        "<= 4 bad central TT",
        "<= 4 bad late TT",
        ">= 2 late TT",
        "<= 3 in-time",
        ">= 1 significant TT in EM layer",
        "spatial overlap"
    ]


    NumberofEvents = MistimedMonAlg.MaxEvents

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(MistimedMonAlg, groupName , mainDir)
    

    
    myGroup.defineHistogram('cutFlowX;1d_cutFlow_mistimedStreamAna',title='Total events selected by the MistimedStream analysis for this run',
                            type='TH1F',
                            path=trigPath,
                            xbins=NumberOfGlobalErrors,xmin=0.,xmax=NumberOfGlobalErrors,xlabels=globalStatus_xlabels,
                            opt='kAlwaysCreate')
    
    groupMapsEvents         = helper.addGroup(MistimedMonAlg, 'Event_', mainDir)
    groupMapsEvents.defineHistogram('eventMonitor_legacy,lbMonitor;1d_selectedEvents_mistimedStreamAna_legacy', title='Events of interest - Lumi Block;Events of interest;Lumi Block',type='TH2I', xbins=NumberofEvents, ybins=NumberofEvents, path=trigPath)
    groupMapsEvents.defineHistogram('eventMonitor_phaseI,lbMonitor;1d_selectedEvents_mistimedStreamAna_phaseI', title='Events of interest - Lumi Block;Events of interest;Lumi Block',type='TH2I', xbins=NumberofEvents, ybins=NumberofEvents, path=trigPath)
    
    groupMapsEvents_all         = helper.addGroup(MistimedMonAlg, 'Event_all_', mainDir)
    groupMapsEvents_all.defineHistogram('eventMonitor_all_legacy,lbMonitor_all;1d_selectedEvents_mistimedStreamAna_notSaved_legacy', title='Events of interest - Lumi Block (not saved);Events of interest;Lumi Block',type='TH2I', path=trigPath)
    groupMapsEvents_all.defineHistogram('eventMonitor_all_phaseI,lbMonitor_all;1d_selectedEvents_mistimedStreamAna_notSaved_phaseI', title='Events of interest - Lumi Block (not saved);Events of interest;Lumi Block',type='TH2I', path=trigPath)

    groupMapsEfexin = helper.addGroup(MistimedMonAlg,  'Efex_maxTOB_in', mainDir)
    groupMapsEfexin.defineHistogram('TOBEta_max, TOBPhi_max;Efex_2d_etaPhi_in_mistimedStreamAna', title='#eta - #phi Map of max Efex TOBs (in-time) ;#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TH2D', path=trigPath)
    groupMapsEfexin.defineHistogram('TOBTransverseEnergy;Efex_TOBenergy_in_mistimedStreamAna', title='Energy of Efex TOBs (in-time);TOB E_T [GeV];Entries', xbins=90,xmin=0.0,xmax=450.0, type='TH1D', path=trigPath)
    
    groupMapsEfexout = helper.addGroup(MistimedMonAlg,  'Efex_maxTOB_out', mainDir)
    groupMapsEfexout.defineHistogram('TOBEta_max, TOBPhi_max;Efex_2d_etaPhi_out_mistimedStreamAna', title='#eta - #phi Map of max Efex TOBs  (out-of-time) ;#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TH2D', path=trigPath)
    groupMapsEfexout.defineHistogram('TOBTransverseEnergy;Efex_TOBenergy_out_mistimedStreamAna', title='Energy of Efex TOBs (out-of-time);TOB E_T [GeV];Entries', xbins=90,xmin=0.0,xmax=450.0, type='TH1D', path=trigPath)

    groupMapsJfex = helper.addGroup(MistimedMonAlg,  'Jfex_maxTOB', mainDir)
    groupMapsJfex.defineHistogram('TOBEta_max, TOBPhi_max;Jfex_2d_etaPhi_mistimedStreamAna', title='#eta - #phi Map of max Jfex TOBs;#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TH2D', path=trigPath)
    groupMapsJfex.defineHistogram('jFexEt;Jfex_TOBenergy_mistimedStreamAna', title='Energy of Jfex TOBs;TOB E_T [GeV];Entries', xbins=90,xmin=0.0,xmax=450.0, type='TH1D', path=trigPath)

    groupMapsGfex = helper.addGroup(MistimedMonAlg,  'Gfex_maxTOB', mainDir)
    groupMapsGfex.defineHistogram('TOBEta_max, TOBPhi_max;Gfex_2d_etaPhi_mistimedStreamAna', title='#eta - #phi Map of max Gfex TOBs;#eta;#phi'  , xbins=50,xmin=-5.0,xmax=5.0,ybins=32,ymin=0.,ymax=2*math.pi, type='TH2D', path=trigPath)
    groupMapsGfex.defineHistogram('gFexEt;Gfex_TOBenergy_mistimedStreamAna', title='Energy of Gfex TOBs;TOB E_T [GeV];Entries', xbins=90,xmin=0.0,xmax=450.0, type='TH1D', path=trigPath)

    # add monitoring algorithm to group, with group name and main directory
    histPath = trigPath+'/EventofInterest'

    for i in range(0, NumberofEvents):

        # LUT per BCN (Both EM & HAD together)
        groupMapsEM = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_TT_EM', mainDir)
        groupMapsEM.defineHistogram('etaTT_2D,phiTT_2D,pulseCat;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_classification_mistimedStreamAna', title='#eta - #phi Map of TT classification, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsEM.defineHistogram('etaTT_2D,phiTT_2D,bcidWord;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_pseBits_mistimedStreamAna', title='#eta - #phi Map of TT PSE Bits, EM layer: event  of interest no. '+str(i)+';#eta;#phi', xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,type='TProfile2D', path=histPath)
        
        groupMapsHAD = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_TT_HAD', mainDir)
        groupMapsHAD.defineHistogram('etaTT_2D,phiTT_2D,pulseCat;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_classification_mistimedStreamAna', title='#eta - #phi Map of TT classification, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,  type='TProfile2D', path=histPath)
        groupMapsHAD.defineHistogram('etaTT_2D,phiTT_2D,bcidWord;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_pseBits_mistimedStreamAna', title='#eta - #phi Map of TT PSE Bits, HAD layer: event of interest no. '+str(i)+';#eta;#phi', xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        
        groupMapsEM0 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM0', mainDir)
        groupMapsEM0.defineHistogram('etalut,philut,emLUT0;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut0_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 0 = BCID-1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsEM1 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM1', mainDir)
        groupMapsEM1.defineHistogram('etalut,philut,emLUT1;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut1_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 1 = BCID, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsEM2 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM2', mainDir)
        groupMapsEM2.defineHistogram('etalut,philut,emLUT2;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut2_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 2 = BCID+1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,  type='TProfile2D', path=histPath)

        groupMapsHAD0 = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD0', mainDir)
        groupMapsHAD0.defineHistogram('etalut,philut,hadLUT0;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut0_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 0 = BCID-1, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsHAD1 = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD1', mainDir)
        groupMapsHAD1.defineHistogram('etalut,philut,hadLUT1;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut1_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 1 = BCID, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsHAD2 = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD2', mainDir)
        groupMapsHAD2.defineHistogram('etalut,philut,hadLUT2;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut2_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 2 = BCID+1, HAD layer: eventof interest no. '+str(i)+';#eta;#phi'   , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)

        groupMapsEfex0 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_Efex0', mainDir)
        groupMapsEfex0.defineHistogram('TOBEta, TOBPhi, TOBTransverseEnergy;EventofInterest_'+str(i)+'_Efex0_2d_etaPhi_out_mistimedStreamAna', title='#eta - #phi Map of Efex TT in timeslice 0 = BCID-1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        groupMapsEfex1 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_Efex1', mainDir)
        groupMapsEfex1.defineHistogram('TOBEta, TOBPhi, TOBTransverseEnergy;EventofInterest_'+str(i)+'_Efex1_2d_etaPhi_mistimedStreamAna', title='#eta - #phi Map of Efex TT in timeslice 1 = BCID, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        groupMapsEfex2 = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_Efex2', mainDir)
        groupMapsEfex2.defineHistogram('TOBEta, TOBPhi, TOBTransverseEnergy;EventofInterest_'+str(i)+'_Efex2_2d_etaPhi_out_mistimedStreamAna', title='#eta - #phi Map of Efex TT in timeslice 2 = BCID+1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        
        groupMapsJfexEmulated = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_JfexEmulated', mainDir)
        groupMapsJfexEmulated.defineHistogram('jFexEta, jFexPhi, jFexEt;EventofInterest_'+str(i)+'_Jfex_2d_etaPhi_emulatedTowers_mistimedStreamAna', title='#eta - #phi Map of Jfex Emulated Towers in time, events of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        
        groupMapsJfexSRJet = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_JfexSRJet', mainDir)
        groupMapsJfexSRJet.defineHistogram('jFexEta, jFexPhi, jFexEt;EventofInterest_'+str(i)+'_Jfex_2d_etaPhi_SRJet_mistimedStreamAna', title='#eta - #phi Map of Jfex SRJets in time, events of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        groupMapsJfexTau = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_JfexTau', mainDir)
        groupMapsJfexTau.defineHistogram('jFexEta, jFexPhi, jFexEt;EventofInterest_'+str(i)+'_Jfex_2d_etaPhi_Tau_mistimedStreamAna', title='#eta - #phi Map of Jfex Tau in time, event of interest no. '+str(i)+';#eta;#phi'  , xbins=100,xmin=-5.0,xmax=5.0,ybins=64,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        
        groupMapsgFexSRJet = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_GfexSRJet', mainDir)
        groupMapsgFexSRJet.defineHistogram('gFexEta, gFexPhi, gFexEt;EventofInterest_'+str(i)+'_Gfex_2d_etaPhi_SRJet_mistimedStreamAna', title='#eta - #phi Map of Gfex Jets in time, events of interest no. '+str(i)+';#eta;#phi'  , xbins=50,xmin=-5.0,xmax=5.0,ybins=32,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        groupMapsgFexLRJet = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_GfexLRJet', mainDir)
        groupMapsgFexLRJet.defineHistogram('gFexEta, gFexPhi, gFexEt;EventofInterest_'+str(i)+'_Gfex_2d_etaPhi_LRJet_mistimedStreamAna', title='#eta - #phi Map of Gfex Jets in time, events of interest no. '+str(i)+';#eta;#phi'  , xbins=50,xmin=-5.0,xmax=5.0,ybins=32,ymin=0.,ymax=2*math.pi, type='TProfile2D', path=histPath)
        
    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
   
    import sys 

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = -1
    flags.IOVDb.GlobalTag = 'CONDBR2-HLTP-2023-01'
    flags.Input.Files = ["/eos/atlas/atlastier0/rucio/data22_13p6TeV/express_express/00423433/data22_13p6TeV.00423433.express_express.recon.ESD.x653/data22_13p6TeV.00423433.express_express.recon.ESD.x653._lb0015._SFO-ALL._0001.2"]
    

    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    

    MistimedStreamMonitorCfg = MistimedStreamMonitorConfig(flags)
    acc.merge(MistimedStreamMonitorCfg)

    MistimedStreamMonitorCfg.getEventAlgo('MistimedStreamMonitorAlg').OutputLevel = 2 # 1/2 INFO/DEBUG
    acc.printConfig(withDetails=True, summariseProps = True)

    sys.exit(acc.run().isFailure())
