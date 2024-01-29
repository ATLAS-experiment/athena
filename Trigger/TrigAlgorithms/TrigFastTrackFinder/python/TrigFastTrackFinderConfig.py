# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool


def TrigFastTrackFinderMonitoring(flags):
    name =    flags.Tracking.ActiveConfig.name
    doResMon= flags.Tracking.ActiveConfig.doResMon
    return TrigFastTrackFinderMonitoringArg(flags, name, doResMon)

    
def TrigFastTrackFinderMonitoringArg(flags, name, doResMon):


    def addSPHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSPsPIX', path='EXPERT',type='TH1F',title="Number of Pixel SPs", xbins = 500, xmin=-0.5, xmax=49999.5)
            montool.defineHistogram('roi_nSPsSCT', path='EXPERT',type='TH1F',title="Number of SCT SPs", xbins = 500, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_phiWidth',path='EXPERT',type='TH1F',title="Phi width of the input RoI",xbins = 100, xmin=0, xmax=6.4)
            montool.defineHistogram('roi_etaWidth',path='EXPERT',type='TH1F',title="Eta width of the input RoI",xbins = 100, xmin=0, xmax=5)
        else:
            montool.defineHistogram('roi_nSPsPIX', path='EXPERT',type='TH1F',title="Number of Pixel SPs", xbins = 50, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_nSPsSCT', path='EXPERT',type='TH1F',title="Number of SCT SPs", xbins = 50, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_phiWidth',path='EXPERT',type='TH1F',title="Phi width of the input RoI",xbins = 100, xmin=0, xmax=1.0)
            montool.defineHistogram('roi_etaWidth',path='EXPERT',type='TH1F',title="Eta width of the input RoI",xbins = 100, xmin=0, xmax=1.0)

        montool.defineHistogram('roi_eta',     path='EXPERT',type='TH1F',title='Eta of the input RoI;;Entries', xbins=100, xmin=-5, xmax=5)
        montool.defineHistogram('roi_phi',     path='EXPERT',type='TH1F',title="Phi of the input RoI",xbins = 100, xmin=-3.2, xmax=3.2)
        montool.defineHistogram('roi_z',       path='EXPERT',type='TH1F',title="z of the input RoI",xbins = 200, xmin=-400, xmax=400)
        montool.defineHistogram('roi_zWidth',  path='EXPERT',type='TH1F',title="z width of the input RoI",xbins = 100, xmin=0, xmax=500)

    def addDataErrorHistograms(montool):
        montool.defineHistogram('roi_lastStageExecuted',path='EXPERT',type='TH1F',title="Last Step Successfully Executed", xbins = 8 , xmin=-0.5, xmax=7.5,
                             xlabels=["Start","GetRoI","GetSPs","ZFinder","Triplets","TrackMaker","TrackFitter","TrackConverter"])

    def addTimingHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=200000.0, ybins = 100, ymin=0.0, ymax=5000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=5000.0,   ybins = 100, ymin=0.0, ymax=5000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_ZFinder',              path='EXPERT',type='TH1F',title="ZFinder time (ms)",           xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=1000.0)
        elif name=='jetSuper':
            montool.defineHistogram('roi_nSPs, TIME_PattR1eco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",   xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=1000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=1000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_ZFinder',              path='EXPERT',type='TH1F',title="ZFinder time (ms)",           xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        elif name in ['beamSpot','beamSpotFS','bphysics','bmumux']:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=2000.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=2000.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins =  50, xmin=0.0, xmax=50.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        elif name=='fullScanLRT':
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=500.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=500.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=5000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins = 200, xmin=0.0, xmax=200.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=2000.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins = 200, xmin=0.0, xmax=200.0)
        else:
            montool.defineHistogram('roi_nSPs, TIME_PattReco',   path='EXPERT',type='TH2F',title="PattReco time; nSPs",    xbins = 200, xmin=0.0, xmax=3000.0, ybins = 100, ymin=0.0, ymax=400.0)
            montool.defineHistogram('roi_nTracks, TIME_PattReco',path='EXPERT',type='TH2F',title="PattReco time; nTracks", xbins = 50,  xmin=0.0, xmax=200.0,  ybins = 100, ymin=0.0, ymax=400.0)
            montool.defineHistogram('TIME_Total',                path='EXPERT',type='TH1F',title="Total time (ms)",             xbins = 200, xmin=0.0, xmax=1000.0)
            montool.defineHistogram('TIME_PattReco',             path='EXPERT',type='TH1F',title="Pure PattReco time (ms)",     xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_SpacePointConversion', path='EXPERT',type='TH1F',title="SP Conversion time (ms)",     xbins =  20, xmin=0.0, xmax=20.0)
            montool.defineHistogram('TIME_Triplets',             path='EXPERT',type='TH1F',title="Triplets Making time (ms)",   xbins = 100, xmin=0.0, xmax=100.0)
            montool.defineHistogram('TIME_CmbTrack',             path='EXPERT',type='TH1F',title="Combined Tracking time (ms)", xbins = 200, xmin=0.0, xmax=400.0)
            montool.defineHistogram('TIME_TrackFitter',          path='EXPERT',type='TH1F',title="Track Fitter time (ms)",      xbins =  50, xmin=0.0, xmax=50.0)



    def addTrackHistograms(montool, name):
        if name in ['FS', 'JetFS', 'FullScan', 'fullScan', 'fullScanUTT', 'jet']:
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins = 1000, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins = 100, xmin=-0.5, xmax=9999.5)
        elif name=='fullScanLRT':
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins = 1000, xmin=-0.5, xmax=99999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins = 100, xmin=-0.5, xmax=5000.5)
        else:
            montool.defineHistogram('roi_nSeeds',     path='EXPERT',type='TH1F',title="Number of seeds",xbins =  100, xmin=-0.5, xmax=4999.5)
            montool.defineHistogram('roi_nTracks',    path='EXPERT',type='TH1F',title="Number of Tracks",xbins =  50, xmin=-0.5, xmax=199.5)

        montool.defineHistogram('roi_nZvertices', path='EXPERT',type='TH1F',title="Number of z vertices",xbins = 60 ,  xmin=-0.5, xmax=49.5)
        montool.defineHistogram('roi_zVertices',  path='EXPERT',type='TH1F',title="ZFinder Vertices",xbins = 501, xmin=-250, xmax=250)
        montool.defineHistogram('roi_nTrk_zVtx',  path='EXPERT',type='TH1F',title="Ntrk ZFinder Vertices",xbins = 100, xmin=-0.5, xmax=49.5)
        montool.defineHistogram('trk_nSiHits',    path='EXPERT',type='TH1F',title="Total number of Silicon Hits per Track",xbins = 20, xmin=-0.5, xmax=19.5)
        montool.defineHistogram('trk_nPIXHits',   path='EXPERT',type='TH1F',title="Number of Pixel Hits per Track",xbins = 10, xmin=-0.5, xmax=9.5)
        montool.defineHistogram('trk_nSCTHits',   path='EXPERT',type='TH1F',title="Number of SCT Hits per Track",xbins = 10, xmin=-0.5, xmax=9.5)
        montool.defineHistogram('trk_chi2dof',    path='EXPERT',type='TH1F',title="ChiSqd / nDoF",xbins = 100, xmin=0.0, xmax=5)
        montool.defineHistogram('trk_pt',         path='EXPERT',type='TH1F',title="pt",xbins = 100, xmin=-2.5e5, xmax=2.5e5)
        montool.defineHistogram('trk_phi0',       path='EXPERT',type='TH1F',title="phi",xbins = 100, xmin=-3.2, xmax=3.2)
        montool.defineHistogram('trk_eta',        path='EXPERT',type='TH1F',title="eta",xbins = 100, xmin=-5, xmax=5)
        montool.defineHistogram('trk_dPhi0',      path='EXPERT',type='TH1F',title="dphi",xbins = 160, xmin=-0.8, xmax=0.8)
        montool.defineHistogram('trk_dEta',       path='EXPERT',type='TH1F',title="deta",xbins = 160, xmin=-0.8, xmax=0.8)
        if name=="Cosmic":
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 100, xmin=-800, xmax=800)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 100, xmin=-800, xmax=800)
        elif name=='fullScanLRT':
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 100, xmin=-300, xmax=300)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 100, xmin=-550, xmax=550)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 100, xmin=-550, xmax=550)
        else:
            montool.defineHistogram('trk_a0',     path='EXPERT',type='TH1F',title="a0",xbins = 200, xmin=-10, xmax=10)
            montool.defineHistogram('trk_a0beam', path='EXPERT',type='TH1F',title="a0beam",xbins = 200, xmin=-10, xmax=10)
            montool.defineHistogram('trk_z0',     path='EXPERT',type='TH1F',title="z0",xbins = 200, xmin=-400, xmax=400)
            montool.defineHistogram('trk_z0beam', path='EXPERT',type='TH1F',title="z0beam",xbins = 200, xmin=-400, xmax=400)

    def addResidualHistograms(self):
        montool.defineHistogram('layer_IBL',                 path='EXPERT',type='TH1F',title="IBL layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_PixB',                path='EXPERT',type='TH1F',title="Pixel Barrel layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_PixE',                path='EXPERT',type='TH1F',title="Pixel Endcap layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_SCTB',                path='EXPERT',type='TH1F',title="SCT Barrel layer",xbins = 10, xmin=0., xmax=10.)
        montool.defineHistogram('layer_SCTE',                path='EXPERT',type='TH1F',title="SCT Endcap layer",xbins = 10, xmin=0., xmax=10.)
        #
        montool.defineHistogram('hit_IBLPhiResidual',        path='EXPERT',type='TH1F',title="IBL hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_IBLEtaResidual',        path='EXPERT',type='TH1F',title="IBL hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_IBLPhiPull',            path='EXPERT',type='TH1F',title="IBL hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_IBLEtaPull',            path='EXPERT',type='TH1F',title="IBL hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_PIXBarrelPhiResidual',  path='EXPERT',type='TH1F',title="Pixel Barrel hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelEtaResidual',  path='EXPERT',type='TH1F',title="Pixel Barrel hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL1PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL1EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L1 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL2PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL2EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L2 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelL3PhiResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXBarrelL3EtaResidual',path='EXPERT',type='TH1F',title="Pixel Barrel L3 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXBarrelPhiPull',      path='EXPERT',type='TH1F',title="Pixel Barrel hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_PIXBarrelEtaPull',      path='EXPERT',type='TH1F',title="Pixel Barrel hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_PIXEndcapPhiResidual',  path='EXPERT',type='TH1F',title="Pixel EC hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapEtaResidual',  path='EXPERT',type='TH1F',title="Pixel EC hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL1PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL1EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L1 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL2PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL2EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L2 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapL3PhiResidual',path='EXPERT',type='TH1F',title="Pixel EC L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_PIXEndcapL3EtaResidual',path='EXPERT',type='TH1F',title="Pixel EC L3 hit-track eta residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_PIXEndcapPhiPull',      path='EXPERT',type='TH1F',title="Pixel EC hit-track phi pull",xbins = 100, xmin=-5., xmax=5.)
        montool.defineHistogram('hit_PIXEndcapEtaPull',      path='EXPERT',type='TH1F',title="Pixel EC hit-track eta pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_SCTBarrelResidual',     path='EXPERT',type='TH1F',title="SCT Barrel hit-track residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL1PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL2PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL3PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelL4PhiResidual',path='EXPERT',type='TH1F',title="SCT Barrel L4 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTBarrelPull',         path='EXPERT',type='TH1F',title="SCT Barrel hit-track pull",xbins = 100, xmin=-5., xmax=5.)
        #
        montool.defineHistogram('hit_SCTEndcapResidual',     path='EXPERT',type='TH1F',title="SCT EC hit-track residual",xbins = 100, xmin=-1.0, xmax=1.0)
        montool.defineHistogram('hit_SCTEndcapL1PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L1 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL2PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L2 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL3PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L3 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL4PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L4 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL5PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L5 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL6PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L6 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL7PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L7 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL8PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L8 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapL9PhiResidual',path='EXPERT',type='TH1F',title="SCT Endcap L9 hit-track phi residual",xbins = 100, xmin=-0.5, xmax=0.5)
        montool.defineHistogram('hit_SCTEndcapPull',         path='EXPERT',type='TH1F',title="SCT EC hit-track pull",xbins = 100, xmin=-5., xmax=5.)

    def addUTTHistograms(montool):
        montool.defineHistogram('trk_dedx',           path='EXPERT',type='TH1F',title="Track dEdx (pT > 3 GeV)", xbins = 140, xmin=-0.5, xmax=6.5)
        montool.defineHistogram('trk_dedx_nusedhits', path='EXPERT',type='TH1F',title="Nr of used hits for dEdx",xbins =  11, xmin=-0.5, xmax=10.5)
        #
        montool.defineHistogram('disTrk_nVtx',        path='EXPERT',type='TH1F',title="Nr of Vertex for disTrk",xbins =  11, xmin=-0.5, xmax=10.5)
        montool.defineHistogram('disTrk_xVtx',        path='EXPERT',type='TH1F',title="X position of primary vertex for disTrk", xbins = 120, xmin=-1.2, xmax=1.2)
        montool.defineHistogram('disTrk_yVtx',        path='EXPERT',type='TH1F',title="Y position of primary vertex for disTrk", xbins = 120, xmin=-1.2, xmax=1.2)
        montool.defineHistogram('disTrk_zVtx',        path='EXPERT',type='TH1F',title="Z position of primary vertex for disTrk", xbins = 150, xmin=-150, xmax=150)
        #
        montool.defineHistogram('disFailTrk_n',       path='EXPERT',type='TH1F',title="Nr of disFailTrk", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disFailTrk_nclone',  path='EXPERT',type='TH1F',title="Nr of disFailTrk (after clone removal)", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disFailTrk_ncand',   path='EXPERT',type='TH1F',title="Nr of disFailTrk (after pre-selection)", xbins = 50, xmin=0, xmax=3000)
        montool.defineHistogram('disCombTrk_n',       path='EXPERT',type='TH1F',title="Nr of disCombTrk", xbins = 20, xmin=0, xmax=100)
        montool.defineHistogram('disCombTrk_nclone',  path='EXPERT',type='TH1F',title="Nr of disCombTrk (after clone removal)", xbins = 20, xmin=0, xmax=100)
        montool.defineHistogram('disCombTrk_ncand',   path='EXPERT',type='TH1F',title="Nr of disCombTrk (after pre-selection)", xbins = 20, xmin=0, xmax=100)
        #
        montool.defineHistogram('TIME_dEdxTrk',           path='EXPERT',type='TH1F',title="Large dEdx search (ms)",            xbins =  20, xmin=0.0, xmax=20.0)
        montool.defineHistogram('TIME_disTrkZVertex',     path='EXPERT',type='TH1F',title="UTT z-vertexing time (ms)",         xbins =  10, xmin=0.0, xmax=10.0)
        montool.defineHistogram('TIME_disappearingTrack', path='EXPERT',type='TH1F',title="Disappearing track reco time (ms)", xbins = 100, xmin=0.0, xmax=300.0)


    montool = GenericMonitoringTool(flags, HistPath = f"TrigFastTrackFinder_{name}")
    addSPHistograms(montool, name)
    addDataErrorHistograms(montool)
    addTimingHistograms(montool, name)
    addTrackHistograms(montool, name)
    if doResMon:
        addResidualHistograms(montool)
    uttMode = flags.Tracking.ActiveConfig.doHitDV or flags.Tracking.ActiveConfig.doDisappearingTrk
    if uttMode:
        addUTTHistograms(montool)

    return montool


from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def TrigZFinderCfg(flags : AthConfigFlags, numberingTool) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  zfargs = {}
  if flags.Tracking.ActiveConfig.name == "beamSpot" :
    zfargs = {
        'TripletMode'   : 1,
        'TripletDZ'     : 1,
        'PhiBinSize'    : 0.1,
        'UseOnlyPixels' : True,
        'MaxLayer'      : 3,
        'NumberOfPeaks' : 3
    }
  elif flags.Tracking.ActiveConfig.name == "jetSuper" :
    zfargs = {
        'TripletMode'   : 1,
        'TripletDZ'     : 10,
        'UseOnlyPixels' : True,
        'MaxLayer'      : 3,  # Uses only barrel pixels
        'NumberOfPeaks' : 1
    }
  
  acc.setPrivateTools(
      CompFactory.TrigZFinder( name="TrigZFinder",
                               LayerNumberTool=numberingTool,
                               FullScanMode = True, #TODO: know this from the RoI anyway - should set for every event
                               **zfargs
                              )
  )
  return acc

def TrigL2LayerNumberToolCfg(flags: AthConfigFlags, **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  kwargs.setdefault("UseNewLayerScheme", True)
  acc.setPrivateTools(CompFactory.TrigL2LayerNumberTool(**kwargs))
  return acc

def ITkTrigL2LayerNumberToolCfg(flags: AthConfigFlags, **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  kwargs.setdefault("UseNewLayerScheme", True)
  acc.setPrivateTools(CompFactory.TrigL2LayerNumberToolITk(**kwargs))
  return acc
  
def TrigSpacePointConversionToolCfg(flags: AthConfigFlags, **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  kwargs.setdefault("UseNewLayerScheme", True)

  if "layerNumberTool" not in kwargs:
    ntargs = {"UseNewLayerScheme" : kwargs.get("UseNewLayerScheme")}
    kwargs.setdefault("layerNumberTool",acc.popToolsAndMerge(TrigL2LayerNumberToolCfg(flags,**ntargs)))
  
  kwargs.setdefault("DoPhiFiltering", flags.Tracking.ActiveConfig.DoPhiFiltering)
  kwargs.setdefault("UseBeamTilt", False)
  kwargs.setdefault("PixelSP_ContainerName", "PixelTrigSpacePoints")
  kwargs.setdefault("SCT_SP_ContainerName", "SCT_TrigSpacePoints")
  kwargs.setdefault("UsePixelSpacePoints",flags.Tracking.ActiveConfig.UsePixelSpacePoints)

  from RegionSelector.RegSelToolConfig import regSelTool_SCT_Cfg, regSelTool_Pixel_Cfg

  if "RegSelTool_Pixel" not in kwargs:
    kwargs.setdefault("RegSelTool_Pixel", acc.popToolsAndMerge( regSelTool_Pixel_Cfg( flags)))

  if "RegSelTool_SCT" not in kwargs:
    kwargs.setdefault("RegSelTool_SCT", acc.popToolsAndMerge( regSelTool_SCT_Cfg( flags)))

  # Spacepoint conversion
  acc.setPrivateTools(CompFactory.TrigSpacePointConversionTool(**kwargs))

  return acc

def ITkTrigSpacePointConversionToolCfg(flags: AthConfigFlags, **kwargs) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  kwargs.setdefault("UseNewLayerScheme", True)
  
  if "layerNumberTool" not in kwargs:
    ntargs = {"UseNewLayerScheme" : kwargs.get("UseNewLayerScheme")}
    kwargs.setdefault("layerNumberTool",acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags,**ntargs)))

  kwargs.setdefault("DoPhiFiltering", flags.Tracking.ActiveConfig.DoPhiFiltering)
  kwargs.setdefault("UseBeamTilt", False)
  kwargs.setdefault("PixelSP_ContainerName", "ITkPixelTrigSpacePoints")
  kwargs.setdefault("UsePixelSpacePoints",flags.Tracking.ActiveConfig.UsePixelSpacePoints)
  kwargs.setdefault("UseSctSpacePoints",False)

  from RegionSelector.RegSelToolConfig import (regSelTool_ITkStrip_Cfg, regSelTool_ITkPixel_Cfg)
  
  if "RegSelTool_Pixel" not in kwargs:
    kwargs.setdefault("RegSelTool_Pixel", acc.popToolsAndMerge( regSelTool_ITkPixel_Cfg( flags) ))

  if "RegSelTool_SCT" not in kwargs:
    kwargs.setdefault("RegSelTool_SCT", acc.popToolsAndMerge( regSelTool_ITkStrip_Cfg( flags) ))

  # Spacepoint conversion
  acc.setPrivateTools(CompFactory.TrigSpacePointConversionTool(**kwargs))

  return acc

def ITkTrigSiTrackMaker_FTF_Cfg(flags, signature) -> ComponentAccumulator:
  acc = ComponentAccumulator()
  
  from InDetConfig.SiCombinatorialTrackFinderToolConfig import ITkSiCombinatorialTrackFinder_xkCfg
  CombinatorialTrackFinderTool = acc.popToolsAndMerge(ITkSiCombinatorialTrackFinder_xkCfg(flags,
                                                                                          name="ITkTrigSiComTrackFinder_"+signature,
                                                                                          PixelClusterContainer='ITkTrigPixelClusters',
                                                                                          SCT_ClusterContainer='ITkTrigStripClusters')
  )
  from InDetConfig.SiTrackMakerConfig import ITkSiTrackMaker_xkCfg
  trackMaker = acc.popToolsAndMerge(ITkSiTrackMaker_xkCfg(flags,
                                  name = "ITkTrigSiTrackMaker_FTF_",
                                  useBremModel = flags.Tracking.ActiveConfig.doBremRecoverySi,
                                  CombinatorialTrackFinder = CombinatorialTrackFinderTool)
  )
  
  acc.addPublicTool(trackMaker)
  return acc


def TrigFastTrackFinderCfg(flags: AthConfigFlags, name: str, RoIs: str, inputTracksName:str = None) -> ComponentAccumulator:
  acc = ComponentAccumulator()

  signature = flags.Tracking.ActiveConfig.input_name
  isCosmicConfig = (signature=="cosmics")

  
  # GPU offloading config begins - perhaps set from configure
  if flags.Trigger.InDetTracking.doGPU:
    
    inDetAccelSvc = CompFactory.TrigInDetAccelerationSvc("TrigInDetAccelerationSvc")
    inDetAccelSvc.useITkGeometry = flags.Detector.GeometryITk
    acc.addService(inDetAccelSvc)

    if flags.Detector.GeometryITk:
        acc.addPublicTool(CompFactory.TrigITkAccelerationTool(name = "TrigITkAccelerationTool_FTF"))
    else:
        acc.addPublicTool(CompFactory.TrigInDetAccelerationTool(name = "TrigInDetAccelerationTool_FTF"))
  # GPU offloading config ends

  useNewLayerNumberScheme = True

  if flags.Detector.GeometryITk:

    spTool = acc.popToolsAndMerge(ITkTrigSpacePointConversionToolCfg(flags))
    numberingTool = acc.popToolsAndMerge(ITkTrigL2LayerNumberToolCfg(flags))

    acc.merge(ITkTrigSiTrackMaker_FTF_Cfg(flags, signature))
    TrackMaker_FTF = acc.getPublicTool("ITkTrigSiTrackMaker_FTF_"+signature)

    acc.addPublicTool( CompFactory.TrigInDetTrackFitter( "TrigInDetTrackFitter_"+signature ) )
  
  else:
  
    spTool = acc.popToolsAndMerge(TrigSpacePointConversionToolCfg(flags,UseNewLayerScheme=useNewLayerNumberScheme))
    numberingTool = acc.popToolsAndMerge(TrigL2LayerNumberToolCfg(flags,UseNewLayerScheme=useNewLayerNumberScheme))

    from InDetConfig.SiTrackMakerConfig import TrigSiTrackMaker_xkCfg
    TrackMaker_FTF = acc.popToolsAndMerge(
        TrigSiTrackMaker_xkCfg(flags, name = 'InDetTrigSiTrackMaker_FTF_'+signature)
    )
    from TrkConfig.TrkRIO_OnTrackCreatorConfig import TrigRotCreatorCfg
    TrigRotCreator = acc.popToolsAndMerge(TrigRotCreatorCfg(flags))
    acc.addPublicTool(TrigRotCreator)

    acc.addPublicTool(
        CompFactory.TrigInDetTrackFitter(
            name = "TrigInDetTrackFitter_"+signature,
            doBremmCorrection = flags.Tracking.ActiveConfig.doBremRecoverySi,
            correctClusterPos = True,  #improved err(z0) estimates in Run 2
            ROTcreator = TrigRotCreator,
        )
    )
  
  acc.addPublicTool(TrackMaker_FTF)

  theTrigInDetTrackFitter = acc.getPublicTool("TrigInDetTrackFitter_"+signature)
  
  if (flags.Tracking.ActiveConfig.doZFinder):
    theTrigZFinder = acc.popToolsAndMerge(TrigZFinderCfg(flags,numberingTool))
  
  if not flags.Tracking.ActiveConfig.doZFinderOnly:
    
    if flags.Detector.GeometryITk:
      monTool = TrigFastTrackFinderMonitoringArg(flags, name = "trigfasttrackfinder_" + signature, doResMon=False)
      from TrkConfig.TrkTrackSummaryToolConfig import ITkTrackSummaryToolCfg
      trackSummaryTool = acc.popToolsAndMerge(ITkTrackSummaryToolCfg(flags))
    else:
      monTool = TrigFastTrackFinderMonitoring(flags)
      from TrkConfig.TrkTrackSummaryToolConfig import InDetTrigTrackSummaryToolCfg, InDetTrigFastTrackSummaryToolCfg
      if flags.Tracking.ActiveConfig.holeSearch_FTF :
        trackSummaryTool = acc.popToolsAndMerge(InDetTrigTrackSummaryToolCfg(flags,name="InDetTrigTrackSummaryTool"))
      else:
        trackSummaryTool = acc.popToolsAndMerge(InDetTrigFastTrackSummaryToolCfg(flags,name="InDetTrigFastTrackSummaryTool"))

    acc.addPublicTool(trackSummaryTool)

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    
    ftf = CompFactory.TrigFastTrackFinder(
        name = name,
        useNewLayerNumberScheme = useNewLayerNumberScheme,
        LayerNumberTool = numberingTool,
        useGPU = flags.Trigger.InDetTracking.doGPU,
        SpacePointProviderTool=spTool,
        MinHits = 5, #Only process RoI with more than 5 spacepoints
        Triplet_MinPtFrac = 1,
        Triplet_nMaxPhiSlice = 53 if "cosmics" not in flags.Tracking.ActiveConfig.name else 2,
        LRT_Mode = flags.Tracking.ActiveConfig.isLRT,
        dodEdxTrk = flags.Tracking.ActiveConfig.dodEdxTrk,
        doDisappearingTrk = flags.Tracking.ActiveConfig.doDisappearingTrk,
        Triplet_MaxBufferLength = 3,
        doSeedRedundancyCheck = flags.Tracking.ActiveConfig.doSeedRedundancyCheck,
        Triplet_D0Max         = flags.Tracking.ActiveConfig.Triplet_D0Max,
        Triplet_D0_PPS_Max    = flags.Tracking.ActiveConfig.Triplet_D0_PPS_Max,
        TrackInitialD0Max     = flags.Tracking.ActiveConfig.TrackInitialD0Max,
        TrackZ0Max            = flags.Tracking.ActiveConfig.TrackZ0Max,
        TripletDoPPS    = flags.Tracking.ActiveConfig.TripletDoPPS,
        TripletDoPSS    = False,
        pTmin           = flags.Tracking.ActiveConfig.minPT[0] if flags.Detector.GeometryITk else flags.Tracking.ActiveConfig.minPT,
        DoubletDR_Max   = flags.Tracking.ActiveConfig.DoubletDR_Max,
        SeedRadBinWidth = flags.Tracking.ActiveConfig.SeedRadBinWidth,
        initialTrackMaker = TrackMaker_FTF,
        trigInDetTrackFitter = theTrigInDetTrackFitter,
        doZFinder = flags.Tracking.ActiveConfig.doZFinder,
        TrackSummaryTool = trackSummaryTool,
        doCloneRemoval = flags.Tracking.ActiveConfig.doCloneRemoval,
        TracksName     = flags.Tracking.ActiveConfig.trkTracks_FTF,
        doResMon = flags.Tracking.ActiveConfig.doResMon,
        MonTool = monTool,
        Extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags)),
        RoIs = RoIs,
        FixSeedPhi = flags.Trigger.InDetTracking.fixSeedPhi,
        ITkMode = True if flags.Detector.GeometryITk else False,
    )
    
  ftf.LRT_D0Min = flags.Tracking.ActiveConfig.LRT_D0Min
  ftf.LRT_HardMinPt = flags.Tracking.ActiveConfig.LRT_HardPtMin
  
  ftf.UseTrigSeedML = flags.Tracking.ActiveConfig.UseTrigSeedML

  if flags.Trigger.InDetTracking.doGPU:
    ftf.TrigAccelerationSvc      = acc.getService("TrigInDetAccelerationSvc")
    if flags.Detector.GeometryITk:
        ftf.TrigAccelerationTool     = acc.getPublicTool("TrigITkAccelerationTool_FTF")
    else:
        ftf.TrigAccelerationTool     = acc.getPublicTool("TrigInDetAccelerationTool_FTF")

  if isCosmicConfig:
    ftf.Doublet_FilterRZ = False

  from TrigEDMConfig.TriggerEDM import recordable
  if flags.Tracking.ActiveConfig.dodEdxTrk:
    ftf.dEdxTrk = recordable("HLT_dEdxTrk")
    ftf.dEdxHit = recordable("HLT_dEdxHit")

  if flags.Tracking.ActiveConfig.doDisappearingTrk:
    ftf.DisTrkCand = recordable("HLT_DisTrkCand")
    from TrkConfig.TrkGlobalChi2FitterConfig import InDetTrigGlobalChi2FitterCfg
    InDetTrigTrackFitter = acc.popToolsAndMerge(InDetTrigGlobalChi2FitterCfg(flags))
    acc.addPublicTool(InDetTrigTrackFitter)
    ftf.DisTrackFitter = InDetTrigTrackFitter

  if flags.Tracking.ActiveConfig.doZFinder:
    ftf.doZFinderOnly = flags.Tracking.ActiveConfig.doZFinderOnly
    ftf.trigZFinder = theTrigZFinder
    ftf.zVertexResolution = 20 if flags.Tracking.ActiveConfig.name == "jetSuper" else 1
    ftf.zVertexResolutionEndcap = 150 if flags.Tracking.ActiveConfig.name == "jetSuper" else ftf.zVertexResolution
    ftf.doFastZVertexSeeding = True

  if inputTracksName:
    ftf.inputTracksName = inputTracksName
    
  acc.addEventAlgo(ftf)
  return acc
    

