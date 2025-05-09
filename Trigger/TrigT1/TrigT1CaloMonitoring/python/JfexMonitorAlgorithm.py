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
    mapGroupName = 'jFEXMaps'
    mapHighPtGroupName = 'jFEXMapsHighPt'
    JfexMonAlg.Grouphist = groupName

    # mainDir = 'L1Calo'
    developerPath = 'Developer/Jfex/'
    expertPath = 'Expert/Outputs/'


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

    eta_bins_central = [-2.5 + 0.1*i for i in range(51)]

    # C-side irregular region
    eta_bins = [-4.8 + 0.1*i for i in range(16)]
    eta_bins_jets = [-4.8 + 0.1*i for i in range(16)]
    # C-side coarse region
    eta_bins += [-3.2,-3.1,-2.9,-2.7]
    eta_bins_jets += [-3.2,-3.15,-3.1,-2.9,-2.7]
    # central region
    eta_bins += eta_bins_central
    eta_bins_jets += eta_bins_central
    # A-side coarse region
    eta_bins += [2.7,2.9,3.1,3.2]
    eta_bins_jets += [2.7,2.9,3.1,3.15,3.2]
    # A-side irregular region
    eta_bins += [3.3 + 0.1*i for i in range(16)]
    eta_bins_jets += [3.3 + 0.1*i for i in range(16)]

    JfexMonAlg.jJEtaBins = eta_bins_jets
    JfexMonAlg.jTauEtaBins = eta_bins_central
    JfexMonAlg.jEMEtaBins = eta_bins

    eta_phi_bins = {
        'xbins': eta_bins,
        'ybins': 64, 'ymin': -TMath.Pi(), 'ymax': TMath.Pi()
    }

    eta_phi_bins_jets = {
        'xbins': eta_bins_jets,
        'ybins': 64, 'ymin': -TMath.Pi(), 'ymax': TMath.Pi()
    }

    eta_phi_bins_central = {
        'xbins': eta_bins_central,
        'ybins': 64, 'ymin': -TMath.Pi(), 'ymax': TMath.Pi()
    }

    n_bins_total = (len(eta_bins) - 1) * 64
    n_bins_total_jets = (len(eta_bins_jets) - 1) * 64
    n_bins_eta_2p5 = (len(eta_bins_central) - 1) * 64
    n_bins_eta_2p3 = 46 * 64
    # number of bins above |eta| = 3.2 that will never be filled with TOBs
    # due to the low granularity and irregular structure of the FCAL
    # (given by the length of the list of empty bin in JfexMapForwardEmptyBins.h - nope this fills overflow bins too, and sometimes double-fills those)
    n_empty_bins_fcal = 1708
    # number of bins in 3.15 < |eta| < 3.2 where no jets are produced
    n_empty_bins_fcal_overlap = 96

    # all bins should be filled with jets, except the always empty ones in FCAL
    n_expected_filled_bins_jJ = n_bins_total_jets - n_empty_bins_fcal - n_empty_bins_fcal_overlap
    # taus are only produced for |eta| < 2.5
    n_expected_filled_bins_jTAU = n_bins_eta_2p5
    # forward electrons are produced for |eta| > 2.3
    n_expected_filled_bins_jEM = n_bins_total - n_empty_bins_fcal - n_bins_eta_2p3

    helper.defineDQAlgorithm("Jfex_etaPhiMapFilled",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_LessThanAbs_Threshold","BinThreshold":"1"}, # counts bins with |value|<1
                             thresholdConfig={"NBins":[0,n_expected_filled_bins_jJ]}, # 0 bins expected empty, warning above that, error if entirely empty (save for known empties)
                             )
    helper.defineDQAlgorithm("Jfex_etaPhiMapFilled_EM",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_LessThanAbs_Threshold","BinThreshold":"1"}, # counts bins with |value|<1
                             thresholdConfig={"NBins":[0,n_expected_filled_bins_jEM]}, # 0 bins expected empty, warning above that, error if entirely empty (save for known empties)
                             )
    helper.defineDQAlgorithm("Jfex_etaPhiMapFilled_TAU",
                             hanConfig={"libname":"libdqm_summaries.so","name":"Bins_LessThanAbs_Threshold","BinThreshold":"1"}, # counts bins with |value|<1
                             thresholdConfig={"NBins":[0,n_expected_filled_bins_jTAU]}, # 0 bins expected empty, warning above that, error if entirely empty (save for known empties)
                             )

    ###### up to date histograms used in web display ######

    ###### jJ ######

    helper.defineHistogram('jJ_Eta,jJ_Phi;h_jJ_EtaPhiMap', title="jFex SRJet #eta vs #phi;#eta;#phi",
                           fillGroup=mapGroupName,
                           type='TH2I',path=expertPath+'jJ/',
                           hanConfig={
                               "algorithm": "Jfex_etaPhiMapFilled",
                               "description": "Bins with negative number of entries (in the FCAL) signify that no TOBs can be produced at that position. In the region 3.1 < |eta| < 3.2 seeded in EMEC and FCAL1 are displayed next to each other, though in reality have similar eta coordinates. Inspect for hot/cold spots - check help for list of known hot/coldspots",
                               "display":"SetPalette(55),Draw=COL1Z"
                           },
                           weight="weight",opt=['kAlwaysCreate'],
                           **eta_phi_bins_jets)

    helper.defineHistogram('jJ_Eta,jJ_Phi;h_jJ_EtaPhiMap_Pt20', title="jFex SRJet #geq 20 GeV #eta vs #phi;#eta;#phi",
                           fillGroup=mapHighPtGroupName,
                           type='TH2I',path=expertPath+'jJ/',
                           hanConfig={
                               "algorithm": "Jfex_etaPhiMapFilled",
                               "description": "Bins with negative number of entries (in the FCAL) signify that no TOBs can be produced at that position. In the region 3.1 < |eta| < 3.2 seeded in EMEC and FCAL1 are displayed next to each other, though in reality have similar eta coordinates. Inspect for hot/cold spots - check help for list of known hot/coldspots",
                               "display":"SetPalette(55),Draw=COL1Z"
                           },
                           weight="weight",opt=['kAlwaysCreate'],
                           **eta_phi_bins_jets)

    helper.defineHistogram('LBN,jJ_binNumber;h_jJ_posVsLBN', title=f"jFex SRJet count vs LB;LB;{len(eta_bins_jets)-1}(x-1)+y",
                           fillGroup=mapGroupName,
                           type="TH2I", path=expertPath+'jJ/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jJ_EtaPhiMap'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=(len(eta_bins_jets)-1)*64,ymin=0.5,ymax=(len(eta_bins_jets)-1)*64+0.5,
                           opt=['kAddBinsDynamically'])

    helper.defineHistogram('LBN,jJ_binNumber;h_jJ_posVsLBN_Pt20', title=f"jFex SRJet #geq 20 GeV count vs LB;LB;{len(eta_bins_jets)-1}(x-1)+y",
                           fillGroup=mapHighPtGroupName,
                           type="TH2I", path=expertPath+'jJ/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jJ_EtaPhiMap_Pt20'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=(len(eta_bins_jets)-1)*64,ymin=0.5,ymax=(len(eta_bins_jets)-1)*64+0.5,
                           opt=['kAddBinsDynamically'])

    ###### jTAU ######

    helper.defineHistogram('jTau_Eta,jTau_Phi;h_jTAU_EtaPhiMap', title="jFex Tau #eta vs #phi;#eta;#phi",
                           fillGroup=mapGroupName,
                           hanConfig={
                               "algorithm": "Jfex_etaPhiMapFilled_TAU",
                               "description": "Inspect for hot/cold spots - check help for list of known hot/coldspots",
                               "display": "SetPalette(87)"
                           },
                           type='TH2I',path=expertPath+'jTAU/',opt=['kAlwaysCreate'], **eta_phi_bins_central)

    helper.defineHistogram('jTau_Eta,jTau_Phi;h_jTAU_EtaPhiMap_Pt10', title="jFex Tau #geq 10 GeV #eta vs #phi;#eta;#phi",
                           fillGroup=mapHighPtGroupName,
                           hanConfig={
                               "algorithm": "Jfex_etaPhiMapFilled_TAU",
                               "description": "Inspect for hot/cold spots - check help for list of known hot/coldspots",
                               "display": "SetPalette(87)"
                           },
                           type='TH2I',path=expertPath+'jTAU/',opt=['kAlwaysCreate'], **eta_phi_bins_central)

    helper.defineHistogram('LBN,jTau_binNumber;h_jTAU_posVsLBN', title="jFex Tau count vs LB;LB;50(y-1)+x",
                           fillGroup=mapGroupName,
                           type="TH2I", path=expertPath+'jTAU/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jTAU_EtaPhiMap'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=50*64,ymin=0.5,ymax=50*64+0.5,
                           opt=['kAddBinsDynamically'])

    helper.defineHistogram('LBN,jTau_binNumber;h_jTAU_posVsLBN_Pt10', title="jFex Tau #geq 10 GeV count vs LB;LB;50(y-1)+x",
                           fillGroup=mapHighPtGroupName,
                           type="TH2I", path=expertPath+'jTAU/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jTAU_EtaPhiMap_Pt10'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=50*64,ymin=0.5,ymax=50*64+0.5,
                           opt=['kAddBinsDynamically'])

    ###### jEM ######

    helper.defineHistogram('jEM_Eta,jEM_Phi;h_jEM_EtaPhiMap', title="jFex EM #eta vs #phi;#eta;#phi",
                           fillGroup=mapGroupName,
                           hanConfig={
                               "algorithm":"Jfex_etaPhiMapFilled_EM",
                               "description": "Bins with negative number of entries (in the FCAL and central region) signify that no TOBs can be produced at that position. Inspect for hot/cold spots, note that there are no jEM TOBs for |eta| < 2.3 - check help for list of known hot/coldspots",
                               "display":"SetPalette(87),Draw=COL1Z"
                           },
                           weight="weight",
                           type='TH2I',path=expertPath+'jEM/',opt=['kAlwaysCreate'],  **eta_phi_bins)

    helper.defineHistogram('jEM_Eta,jEM_Phi;h_jEM_EtaPhiMap_Pt10', title="jFex EM #geq 10 GeV #eta vs #phi;#eta;#phi",
                           fillGroup=mapHighPtGroupName,
                           hanConfig={
                               "algorithm":"Jfex_etaPhiMapFilled_EM",
                               "description": "Bins with negative number of entries (in the FCAL and central region) signify that no TOBs can be produced at that position. Inspect for hot/cold spots, note that there are no jEM TOBs for |eta| < 2.3 - check help for list of known hot/coldspots",
                               "display":"SetPalette(87),Draw=COL1Z"
                           },
                           weight="weight",
                           type='TH2I',path=expertPath+'jEM/',opt=['kAlwaysCreate'],  **eta_phi_bins)

    helper.defineHistogram('LBN,jEM_binNumber;h_jEM_posVsLBN', title=f"jFex EM count vs LB;LB;{len(eta_bins)-1}(y-1)+x",
                           fillGroup=mapGroupName,
                           type="TH2I", path=expertPath+'jEM/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jEM_EtaPhiMap'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=(len(eta_bins)-1)*64,ymin=0.5,ymax=(len(eta_bins)-1)*64+0.5,
                           opt=['kAddBinsDynamically'])

    helper.defineHistogram('LBN,jEM_binNumber;h_jEM_posVsLBN_Pt10', title=f"jFex EM #geq 10 GeV count vs LB;LB;{len(eta_bins)-1}(y-1)+x",
                           fillGroup=mapHighPtGroupName,
                           type="TH2I", path=expertPath+'jEM/detail/',
                           hanConfig={
                           "description": "Timeseries of TOB counts at each location, y-axis relates to x and y bin numbers from <a href='../h_jEM_EtaPhiMap_Pt10'>eta-phi map</a>. Use projection X1 for 1D plot."
                           },
                           xbins=1, xmin=0, xmax=10,
                           ybins=(len(eta_bins)-1)*64,ymin=0.5,ymax=(len(eta_bins)-1)*64+0.5,
                           opt=['kAddBinsDynamically'])


    ###### outdated histograms not currently used ######

    ######  jJ  ######
    helper.defineHistogram('jJ_jFexNumber;h_jJ_jFexNumber', title='jFex SRJet Module;Module number;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jJ/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jJ_fpgaNumber;h_jJ_fpgaNumber', title='jFex SRJet FPGA;FPGA number;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jJ/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jJ_jFexNumber,jJ_fpgaNumber;h_jJ_DetectorMap', title="jFex SRJet module vs FPGA; jFEX module; FPGA",
                           fillGroup=groupName,
                           type='TH2I',path=developerPath+'jJ/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jJ_Et;h_jJ_Et', title='jFex SRJet Transverse Energy;tobEt [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jJ/', xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jJ_Eta;h_jJ_Eta', title='jFex SRJet #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jJ/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jJ_Phi;h_jJ_Phi', title='jFex SRJet #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jJ/',**phi_bins)

    helper.defineHistogram('jJ_GlobalEta;h_jJ_GlobalEta', title='jFex SRJet Global #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jJ/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jJ_GlobalPhi;h_jJ_GlobalPhi', title='jFex SRJet Global #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jJ/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jJ_GlobalEta,jJ_GlobalPhi;h_jJ_GlobalEtaPhiMap', title="jFex SRJet Global #eta vs #phi;(int) #eta;(int) #phi",
                           fillGroup=groupName,
                           type='TH2F',path=developerPath+'jJ/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)

    ######  jLJ  ######
    helper.defineHistogram('jLJ_jFexNumber;h_jLJ_jFexNumber', title='jFex LRJet Module;Module number;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jLJ/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jLJ_fpgaNumber;h_jLJ_fpgaNumber', title='jFex LRJet FPGA;FPGA number;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jLJ/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jLJ_jFexNumber,jLJ_fpgaNumber;h_jLJ_DetectorMap', title="jFex LRJet module vs FPGA; jFEX module; FPGA",
                           fillGroup=groupName,
                           type='TH2I',path=developerPath+'jLJ/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jLJ_Et;h_jLJ_Et', title='jFex LRJet Transverse Energy;tobEt [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jLJ/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jLJ_Eta;h_jLJ_Eta', title='jFex LRJet #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jLJ/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jLJ_Phi;h_jLJ_Phi', title='jFex LRJet #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jLJ/',**phi_bins)

    helper.defineHistogram('jLJ_Eta,jLJ_Phi;h_jLJ_EtaPhiMap', title="jFEX LRJet #eta vs #phi;#eta;#phi",
                           fillGroup=groupName,
                           type='TH2F',path=developerPath+'jLJ/', **eta_phi_bins)

    helper.defineHistogram('jLJ_GlobalEta;h_jLJ_GlobalEta', title='jFex LRJet Global #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jLJ/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jLJ_GlobalPhi;h_jLJ_GlobalPhi', title='jFex LRJet Global #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jLJ/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jLJ_GlobalEta,jLJ_GlobalPhi;h_jLJ_GlobalEtaPhiMap', title="jFex LRJet #eta vs #phi;(int) #eta; (int) #phi",
                           fillGroup=groupName,
                           type='TH2F',path=developerPath+'jLJ/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)
    ######  jTAU  ######
    helper.defineHistogram('jTau_jFexNumber;h_jTAU_jFexNumber', title='jFex Tau Module;Module number;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jTAU/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jTau_fpgaNumber;h_jTAU_fpgaNumber', title='jFex Tau FPGA;FPGA number;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTAU/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jTau_jFexNumber,jTau_fpgaNumber;h_jTAU_DetectorMap', title="jFex Tau module vs FPGA; jFEX module; FPGA",
                           fillGroup=groupName,
                           type='TH2I',path=developerPath+'jTAU/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jTau_Et;h_jTAU_Et', title='jFex Tau Transverse Energy;tobEt [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jTAU/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jTau_Iso;h_jTAU_Iso', title='jFex Tau Isolation;tobIso [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jTAU/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jTau_Eta;h_jTAU_Eta', title='jFex Tau #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTAU/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jTau_Phi;h_jTAU_Phi', title='jFex Tau #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTAU/',**phi_bins)

    helper.defineHistogram('jTau_GlobalEta;h_jTAU_GlobalEta', title='jFex Tau Global #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTAU/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jTau_GlobalPhi;h_jTAU_GlobalPhi', title='jFex Tau Global #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTAU/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jTau_GlobalEta,jTAU_GlobalPhi;h_jTAU_GlobalEtaPhiMap', title="jFex Tau Global #eta vs #phi;(int) #eta; (int) #phi",
                           fillGroup=groupName,
                           type='TH2F',path=developerPath+'jTAU/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)
    ######  jEM  ######
    helper.defineHistogram('jEM_jFexNumber;h_jEM_jFexNumber', title='jFex EM Module;Module number;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jEM/', xbins=6,xmin=0,xmax=6)

    helper.defineHistogram('jEM_fpgaNumber;h_jEM_fpgaNumber', title='jFex EM FPGA;FPGA number;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jEM/',xbins=4,xmin=0,xmax=4)

    helper.defineHistogram('jEM_jFexNumber,jEM_fpgaNumber;h_jEM_DetectorMap', title="jFex EM module vs FPGA; jFEX module; FPGA",
                           fillGroup=groupName,
                           type='TH2I',path=developerPath+'jEM/', xbins=6,xmin=0,xmax=6,ybins=4,ymin=0,ymax=4,xlabels=Modules_names,ylabels=FPGA_names)

    helper.defineHistogram('jEM_Et;h_jEM_Et', title='jFex EM Transverse Energy;tobEt [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jEM/',xbins=512,xmin=0,xmax=2048)

    helper.defineHistogram('jEM_Eta;h_jEM_Eta', title='jFex EM #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jEM/',xbins=100,xmin=-5.0,xmax=5.0)

    helper.defineHistogram('jEM_Phi;h_jEM_Phi', title='jFex EM #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jEM/',**phi_bins)

    em_labels = ['None','loose','medium','tight']
    helper.defineHistogram('jEM_Iso;h_jEM_Iso', title='jFex EM Isolation;tobIso;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_f1;h_jEM_f1', title='jFex EM Frac1;EM Frac1;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_f2;h_jEM_f2', title='jFex EM Frac2;EM Frac2;Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jEM/',xbins=4,xmin=0,xmax=4,xlabels=em_labels)

    helper.defineHistogram('jEM_GlobalEta;h_jEM_GlobalEta', title='jFex EM Global #eta;#eta;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jEM/',xbins=100,xmin=-50,xmax=50)

    helper.defineHistogram('jEM_GlobalPhi;h_jEM_GlobalPhi', title='jFex EM Global #phi;#phi;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jEM/',xbins=67,xmin=-1,xmax=65)

    helper.defineHistogram('jEM_GlobalEta,jEM_GlobalPhi;h_jEM_GlobalEtaPhiMap', title="jFex EM Global #eta vs #phi;(int) #eta; (int) #phi",
                           fillGroup=groupName,
                           type='TH2F',path=developerPath+'jEM/', xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)
    ######  jXE  ######
    helper.defineHistogram('jXE_X;h_jXE_X', title='jFex MET X component (tobEx);tobEx [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jXE/',xbins=1000,xmin=-500,xmax=500)

    helper.defineHistogram('jXE_Y;h_jXE_Y', title='jFex MET Y component (tobEy);tobEy [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jXE/',xbins=1000,xmin=-500,xmax=500)

    helper.defineHistogram('jXE_MET;h_jXE_MET', title='jFex MET ;Total jXE [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jXE/',xbins=500,xmin=0,xmax=1000)

    helper.defineHistogram('jXE_phi;h_jXE_phi', title='jFex MET phi ;#phi=atan(Ey/Ex) values;Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jXE/',xbins=66,xmin=-math.pi,xmax=math.pi)


    ######  jTE  ######
    helper.defineHistogram('jTE_low;h_jTE_low', title='jFex SumEt low #eta;tob Et_lower [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jTE/',xbins=700,xmin=0,xmax=700)

    helper.defineHistogram('jTE_high;h_jTE_high', title='jFex SumEt high #eta;tob Et_upper [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1I', path=developerPath+'jTE/',xbins=700,xmin=0,xmax=700)

    helper.defineHistogram('jTE_SumEt;h_jTE_SumEt', title='jFex SumEt total ;Total jTE [200 MeV Scale];Counts',
                           fillGroup=groupName,
                           type='TH1F', path=developerPath+'jTE/',xbins=1000,xmin=0,xmax=4000)


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

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JfexMonitorCfg = JfexMonitoringConfig(flags)
    cfg.merge(JfexMonitorCfg)

    cfg.run()
