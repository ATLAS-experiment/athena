#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetAlignMonPVBiasesAlgCfg.py
@author PerJohansson
@date 2022
@brief Configuration for Run 3 based on InDetAlignPVBiases.cxx
'''

from math import pi as M_PI

def IDAlignMonPVBiasesAlgCfg(helper, alg, **kwargs):
    '''Function to configures some algorithms in the monitoring system.'''

    #Values
    m_nPhiBinsMap = 20
    m_maxPhi = M_PI
    m_nEtaBinsMap = 20
    m_maxEta = 2.5
    m_nD0Bins = 10000
    m_maxD0 = 5
    
    # this creates a "pvGroup" called "alg" which will put its histograms into the subdirectory "PVBiases"
    pvGroup = helper.addGroup(alg, 'PVBiases')
    pathPVBiases = '/IDAlignMon/ExtendedTracks/PVBiases'

    # Histograms for the Alignment PVBiases monitoring:    
    #400-600MeV
    varName = 'm_Phi_46p,m_d0_46p;trk_d0_wrtPV_vs_phi_400MeV_600MeV_positive'
    title = 'd0 vs phi 400MeV-600MeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=m_maxD0)

    varName = 'm_Phi_46p,m_d0_46p;trk_d0_wrtPV_vs_phi_400MeV_600MeV_positive_profile'
    title = 'd0 vs phi 400MeV-600MeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_46p,m_d0_46p;trk_d0_wrtPV_vs_eta_400MeV_600MeV_positive'
    title = 'd0 vs eta 400MeV-600MeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_46p,m_d0_46p;trk_d0_wrtPV_vs_eta_400MeV_600MeV_positive_profile'
    title = 'd0 vs eta 400MeV-600MeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_46n,m_d0_46n;trk_d0_wrtPV_vs_phi_400MeV_600MeV_negative'
    title = 'd0 vs phi 400MeV-600MeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_46n,m_d0_46n;trk_d0_wrtPV_vs_phi_400MeV_600MeV_negative_profile'
    title = 'd0 vs phi 400MeV-600MeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_46n,m_d0_46n;trk_d0_wrtPV_vs_eta_400MeV_600MeV_negative'
    title = 'd0 vs eta 400MeV-600MeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_46n,m_d0_46n;trk_d0_wrtPV_vs_eta_400MeV_600MeV_negative_profile'
    title = 'd0 vs eta 400MeV-600MeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    #600-1000MeV
    varName = 'm_Phi_61p,m_d0_61p;trk_d0_wrtPV_vs_phi_600MeV_1GeV_positive'
    title = 'd0 vs phi 600MeV-1GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_61p,m_d0_61p;trk_d0_wrtPV_vs_phi_600MeV_1GeV_positive_profile'
    title = 'd0 vs phi 600MeV-1GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_61p,m_d0_61p;trk_d0_wrtPV_vs_eta_600MeV_1GeV_positive'
    title = 'd0 vs eta 600MeV-1GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_61p,m_d0_61p;trk_d0_wrtPV_vs_eta_600MeV_1GeV_positive_profile'
    title = 'd0 vs eta 600MeV-1GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_61n,m_d0_61n;trk_d0_wrtPV_vs_phi_600MeV_1GeV_negative'
    title = 'd0 vs phi 600MeV-1GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_61n,m_d0_61n;trk_d0_wrtPV_vs_phi_600MeV_1GeV_negative_profile'
    title = 'd0 vs phi 600MeV-1GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_61n,m_d0_61n;trk_d0_wrtPV_vs_eta_600MeV_1GeV_negative'
    title = 'd0 vs eta 600MeV-1GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_61n,m_d0_61n;trk_d0_wrtPV_vs_eta_600MeV_1GeV_negative_profile'
    title = 'd0 vs eta 600MeV-1GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    #1-2GeV
    varName = 'm_Phi_12p,m_d0_12p;trk_d0_wrtPV_vs_phi_1GeV_2GeV_positive'
    title = 'd0 vs phi 1GeV-2GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_12p,m_d0_12p;trk_d0_wrtPV_vs_phi_1GeV_2GeV_positive_profile'
    title = 'd0 vs phi 1GeV-2GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_12p,m_d0_12p;trk_d0_wrtPV_vs_eta_1GeV_2GeV_positive'
    title = 'd0 vs eta 1GeV-2GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_12p,m_d0_12p;trk_d0_wrtPV_vs_eta_1GeV_2GeV_positive_profile'
    title = 'd0 vs eta 1GeV-2GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_12n,m_d0_12n;trk_d0_wrtPV_vs_phi_1GeV_2GeV_negative'
    title = 'd0 vs phi 1GeV-2GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_12n,m_d0_12n;trk_d0_wrtPV_vs_phi_1GeV_2GeV_negative_profile'
    title = 'd0 vs phi 1GeV-2GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_12n,m_d0_12n;trk_d0_wrtPV_vs_eta_1GeV_2GeV_negative'
    title = 'd0 vs eta 1GeV-2GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_12n,m_d0_12n;trk_d0_wrtPV_vs_eta_1GeV_2GeV_negative_profile'
    title = 'd0 vs eta 1GeV-2GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    #2-5GeV
    varName = 'm_Phi_25p,m_d0_25p;trk_d0_wrtPV_vs_phi_2GeV_5GeV_positive'
    title = 'd0 vs phi 2GeV-5GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_25p,m_d0_25p;trk_d0_wrtPV_vs_phi_2GeV_5GeV_positive_profile'
    title = 'd0 vs phi 2GeV-5GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_25p,m_d0_25p;trk_d0_wrtPV_vs_eta_2GeV_5GeV_positive'
    title = 'd0 vs eta 2GeV-5GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_25p,m_d0_25p;trk_d0_wrtPV_vs_eta_2GeV_5GeV_positive_profile'
    title = 'd0 vs eta 2GeV-5GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_25n,m_d0_25n;trk_d0_wrtPV_vs_phi_2GeV_5GeV_negative'
    title = 'd0 vs phi 2GeV-5GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_25n,m_d0_25n;trk_d0_wrtPV_vs_phi_2GeV_5GeV_negative_profile'
    title = 'd0 vs phi 2GeV-5GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_25n,m_d0_25n;trk_d0_wrtPV_vs_eta_2GeV_5GeV_negative'
    title = 'd0 vs eta 2GeV-5GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_25n,m_d0_25n;trk_d0_wrtPV_vs_eta_2GeV_5GeV_negative_profile'
    title = 'd0 vs eta 2GeV-5GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    #5-10GeV
    varName = 'm_Phi_510p,m_d0_510p;trk_d0_wrtPV_vs_phi_5GeV_10GeV_positive'
    title = 'd0 vs phi 5GeV-10GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_510p,m_d0_510p;trk_d0_wrtPV_vs_phi_5GeV_10GeV_positive_profile'
    title = 'd0 vs phi 5GeV-10GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_510p,m_d0_510p;trk_d0_wrtPV_vs_eta_5GeV_10GeV_positive'
    title = 'd0 vs eta 5GeV-10GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_510p,m_d0_510p;trk_d0_wrtPV_vs_eta_5GeV_10GeV_positive_profile'
    title = 'd0 vs eta 5GeV-10GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_510n,m_d0_510n;trk_d0_wrtPV_vs_phi_5GeV_10GeV_negative'
    title = 'd0 vs phi 5GeV-10GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_510n,m_d0_510n;trk_d0_wrtPV_vs_phi_5GeV_10GeV_negative_profile'
    title = 'd0 vs phi 2GeV-10GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_510n,m_d0_510n;trk_d0_wrtPV_vs_eta_5GeV_10GeV_negative'
    title = 'd0 vs eta 5GeV-10GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_510n,m_d0_510n;trk_d0_wrtPV_vs_eta_5GeV_10GeV_negative_profile'
    title = 'd0 vs eta 5GeV-10GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    #>10GeV
    varName = 'm_Phi_g10p,m_d0_g10p;trk_d0_wrtPV_vs_phi_10GeV_positive'
    title = 'd0 vs phi >10GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_g10p,m_d0_g10p;trk_d0_wrtPV_vs_phi_10GeV_positive_profile'
    title = 'd0 vs phi >10GeV positive; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_g10p,m_d0_g10p;trk_d0_wrtPV_vs_eta_10GeV_positive'
    title = 'd0 vs eta >10GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_g10p,m_d0_g10p;trk_d0_wrtPV_vs_eta_10GeV_positive_profile'
    title = 'd0 vs eta >10GeV positive; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_g10n,m_d0_g10n;trk_d0_wrtPV_vs_phi_10GeV_negative'
    title = 'd0 vs phi >10GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Phi_g10n,m_d0_g10n;trk_d0_wrtPV_vs_phi_10GeV_negative_profile'
    title = 'd0 vs phi >10GeV negative; #phi; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nPhiBinsMap, xmin=-m_maxPhi, xmax=m_maxPhi, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_g10n,m_d0_g10n;trk_d0_wrtPV_vs_eta_10GeV_negative'
    title = 'd0 vs eta >10GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TH2F', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ybins=m_nD0Bins, ymin=-m_maxD0, ymax=-m_maxD0)

    varName = 'm_Eta_g10n,m_d0_g10n;trk_d0_wrtPV_vs_eta_10GeV_negative_profile'
    title = 'd0 vs eta >10GeV negative; #eta; d0 [mm]'
    pvGroup.defineHistogram(varName, type='TProfile', path=pathPVBiases, title=title, xbins=m_nEtaBinsMap, xmin=-m_maxEta, xmax=m_maxEta, ymin=-m_maxD0, ymax=-m_maxD0)

    # end histograms
