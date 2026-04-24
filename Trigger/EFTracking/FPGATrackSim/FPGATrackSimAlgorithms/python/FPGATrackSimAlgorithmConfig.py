# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
'''
@author Riley Xu - rixu@cern.ch
@date Feb 6th 2020
@brief This file declares functions that configure the tools and algorithms in FPGATrackSimAlgorithms.
'''


## original monitor histograms
def FPGATrackSimDataPrepMonitoringCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    nbin=1000
    low=-0.5
    high=99999.5
    if flags.Trigger.FPGATrackSim.singleTrackSample:
        nbin=100
        high=99.5

    monTool.defineHistogram('regionID', path='EXPERT', type='TH1I', title='regionID', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('nHits_1st', path='EXPERT', type='TH1I', title='nHits_1st', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('nHits_1st_unmapped', path='EXPERT', type='TH1I', title='nHits_1st_unmapped', xbins=nbin, xmin=low, xmax=high)
    result.setPrivateTools(monTool)

    return result


## new monitoring tool
def FPGATrackSimTrackMonCfg(name,flags,variety='road'):
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import getPhiRange, getEtaRange, nameWithRegionSuffix
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    nbin=200000
    low=-0.5
    high=1999.5
    if flags.Trigger.FPGATrackSim.singleTrackSample:
        nbin=1000
        high=99.5
    
    phis=getPhiRange(flags)
    etas=getEtaRange(flags)
    phimin=phis[0]
    phimax=phis[1]
    etamin=etas[0]
    etamax=etas[1]

    phimin = phimin-flags.Trigger.FPGATrackSim.phiShift
    phimax = phimax-flags.Trigger.FPGATrackSim.phiShift
    
    ## (name should match FPGATrackSimAlgorithm/src/FPGATrackSimTrackMonitor.cxx)
    ## hisotgram for roads
    if variety=='road':
        monTool.defineHistogram('nRoads', path='EXPERT', type='TH1I', title='nRoads', xbins=nbin, xmin=low, xmax=high)
        monTool.defineHistogram('layerIDs', path='EXPERT', type='TH1I', title='layerIDs', xbins=20, xmin=-0.5, xmax = 19.5)
        monTool.defineHistogram('nLayers', path='EXPERT', type='TH1I', title='nLayers', xbins=nbin, xmin=low, xmax=high)
        
    ## hisotgram for tracks
    elif variety=='track':
        # number of tracks
        monTool.defineHistogram('nTracks', path='EXPERT', type='TH1I', title='nTracks', xbins=nbin, xmin=low, xmax=high)
        monTool.defineHistogram('chi2_all', path='EXPERT', type='TH1F', title='chi2_all', xbins=nbin, xmin=low, xmax=high)
        monTool.defineHistogram('best_chi2', path='EXPERT', type='TH1F', title='best_chi2', xbins=nbin, xmin=low, xmax=high)
    
    # number of hits
    monTool.defineHistogram('nHits', path='EXPERT', type='TH1I', title='nHits', xbins=20, xmin=-0.5, xmax = 19.5)

    # all tracks (all hits with any number of hits)
    nbinchi2 = 10000
    xmaxchi2 = {'chi2':10.0, 'chi2Eta': 1.0, 'chi2Phi' : 0.1}
    for var in ['chi2','chi2Eta', 'chi2Phi']:
        for nhit in ['','_4','_5']:
            for sel in ['','_best']:
                monTool.defineHistogram(f'{var}{sel}{nhit}', path='EXPERT', type='TH1F', title=f'{var}{sel}{nhit}', xbins=nbinchi2, xmin=0.0, xmax=xmaxchi2[var])
   
    ## efficiency histograms
    monTool.defineHistogram(f'eff_{variety},pT_zoom', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs pT_zoom', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram(f'eff_{variety},pT', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs pT', xbins=20, xmin=0, xmax=100)
    monTool.defineHistogram(f'eff_{variety},eta', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs eta', xbins=20, xmin=etamin, xmax=etamax)
    monTool.defineHistogram(f'eff_{variety},phi', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs phi', xbins=20, xmin=phimin, xmax=phimax)
    monTool.defineHistogram(f'eff_{variety},d0', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs d0', xbins=20, xmin=-2.0, xmax=2.0)
    monTool.defineHistogram(f'eff_{variety},z0', path='EXPERT', type='TEfficiency', title=f'eff_{variety} vs z0', xbins=20, xmin=-150.0, xmax=150.0)

    ## creates a c++ instance of the FPGATrackSimMonitor (added in FPGATrackSimTrackMonitor.h/.cxx) tool via CompFactory
    trackmon = CompFactory.FPGATrackSimTrackMonitor(nameWithRegionSuffix(flags,f"{variety}_monitor_{name}"))
    
    ## attaches a generic monitoring tool (the tool can still use the standard athena monitoring infraestructure)
    trackmon.MonTool = monTool

    ## set as a private tool
    result.setPrivateTools(trackmon)

    return result


def FPGATrackSimOverlapRemovalToolMonitoringCfg(flags):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')

    monTool.defineHistogram('ntrack_passOR', path='EXPERT', type='TH1I', title='ntrack_passOR', xbins=20, xmin=0, xmax=10)
    monTool.defineHistogram('barcodeFrac_passOR', path='EXPERT', type='TH1I', title='barcodeFrac_passOR', xbins=20, xmin=0, xmax=1.5)
  
    result.setPrivateTools(monTool)

    return result


def FPGATrackSimSecondStageAlgMonitoringCfg(flags):
    from FPGATrackSimConfTools.FPGATrackSimDataPrepConfig import getPhiRange,getEtaRange    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool')
    nbin=100
    low=-0.5
    high=99.5

    phis=getPhiRange(flags)
    etas=getEtaRange(flags)
    phimin=phis[0]
    phimax=phis[1]
    etamin=etas[0]
    etamax=etas[1]    
    
    phimin = phimin-flags.Trigger.FPGATrackSim.phiShift
    phimax = phimax-flags.Trigger.FPGATrackSim.phiShift    
    
    monTool.defineHistogram('nHits_2nd', path='EXPERT', type='TH1I', title='nHits_2nd', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('nHits_2nd_unmapped', path='EXPERT', type='TH1I', title='nHits_2nd_unmapped', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('nroads_2nd', path='EXPERT', type='TH1I', title='nroads_2nd', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('nroads_2nd_postfilter', path='EXPERT', type='TH1I', title='nroads_2nd_postfilter', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('layerIDs_2nd', path='EXPERT', type='TH1I', title='layerIDs_2nd', xbins=20, xmin=-0.5, xmax = 19.5)
    monTool.defineHistogram('layerIDs_2nd_best', path='EXPERT', type='TH1I', title='layerIDs_2nd_best', xbins=20, xmin=-0.5, xmax = 19.5)    
    monTool.defineHistogram('completed_roads_NN', path='EXPERT', type='TH1I', title='completed_roads_NN', xbins=20, xmin=-0.5, xmax = 19.5)
    monTool.defineHistogram('chi2_2nd_all', path='EXPERT', type='TH1F', title='chi2_2nd_all', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('chi2_2nd_afterOLR', path='EXPERT', type='TH1F', title='chi2_2nd_afterOLR', xbins=nbin, xmin=0, xmax=10.0)
    monTool.defineHistogram('best_chi2_2nd', path='EXPERT', type='TH1F', title='best_chi2_2nd', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('ntrack_2nd', path='EXPERT', type='TH1F', title='ntrack_2nd', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('ntrack_2nd_afterOLR', path='EXPERT', type='TH1F', title='ntrack_2nd_afterOLR', xbins=nbin, xmin=low, xmax=high)
    monTool.defineHistogram('eff_road_2nd,pT', path='EXPERT', type='TEfficiency', title='eff_road_pt', xbins=20, xmin=0, xmax=100)
    monTool.defineHistogram('eff_track_2nd,pT', path='EXPERT', type='TEfficiency', title='eff_track_pt', xbins=20, xmin=0, xmax=100)
    monTool.defineHistogram('eff_track_chi2_2nd,pT', path='EXPERT', type='TEfficiency', title='eff_track_chi2_pt', xbins=20, xmin=0, xmax=100)
    monTool.defineHistogram('eff_road_2nd,pT_zoom', path='EXPERT', type='TEfficiency', title='eff_road_pt_zoom', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram('eff_track_2nd,pT_zoom', path='EXPERT', type='TEfficiency', title='eff_track_pt_zoom', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram('eff_track_chi2_2nd,pT_zoom', path='EXPERT', type='TEfficiency', title='eff_track_chi2_pt_zoom', xbins=10, xmin=0, xmax=10)
    monTool.defineHistogram('eff_road_2nd,eta', path='EXPERT', type='TEfficiency', title='eff_road_eta', xbins = 20, xmin=etamin, xmax=etamax)
    monTool.defineHistogram('eff_track_2nd,eta', path='EXPERT', type='TEfficiency', title='eff_track_eta', xbins = 20, xmin=etamin, xmax=etamax)
    monTool.defineHistogram('eff_track_chi2_2nd,eta', path='EXPERT', type='TEfficiency', title='eff_track_chi2_eta', xbins = 20, xmin=etamin, xmax=etamax)
    monTool.defineHistogram('eff_road_2nd,phi', path='EXPERT', type='TEfficiency', title='eff_road_phi', xbins = 20, xmin=phimin, xmax=phimax)
    monTool.defineHistogram('eff_track_2nd,phi', path='EXPERT', type='TEfficiency', title='eff_track_phi', xbins = 20, xmin=phimin, xmax=phimax)
    monTool.defineHistogram('eff_track_chi2_2nd,phi', path='EXPERT', type='TEfficiency', title='eff_track_chi2_phi', xbins = 20, xmin=phimin, xmax=phimax)
    monTool.defineHistogram('eff_road_2nd,d0', path='EXPERT', type='TEfficiency', title='eff_road_d0', xbins = 20, xmin = -2.0, xmax = 2.0)
    monTool.defineHistogram('eff_track_2nd,d0', path='EXPERT', type='TEfficiency', title='eff_track_d0', xbins = 20, xmin = -2.0, xmax = 2.0)
    monTool.defineHistogram('eff_track_chi2_2nd,d0', path='EXPERT', type='TEfficiency', title='eff_track_chi2_d0', xbins = 20, xmin = -2.0, xmax = 2.0)
    monTool.defineHistogram('eff_road_2nd,z0', path='EXPERT', type='TEfficiency', title='eff_road_z0', xbins = 20, xmin = -150, xmax = 150.0)
    monTool.defineHistogram('eff_track_2nd,z0', path='EXPERT', type='TEfficiency', title='eff_track_z0', xbins = 20, xmin = -150.0, xmax = 150.0)
    monTool.defineHistogram('eff_track_chi2_2nd,z0', path='EXPERT', type='TEfficiency', title='eff_track_chi2_z0', xbins = 20, xmin = -150.0, xmax = 150.0)

    result.setPrivateTools(monTool)

    return result
