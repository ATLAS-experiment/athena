# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
'''
@author Riley Xu - rixu@cern.ch
@date Feb 6th 2020
@brief This file declares functions that configure the tools and algorithms in FPGATrackSimAlgorithms.
'''

from FPGATrackSimHough.FPGATrackSimHoughConf import FPGATrackSimRoadUnionTool, FPGATrackSimHoughTransformTool, FPGATrackSimHough1DShiftTool
from FPGATrackSimLRT.FPGATrackSimLRTConf import FPGATrackSimHoughTransform_d0phi0_Tool, FPGATrackSimLLPDoubletHoughTransformTool

import FPGATrackSimMaps.FPGATrackSimMapConfig as FPGATrackSimMaps

def intList(string):
    return [ int(v) for v in string.split(',') if v != '' ]

def floatList(floatStr):
    '''
    Converts a list of floats in string form to an actual list. This is needed since
    trfArgClasses doesn't have a argFloatList class
    '''
    return [ float(v) for v in floatStr.split(',') if v != '' ]

def applyTag(obj, tag):
    '''
    Applies the parameters in the supplied tag to the given FPGATrackSimAlgorithm object.
    '''

    params = { # List of configurable parameters for the given object type
        'FPGATrackSimPatternMatchTool': [
                'max_misses',
        ],
        'FPGATrackSimSectorMatchTool': [
                'max_misses',
        ],
        'FPGATrackSimTrackFitterTool': [
                'chi2DofRecoveryMin',
                'chi2DofRecoveryMax',
                'doMajority',
                'nHits_noRecovery',
                'GuessHits',
                'DoMissingHitsChecks',
                'IdealCoordFitType',
                'DoDeltaGPhis'
        ],
    }

    for param in params[obj.getType()]:
        setattr(obj, param, tag[param])


def addHoughTool(map_tag, algo_tag, doHitTracing):
    '''
    Creates and adds the Hough transform tools to the tool svc
    '''

    union = FPGATrackSimRoadUnionTool()

    if algo_tag['xVar'] == 'phi':
        x_min = algo_tag['phi_min']
        x_max = algo_tag['phi_max']
    else:
        raise NotImplementedError("x != phi")

    x_buffer = (x_max - x_min) / algo_tag['xBins'] * algo_tag['xBufferBins']
    x_min -= x_buffer
    x_max += x_buffer

    if algo_tag['yVar'] == 'q/pt':
        y_min = algo_tag['qpt_min']
        y_max = algo_tag['qpt_max']
    else:
        raise NotImplementedError("y != q/pt")

    y_buffer = (y_max - y_min) / algo_tag['yBins'] * algo_tag['yBufferBins']
    y_min -= y_buffer
    y_max += y_buffer

    tools = []
    nSlice = FPGATrackSimMaps.getNSubregions(map_tag) if algo_tag['slicing'] else 1

    d0_list = algo_tag['d0_slices'] or [0]

    for d0 in d0_list:
        for iSlice in range(nSlice):
            t = FPGATrackSimHoughTransformTool("HoughTransform_" + str(d0) + '_' + str(iSlice))

            t.subRegion = iSlice if nSlice > 1 else -1
            t.phi_min = x_min
            t.phi_max = x_max
            t.qpT_min = y_min
            t.qpT_max = y_max
            t.d0_min = d0
            t.d0_max = d0
            t.nBins_x = algo_tag['xBins'] + 2 * algo_tag['xBufferBins']
            t.nBins_y = algo_tag['yBins'] + 2 * algo_tag['yBufferBins']
            t.threshold = algo_tag['threshold']
            t.convolution = algo_tag['convolution']
            t.combine_layers = algo_tag['combine_layers']
            t.scale = algo_tag['scale']
            t.convSize_x = algo_tag['convSize_x']
            t.convSize_y = algo_tag['convSize_y']
            t.traceHits = doHitTracing
            t.localMaxWindowSize = algo_tag['localMaxWindowSize']
            t.fieldCorrection = algo_tag['fieldCorrection']
            if algo_tag['DoDeltaGPhis']:
                t.IdealGeoRoads = True
            try:
                t.hitExtend_x = algo_tag['hitExtend_x']
            except Exception:
                t.hitExtend_x = intList(algo_tag['hitExtend_x']) 

            tools.append(t)

    union.tools = tools # NB don't manipulate union.tools directly; for some reason the attributes get unset. Only set like is done here


    from AthenaCommon.AppMgr import ToolSvc 
    ToolSvc += union

    return union

# This is only for LRT, so draw on the LRT variables.
# That way we don't interfere with the standard Hough for first pass tracking,
# if using it.
def addHough_d0phi0_Tool(map_tag, algo_tag, doHitTracing):
    '''
    Creates and adds the Hough transform tools to the tool svc
    '''

    union = FPGATrackSimRoadUnionTool("LRTRoadUnionTool")

    if algo_tag['lrt_straighttrack_xVar'] == 'phi':
        x_min = algo_tag['lrt_straighttrack_phi_min']
        x_max = algo_tag['lrt_straighttrack_phi_max']
    else:
        raise NotImplementedError("x != phi")

    x_buffer = (x_max - x_min) / algo_tag['lrt_straighttrack_xBins'] * algo_tag['lrt_straighttrack_xBufferBins']
    x_min -= x_buffer
    x_max += x_buffer

    if algo_tag['lrt_straighttrack_yVar'] == 'd0':
        y_min = algo_tag['lrt_straighttrack_d0_min']
        y_max = algo_tag['lrt_straighttrack_d0_max']
    else:
        raise NotImplementedError("y != d0")

    y_buffer = (y_max - y_min) / algo_tag['lrt_straighttrack_yBins'] * algo_tag['lrt_straighttrack_yBufferBins']
    y_min -= y_buffer
    y_max += y_buffer

    tools = []
    nSlice = FPGATrackSimMaps.getNSubregions(map_tag) if algo_tag['lrt_straighttrack_slicing'] else 1

    for iSlice in range(nSlice):
        t = FPGATrackSimHoughTransform_d0phi0_Tool("HoughTransform_d0phi0_" + str(iSlice))

        t.subRegion = iSlice if nSlice > 1 else -1
        t.phi_min = x_min
        t.phi_max = x_max
        t.d0_min = y_min
        t.d0_max = y_max
        t.nBins_x = algo_tag['lrt_straighttrack_xBins'] + 2 * algo_tag['lrt_straighttrack_xBufferBins']
        t.nBins_y = algo_tag['lrt_straighttrack_yBins'] + 2 * algo_tag['lrt_straighttrack_yBufferBins']
        t.threshold = algo_tag['lrt_straighttrack_threshold']
        t.convolution = algo_tag['lrt_straighttrack_convolution']
        t.combine_layers = algo_tag['lrt_straighttrack_combine_layers']
        t.scale = algo_tag['lrt_straighttrack_scale']
        t.convSize_x = algo_tag['lrt_straighttrack_convSize_x']
        t.convSize_y = algo_tag['lrt_straighttrack_convSize_y']
        t.traceHits = doHitTracing
        t.stereo = algo_tag['lrt_straighttrack_stereo']
        t.localMaxWindowSize = algo_tag['lrt_straighttrack_localMaxWindowSize']

        try:
            t.hitExtend_x = algo_tag['lrt_straighttrack_hitExtend_x']
        except Exception:
            t.hitExtend_x = intList(algo_tag['lrt_straighttrack_hitExtend_x']) 


        tools.append(t)

    union.tools = tools # NB don't manipulate union.tools directly; for some reason the attributes get unset. Only set like is done here

    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += union

    return union

def addHough1DShiftTool(map_tag, algo_tag):

    union = FPGATrackSimRoadUnionTool()

    tools = []
    nSlice = FPGATrackSimMaps.getNSubregions(map_tag) if algo_tag['slicing'] else 1

    splitpt=algo_tag['splitpt']
    for ptstep in range(splitpt):
        qpt_min = algo_tag['qpt_min']
        qpt_max = algo_tag['qpt_max']
        lowpt = qpt_min + (qpt_max-qpt_min)/splitpt*ptstep
        highpt = qpt_min + (qpt_max-qpt_min)/splitpt*(ptstep+1)

        for iSlice in range(nSlice):
            tool = FPGATrackSimHough1DShiftTool("Hough1DShift_" + str(iSlice)+(("_pt{}".format(ptstep))  if splitpt>1 else ""))
            tool.subRegion = iSlice if nSlice > 1 else -1
            if algo_tag['radiiFile'] is not None:
                tool.radiiFile = algo_tag['radiiFile']
            tool.phi_min = algo_tag['phi_min']
            tool.phi_max = algo_tag['phi_max']
            tool.qpT_min = lowpt
            tool.qpT_max = highpt
            tool.nBins = algo_tag['xBins']
            tool.useDiff = True
            tool.variableExtend = True
            tool.phiRangeCut = algo_tag['phiRangeCut']
            tool.d0spread=-1.0 # mm
            tool.iterStep = 0 
            tool.iterLayer = 7 
            tool.threshold = algo_tag['threshold'][0] 

            try:
                tool.hitExtend = algo_tag['hitExtend_x']
            except Exception:
                tool.hitExtend = floatList(algo_tag['hitExtend_x'])

            tools.append(tool)

    union.tools = tools # NB don't manipulate union.tools directly; for some reason the attributes get unset. Only set like is done here

    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += union
    return union

def addLRTDoubletFPGATrackSimool(algo_tag):
    tool = FPGATrackSimLLPDoubletHoughTransformTool()
    tool.nBins_x   = algo_tag['lrt_doublet_d0_bins']
    tool.d0_range  = algo_tag['lrt_doublet_d0_range']
    tool.nBins_y   = algo_tag['lrt_doublet_qpt_bins']
    tool.qpT_range = algo_tag['lrt_doublet_qpt_range']
    from AthenaCommon.AppMgr import ToolSvc
    ToolSvc += tool
    return tool


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

    nbin=1000
    low=-0.5
    high=99999.5
    if flags.Trigger.FPGATrackSim.singleTrackSample:
        nbin=100
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
        monTool.defineHistogram('nTracks', path='EXPERT', type='TH1I', title='nTracks', xbins=nbin, xmin=low, xmax=high)
        monTool.defineHistogram('chi2_all', path='EXPERT', type='TH1F', title='chi2_all', xbins=nbin, xmin=low, xmax=high)
        monTool.defineHistogram('best_chi2', path='EXPERT', type='TH1F', title='best_chi2', xbins=nbin, xmin=low, xmax=high)
    
    ## efficiency histograms  ## cannot add x-y labels
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
