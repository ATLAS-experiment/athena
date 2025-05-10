"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
2020 Matthias Schott - Uni Mainz
"""

from AthenaConfiguration.Enums import BeamType


def MuonTrackConfig(flags, **kwargs):
    from AthenaMonitoring import AthMonitorCfgHelper
    from AthenaConfiguration.ComponentFactory import CompFactory

    helper = AthMonitorCfgHelper(flags, "MuonTrackMonitoringConfig")
    if flags.Beam.Type != BeamType.Collisions:
        kwargs.setdefault("PrimaryVerticesKey", "")
    if flags.Common.isOnline or flags.Beam.Type != BeamType.Collisions:
        kwargs.setdefault("RequireBeamSpot", False)

    muonTrackAlg = helper.addAlgorithm(CompFactory.MuonTrackMonitorAlgorithm, "MuonTrackMonitorAlg", **kwargs)

    myGroup = helper.addGroup(muonTrackAlg, "MuonTrackMonitorAlgorithm", "MuonPhysics/")


###########################################################
### Muons
###########################################################

    # Muons/Jpsi
    # -----------------------------------------------------
    myGroup.defineHistogram('JpsiMuonEta,JpsiMuonPhi;Muons_Jpsi_Origin_eta_phi',                
            title='Muons_Jpsi_Origin_eta_phi;eta;phi', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonEtaTight,JpsiMuonPhiTight;Muons_Jpsi_eta_phi_tight', 
            title='Muons_Jpsi_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonEtaMedium,JpsiMuonPhiMedium;Muons_Jpsi_eta_phi_medium', 
            title='Muons_Jpsi_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonD0;Muons_Jpsi_d0', 
            title='Muons_Jpsi_d0;d0;Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=30, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonZ0;Muons_Jpsi_z0', 
            title='Muons_Jpsi_z0;z0;Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonDPTIDME;Muons_Jpsi_ddpt_idme', 
            title='Muons_Jpsi_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=30, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonDPTIDMS;Muons_Jpsi_ddpt_idms', 
            title='Muons_Jpsi_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=30, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonDPTCBME;Muons_Jpsi_ddpt_cbme', 
            title='Muons_Jpsi_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=30, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonPt;Muons_Jpsi_pt', 
            title='Muons_Jpsi_pt; pT[GeV];Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonPt;Muons_Jpsi_pt_broad', 
            title='Muons_Jpsi_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/Jpsi', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass2D,muMinusEta;JpsimuMinusEta', 
            title='JpsimuMinusEta;JPsi mass;#eta_{#mu^{-}}', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass2D,muPlusEta;JpsimuPlusEta', 
            title='JpsimuPlusEta;JPsi mass;#eta_{#mu^{+}}', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('muMinusEta,muPlusEta;JpsimuPlusMinus', 
            title='JpsimuPlusMinus;#eta_{#mu^{-}};#eta_{#mu^{+}}', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=31, xmin=-3.15, xmax=3.15, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass,JpsiEta;JpsiMassAverage', 
            title='JpsiMassAverage;mass;eta', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass2D,Eta2D;JpsiMassEta2D', 
            title='JpsiMassEta2D;mass;eta(#mu^{-}#mu^{+})', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=27, xmin=2.6, xmax=3.6, ybins=16, ymin=0.5, ymax=16.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonEta,CBMuonPt;Muons_JpsiMuons_eta_pt', 
            title='Muons_JpsiMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonsEtaHitsLayer1,JpsiMuonsPhiHitsLayer1;Jpsi_HitsLayer1', 
            title='Jpsi_HitsLayer1', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonsEtaHitsLayer2,JpsiMuonsPhiHitsLayer2;Jpsi_HitsLayer2', 
            title='Jpsi_HitsLayer2', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonsEtaHitsLayer3,JpsiMuonsPhiHitsLayer3;Jpsi_HitsLayer3', 
            title='Jpsi_HitsLayer3', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonsEtaHitsLayer4,JpsiMuonsPhiHitsLayer4;Jpsi_HitsLayer4', 
            title='Jpsi_HitsLayer4', 
            type='TH2F', path='Muons/Jpsi', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # Muons/Z
    # -----------------------------------------------------
    myGroup.defineHistogram('ZMuonEta,ZMuonPhi;Muons_Z_Origin_eta_phi', 
            title='Muons_Z_Origin_eta_phi;eta;phi', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonEtaTight,ZMuonPhiTight;Muons_Z_eta_phi_tight', 
            title='Muons_Z_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonEtaMedium,ZMuonPhiMedium;Muons_Z_eta_phi_medium', 
            title='Muons_Z_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonD0;Muons_Z_d0', 
            title='Muons_Z_d0;d0;Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonZ0;Muons_Z_z0', 
            title='Muons_Z_z0;z0;Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonDPTIDME;Muons_Z_ddpt_idme', 
            title='Muons_Z_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonDPTIDMS;Muons_Z_ddpt_idms', 
            title='Muons_Z_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonDPTCBME;Muons_Z_ddpt_cbme', 
            title='Muons_Z_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonPt;Muons_Z_pt', 
            title='Muons_Z_pt; pT[GeV];Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonPt;Muons_Z_pt_broad', 
            title='Muons_Z_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/Z', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,muMinusEta;ZmuMinusEta', 
            title='ZmuMinusEta;Z mass;#eta_{#mu^{-}}', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=76., xmax=106., ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,muPlusEta;ZmuPlusEta', 
            title='ZmuPlusEta;Z mass;#eta_{#mu^{+}}', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=76., xmax=106., ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('muMinusEta,muPlusEta;ZmuPlusMinus', 
            title='ZmuPlusMinus;#eta_{#mu^{-}};#eta_{#mu^{+}}', 
            type='TH2F', path='Muons/Z', 
            xbins=31, xmin=-3.15, xmax=3.15, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,ZEta;ZMassAverage', 
            title='ZMassAverage;mass;eta', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=76., xmax=106., ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,MuPlusEta;ZMassEtaPlus', 
            title='ZMassEtaPlus;mass;eta(#mu^{-}#mu^{+})', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=76., xmax=106., ybins=16, ymin=0.5, ymax=16.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,muMinusEta;ZMassEtaMinus', 
            title='ZMassEtaMinus;mass;eta(#mu^{-}#mu^{+})', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=76., xmax=106., ybins=16, ymin=0.5, ymax=16.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonEta,CBMuonPt;Muons_ZMuons_eta_pt', 
            title='Muons_ZMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonsEtaHitsLayer1,ZMuonsPhiHitsLayer1;Z_HitsLayer1', 
            title='Z_HitsLayer1', 
            type='TH2F', path='Muons/Z', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonsEtaHitsLayer2,ZMuonsPhiHitsLayer2;Z_HitsLayer2', 
            title='Z_HitsLayer2', 
            type='TH2F', path='Muons/Z', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonsEtaHitsLayer3,ZMuonsPhiHitsLayer3;Z_HitsLayer3', 
            title='Z_HitsLayer3', 
            type='TH2F', path='Muons/Z', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonsEtaHitsLayer4,ZMuonsPhiHitsLayer4;Z_HitsLayer4', 
            title='Z_HitsLayer4', 
            type='TH2F', path='Muons/Z', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # Muons/AllCBMuons
    # -----------------------------------------------------
    myGroup.defineHistogram('AllCBMuonEta,AllCBMuonPhi;Muons_AllCBMuons_eta_phi', 
            title='Muons_AllCBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonEtaTight,AllCBMuonPhiTight;Muons_AllCBMuons_eta_phi_tight', 
            title='Muons_AllCBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonEtaMedium,AllCBMuonPhiMedium;Muons_AllCBMuons_eta_phi_medium', 
            title='Muons_AllCBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonD0;Muons_AllCBMuons_d0', 
            title='Muons_AllCBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonZ0;Muons_AllCBMuons_z0', 
            title='Muons_AllCBMuons_z0;z0;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonIDchi2ndof;Muons_AllCBMuons_IDtndof', 
            title='Muons_AllCBMuons_IDtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonMEchi2ndof;Muons_AllCBMuons_MEtndof', 
            title='Muons_AllCBMuons_MEtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonDPTIDME;Muons_AllCBMuons_ddpt_idme', 
            title='Muons_AllCBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonDPTIDMS;Muons_AllCBMuons_ddpt_idms', 
            title='Muons_AllCBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonDPTCBME;Muons_AllCBMuons_ddpt_cbme', 
            title='Muons_AllCBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonPt;Muons_AllCBMuons_pt', 
            title='Muons_AllCBMuons_pt;pT[GeV];Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonPt;Muons_AllCBMuons_pt_broad', 
            title='Muons_AllCBMuons_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonAuthor;Muons_AllCBMuons_Author', 
            title='Muons_AllCBMuons_Author;Author;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonType;Muons_AllCBMuons_Type', 
            title='Muons_AllCBMuons_Type;Type;Entries', 
            type='TH1F', path='Muons/AllCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonEta1Triggered,AllCBMuonMuonPhi1Triggered;Muons_AllCBMuons_eta_phi_1Triggered', 
            title='Muons_AllCBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonEta1All,AllCBMuonMuonPhi1All;Muons_AllCBMuons_eta_phi_1All', 
            title='Muons_AllCBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonEta,AllCBMuonPt;Muons_AllCBMuons_eta_pt', 
            title='Muons_AllCBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonsEtaHitsLayer1,AllCBMuonsPhiHitsLayer1;AllCBMuons_HitsLayer1', 
            title='AllCBMuons_HitsLayer1', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonsEtaHitsLayer2,AllCBMuonsPhiHitsLayer2;AllCBMuons_HitsLayer2', 
            title='AllCBMuons_HitsLayer2', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonsEtaHitsLayer3,AllCBMuonsPhiHitsLayer3;AllCBMuons_HitsLayer3', 
            title='AllCBMuons_HitsLayer3', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonsEtaHitsLayer4,AllCBMuonsPhiHitsLayer4;AllCBMuons_HitsLayer4', 
            title='AllCBMuons_HitsLayer4', 
            type='TH2F', path='Muons/AllCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    #myGroup.defineHistogram('AllCBMuonsNBHits;Muons_AllCBMuons_NBHits', 
    #        title='Muons_AllCBMuons_NBHits', 
    #        type='TH1F', path='Muons/NoTrigCBMuons', 
    #        xbins=, xmin=, xmax=, opt='kAlwaysCreate')
    #myGroup.defineHistogram('', 
    #        title='', 
    #        type='TH1F', path='Muons/NoTrigCBMuons', 
    #        xbins=, xmin=, xmax=, opt='kAlwaysCreate')
    #myGroup.defineHistogram('', 
    #        title='', 
    #        type='TH2F', path='Muons/AllCBMuons', 
    #        xbins=, xmin=, xmax=, ybins=, ymin=, ymax=, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # Muons/CBMuons
    # -----------------------------------------------------
    myGroup.defineHistogram('CBMuonEta,CBMuonPhi;Muons_CBMuons_eta_phi', 
            title='Muons_CBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonEtaTight,CBMuonPhiTight;Muons_CBMuons_eta_phi_tight', 
            title='Muons_CBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonEtaMedium,CBMuonPhiMedium;Muons_CBMuons_eta_phi_medium', 
            title='Muons_CBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonD0;Muons_CBMuons_d0', 
            title='Muons_CBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonZ0;Muons_CBMuons_z0', 
            title='Muons_CBMuons_z0;z0;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonIDchi2ndof;Muons_CBMuons_IDtndof', 
            title='Muons_CBMuons_IDtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMEchi2ndof;Muons_CBMuons_MEtndof', 
            title='Muons_CBMuons_MEtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonDPTIDME;Muons_CBMuons_ddpt_idme', 
            title='Muons_CBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonDPTIDMS;Muons_CBMuons_ddpt_idms', 
            title='Muons_CBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonDPTCBME;Muons_CBMuons_ddpt_cbme', 
            title='Muons_CBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonPt;Muons_CBMuons_pt', 
            title='Muons_CBMuons_pt;pT[GeV];Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonPt;Muons_CBMuons_pt_broad', 
            title='Muons_CBMuons_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonAuthor;Muons_CBMuons_Author', 
            title='Muons_CBMuons_Author;Author;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonType;Muons_CBMuons_Type', 
            title='Muons_CBMuons_Type;Type;Entries', 
            type='TH1F', path='Muons/CBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonEta1Triggered,CBMuonPhi1Triggered;Muons_CBMuons_1Triggered_eta_phi', 
            title='Muons_CBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonEta1All,CBMuonPhi1All;Muons_CBMuons_1All_eta_phi', 
            title='Muons_CBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonEta,CBMuonPt;Muons_CBMuons_eta_pt', 
            title='Muons_CBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonsEtaHitsLayer1,CBMuonsPhiHitsLayer1;CB_HitsLayer1', 
            title='CB_HitsLayer1', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonsEtaHitsLayer2,CBMuonsPhiHitsLayer2;CB_HitsLayer2', 
            title='CB_HitsLayer2', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonsEtaHitsLayer3,CBMuonsPhiHitsLayer3;CB_HitsLayer3', 
            title='CB_HitsLayer3', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonsEtaHitsLayer4,CBMuonsPhiHitsLayer4;CB_HitsLayer4', 
            title='CB_HitsLayer4', 
            type='TH2F', path='Muons/CBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # Muons/AllNonCBMuons
    # -----------------------------------------------------
    myGroup.defineHistogram('AllNonCBMuonEta,AllNonCBMuonPhi;Muons_AllNonCBMuons_Origin_eta_phi', 
            title='Muons_AllNonCBMuons_Origin_eta_phi;eta;phi;', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEta,AllNonCBMuonPhi;Muons_AllNonCBMuons_eta_phi', 
            title='Muons_AllNonCBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEtaTight,AllNonCBMuonPhiTight;Muons_AllNonCBMuons_eta_phi_tight', 
            title='Muons_AllNonCBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEtaMedium,AllNonCBMuonPhiMedium;Muons_AllNonCBMuons_eta_phi_medium', 
            title='Muons_AllNonCBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonD0;Muons_AllNonCBMuons_d0', 
            title='Muons_AllNonCBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonZ0;Muons_AllNonCBMuons_z0', 
            title='Muons_AllNonCBMuons_z0;z0;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonIDchi2ndof;Muons_AllNonCBMuons_IDtndof', 
            title='Muons_AllNonCBMuons_IDtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonMEchi2ndof;Muons_AllNonCBMuons_MEtndof', 
            title='Muons_AllNonCBMuons_MEtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonDPTIDME;Muons_AllNonCBMuons_ddpt_idme', 
            title='Muons_AllNonCBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonDPTIDMS;Muons_AllNonCBMuons_ddpt_idms', 
            title='Muons_AllNonCBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonDPTCBME;Muons_AllNonCBMuons_ddpt_cbme', 
            title='Muons_AllNonCBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonPt;Muons_AllNonCBMuons_pt', 
            title='Muons_AllNonCBMuons_pt;pT[GeV];Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonPt;Muons_AllNonCBMuons_pt_broad', 
            title='Muons_AllNonCBMuons_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonAuthor;Muons_AllNonCBMuons_Author', 
            title='Muons_AllNonCBMuons_Author;Author;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonType;Muons_AllNonCBMuons_Type', 
            title='Muons_AllNonCBMuons_Type;Type;Entries', 
            type='TH1F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEta1Triggered,AllNonCBMuonPhi1Triggered;Muons_AllNonCBMuons_1Triggered_eta_phi', 
            title='Muons_AllNonCBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEta1All,AllNonCBMuonPhi1All;Muons_AllNonCBMuons_1All_eta_phi', 
            title='Muons_AllNonCBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonEta,AllNonCBMuonPt;Muons_AllNonCBMuons_eta_pt', 
            title='Muons_AllNonCBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonsEtaHitsLayer1,AllNonCBMuonsPhiHitsLayer1;AllNonCBMuons_HitsLayer1', 
            title='AllNonCBMuons_HitsLayer1', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonsEtaHitsLayer2,AllNonCBMuonsPhiHitsLayer2;AllNonCBMuons_HitsLayer2', 
            title='AllNonCBMuons_HitsLayer2', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonsEtaHitsLayer3,AllNonCBMuonsPhiHitsLayer3;AllNonCBMuons_HitsLayer3', 
            title='AllNonCBMuons_HitsLayer3', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonsEtaHitsLayer4,AllNonCBMuonsPhiHitsLayer4;AllNonCBMuons_HitsLayer4', 
            title='AllNonCBMuons_HitsLayer4', 
            type='TH2F', path='Muons/AllNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # Muons/NonCBMuons
    # -----------------------------------------------------
    myGroup.defineHistogram('NonCBMuonEta,NonCBMuonPhi;Muons_NonCBMuons_Origin_eta_phi', 
            title='Muons_NonCBMuons_Origin_eta_phi;eta;phi;', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEta,NonCBMuonPhi;Muons_NonCBMuons_eta_phi', 
            title='Muons_NonCBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEtaTight,NonCBMuonPhiTight;Muons_NonCBMuons_eta_phi_tight', 
            title='Muons_NonCBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEtaMedium,NonCBMuonPhiMedium;Muons_NonCBMuons_eta_phi_medium', 
            title='Muons_NonCBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonD0;Muons_NonCBMuons_d0', 
            title='Muons_NonCBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonZ0;Muons_NonCBMuons_z0', 
            title='Muons_NonCBMuons_z0;z0;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonIDchi2ndof;Muons_NonCBMuons_IDtndof', 
            title='Muons_NonCBMuons_IDtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMEchi2ndof;Muons_NonCBMuons_MEtndof', 
            title='Muons_NonCBMuons_MEtndof;TotalNumberDOF;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonDPTIDME;Muons_NonCBMuons_ddpt_idme', 
            title='Muons_NonCBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonDPTIDMS;Muons_NonCBMuons_ddpt_idms', 
            title='Muons_NonCBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonDPTCBME;Muons_NonCBMuons_ddpt_cbme', 
            title='Muons_NonCBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=40, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonPt;Muons_NonCBMuons_pt', 
            title='Muons_NonCBMuons_pt;pT[GeV];Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonPt;Muons_NonCBMuons_pt_broad', 
            title='Muons_NonCBMuons_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonAuthor;Muons_NonCBMuons_Author', 
            title='Muons_NonCBMuons_Author;Author;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonType;Muons_NonCBMuons_Type', 
            title='Muons_NonCBMuons_Type;Type;Entries', 
            type='TH1F', path='Muons/NonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEta1Triggered,NonCBMuonPhi1Triggered;Muons_NonCBMuons_1Triggered_eta_phi', 
            title='Muons_NonCBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEta1All,NonCBMuonPhi1All;Muons_NonCBMuons_1All_eta_phi', 
            title='Muons_NonCBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonEta,CBMuonPt;Muons_NonCBMuons_eta_pt', 
            title='Muons_NonCBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonsEtaHitsLayer1,NonCBMuonsPhiHitsLayer1;NonCBMuons_HitsLayer1', 
            title='NonCBMuons_HitsLayer1', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonsEtaHitsLayer2,NonCBMuonsPhiHitsLayer2;NonCBMuons_HitsLayer2', 
            title='NonCBMuons_HitsLayer2', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonsEtaHitsLayer3,NonCBMuonsPhiHitsLayer3;NonCBMuons_HitsLayer3', 
            title='NonCBMuons_HitsLayer3', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonsEtaHitsLayer4,NonCBMuonsPhiHitsLayer4;NonCBMuons_HitsLayer4', 
            title='NonCBMuons_HitsLayer4', 
            type='TH2F', path='Muons/NonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # MuonPhysics/Muons/NoTrigCBMuons/
    # -----------------------------------------------------
    myGroup.defineHistogram('NoTrigCBMuonAuthor;Muons_NoTrigCBMuons_author', 
            title='Muons_NoTrigCBMuons_author;Author;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonType;Muons_NoTrigCBMuons_type', 
            title='Muons_NoTrigCBMuons_type;Type;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonIDchi2ndof;Muons_NoTrigCBMuons_chi2ndof', 
            title='Muons_NoTrigCBMuons_chi2ndof;TrackFitChi2/ndof', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=220, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonD0;Muons_NoTrigCBMuons_d0', 
            title='Muons_NoTrigCBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonDPTIDME;Muons_NoTrigCBMuons_ddpt_idme', 
            title='Muons_NoTrigCBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=50, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonDPTIDMS;Muons_NoTrigCBMuons_ddpt_idms', 
            title='Muons_NoTrigCBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=50, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonDPTCBME;Muons_NoTrigCBMuons_ddpt_cbme', 
            title='Muons_NoTrigCBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=200, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEta;Muons_NoTrigCBMuons_eta', 
            title='Muons_NoTrigCBMuons_eta;#eta;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=50, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonType;Muons_NoTrigCBMuons_muonType', 
            title='Muons_NoTrigCBMuons_muonType;MuonType;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=6, xmin=-0.5, xmax=5.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonPhi;Muons_NoTrigCBMuons_phi', 
            title='Muons_NoTrigCBMuons_phi;#varphi;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=-3.14, xmax=3.14, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonPt;Muons_NoTrigCBMuons_pt', 
            title='Muons_NoTrigCBMuons_pt;p_{T};Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonPt;Muons_NoTrigCBMuons_pt_broad', 
            title='Muons_NoTrigCBMuons_pt_broad;p_{T};Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonIDchi2ndof;Muons_NoTrigCBMuons_IDtndof', 
            title='Muons_NoTrigCBMuons_IDtndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonMEchi2ndof;Muons_NoTrigCBMuons_MEtndof', 
            title='Muons_NoTrigCBMuons_MEtndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonZ0;Muons_NoTrigCBMuons_z0', 
            title='Muons_NoTrigCBMuons_z0;SignedImpactParameterZ0(mm);Entries', 
            type='TH1F', path='Muons/NoTrigCBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEta,NoTrigCBMuonPhi;Muons_NoTrigCBMuons_eta_phi', 
            title='Muons_NoTrigCBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEtaTight,NoTrigCBMuonPhiTight;Muons_NoTrigCBMuons_eta_phi_tight', 
            title='Muons_NoTrigCBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEtaMedium,NoTrigCBMuonPhiMedium;Muons_NoTrigCBMuons_eta_phi_medium', 
            title='Muons_NoTrigCBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEta1,NoTrigCBMuonPhi1;Muons_NoTrigCBMuons_1Triggered_eta_phi', 
            title='Muons_NoTrigCBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEta1All,NoTrigCBMuonPhi1All;Muons_NoTrigCBMuons_1All_eta_phi', 
            title='Muons_NoTrigCBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonEta,NoTrigCBMuonPt;Muons_NoTrigCBMuons_eta_pt', 
            title='Muons_NoTrigCBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonsEtaHitsLayer1,NoTrigCBMuonsPhiHitsLayer1;NoTrigCBMuons_HitsLayer1', 
            title='NoTrigCBMuons_HitsLayer1', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonsEtaHitsLayer2,NoTrigCBMuonsPhiHitsLayer2;NoTrigCBMuons_HitsLayer2', 
            title='NoTrigCBMuons_HitsLayer2', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonsEtaHitsLayer3,NoTrigCBMuonsPhiHitsLayer3;NoTrigCBMuons_HitsLayer3', 
            title='NoTrigCBMuons_HitsLayer3', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonsEtaHitsLayer4,NoTrigCBMuonsPhiHitsLayer4;NoTrigCBMuons_HitsLayer4', 
            title='NoTrigCBMuons_HitsLayer4', 
            type='TH2F', path='Muons/NoTrigCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=20, ymin=-1, ymax=18, opt='kAlwaysCreate')
    # -----------------------------------------------------

    # MuonPhysics/Muons/NoTrigNonCBMuons/
    # -----------------------------------------------------
    myGroup.defineHistogram('NoTrigNonCBMuonAuthor;Muons_NoTrigNonCBMuons_author', 
            title='Muons_NoTrigNonCBMuons_author;Author;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonType;Muons_NoTrigNonCBMuons_type', 
            title='Muons_NoTrigNonCBMuons_type;Type;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonIDchi2ndof;Muons_NoTrigNonCBMuons_chi2ndof', 
            title='Muons_NoTrigNonCBMuons_chi2ndof;TrackFitChi2/ndof', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=220, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonD0;Muons_NoTrigNonCBMuons_d0', 
            title='Muons_NoTrigNonCBMuons_d0;d0;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=40, xmin=-1, xmax=1, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonDPTIDME;Muons_NoTrigNonCBMuons_ddpt_idme', 
            title='Muons_NoTrigNonCBMuons_ddpt_idme;(ptID-ptME)/ptID;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=50, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonDPTIDMS;Muons_NoTrigNonCBMuons_ddpt_idms', 
            title='Muons_NoTrigNonCBMuons_ddpt_idms;(ptID-ptMS)/ptID;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=50, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonDPTCBME;Muons_NoTrigNonCBMuons_ddpt_cbme', 
            title='Muons_NoTrigNonCBMuons_ddpt_cbme;(ptCB-ptME)/ptCB;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=200, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEta;Muons_NoTrigNonCBMuons_eta', 
            title='Muons_NoTrigNonCBMuons_eta;#eta;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=50, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonType;Muons_NoTrigNonCBMuons_muonType', 
            title='Muons_NoTrigNonCBMuons_muonType;MuonType;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=6, xmin=-0.5, xmax=5.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonPhi;Muons_NoTrigNonCBMuons_phi', 
            title='Muons_NoTrigNonCBMuons_phi;#varphi;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=-3.14, xmax=3.14, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonPt;Muons_NoTrigNonCBMuons_pt', 
            title='Muons_NoTrigNonCBMuons_pt;p_{T};Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonPt;Muons_NoTrigNonCBMuons_pt_broad', 
            title='Muons_NoTrigNonCBMuons_pt_broad;p_{T};Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonIDchi2ndof;Muons_NoTrigNonCBMuons_IDtndof', 
            title='Muons_NoTrigNonCBMuons_IDtndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonMEchi2ndof;Muons_NoTrigNonCBMuons_MEtndof', 
            title='Muons_NoTrigNonCBMuons_MEtndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonZ0;Muons_NoTrigNonCBMuons_z0', 
            title='Muons_NoTrigNonCBMuons_z0;SignedImpactParameterZ0(mm);Entries', 
            type='TH1F', path='Muons/NoTrigNonCBMuons', 
            xbins=100, xmin=-200, xmax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEta,NoTrigNonCBMuonPhi;Muons_NoTrigNonCBMuons_eta_phi', 
            title='Muons_NoTrigNonCBMuons_eta_phi;eta;phi', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEtaTight,NoTrigNonCBMuonPhiTight;Muons_NoTrigNonCBMuons_eta_phi_tight', 
            title='Muons_NoTrigNonCBMuons_eta_phi_tight;eta;phi', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEtaMedium,NoTrigNonCBMuonPhiMedium;Muons_NoTrigNonCBMuons_eta_phi_medium', 
            title='Muons_NoTrigNonCBMuons_eta_phi_medium;eta;phi', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEta1Triggered,NoTrigNonCBMuonPhi1Triggered;Muons_NoTrigNonCBMuons_1Triggered_eta_phi', 
            title='Muons_NoTrigNonCBMuons_eta_phi_1Triggered;eta;phi', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEta1All,NoTrigNonCBMuonPhi1All;Muons_NoTrigNonCBMuons_1All_eta_phi', 
            title='Muons_NoTrigNonCBMuons_eta_phi_1All;eta;phi', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonEta,NoTrigNonCBMuonPt;Muons_NoTrigNonCBMuons_eta_pt', 
            title='Muons_NoTrigNonCBMuons_eta_pt;eta;pt', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=100, ymin=0, ymax=200, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonsEtaHitsLayer1,NoTrigNonCBMuonsPhiHitsLayer1;Muons_NoTrigNonCBMuons_HitsLayer1', 
            title='Muons_NoTrigNonCBMuons_HitsLayer1', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonsEtaHitsLayer2,NoTrigNonCBMuonsPhiHitsLayer2;Muons_NoTrigNonCBMuons_HitsLayer2', 
            title='Muons_NoTrigNonCBMuons_HitsLayer2', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonsEtaHitsLayer3,NoTrigNonCBMuonsPhiHitsLayer3;Muons_NoTrigNonCBMuons_HitsLayer3', 
            title='Muons_NoTrigNonCBMuons_HitsLayer3', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonsEtaHitsLayer4,NoTrigNonCBMuonsPhiHitsLayer4;Muons_NoTrigNonCBMuons_HitsLayer4', 
            title='Muons_NoTrigNonCBMuons_HitsLayer4', 
            type='TH2F', path='Muons/NoTrigNonCBMuons', 
            xbins=20, xmin=-1, xmax=18, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    # -----------------------------------------------------


###########################################################
### Overview
###########################################################

    # Overview/General
    myGroup.defineHistogram('LumiBlock,LumiBlockTrackCategory;Overview_LB_nMuonTrack',
            title='Overview_LB_nMuonTrack;LumiBlock;LumiBlockTrackCategory',
            type='TH2F', path='Overview/General',
            xbins=2500, xmin=0, xmax=2500,
            ybins=27, ymin=-1, ymax=26, ylabels=["", "", "NoTrigNonCB", "NoTrigCB", "NonCB", "CB", "AllNonCB", "AllCB", "", "Z", "Jpsi", "", "ME_EC", "ME_BC", "ME_BA", "ME_EA", "", "MS_EC", "MS_BC", "MS_BA", "MS_EA", "", "ID_EC", "ID_BC", "ID_BA", "ID_EA", ""],
            opt='kAlwaysCreate')

    # Overview/Jpsi
    myGroup.defineHistogram('JpsiMuonLumiBlock;Overview_Jpsi_nJpsi_LB', 
            title='Overview_Jpsi_nJpsi_LB;LumiBlock;NumberOfJpsis', 
            type='TH1F', path='Overview/Jpsi', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')

    # Overview/Z
    myGroup.defineHistogram('ZMuonLumiBlock;Overview_Z_nZ_LB', 
            title='Overview_Z_nZ_LB;LumiBlock;NumberOfZs', 
            type='TH1F', path='Overview/Z', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')

    # Overview/CBMuons
    myGroup.defineHistogram('CBMuonLumiBlock;Overview_CBMuons_nMuon_LB', 
            title='Overview_CBMuons_nMuon_LB;LumiBlock;NumberOfCBMuons', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')
    myGroup.defineHistogram('MuonPrefix;Overview_CBMuons_Trig', 
            title='Overview_CBMuons_Trig;MuonPrefix;Muons', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=4, xmin=0, xmax=4, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuons;Overview_muons_per_event', 
            title='Overview_muons_per_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuonsTrig;Overview_muons_per_trig_event', 
            title='Overview_muons_per_trig_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuonsTrigCB;Overview_CBmuons_per_trig_event', 
            title='Overview_CBmuons_per_trig_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuonsNoTrigCB;Overview_CBmuons_per_notrig_event', 
            title='Overview_CBmuons_per_notrig_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')

    # Overview/NonCBMuons
    myGroup.defineHistogram('NonCBMuonLumiBlock;Overview_NonCBMuons_nMuon_LB', 
            title='Overview_NonCBMuons_nMuon_LB;LumiBlock;NumberOfNonCBMuons', 
            type='TH1F', path='Overview/NonCBMuons', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuonsTrigNonCB;Overview_NonCBmuons_per_trig_event', 
            title='Overview_NonCBmuons_per_trig_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/NonCBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NMuonsNoTrigNonCB;Overview_NonCBmuons_per_notrig_event', 
            title='Overview_NonCBmuons_per_notrig_event;NumberOfMuons;Events', 
            type='TH1F', path='Overview/NonCBMuons', 
            xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')

    # NoTrig/Overview/CBMuons
    myGroup.defineHistogram('CBMuonLumiBlock;NoTrig_Overview_CBMuons_nMuon_LB', 
            title='NoTrig_Overview_CBMuons_nMuon_LB;LumiBlock;NumberOfCBMuons', 
            type='TH1F', path='Overview/CBMuons', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')

    # NoTrig/Overview/NonCBMuons
    myGroup.defineHistogram('NonCBMuonLumiBlock;NoTrig_Overview_NonCBMuons_nMuon_LB', 
            title='NoTrig_Overview_NonCBMuons_nMuon_LB;LumiBlock;NumberOfNonCBMuons', 
            type='TH1F', path='Overview/NonCBMuons', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')

    # Overview/Container
    myGroup.defineHistogram('MSLumiBlockNumberOfMuonTracks;Overview_Container_nMuonTrack_LB', 
            title='Overview_Container_nMuonTrack_LB;LumiBlock;NumberOfMuonTracks', 
            type='TH1F', path='Overview/Container', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')
    myGroup.defineHistogram('MSLumiBlockNumberOfSegments;Overview_Container_nSegment_LB', 
            title='Overview_Container_nSegment_LB;LumiBlock;NumberOfMuonSegments', 
            type='TH1F', path='Overview/Container', 
            xbins=2500, xmin=0, xmax=2500, opt='kAlwaysCreate')


###########################################################
### TracksID
###########################################################

    # TracksID/Jpsi
    myGroup.defineHistogram('JpsiMuonNBHits;TracksID_Jpsi_HitContent_NBlayerHits', 
            title='TracksID_Jpsi_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonNPixHits;TracksID_Jpsi_HitContent_NPixelHits', 
            title='TracksID_Jpsi_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonNSCTHits;TracksID_Jpsi_HitContent_NSCTHits', 
            title='TracksID_Jpsi_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonNTRTHits;TracksID_Jpsi_HitContent_NTRTHits', 
            title='TracksID_Jpsi_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDchi2ndof;TracksID_Jpsi_chi2ndof', 
            title='TracksID_Jpsi_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDPt;TracksID_Jpsi_pt', 
            title='TracksID_Jpsi_pt;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDPtHi;TracksID_Jpsi_pt_broad', 
            title='TracksID_Jpsi_pt_broad;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDTNDOF;TracksID_Jpsi_tndof', 
            title='TracksID_Jpsi_tndof;TrackID total number DOF;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDZ0;TracksID_Jpsi_z0', 
            title='TracksID_Jpsi_z0;Track ID Z0;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDD0;TracksID_Jpsi_d0', 
            title='TracksID_Jpsi_d0;Track ID D0;Entries', 
            type='TH1F', path='TracksID/Jpsi', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDEta,JpsiMuonIDPhi;TracksID_Jpsi_eta_phi', 
            title='TracksID_Jpsi_eta_phi;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMuonIDEtaHi,JpsiMuonIDPhiHi;TracksID_Jpsi_eta_phi_broad', 
            title='TracksID_Jpsi_eta_phi_broad;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksID/Z
    myGroup.defineHistogram('ZMuonNBHits;TracksID_Z_HitContent_NBlayerHits', 
            title='TracksID_Z_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonNPixHits;TracksID_Z_HitContent_NPixelHits', 
            title='TracksID_Z_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonNSCTHits;TracksID_Z_HitContent_NSCTHits', 
            title='TracksID_Z_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonNTRTHits;TracksID_Z_HitContent_NTRTHits', 
            title='TracksID_Z_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDchi2ndof;TracksID_Z_chi2ndof', 
            title='TracksID_Z_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDPt;TracksID_Z_pt', 
            title='TracksID_Z_pt;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDPtHi;TracksID_Z_pt_broad', 
            title='TracksID_Z_pt_broad;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDTNDOF;TracksID_Z_tndof', 
            title='TracksID_Z_tndof;TrackID total number DOF;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDZ0;TracksID_Z_z0', 
            title='TracksID_Z_z0;Track ID Z0;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDD0;TracksID_Z_d0', 
            title='TracksID_Z_d0;Track ID D0;Entries', 
            type='TH1F', path='TracksID/Z', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDEta,ZMuonIDPhi;TracksID_Z_eta_phi', 
            title='TracksID_Z_eta_phi;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMuonIDEtaHi,ZMuonIDPhiHi;TracksID_Z_eta_phi_broad', 
            title='TracksID_Z_eta_phi_broad;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksID/CBMuons
    myGroup.defineHistogram('CBMuonNBHits;TracksID_CBMuons_HitContent_NBlayerHits', 
            title='TracksID_CBMuons_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonNPixHits;TracksID_CBMuons_HitContent_NPixelHits', 
            title='TracksID_CBMuons_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonNSCTHits;TracksID_CBMuons_HitContent_NSCTHits', 
            title='TracksID_CBMuons_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonNTRTHits;TracksID_CBMuons_HitContent_NTRTHits', 
            title='TracksID_CBMuons_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonIDchi2ndof;TracksID_CBMuons_chi2ndof', 
            title='TracksID_CBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDPt;TracksID_CBMuons_pt', 
            title='TracksID_CBMuons_pt;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDPtHi;TracksID_CBMuons_pt_broad', 
            title='TracksID_CBMuons_pt_broad;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDTNDOF;TracksID_CBMuons_tndof', 
            title='TracksID_CBMuons_tndof;TrackID total number DOF;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDZ0;TracksID_CBMuons_z0', 
            title='TracksID_CBMuons_z0;Track ID Z0;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDD0;TracksID_CBMuons_d0', 
            title='TracksID_CBMuons_d0;Track ID D0;Entries', 
            type='TH1F', path='TracksID/CBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDEta,CBMuonsMuonIDPhi;TracksID_CBMuons_eta_phi', 
            title='TracksID_CBMuons_eta_phi;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonMuonIDEtaHi,CBMuonsMuonIDPhiHi;TracksID_CBMuons_eta_phi_broad', 
            title='TracksID_CBMuons_eta_phi_broad;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksID/NonCBMuons
    myGroup.defineHistogram('NonCBMuonNBHits;TracksID_NonCBMuons_HitContent_NBlayerHits', 
            title='TracksID_NonCBMuons_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonNPixHits;TracksID_NonCBMuons_HitContent_NPixelHits', 
            title='TracksID_NonCBMuons_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonNSCTHits;TracksID_NonCBMuons_HitContent_NSCTHits', 
            title='TracksID_NonCBMuons_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonNTRTHits;TracksID_NonCBMuons_HitContent_NTRTHits', 
            title='TracksID_NonCBMuons_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonIDchi2ndof;TracksID_NonCBMuons_chi2ndof', 
            title='TracksID_NonCBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDPt;TracksID_NonCBMuons_pt', 
            title='TracksID_NonCBMuons_pt;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDPtHi;TracksID_NonCBMuons_pt_broad', 
            title='TracksID_NonCBMuons_pt_broad;IDTrack p_{T};Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDTNDOF;TracksID_NonCBMuons_tndof', 
            title='TracksID_NonCBMuons_tndof;TrackID total number DOF;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDZ0;TracksID_NonCBMuons_z0', 
            title='TracksID_NonCBMuons_z0;Track ID Z0;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDD0;TracksID_NonCBMuons_d0', 
            title='TracksID_NonCBMuons_d0;Track ID D0;Entries', 
            type='TH1F', path='TracksID/NonCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDEta,NonCBMuonsMuonIDPhi;TracksID_NonCBMuons_eta_phi', 
            title='TracksID_NonCBMuons_eta_phi;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonMuonIDEtaHi,NonCBMuonsMuonIDPhiHi;TracksID_NonCBMuons_eta_phi_broad', 
            title='TracksID_NonCBMuons_eta_phi_broad;TrackID #eta;TrackID #phi', 
            type='TH2F', path='TracksID/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # NoTrig/TracksID/CBMuons
    myGroup.defineHistogram('NoTrigCBMuonNBHits;NoTrig_TracksID_CBMuons_HitContent_NBlayerHits', 
            title='NoTrig_TracksID_CBMuons_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/NoTrigCBMuons', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonNPixHits;NoTrig_TracksID_CBMuons_HitContent_NPixelHits', 
            title='NoTrig_TracksID_CBMuons_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/NoTrigCBMuons', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonNSCTHits;NoTrig_TracksID_CBMuons_HitContent_NSCTHits', 
            title='NoTrig_TracksID_CBMuons_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/NoTrigCBMuons', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonNTRTHits;NoTrig_TracksID_CBMuons_HitContent_NTRTHits', 
            title='NoTrig_TracksID_CBMuons_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/NoTrigCBMuons', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonIDchi2ndof;NoTrig_TracksID_CBMuons_chi2ndof', 
            title='NoTrig_TracksID_CBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')

    # NoTrig/TracksID/NonCBMuons
    myGroup.defineHistogram('NoTrigNonCBMuonNBHits;NoTrig_TracksID_NonCBMuons_HitContent_NBlayerHits', 
            title='NoTrig_TracksID_NonCBMuons_HitContent_NBlayerHits;NumberOfBLayerHits;Entries', 
            type='TH1F', path='TracksID/NoTrigNonCBMuons', 
            xbins=5, xmin=-0.5, xmax=4.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonNPixHits;NoTrig_TracksID_NonCBMuons_HitContent_NPixelHits', 
            title='NoTrig_TracksID_NonCBMuons_HitContent_NPixelHits;NumberOfPixelHits;Entries', 
            type='TH1F', path='TracksID/NoTrigNonCBMuons', 
            xbins=10, xmin=-0.5, xmax=9.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonNSCTHits;NoTrig_TracksID_NonCBMuons_HitContent_NSCTHits', 
            title='NoTrig_TracksID_NonCBMuons_HitContent_NSCTHits;NumberOfSCTHits;Entries', 
            type='TH1F', path='TracksID/NoTrigNonCBMuons', 
            xbins=20, xmin=-0.5, xmax=19.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonNTRTHits;NoTrig_TracksID_NonCBMuons_HitContent_NTRTHits', 
            title='NoTrig_TracksID_NonCBMuons_HitContent_NTRTHits;NumberOfTRTHits;Entries', 
            type='TH1F', path='TracksID/NoTrigNonCBMuons', 
            xbins=50, xmin=-0.5, xmax=49.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonIDchi2ndof;NoTrig_TracksID_NonCBMuons_chi2ndof', 
            title='NoTrig_TracksID_NonCBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksID/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')


###########################################################
### TracksME
###########################################################

    # TracksME/Jpsi
    myGroup.defineHistogram('JpsiMEchi2ndof;TracksME_Jpsi_chi2ndof', 
            title='TracksME_Jpsi_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEPt;TracksME_Jpsi_pt', 
            title='TracksME_Jpsi_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEPtHi;TracksME_Jpsi_pt_broad', 
            title='TracksME_Jpsi_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMETNDOF;TracksME_Jpsi_tndof', 
            title='TracksME_Jpsi_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEdeltaZ0;TracksME_Jpsi_deltaZ0', 
            title='TracksME_Jpsi_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEZ0;TracksME_Jpsi_z0', 
            title='TracksME_Jpsi_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMED0sig;TracksME_JpsiMuons_d0sig', 
            title='TracksME_JpsiMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMED0;TracksME_Jpsi_d0', 
            title='TracksME_Jpsi_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/Jpsi', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEEta,JpsiMEPhi;TracksME_Jpsi_eta_phi', 
            title='TracksME_Jpsi_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMEEtaHi,JpsiMEPhiHi;TracksME_Jpsi_eta_phi_broad', 
            title='TracksME_Jpsi_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksME/Z
    myGroup.defineHistogram('ZMEchi2ndof;TracksME_Z_chi2ndof', 
            title='TracksME_Z_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEPt;TracksME_Z_pt', 
            title='TracksME_Z_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEPtHi;TracksME_Z_pt_broad', 
            title='TracksME_Z_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMETNDOF;TracksME_Z_tndof', 
            title='TracksME_Z_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEdeltaZ0;TracksME_Z_deltaZ0', 
            title='TracksME_Z_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEZ0;TracksME_Z_z0', 
            title='TracksME_Z_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMED0sig;TracksME_ZMuons_d0sig', 
            title='TracksME_ZMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMED0;TracksME_Z_d0', 
            title='TracksME_Z_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/Z', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEEta,ZMEPhi;TracksME_Z_eta_phi', 
            title='TracksME_Z_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMEEtaHi,ZMEPhiHi;TracksME_Z_eta_phi_broad', 
            title='TracksME_Z_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksME/CBMuons
    myGroup.defineHistogram('CBMEchi2ndof;TracksME_CBMuons_chi2ndof', 
            title='TracksME_CBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEPt;TracksME_CBMuons_pt', 
            title='TracksME_CBMuons_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEPtHi;TracksME_CBMuons_pt_broad', 
            title='TracksME_CBMuons_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMETNDOF;TracksME_CBMuons_tndof', 
            title='TracksME_CBMuons_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEdeltaZ0;TracksME_CBMuons_deltaZ0', 
            title='TracksME_CBMuons_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEZ0;TracksME_CBMuons_z0', 
            title='TracksME_CBMuons_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMED0sig;TracksME_CBMuons_d0sig', 
            title='TracksME_CBMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMED0;TracksME_CBMuons_d0', 
            title='TracksME_CBMuons_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/CBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEEta,CBMEPhi;TracksME_CBMuons_eta_phi', 
            title='TracksME_CBMuons_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMEEtaHi,CBMEPhiHi;TracksME_CBMuons_eta_phi_broad', 
            title='TracksME_CBMuons_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksME/NoTrigCBMuons
    myGroup.defineHistogram('NoTrigCBMEchi2ndof;TracksME_NoTrigCBMuons_chi2ndof', 
            title='TracksME_NoTrigCBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEPt;TracksME_NoTrigCBMuons_pt', 
            title='TracksME_NoTrigCBMuons_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEPtHi;TracksME_NoTrigCBMuons_pt_broad', 
            title='TracksME_NoTrigCBMuons_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMETNDOF;TracksME_NoTrigCBMuons_tndof', 
            title='TracksME_NoTrigCBMuons_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEdeltaZ0;TracksME_NoTrigCBMuons_deltaZ0', 
            title='TracksME_NoTrigCBMuons_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEZ0;TracksME_NoTrigCBMuons_z0', 
            title='TracksME_NoTrigCBMuons_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMED0sig;TracksME_NoTrigCBMuons_d0sig', 
            title='TracksME_NoTrigCBMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMED0;TracksME_NoTrigCBMuons_d0', 
            title='TracksME_NoTrigCBMuons_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/NoTrigCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEEta,NoTrigCBMEPhi;TracksME_NoTrigCBMuons_eta_phi', 
            title='TracksME_NoTrigCBMuons_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMEEtaHi,NoTrigCBMEPhiHi;TracksME_NoTrigCBMuons_eta_phi_broad', 
            title='TracksME_NoTrigCBMuons_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksME/CBMuons
    myGroup.defineHistogram('NonCBMEchi2ndof;TracksME_NonCBMuons_chi2ndof', 
            title='TracksME_NonCBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEPt;TracksME_NonCBMuons_pt', 
            title='TracksME_NonCBMuons_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEPtHi;TracksME_NonCBMuons_pt_broad', 
            title='TracksME_NonCBMuons_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMETNDOF;TracksME_NonCBMuons_tndof', 
            title='TracksME_NonCBMuons_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEdeltaZ0;TracksME_NonCBMuons_deltaZ0', 
            title='TracksME_NonCBMuons_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEZ0;TracksME_NonCBMuons_z0', 
            title='TracksME_NonCBMuons_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMED0sig;TracksME_NonCBMuons_d0sig', 
            title='TracksME_NonCBMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMED0;TracksME_NonCBMuons_d0', 
            title='TracksME_NonCBMuons_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/NonCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEEta,NonCBMEPhi;TracksME_NonCBMuons_eta_phi', 
            title='TracksME_NonCBMuons_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMEEtaHi,NonCBMEPhiHi;TracksME_NonCBMuons_eta_phi_broad', 
            title='TracksME_NonCBMuons_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')

    # TracksME/NoTrigNonCBMuonsNoTrig
    myGroup.defineHistogram('NoTrigNonCBMEchi2ndof;TracksME_NoTrigNonCBMuons_chi2ndof', 
            title='TracksME_NoTrigNonCBMuons_chi2ndof;TrackFitchi2ndof;Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEPt;TracksME_NoTrigNonCBMuons_pt', 
            title='TracksME_NoTrigNonCBMuons_pt;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEPtHi;TracksME_NoTrigNonCBMuons_pt_broad', 
            title='TracksME_NoTrigNonCBMuons_pt_broad;METrack p_{T};Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMETNDOF;TracksME_NoTrigNonCBMuons_tndof', 
            title='TracksME_NoTrigNonCBMuons_tndof;TrackME total number DOF;Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEdeltaZ0;TracksME_NoTrigNonCBMuons_deltaZ0', 
            title='TracksME_NoTrigNonCBMuons_deltaZ0;Track ME #delta(Z0);Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEZ0;TracksME_NoTrigNonCBMuons_z0', 
            title='TracksME_NoTrigNonCBMuons_z0;Track ME Z0;Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMED0sig;TracksME_NoTrigNonCBMuons_d0sig', 
            title='TracksME_NoTrigNonCBMuons_d0sig;Track ME D0 significance;Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMED0;TracksME_NoTrigNonCBMuons_d0', 
            title='TracksME_NoTrigNonCBMuons_d0;Track ME D0;Entries', 
            type='TH1F', path='TracksME/NoTrigNonCBMuons', 
            xbins=100, xmin=-5, xmax=5, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEEta,NoTrigNonCBMEPhi;TracksME_NoTrigNonCBMuons_eta_phi', 
            title='TracksME_NoTrigNonCBMuons_eta_phi;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMEEtaHi,NoTrigNonCBMEPhiHi;TracksME_NoTrigNonCBMuons_eta_phi_broad', 
            title='TracksME_NoTrigNonCBMuons_eta_phi_broad;TrackME #eta;TrackME #phi', 
            type='TH2F', path='TracksME/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=0, ymax=3.15, opt='kAlwaysCreate')


###########################################################
### MuonTrkPhys
###########################################################

    # MuonTrkPhys/Jpsi
    myGroup.defineHistogram('JpsiMass;m_Jpsi_Mass', 
            title='m_Jpsi_Mass;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiEta2D;m_Jpsi_2occupancy', 
            title='m_Jpsi_2occupancy;#etaRegionPermutations[+#mu,-#mu];N_{#mu}', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=16, xmin=-0.5, xmax=15.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiEta2;m_Jpsi_occy', 
            title='m_Jpsi_occy;#etaRegionPermutations[+#mu,-#mu];N_{#mu}', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=16, xmin=-0.5, xmax=15.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BA_BA;m_Jpsi_M_BA_BA', 
            title='m_Jpsi_M_BA_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BA_BC;m_Jpsi_M_BA_BC', 
            title='m_Jpsi_M_BA_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BA_EA;m_Jpsi_M_BA_EA', 
            title='m_Jpsi_M_BA_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BA_EC;m_Jpsi_M_BA_EC', 
            title='m_Jpsi_M_BA_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BC_BA;m_Jpsi_M_BC_BA', 
            title='m_Jpsi_M_BC_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BC_BC;m_Jpsi_M_BC_BC', 
            title='m_Jpsi_M_BC_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BC_EA;m_Jpsi_M_BC_EA', 
            title='m_Jpsi_M_BC_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_BC_EC;m_Jpsi_M_BC_EC', 
            title='m_Jpsi_M_BC_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EA_BA;m_Jpsi_M_EA_BA', 
            title='m_Jpsi_M_EA_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EA_BC;m_Jpsi_M_EA_BC', 
            title='m_Jpsi_M_EA_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EA_EA;m_Jpsi_M_EA_EA', 
            title='m_Jpsi_M_EA_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EA_EC;m_Jpsi_M_EA_EC', 
            title='m_Jpsi_M_EA_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EC_BA;m_Jpsi_M_EC_BA', 
            title='m_Jpsi_M_EC_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EC_BC;m_Jpsi_M_EC_BC', 
            title='m_Jpsi_M_EC_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EC_EA;m_Jpsi_M_EC_EA', 
            title='m_Jpsi_M_EC_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass_EC_EC;m_Jpsi_M_EC_EC', 
            title='m_Jpsi_M_EC_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsimuMinusEta;m_Jpsi_M_minus_Eta', 
            title='m_Jpsi_M_minus_Eta;#eta(#mu^{-});Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsimuPlusEta;m_Jpsi_M_plus_Eta', 
            title='m_Jpsi_M_plud_Eta;#eta(#mu^{+});Entries', 
            type='TH1F', path='MuonTrkPhys/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsimuMinusEta,JpsimuPlusEta;m_Jpsi_plus_minus_Eta', 
            title='m_Jpsi_plus_minus_Eta;#eta_{#mu^{-}};#eta_{#mu^{+}}', 
            type='TH2F', path='MuonTrkPhys/Jpsi', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('JpsiMass,JpsiEta2D;m_Jpsi_M_Eta_region', 
            title='m_Jpsi_M_Eta_region;mass;eta', 
            type='TH2F', path='MuonTrkPhys/Jpsi', 
            xbins=50, xmin=2.6, xmax=3.6, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate',
            merge="merge")
                

    # MuonTrkPhys/Z
    myGroup.defineHistogram('ZMass;m_Z_Mass', 
            title='m_Z_Mass;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZEta2D;m_Z_2occupancy', 
            title='m_Z_2occupancy;#etaRegionPermutations[+#mu,-#mu];N_{#mu}', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=16, xmin=-0.5, xmax=15.5, opt='kAlwaysCreate', merge="merge")
    myGroup.defineHistogram('ZMass_BA_BA;m_Z_M_BA_BA', 
            title='m_Z_M_BA_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BA_BC;m_Z_M_BA_BC', 
            title='m_Z_M_BA_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BA_EA;m_Z_M_BA_EA', 
            title='m_Z_M_BA_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BA_EC;m_Z_M_BA_EC', 
            title='m_Z_M_BA_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BC_BA;m_Z_M_BC_BA', 
            title='m_Z_M_BC_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BC_BC;m_Z_M_BC_BC', 
            title='m_Z_M_BC_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BC_EA;m_Z_M_BC_EA', 
            title='m_Z_M_BC_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_BC_EC;m_Z_M_BC_EC', 
            title='m_Z_M_BC_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EA_BA;m_Z_M_EA_BA', 
            title='m_Z_M_EA_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EA_BC;m_Z_M_EA_BC', 
            title='m_Z_M_EA_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EA_EA;m_Z_M_EA_EA', 
            title='m_Z_M_EA_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EA_EC;m_Z_M_EA_EC', 
            title='m_Z_M_EA_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EC_BA;m_Z_M_EC_BA', 
            title='m_Z_M_EC_BA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EC_BC;m_Z_M_EC_BC', 
            title='m_Z_M_EC_BC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EC_EA;m_Z_M_EC_EA', 
            title='m_Z_M_EC_EA;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass_EC_EC;m_Z_M_EC_EC', 
            title='m_Z_M_EC_EC;M_{#mu#mu};Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=50, xmin=76., xmax=106., opt='kAlwaysCreate')
    myGroup.defineHistogram('ZmuMinusEta;m_Z_M_minus_Eta', 
            title='m_Z_M_minus_Eta;#eta(#mu^{-});Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZmuPlusEta;m_Z_M_plus_Eta', 
            title='m_Z_M_plus_Eta;#eta(#mu^{+});Entries', 
            type='TH1F', path='MuonTrkPhys/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZmuMinusEta,ZmuPlusEta;m_Z_plus_minus_Eta', 
            title='m_Z_plus_minus_Eta;#eta_{#mu^{-}};#eta_{#mu^{+}}', 
            type='TH2F', path='MuonTrkPhys/Z', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate')
    myGroup.defineHistogram('ZMass,ZEta2D;m_Z_M_Eta_region', 
            title='m_Z_M_Eta_region;mass;eta', 
            type='TH2F', path='MuonTrkPhys/Z', 
            xbins=27, xmin=76., xmax=106., ybins=27, ymin=-2.7, ymax=2.7, opt='kAlwaysCreate',
            merge="merge")


###########################################################
### Segments
###########################################################


    # Segments/AllCBs
    myGroup.defineHistogram('AllCBMuonSector,AllCBMuonCIndex;Segments_AllCBMuons_chamberIndex_perSector', 
            title='Segments_AllCBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonSectorEta,AllCBMuonSectorPhi;Segments_AllCBMuons_etaphidir', 
            title='Segments_AllCBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonLargeSectorZ,AllCBMuonLargeSectorR;Segments_AllCBMuons_rzpos_sectorLarge', 
            title='Segments_AllCBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBMuonSmallSectorZ,AllCBMuonSmallSectorR;Segments_AllCBMuons_rzpos_sectorSmall', 
            title='Segments_AllCBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBSegmentFitchi2ndof;Segments_AllCBMuons_segmentfitChi2oNdof', 
            title='Segments_AllCBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBSegmentT0;Segments_AllCBMuons_t0', 
            title='Segments_AllCBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBSegmentT0Err;Segments_AllCBMuons_t0err', 
            title='Segments_AllCBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBSegmentXYPosEndcap;Segments_AllCBMuons_xypos_endcap', 
            title='Segments_AllCBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllCBSegmentXYPosBarrel;Segments_AllCBMuons_xypos_barrel', 
            title='Segments_AllCBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # Segments/AllNonCBs
    myGroup.defineHistogram('AllNonCBMuonSector,AllNonCBMuonCIndex;Segments_AllNonCBMuons_chamberIndex_perSector', 
            title='Segments_AllNonCBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonSectorEta,AllNonCBMuonSectorPhi;Segments_AllNonCBMuons_etaphidir', 
            title='Segments_AllNonCBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonLargeSectorZ,AllNonCBMuonLargeSectorR;Segments_AllNonCBMuons_rzpos_sectorLarge', 
            title='Segments_AllNonCBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBMuonSmallSectorZ,AllNonCBMuonSmallSectorR;Segments_AllNonCBMuons_rzpos_sectorSmall', 
            title='Segments_AllNonCBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBSegmentFitchi2ndof;Segments_AllNonCBMuons_segmentfitChi2oNdof', 
            title='Segments_AllNonCBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBSegmentT0;Segments_AllNonCBMuons_t0', 
            title='Segments_AllNonCBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBSegmentT0Err;Segments_AllNonCBMuons_t0err', 
            title='Segments_AllNonCBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBSegmentXYPosEndcap;Segments_AllNonCBMuons_xypos_endcap', 
            title='Segments_AllNonCBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('AllNonCBSegmentXYPosBarrel;Segments_AllNonCBMuons_xypos_barrel', 
            title='Segments_AllNonCBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # Segments/CBMuons
    myGroup.defineHistogram('CBMuonSector,CBMuonCIndex;Segments_CBMuons_chamberIndex_perSector', 
            title='Segments_CBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonSectorEta,CBMuonSectorPhi;Segments_CBMuons_etaphidir', 
            title='Segments_CBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonLargeSectorZ,CBMuonLargeSectorR;Segments_CBMuons_rzpos_sectorLarge', 
            title='Segments_CBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBMuonSmallSectorZ,CBMuonSmallSectorR;Segments_CBMuons_rzpos_sectorSmall', 
            title='Segments_CBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/CBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBSegmentFitchi2ndof;Segments_CBMuons_segmentfitChi2oNdof', 
            title='Segments_CBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBSegmentT0;Segments_CBMuons_t0', 
            title='Segments_CBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBSegmentT0Err;Segments_CBMuons_t0err', 
            title='Segments_CBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBSegmentXYPosEndcap;Segments_CBMuons_xypos_endcap', 
            title='Segments_CBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('CBSegmentXYPosBarrel;Segments_CBMuons_xypos_barrel', 
            title='Segments_CBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/CBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # Segments/NonCBs
    myGroup.defineHistogram('NonCBMuonSector,NonCBMuonCIndex;Segments_NonCBMuons_chamberIndex_perSector', 
            title='Segments_NonCBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonSectorEta,NonCBMuonSectorPhi;Segments_NonCBMuons_etaphidir', 
            title='Segments_NonCBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonLargeSectorZ,NonCBMuonLargeSectorR;Segments_NonCBMuons_rzpos_sectorLarge', 
            title='Segments_NonCBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBMuonSmallSectorZ,NonCBMuonSmallSectorR;Segments_NonCBMuons_rzpos_sectorSmall', 
            title='Segments_NonCBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBSegmentFitchi2ndof;Segments_NonCBMuons_segmentfitChi2oNdof', 
            title='Segments_NonCBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBSegmentT0;Segments_NonCBMuons_t0', 
            title='Segments_NonCBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBSegmentT0Err;Segments_NonCBMuons_t0err', 
            title='Segments_NonCBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBSegmentXYPosEndcap;Segments_NonCBMuons_xypos_endcap', 
            title='Segments_NonCBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NonCBSegmentXYPosBarrel;Segments_NonCBMuons_xypos_barrel', 
            title='Segments_NonCBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH1F', path='Segments/NonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # NoTrig/Segments/CBMuons
    myGroup.defineHistogram('NoTrigCBMuonSector,CBMuonCIndex;Segments_NoTrig_CBMuons_chamberIndex_perSector', 
            title='Segments_NoTrig_CBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonSectorEta,NoTrigCBMuonSectorPhi;Segments_NoTrig_CBMuons_etaphidir', 
            title='Segments_NoTrig_CBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonLargeSectorZ,NoTrigCBMuonLargeSectorR;Segments_NoTrig_CBMuons_rzpos_sectorLarge', 
            title='Segments_NoTrig_CBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBMuonSmallSectorZ,NoTrigCBMuonSmallSectorR;Segments_NoTrig_CBMuons_rzpos_sectorSmall', 
            title='Segments_NoTrig_CBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBSegmentFitchi2ndof;Segments_NoTrig_CBMuons_segmentfitChi2oNdof', 
            title='Segments_NoTrig_CBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/NoTrigCBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBSegmentT0;Segments_NoTrig_CBMuons_t0', 
            title='Segments_NoTrig_CBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/NoTrigCBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBSegmentT0Err;Segments_NoTrig_CBMuons_t0err', 
            title='Segments_NoTrig_CBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/NoTrigCBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBSegmentXPosEndcap,NoTrigCBSegmentYPosEndcap;Segments_NoTrig_CBMuons_xypos_endcap', 
            title='Segments_NoTrig_CBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigCBSegmentXPosBarrel,NoTrigCBSegmentYPosBarrel;Segments_NoTrig_CBMuons_xypos_barrel', 
            title='Segments_NoTrig_CBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH2F', path='Segments/NoTrigCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # NoTrig/Segments/NonCBMuons
    myGroup.defineHistogram('NoTrigNonCBMuonSector,NoTrigNonCBMuonCIndex;Segments_NoTrig_NonCBMuons_chamberIndex_perSector', 
            title='Segments_NoTrig_NonCBMuons_chamberIndex_perSector;Sector;ChamberIndex', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=30, xmin=-15, xmax=15, ybins=17, ymin=0, ymax=17, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonSectorEta,NoTrigNonCBMuonSectorPhi;Segments_NoTrig_NonCBMuons_etaphidir', 
            title='Segments_NoTrig_NonCBMuons_etaphidir;#eta_{dir};#varphi_{dir}', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonLargeSectorZ,NoTrigNonCBMuonLargeSectorR;Segments_NoTrig_NonCBMuons_rzpos_sectorLarge', 
            title='Segments_NoTrig_NonCBMuons_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBMuonSmallSectorZ,NoTrigNonCBMuonSmallSectorR;Segments_NoTrig_NonCBMuons_rzpos_sectorSmall', 
            title='Segments_NoTrig_NonCBMuons_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBSegmentFitchi2ndof;Segments_NoTrig_NonCBMuons_segmentfitChi2oNdof', 
            title='Segments_NoTrig_NonCBMuons_segmentfitChi2oNdof;SegmentFit#Chi^{2}/N_{dof};Entries', 
            type='TH1F', path='Segments/NoTrigNonCBMuons', 
            xbins=120, xmin=0, xmax=12, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBSegmentT0;Segments_NoTrig_NonCBMuons_t0', 
            title='Segments_NoTrig_NonCBMuons_t0;t_{0};Entries', 
            type='TH1F', path='Segments/NoTrigNonCBMuons', 
            xbins=200, xmin=-25, xmax=25, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBSegmentT0Err;Segments_NoTrig_NonCBMuons_t0err', 
            title='Segments_NoTrig_NonCBMuons_t0err;t_{0}Error;Entries', 
            type='TH1F', path='Segments/NoTrigNonCBMuons', 
            xbins=100, xmin=0, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBSegmentXPosEndcap,NoTrigNonCBSegmentYPosEndcap;Segments_NoTrig_NonCBMuons_xypos_endcap', 
            title='Segments_NoTrig_NonCBMuons_xypos_endcap;x_{pos};y_{pos}', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')
    myGroup.defineHistogram('NoTrigNonCBSegmentXPosBarrel,NoTrigNonCBSegmentYPosBarrel;Segments_NoTrig_NonCBMuons_xypos_barrel', 
            title='Segments_NoTrig_NonCBMuons_xypos_barrel;x_{pos};y_{pos}', 
            type='TH2F', path='Segments/NoTrigNonCBMuons', 
            xbins=24, xmin=-12000, xmax=12000, ybins=24, ymin=-12000, ymax=12000, opt='kAlwaysCreate')

    # Segments/Container
    myGroup.defineHistogram('MSLargeSectorZ,MSLargeSectorR;Segments_Container_rzpos_sectorLarge', 
            title='Segments_Container_rzpos_sectorLarge;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/Container', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')
    myGroup.defineHistogram('MSSmallSectorZ,MSSmallSectorR;Segments_Container_rzpos_sectorSmall', 
            title='Segments_Container_rzpos_sectorSmall;zPos[mm];r[mm]', 
            type='TH2F', path='Segments/Container', 
            xbins=220, xmin=-22000, xmax=22000, ybins=100, ymin=0, ymax=15000, opt='kAlwaysCreate')



###########################################################
### TracksMS
###########################################################

    # TracksMS/Container
    myGroup.defineHistogram('ContainerMSAuthor;TracksMS_Container_MSAuthor', 
            title='TracksMS_Container_Author;MS-Author;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSQuality;TracksMS_Container_Quality', 
            title='TracksMS_Container_Quality;MS-Quality;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSType;TracksMS_Container_Type', 
            title='TracksMS_Container_Type;MS-Type;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSEta,MSPhi;TracksMS_Container_eta_phi', 
            title='TracksMS_Container_eta_phi;eta;phi', 
            type='TH2F', path='TracksMS/Container', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSEta,MSPhi;TracksMS_Container_eta_phi_broad', 
            title='TracksMS_Container_eta_phi_broad;eta;phi', 
            type='TH2F', path='TracksMS/Container', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSPt;TracksMS_Container_pt', 
            title='TracksMS_Container_pt;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSPt;TracksMS_Container_pt_broad', 
            title='TracksMS_Container_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSD0sig;TracksMS_Container_d0sig', 
            title='TracksMS_Container_d0sig;d0sig;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSD0;TracksMS_Container_d0', 
            title='TracksMS_Container_d0;d0;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSdeltaZ0;TracksMS_Container_deltaZ0', 
            title='TracksMS_Container_deltaZ0;#delta(Z0);Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSZ0;TracksMS_Container_z0', 
            title='TracksMS_Container_z0;z0;Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerMSchi2ndof;TracksMS_Container_chi2ndof', 
            title='TracksMS_Container_chi2ndof;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/Container', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')

    # TracksMS/ContainerNoTrig
    myGroup.defineHistogram('ContainerNoTrigMSAuthor;TracksMS_ContainerNoTrig_Author', 
            title='TracksMS_ContainerNoTrig_Author;MS-Author;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSQuality;TracksMS_ContainerNoTrig_Quality', 
            title='TracksMS_ContainerNoTrig_Quality;MS-Quality;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSType;TracksMS_ContainerNoTrig_Type', 
            title='TracksMS_ContainerNoTrig_Type;MS-Type;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=15, xmin=0, xmax=15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSEta,ContainerNoTrigMSPhi;TracksMS_ContainerNoTrig_eta_phi', 
            title='TracksMS_ContainerNoTrig_eta_phi;eta;phi', 
            type='TH2F', path='TracksMS/ContainerNoTrig', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSEta,ContainerNoTrigMSPhi;TracksMS_ContainerNoTrig_eta_phi_broad', 
            title='TracksMS_ContainerNoTrig_eta_phi_broad;eta;phi', 
            type='TH2F', path='TracksMS/ContainerNoTrig', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSPt;TracksMS_ContainerNoTrig_pt', 
            title='TracksMS_ContainerNoTrig_pt;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSPt;TracksMS_ContainerNoTrig_pt_broad', 
            title='TracksMS_ContainerNoTrig_pt_broad;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSD0sig;TracksMS_ContainerNoTrig_d0sig', 
            title='TracksMS_ContainerNoTrig_d0sig;d0sig;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSD0;TracksMS_ContainerNoTrig_d0', 
            title='TracksMS_ContainerNoTrig_d0;d0;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=-0.5, xmax=0.5, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSdeltaZ0;TracksMS_ContainerNoTrig_deltaZ0', 
            title='TracksMS_ContainerNoTrig_deltaZ0;#delta(Z0);Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=100, xmin=-10, xmax=10, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSZ0;TracksMS_ContainerNoTrig_z0', 
            title='TracksMS_ContainerNoTrig_z0;z0;Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=-100, xmax=100, opt='kAlwaysCreate')
    myGroup.defineHistogram('ContainerNoTrigMSchi2ndof;TracksMS_ContainerNoTrig_chi2ndof', 
            title='TracksMS_ContainerNoTrig_chi2ndof;pT[GeV];Entries', 
            type='TH1F', path='TracksMS/ContainerNoTrig', 
            xbins=50, xmin=0, xmax=100, opt='kAlwaysCreate')

    # MuonPhysics/NoTrig/MSVertices
    myGroup.defineHistogram('nMDT;NoTrig_MSVertices_m_MSVx_nMDT', 
            title='NoTrig_MSVertices_m_MSVx_nMDT;;N_{MDT}', 
            type='TH1F', path='MSVertices', 
            xbins=200, xmin=0, xmax=3000, opt='kAlwaysCreate')
    myGroup.defineHistogram('nRPC;NoTrig_MSVertices_m_MSVx_nRPC', 
            title='NoTrig_MSVertices_m_MSVx_nRPC;;N_{RPC}', 
            type='TH1F', path='MSVertices', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('nTGC;NoTrig_MSVertices_m_MSVx_nTGC', 
            title='NoTrig_MSVertices_m_MSVx_nTGC;;N_{TGC}', 
            type='TH1F', path='MSVertices', 
            xbins=100, xmin=0, xmax=1000, opt='kAlwaysCreate')
    myGroup.defineHistogram('nTracklets;NoTrig_MSVertices_m_MSVx_nTracklets', 
            title='NoTrig_MSVertices_m_MSVx_nTracklets;N_{trackletS};', 
            type='TH1F', path='MSVertices', 
            xbins=20, xmin=0, xmax=20, opt='kAlwaysCreate')
    myGroup.defineHistogram('MSEta,MSPhi;NoTrig_MSVertices_m_VertexEtaPhi', 
            title='NoTrig_MSVertices_m_VertexEtaPhi;#eta;#varphi', 
            type='TH2F', path='MSVertices', 
            xbins=27, xmin=-2.7, xmax=2.7, ybins=31, ymin=-3.15, ymax=3.15, opt='kAlwaysCreate')


    #myGroup.defineHistogram('MuonType;MuonType', 
    #        title='MuonType', 
    #        type='TH1F', path='Muons', 
    #        xbins=10, xmin=0, xmax=10, opt='kAlwaysCreate')
    #myGroup.defineHistogram('', 
    #        title='', 
    #        type='TH1F', path='Muons/NoTrigCBMuons', 
    #        xbins=, xmin=, xmax=, opt='kAlwaysCreate')

    return helper.result()



if __name__=="__main__":
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    TestFiles = ['/eos/atlas/atlascerngroupdisk/det-rpc/data/DESDM_MCP/data18_13TeV.00358615.physics_Main.merge.DESDM_MCP.f961_m2024/data18_13TeV.00358615.physics_Main.merge.DESDM_MCP.f961_m2024._0084.1']
    flags = initConfigFlags()
    flags.Input.Files = TestFiles
    flags.Output.HISTFileName = 'TestOutput.root'
    flags.lock()
    # Initialize configuration object, add accumulator, merge and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    acc = MuonTrackConfig(flags)
    cfg.merge(acc)
    cfg.printConfig(withDetails=False)
    cfg.run(20)
