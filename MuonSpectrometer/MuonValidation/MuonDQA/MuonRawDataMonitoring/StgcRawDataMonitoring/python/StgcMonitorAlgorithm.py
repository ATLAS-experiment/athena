#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration #
#

from AthenaConfiguration.ComponentFactory import CompFactory
from StgcRawDataMonitoring.StgcMonitorUtils import columnLabels_AL, columnLabels_CL, columnLabels_AS, columnLabels_CS, rowLabels, wireGroupNumberLabel, FebLabels
import math

def sTgcMonitoringConfig(inputFlags,NSW_PadTrigKey=''):
    '''Function to configures some algorithms in the monitoring system.'''
    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # Make sure muon geometry is configured
    from MuonConfig.MuonGeometryConfig import MuonGeoModelCfg
    result.merge(MuonGeoModelCfg(inputFlags))

    # The following class will make a sequence, configure algorithms, and link them to GenericMonitoringTools

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags, 'StgcAthMonitorCfg')

    # Adding an algorithm to the helper.
    sTgcMonAlg = helper.addAlgorithm(CompFactory.sTgcRawDataMonAlg,'sTgcMonAlg')
    sTgcMonAlg.cutPt = 15000.
    sTgcMonAlg.cutEtaDown = 1.3
    sTgcMonAlg.cutEtaUp = 2.4
    sTgcMonAlg.minDeltaR = 0.1
    sTgcMonAlg.cutTriggerPhiId = 63
    sTgcMonAlg.cutTriggerBandId = 255
    sTgcMonAlg.NSW_PadTriggerDataKey = NSW_PadTrigKey

    globalPath = 'Muon/MuonRawDataMonitoring/STG/'

    # Layered and occupancy histograms
    tech          = ['pad', 'strip', 'wire']
    side          = ['A', 'C']
    size          = ['L', 'S']    
    stationEtaMax = 3
    sectorMax     = 16
    layerMax      = 8

    # Custom labels
    # Pad trigger occupancy
    columnLabels = [columnLabels_AL, columnLabels_AS, columnLabels_CL, columnLabels_CS]
    
    # Shifter
    OverviewGroup  = helper.addGroup(sTgcMonAlg, 'Overview', globalPath + 'Shifter/Overview')
    OverviewGroup.defineHistogram('muonRecoTriggerMatch,etaRecoMuonEff,phiRecoMuonEff;padTrigger_Efficiency_per_etaPhi', type = 'TEfficiency', title = '; #eta (reco); #phi (reco); Pad trigger efficiency wrt. reco. muon', path = '', xbins = 100, xmin = -3., xmax = 3., ybins = 100, ymin = -math.pi, ymax = math.pi, opt = 'kAlwaysCreate')
    OverviewGroup.defineHistogram('etaRecoMuon,phiRecoMuon;recoMuon_Map_per_etaPhi', type = 'TH2F', title = '; #eta (reco); #phi (reco); Entries', path = '', xbins = 100, xmin = -3., xmax = 3., ybins = 100, ymin = -math.pi, ymax = math.pi, opt = 'kAlwaysCreate')  
    OverviewGroup.defineHistogram('etaPadTrigger,phiPadTrigger;padTrigger_Map_per_etaPhi', type = 'TH2F', title = '; #eta (trig); #phi (trig); Entries', path = '', xbins = 100, xmin = -3., xmax = 3., ybins = 100, ymin = -math.pi, ymax = math.pi, opt = 'kAlwaysCreate')
    OverviewGroup.defineHistogram('etaPadTrigger;padTrigger_Map_per_eta', type = 'TH1F', title = '; #eta (trig); Entries', path = '', xbins = 100, xmin = -3., xmax = 3., opt = 'kAlwaysCreate')       
    
    #OccupancyShifterGroup = helper.addGroup(sTgcMonAlg, 'OccupancyShifter', globalPath + 'Expert/Occupancy/')
    sTgcTimingGroup = helper.addGroup(sTgcMonAlg, 'sTgcTiming', globalPath + 'Expert/Timing/')
    LBShifterGroup = helper.addGroup(sTgcMonAlg, 'LBShifterGroup', globalPath + 'Shifter/OccupancyGlobal/')            
    sTgcPadTriggerShifterGroup = helper.addGroup(sTgcMonAlg, 'padTriggerShifter', globalPath + 'Shifter/')
    OverviewGroup.defineHistogram('lb,sector;OccupancySector_vs_LB', type = 'TH2F', title = '; LB; Sector; Number of triggers', path = '', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 2*sectorMax + 1, ymin = -sectorMax - 0.5, ymax = sectorMax + 0.5, opt = 'kAlwaysCreate,kAddBinsDynamically', weight = 'numberOfTriggers')
      
    # Expert
    OccupancyGroup = helper.addGroup(sTgcMonAlg, 'Occupancy', globalPath + 'Expert/Occupancy')
    LBExpertGroup = helper.addGroup(sTgcMonAlg, 'LBExpertGroup', globalPath + 'Expert/Lumiblock/')            
    sTgcPadTriggerExpertGroup = helper.addGroup(sTgcMonAlg, 'padTriggerExpert', globalPath + 'Expert/')
    sTgcPadTriggerExpertGroup.defineHistogram('sector,hitPfebs;OccupancypFEB_vs_Sector', type = 'TH2F', title = '; Sector; Hit pFEB; Pad Trigger hits associated to reco muons', path = 'PadTrigger/Hits', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 25, ymin = -0.5, ymax = 24.5, opt = 'kAlwaysCreate')
    sTgcPadTriggerExpertGroup.defineHistogram('sector,hitRelBCID;relBCID_vs_Sector', type = 'TH2F', title = '; Sector; Hit relBCID; Pad Trigger hits associated to reco muons', path = 'PadTrigger/Hits', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 7, ymin = -0.5, ymax = 6.5, opt = 'kAlwaysCreate')
    sTgcPadTriggerExpertGroup.defineHistogram('lb,relBCID;RelBCID_vs_LB', type = 'TH2F', title = '; LB; Trigger relBCID; Pad Trigger hits', path = 'PadTrigger/Triggers', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 7, ymin = -0.5, ymax = 6.5, opt = 'kAlwaysCreate,kAddBinsDynamically')
    padTriggerOccupancyGroup = helper.addGroup(sTgcMonAlg, 'padTriggerOccupancy', globalPath + 'Expert/PadTrigger/Hits/')

    for tIdx in tech:
        OccupancyGroup.defineHistogram(f'{tIdx}Sector,{tIdx}Feb;Feb_vs_sector_{tIdx}', type = 'TH2F', title = ';Sector; FEB; Hits', path = f'{tIdx[0].capitalize()+tIdx[1:]}', xbins = 33, xmin = -16.5, xmax = 16.5, ybins = 24, ymin = -0.5, ymax = 23.5, ylabels = FebLabels, opt='kAlwaysCreate')  
        LBShifterGroup.defineHistogram(f'{tIdx}Lb,{tIdx}Sector;Summary_LB_vs_Sector_{tIdx}',type = 'TH2F',title = ';LB; Sector; Hits', path = '', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 33, ymin = -16.5, ymax = 16.5, opt = 'kAlwaysCreate, kAddBinsDynamically')

    layerCounter=0
    for layerIndex in range(1, layerMax + 1):
        layerCounter+=1
        sTgcPadTriggerExpertGroup.defineHistogram(f'stripTrackSectorSided_layer_{layerIndex},stripTrackClusterSize_layer_{layerIndex};Strip_cluster_size_ontrk_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Strip Cluster Size (on-track); Hits', path = 'StripClusterSize', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 13, ymin = -0.5, ymax = 12.5, opt = 'kAlwaysCreate')
        sTgcTimingGroup.defineHistogram(f'padTrackSectorSided_layer_{layerIndex},padTrackTiming_layer_{layerIndex};All_pad_timing_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Pad Timing (on-track) [ns]; Hits', path = 'Pad/Layer', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 225, ymin = -100., ymax = 125., opt = 'kAlwaysCreate')
        sTgcTimingGroup.defineHistogram(f'stripTrackSectorSided_layer_{layerIndex},stripTrackTiming_layer_{layerIndex};All_strip_timing_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Strip Cluster Timing (on-track) [ns]; Hits', path = 'Strip/Layer', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 225, ymin = -100., ymax = 125., opt = 'kAlwaysCreate')
        sTgcTimingGroup.defineHistogram(f'wireGroupTrackSectorSided_layer_{layerIndex},wireGroupTrackTiming_layer_{layerIndex};All_wire_timing_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Wire Group timing (on-track) [ns]; Hits', path = 'Wire/Layer', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 225, ymin = -100., ymax = 125., opt = 'kAlwaysCreate')
        OccupancyGroup.defineHistogram(f'sector_layer_{layerIndex},padNumber_layer_{layerIndex};Pad_ch_occupancy_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Pad Number; Hits', path = 'Pad', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 317, ymin = 0., ymax = 317., opt = 'kAlwaysCreate')
        OccupancyGroup.defineHistogram(f'sector_layer_{layerIndex},stripNumber_layer_{layerIndex};Strip_ch_occupancy_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Sector; Strip Number; Hits', path = 'Strip', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 1130, ymin = 0., ymax = 1130., opt = 'kAlwaysCreate')
        OccupancyGroup.defineHistogram(f'wireGroupNumber_layer_{layerIndex},stationEta_layer_{layerIndex};Wire_ch_occupancy_per_sector_Layer{layerIndex}', type = 'TH2F', title = f'L{layerIndex}; Wire Group Number; Quad; Hits', path = 'Wire', xbins = 58*sectorMax + 1, xmin = -0.5, xmax = 58*sectorMax + 0.5, xlabels = wireGroupNumberLabel, ybins = 2*stationEtaMax + 1, ymin = -stationEtaMax - 0.5, ymax = stationEtaMax + 0.5, opt = 'kAlwaysCreate')

        columnLabelsCounter=0
        sizeCounter=0
        for sizeIndex in size:
            sizeCounter+=1
            for sideIndex in side:
                padTriggerOccupancyGroup.defineHistogram(f'padPhi_{sideIndex}_{sizeIndex}_layer_{layerIndex},padEta_{sideIndex}_{sizeIndex}_layer_{layerIndex};padEtaPhiOcc_{layerIndex}{sideIndex}{sizeIndex}', type = 'TH2F', title = f'{layerIndex}{sideIndex}{sizeIndex}; Pad column; Pad row; Hits', path = 'padTriggerOccupancy', xbins = 71, xmin = 0.5, xmax = 71.5, xlabels = columnLabels[columnLabelsCounter], ybins = 56, ymin = 0.5, ymax = 56.5, ylabels = rowLabels, opt = 'kAlwaysCreate')
                columnLabelsCounter += 1
                if sizeCounter==1:
                    sTgcPadTriggerShifterGroup.defineHistogram(f'hitLayer,xPosStrip_{sideIndex}_layer_{layerIndex},yPosStrip_{sideIndex}_layer_{layerIndex};strip_efficiency_per_mm_squared_Wheel{sideIndex}_layer{layerIndex}', type = 'TEfficiency', title = f'{sideIndex}L{layerIndex}; sTgc-GlobalX-Strip (on track) [mm]; sTgc-GlobalY-Strip (on track) [mm]; Efficiency sTGC strip {sideIndex}L{layerIndex}', path = 'StripEfficiency', xbins = 500, xmin = -5000., xmax = 5000., ybins = 500, ymin = -5000., ymax = 5000., opt = 'kAlwaysCreate')
                if layerCounter==1:
                    sTgcPadTriggerExpertGroup.defineHistogram(f'phiIds_{sideIndex}_{sizeIndex},bandIds_{sideIndex}_{sizeIndex};bandIds_vs_phiIds_Side{sideIndex}_Size{sizeIndex}', type = 'TH2F', title = f'{sideIndex}{sizeIndex}; Trigger phiID; Trigger bandID; Pad Trigger hits', path = 'PadTrigger/Triggers', xbins = 65, xmin = -32.5, xmax = 32.5, ybins = 101, ymin = -0.5, ymax = 100.5, opt = 'kAlwaysCreate')

    sideCounter=0
    for sideIndex in side:
        sideCounter+=1
        
        sectorCounter=0
        for sectorIndex in range(1, sectorMax + 1):
            sectorCounter+=1
            efficiencyGlobalRgroup = helper.addGroup(sTgcMonAlg, f'rPosStrip_{sideIndex}{sectorIndex}', globalPath + 'Expert/Efficiency/')
            sTgcPadTriggerExpertGroup.defineHistogram(f'lb_{sideIndex}_sector_{sectorIndex},bandIds_{sideIndex}_sector_{sectorIndex};OccupancyBandId_vs_LB_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; LB; Trigger bandID; number of triggers', path = 'PadTrigger/Triggers/OccupancyBandIDvsLB', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 101, ymin = -0.5, ymax = 100.5, opt = 'kAlwaysCreate,kAddBinsDynamically', weight = f'numberOfTriggers_{sideIndex}_sector_{sectorIndex}')
            sTgcPadTriggerExpertGroup.defineHistogram(f'phiIds_{sideIndex}_sector_{sectorIndex},bandIds_{sideIndex}_sector_{sectorIndex};bandIds_vs_phiIds_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; Trigger phiID; Trigger bandID; Pad Trigger hits', path = 'PadTrigger/Triggers/OccupancyBandIDvsPhiId', xbins = 65, xmin = -32.5, xmax = 32.5, ybins = 101, ymin = -0.5, ymax = 100.5, opt = 'kAlwaysCreate')
            sTgcPadTriggerExpertGroup.defineHistogram(f'lb_{sideIndex}_sector_{sectorIndex},relBCID_{sideIndex}_sector_{sectorIndex};RelBCID_vs_LB_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; LB; Trigger relBCID; Pad Trigger hits', path = 'PadTrigger/Triggers/RelBCIDvsLB', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 7, ymin = -0.5, ymax = 6.5, opt = 'kAlwaysCreate,kAddBinsDynamically')
            sTgcPadTriggerExpertGroup.defineHistogram(f'relBCID_{sideIndex}_sector_{sectorIndex},phiIds_{sideIndex}_sector_{sectorIndex};Trigger_PhiID_vs_RelBCID_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; Trigger relBCID; Trigger phiID; Pad Trigger hits', path = 'PadTrigger/Triggers/PhiIDvsRelBCID', xbins = 7, xmin = -0.5, xmax = 6.5, ybins = 65, ymin = -32.5, ymax = 32.5, opt = 'kAlwaysCreate')
            sTgcPadTriggerExpertGroup.defineHistogram(f'relBCID_{sideIndex}_sector_{sectorIndex},bandID_{sideIndex}_sector_{sectorIndex};Trigger_BandID_vs_RelBCID_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; Trigger relBCID; Trigger bandID; Pad Trigger hits', path = 'PadTrigger/Triggers/BandIDvsRelBCID', xbins = 7, xmin = -0.5, xmax = 6.5, ybins = 101, ymin = -0.5, ymax = 100.5, opt = 'kAlwaysCreate')
            sTgcPadTriggerExpertGroup.defineHistogram(f'hitRelBCID_{sideIndex}_sector_{sectorIndex},hitPfebs_{sideIndex}_sector_{sectorIndex};pFEB_vs_relBCID_Side{sideIndex}_Sector{sectorIndex}', type = 'TH2F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + '; Hit relBCID; Hit pFEB; Pad Trigger hits associated to reco muons', path = 'PadTrigger/Hits/PFEBvsRelBCID', xbins = 7, xmin = -0.5, xmax = 6.5, ybins = 25, ymin = -0.5, ymax = 24.5, opt = 'kAlwaysCreate')

            for tIdx in tech:
                LBExpertGroup.defineHistogram(f'{tIdx}LBsector{sideIndex}{sectorIndex},{tIdx}FEBsector{sideIndex}{sectorIndex};LB_vs_FEB_perSector_{tIdx}_{sideIndex}'+f'{sectorIndex}'.zfill(2), type = 'TH2F', title = 'FEB vs LB; LB; FEB', path = f'{tIdx[0].capitalize()+tIdx[1:]}'+'/Side'+f'{sideIndex}', xbins = 100, xmin = -0.5, xmax = 99.5, ybins = 24, ymin = -0.5, ymax = 23.5, ylabels = FebLabels, opt = 'kAlwaysCreate,kAddBinsDynamically')        
                outdir = sideIndex+f'{sectorIndex}'.zfill(2)
                OccupancyGroup.defineHistogram(f'{tIdx}layer_{sideIndex}{sectorIndex},{tIdx}quad_{sideIndex}{sectorIndex};{tIdx}_quad_occupancy_per_layer_{sideIndex}{sectorIndex}'.zfill(2), type = 'TH2F', title = f'{tIdx} layers vs quad; Layer; Quad; Hits', path = f'{tIdx[0].capitalize()+tIdx[1:]}'+'/Side'+f'{sideIndex}', xbins = layerMax, xmin = 0.5, xmax = layerMax + 0.5, ybins = stationEtaMax, ymin = 0.5, ymax = stationEtaMax + 0.5, opt = 'kAlwaysCreate')
                sTgcTimingGroup.defineHistogram(f'{tIdx}Timing{sideIndex}{sectorIndex},{tIdx}FEB{sideIndex}{sectorIndex};{tIdx}_timing_{outdir}', type = 'TH2F', title = f'{tIdx} FEBs vs Timing; Time [ns]; FEB; Hits', path = f'{tIdx[0].capitalize()+tIdx[1:]}/Sector', xbins = 9, xmin = -112.5, xmax = 112.5, ybins = 24, ymin = -0.5, ymax = 23.5, ylabels = FebLabels, opt = 'kAlwaysCreate')

            stationEtaCounter=0
            for stationEtaIndex in range(1, stationEtaMax + 1):
                stationEtaCounter+=1
                padChargeGroup = helper.addGroup(sTgcMonAlg, f'padCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', globalPath + f'Expert/Charge/Pad/{sideIndex}' + f'{sectorIndex}'.zfill(2))
                stripChargeGroup = helper.addGroup(sTgcMonAlg, f'stripCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', globalPath + f'Expert/Charge/Strip/{sideIndex}' + f'{sectorIndex}'.zfill(2))
                wireGroupChargeGroup = helper.addGroup(sTgcMonAlg, f'wireGroupCharge_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', globalPath + f'Expert/Charge/Wire/{sideIndex}' + f'{sectorIndex}'.zfill(2))
                residualGroup = helper.addGroup(sTgcMonAlg, f'sTgcResiduals_{sideIndex}{sectorIndex}_quad_{stationEtaIndex}', globalPath + f'Expert/Residuals/{sideIndex}' + f'{sectorIndex}'.zfill(2)+'/')
                
                if sideCounter==1 and sectorCounter==1:
                    sTgcPadTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'padTiming_quad_{stationEtaIndex}', globalPath + 'Expert/Timing/Pad')
                    sTgcStripTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'stripTiming_quad_{stationEtaIndex}', globalPath + 'Expert/Timing/Strip')
                    sTgcWireTimingExpertGroup = helper.addGroup(sTgcMonAlg, f'wireTiming_quad_{stationEtaIndex}', globalPath + 'Expert/Timing/Wire')

                for layerIndex in range(1, layerMax + 1):
                    padChargeGroup.defineHistogram(f'padTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_pad_charge_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH1F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Pad Charge (on-track) [fC]; Number of Entries', path = f'Q{stationEtaIndex}', xbins = 100, xmin = 0., xmax = 1000., opt = 'kAlwaysCreate')
                    stripChargeGroup.defineHistogram(f'stripTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_strip_charge_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH1F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Strip Cluster Charge (on-track) [fC]; Number of Entries', path = f'Q{stationEtaIndex}', xbins = 120, xmin = 0., xmax = 1200., opt = 'kAlwaysCreate')
                    wireGroupChargeGroup.defineHistogram(f'wireGroupTrackCharge_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};All_wire_charge_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH1F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Wire Group Charge (on-track) [fC]; Number of Entries', path = f'Q{stationEtaIndex}', xbins = 100, xmin = 0., xmax = 1000., opt = 'kAlwaysCreate')
                    residualGroup.defineHistogram(f'residual_{sideIndex}_quad_{stationEtaIndex}_sector_{sectorIndex}_layer_{layerIndex};Residuals_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH1F', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}Q{stationEtaIndex}; Residual [mm]; Number of Entries', path = f'Q{stationEtaIndex}', xbins = 1000, xmin = -2., xmax = 2., opt = 'kAlwaysCreate')

                    if stationEtaCounter==1:
                        efficiencyGlobalRgroup.defineHistogram(f'hitLayer,rPosStrip_{sideIndex}_sector_{sectorIndex}_layer_{layerIndex};Efficiency_per_Radius_Layer{layerIndex}', type = 'TEfficiency', title = f'{sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}; sTgc-GlobalR-Strip (on track) [mm]' + f'; Efficiency sTGC strip {sideIndex}' + f'{sectorIndex}'.zfill(2) + f'L{layerIndex}', path = f'{sideIndex}' + f'{sectorIndex}'.zfill(2), xbins = 100, xmin = 0., xmax = 5000., opt = 'kAlwaysCreate')

                    if sideCounter==1 and sectorCounter==1:
                        sTgcPadTimingExpertGroup.defineHistogram(f'padTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},padTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_pad_timing_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH2F', title = f'Q{stationEtaIndex}L{layerIndex}; Sector; Pad Timing (on-track) [ns]; Hits', path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 201, ymin = -75.5, ymax = 125.5, opt = 'kAlwaysCreate')
                        sTgcStripTimingExpertGroup.defineHistogram(f'stripTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},stripTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_strip_timing_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH2F', title = f'Q{stationEtaIndex}L{layerIndex}; Sector; Strip Cluster Timing (on-track) [ns]; Hits', path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 201, ymin = -75.5, ymax = 125.5, opt = 'kAlwaysCreate')
                        sTgcWireTimingExpertGroup.defineHistogram(f'wireTrackSectorSided_quad_{stationEtaIndex}_layer_{layerIndex},wireTrackTiming_quad_{stationEtaIndex}_layer_{layerIndex};All_wire_timing_in_Q{stationEtaIndex}_Layer{layerIndex}', type = 'TH2F', title = f'Q{stationEtaIndex}L{layerIndex}; Sector; Wire Group Timing (on-track) [ns]; Hits', path = f'Q{stationEtaIndex}', xbins = 2*sectorMax + 1, xmin = -sectorMax - 0.5, xmax = sectorMax + 0.5, ybins = 201, ymin = -75.5, ymax = 125.5, opt = 'kAlwaysCreate')            
        
    acc = helper.result()
    result.merge(acc)
    return result

if __name__=='__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--events", default = 100, type = int, help = 'Number of events that you want to run.')
    parser.add_argument("--samples", nargs = "+", default = None, help = 'Path to the input samples. If you want to run multiple samples at once you have to introduce them separated by blank spaces.')
    parser.add_argument("--output", default = "HIST.root", help = 'Name of the output ROOT file.')
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Input.Files += args.samples 
    
    flags.Output.HISTFileName = args.output

    flags.Detector.GeometrysTGC = True
    flags.DQ.useTrigger = False
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2024-04' 

    flags.lock()
    flags.dump()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    sTgcMonitorAcc  =  sTgcMonitoringConfig(flags)
    sTgcMonitorAcc.OutputLevel = 2
    cfg.merge(sTgcMonitorAcc)           
    
    # number of events selected in the ESD
    cfg.run(args.events)
