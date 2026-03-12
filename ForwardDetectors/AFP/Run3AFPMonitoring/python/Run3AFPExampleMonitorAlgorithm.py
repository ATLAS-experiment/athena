# 
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

'''
@file Run3AFPExampleMonitorAlgorithm.py
@author N. Dikic
@date 2020-08-12
'''

def Run3AFPExampleMonitoringConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # don't run in RAWtoESD
    if inputFlags.DQ.Environment in ('tier0Raw',):
        return result

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(inputFlags))
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'Run3AFPMonitorCfg')
    
    from AthenaConfiguration.ComponentFactory import CompFactory

    #from Run3AFPMonitoring.Run3AFPMonitoringConf import AFPSiLayerAlgorithm
    afpSiLayerAlgorithmFac = CompFactory.AFPSiLayerAlgorithm
    afpSiLayerAlgorithm = helper.addAlgorithm(afpSiLayerAlgorithmFac,'AFPSiLayerAlg')

    #from Run3AFPMonitoring.Run3AFPMonitoringConf import AFPToFAlgorithm
    afpToFAlgorithmFac = CompFactory.AFPToFAlgorithm
    afpToFAlgorithm = helper.addAlgorithm(afpToFAlgorithmFac,'AFPToFAlg')

    afpToFSiTAlgorithmFac = CompFactory.AFPToFSiTAlgorithm
    afpToFSiTAlgorithm = helper.addAlgorithm(afpToFSiTAlgorithmFac,'AFPToFSiTAlg')



    # Add a generic monitoring tool (a "group" in old language). 
    # The returned object here is the standard GenericMonitoringTool.
    AFPSiGroup = helper.addGroup(afpSiLayerAlgorithm, 'AFPSiLayerTool', 'AFP/') 
    AFPToFGroup = helper.addGroup(afpToFAlgorithm, 'AFPToFTool', 'AFP/')
    AFPToFSiTGroup = helper.addGroup(afpToFSiTAlgorithm, 'AFPToFSiTTool', 'AFP/')
    
    xLabelsStations = ['farAside', 'nearAside', 'nearCside', 'farCside']
    xLabelsStationsPlanes = ['fA3','fA2','fA1','fA0','nA3','nA2','nA1','nA0','nC0','nC1','nC2','nC3','fC0','fC1','fC2','fC3']
    xLabelsForEventsPerStation = [ 'fA', '-','-','-', 'nA', '-', '-', '-', 'nC', '-', '-', '-', 'fC', '-', '-', '-', ]
    xLabelsHitBarVsTrain = [ 'A', 'B', 'C', 'D']
    yLabelsHitBarVsTrain = [ '0', '1', '2', '3']
    xLabelsToFEff = [ 'A', 'B', 'C', 'D', 'Tr']
    yLabelsToFEff = [ '0', '1', '2', '3']

    AFPSiGroup.defineHistogram('lb,nSiHits', title='Total number of hits divided by number of events;lumiblock;total number of hits', type='TProfile', path='SiT/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('eventsPerStation', title='Number of events per stations; station; events', type='TH1I', path='SiT/Aux/', xbins=16, xmin=-0.5, xmax=15.5, xlabels=xLabelsForEventsPerStation )
    AFPSiGroup.defineHistogram('clustersInPlanes', title='Number of clusters per planes; plane; clusters', type='TH1I', path='SiT/Aux/', xbins=16, xmin=-0.5, xmax=15.5, xlabels=xLabelsStationsPlanes )
    AFPSiGroup.defineHistogram('lb,muPerBX', title='<mu>;lumiBlock;<mu>', type='TProfile', path='SiT/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('planeHitsAllMU', title='Number of hits per plane divided by <mu>;plane; hits/<mu>', type='TH1F', path='SiT/HitsPerPlanes/', xbins=16, xmin=-0.5, xmax=15.5, weight = 'weightAllPlanes', xlabels=xLabelsStationsPlanes )
    AFPSiGroup.defineHistogram('numberOfHitsPerStation', title='Number of hits per station; station; hits', type='TH1I', path='SiT/Aux/', xbins=4, xmin=-0.5, xmax=3.5, xlabels=xLabelsStations)
    AFPSiGroup.defineHistogram('lbEvents;NumberOfEventsPerLumiblock',title='Number of events per lumiblock; lumiblock; events', type='TH1I', path='SiT/Aux/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('lbHits;NumberOfHitsPerLumiblock',title='Number of hits per lumiblock; lumiblock; hits', type='TH1I', path='SiT/Aux/', xbins=2000, xmin=0.5, xmax=2000.5)
    
    #SIT: BCID histograms
    AFPSiGroup.defineHistogram('bcidAll', title='(All) Paired bunches - SiT; BX; entries', type='TH1I', path='SiT/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPSiGroup.defineHistogram('bcidFront', title='(Front) Paired bunches - SiT; BX; entries', type='TH1I', path='SiT/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPSiGroup.defineHistogram('bcidMiddle', title='(Middle) Paired bunches - SiT; BX; entries', type='TH1I', path='SiT/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPSiGroup.defineHistogram('bcidEnd', title='(End) Paired bunches - SiT; BX; entries', type='TH1I', path='SiT/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    
    AFPSiGroup.defineHistogram('lbEventsStationsAll', title='Number of events per lumiblock for all stations;lumiblock; events', type='TH1I', path='SiT/StationEvents/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('numberOfEventsPerLumiblockFront', title='(Front) Number of events per lumiblock for all stations;lumiblock; events', type='TH1I', path='SiT/Aux/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('numberOfEventsPerLumiblockMiddle', title='(Middle) Number of events per lumiblock for all stations;lumiblock; events', type='TH1I', path='SiT/Aux/', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPSiGroup.defineHistogram('numberOfEventsPerLumiblockEnd', title='(End) Number of events per lumiblock for all stations;lumiblock; events', type='TH1I', path='SiT/Aux/', xbins=2000, xmin=0.5, xmax=2000.5)


    AFPToFGroup.defineHistogram('lb,nTofHits', title='Multiplicity;lumiblock;total number of Hits', type='TProfile', path='ToF/', xbins=2000, xmin=0.5, xmax=2000.5) 
    
    AFPToFGroup.defineHistogram('lb,muPerBXToF', title='<mu>;lumiblock;<mu>', type='TProfile', path='ToF/', xbins=2000, xmin=0.5, xmax=2000.5)
    
    AFPToFGroup.defineHistogram('lbAandCToFEvents', title='Number of events in ToF stations A and C vs lb; lumiblock; events', type='TH1I', path='ToF/Events', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPToFGroup.defineHistogram('lbAToFEvents', title='Number of events in ToF station A vs lb; lumiblock; events', type='TH1I', path='ToF/Events', xbins=2000, xmin=0.5, xmax=2000.5)
    AFPToFGroup.defineHistogram('lbCToFEvents', title='Number of events in ToF station C vs lb; lumiblock; events', type='TH1I', path='ToF/Events', xbins=2000, xmin=0.5, xmax=2000.5)
    
    # TOF: BCID histograms
    AFPToFGroup.defineHistogram('bcidAllToF', title='(All) Paired bunches - ToF; BX; entries', type='TH1I', path='ToF/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPToFGroup.defineHistogram('bcidFrontToF', title='(Front) Paired bunches - ToF; BX; entries', type='TH1I', path='ToF/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPToFGroup.defineHistogram('bcidMiddleToF', title='(Middle) Paired bunches - ToF; BX; entries', type='TH1I', path='ToF/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)
    AFPToFGroup.defineHistogram('bcidEndToF', title='(End) Paired bunches - ToF; BX; entries', type='TH1I', path='ToF/BCID_Mask', xbins=4000, xmin=-0.5, xmax=3999.5)

    # TOF: AFPToFSiT histograms
    AFPToFSiTGroup.defineHistogram('lqBar_tight_A,fsp0_rows_tight_A;ToFSiTCorrTightXA', title='LQBar vs FSP0 X dim with FSP2 Side A;Train;FSP0 x-pix [50 um]', type='TH2I', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=34, ymin=-0.05, ymax=335.5)
    AFPToFSiTGroup.defineHistogram('lqBar_tight_C,fsp0_rows_tight_C;ToFSiTCorrTightXC', title='LQBar vs FSP0 X dim with FSP2 Side C;Train;FSP0 x-pix [50 um]', type='TH2I', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=34, ymin=-0.05, ymax=335.5)
    AFPToFSiTGroup.defineHistogram('lqBar_tight_A,fsp0_columns_tight_A;ToFSiTCorrTightYA', title='LQBar vs FSP0 columns with FSP2 hit Side A;Train;FSP0 y-pix [250 um]', type='TH2I', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=80, ymin=-0.5, ymax=79.5)
    AFPToFSiTGroup.defineHistogram('lqBar_tight_C,fsp0_columns_tight_C;ToFSiTCorrTightYC', title='LQBar vs FSP0 columns with FSP2 hit Side C;Train;FSP0 y-pix [250 um]', type='TH2I', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=80, ymin=-0.5, ymax=79.5)

    AFPToFSiTGroup.defineHistogram('trainHits_A,fsp0_rows_tight_A;ToFSiTCorrTrainHitsXA', title='# hits in train vs FSP0 X dim with FSP2 Side A;Train hits (train+numHits/5);FSP0 x-pix [50 um]', type='TH2I', path='ToFSiTCorr/', xbins=25, xmin=-1, xmax=4, ybins=34, ymin=-0.05, ymax=335.5)
    AFPToFSiTGroup.defineHistogram('trainHits_C,fsp0_rows_tight_C;ToFSiTCorrTrainHitsXC', title='# hits in train vs FSP0 X dim with FSP2 Side C;Train hits (train+numHits/5);FSP0 x-pix [50 um]', type='TH2I', path='ToFSiTCorr/', xbins=25, xmin=-1, xmax=4, ybins=34, ymin=-0.05, ymax=335.5)
    AFPToFSiTGroup.defineHistogram('trainHits_A,fsp0_columns_tight_A;ToFSiTCorrTrainHitsYA', title='# hits in train vs FSP0 columns with FSP2 hit Side A;Train hits (train+numHits/5);FSP0 y-pix [250 um]', type='TH2I', path='ToFSiTCorr/', xbins=25, xmin=-1, xmax=4, ybins=80, ymin=-0.5, ymax=79.5)
    AFPToFSiTGroup.defineHistogram('trainHits_C,fsp0_columns_tight_C;ToFSiTCorrTrainHitsYC', title='# hits in train vs FSP0 columns with FSP2 hit Side C;Train hits (train+numHits/5);FSP0 y-pix [250 um]', type='TH2I', path='ToFSiTCorr/', xbins=25, xmin=-1, xmax=4, ybins=80, ymin=-0.5, ymax=79.5)

    AFPToFSiTGroup.defineHistogram('tofHits_A,fsp0Hits_A;ToFSiTNumHitsA', title='TOF vs FSP0 num. hits Side A;#Hit bars;FSP0 multiplicity', type='TH2F', path='ToFSiTCorr/', xbins=17, xmin=0, xmax=16, ybins=150, ymin=-0.5, ymax=149.5)
    AFPToFSiTGroup.defineHistogram('tofHits_C,fsp0Hits_C;ToFSiTNumHitsC', title='TOF vs FSP0 num. hits Side C;#Hit bars;FSP0 multiplicity', type='TH2F', path='ToFSiTCorr/', xbins=17, xmin=0, xmax=16, ybins=150, ymin=-0.5, ymax=149.5)

    AFPToFSiTGroup.defineHistogram('tof_eff_OFF_passed_A, tof_eff_OFF_bars_A, tof_eff_OFF_trains_A ;Efficiency_A', title='Efficiency ToF Side A;Bar;Train;Efficiency', type='TEfficiency', path='ToFSiT/Efficiency/', xbins=5, xmin=0, xmax=5, ybins=4, ymin=0, ymax=4, xlabels=xLabelsToFEff, ylabels=yLabelsToFEff)
    AFPToFSiTGroup.defineHistogram('tof_eff_OFF_passed_C, tof_eff_OFF_bars_C, tof_eff_OFF_trains_C ;Efficiency_C', title='Efficiency ToF Side C;Bar;Train;Efficiency', type='TEfficiency', path='ToFSiT/Efficiency/', xbins=5, xmin=0, xmax=5, ybins=4, ymin=0, ymax=4, xlabels=xLabelsToFEff, ylabels=yLabelsToFEff)

    AFPToFSiTGroup.defineHistogram('lqBar_A,fs_rows_full_A;ToFSiTCorr_FullXA', title='LQBar vs FS rows Full Side A;Train;FSP x-pix [50 um]', type='TH2F', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=346, ymin=-10.5, ymax=335.5)
    AFPToFSiTGroup.defineHistogram('lqBar_C,fs_rows_full_C;ToFSiTCorr_FullXC', title='LQBar vs FS rows Full Side C;Train;FSP x-pix [50 um]', type='TH2F', path='ToFSiTCorr/', xbins=20, xmin=-1, xmax=4, ybins=346, ymin=-10.5, ymax=335.5)
    
    # Plane Occupancy
    AFPSiGroup.defineHistogram('hitPerPlaneEventMuIndex,hitsPerPlaneEventsMu',
        title='Plane Occupancy;Global Plane Index;Hits on Plane',
        type='TProfile',
        path='SiT/PlaneOccupancy/', 
        xbins=16, xmin=-0.5, xmax=15.5,
        xlabels=xLabelsStationsPlanes) 
    
    planeList = ['P0', 'P1', 'P2', 'P3'] 
    array_corr = helper.addArray([planeList], afpToFSiTAlgorithm, 'AFPToFSiTTool', topPath='AFP/ToFSiTCorr/')
    array_corr.defineHistogram('lqBar_A,fs_rows_A;ToFSiTCorrXA_{0}', type='TH2F', title='LQBar vs FS{0} rows Side A;Train;FS{0} x-pix [50 um]', xbins=20, xmin=-1, xmax=4, ybins=346, ymin=-10.5, ymax=335.5)
    array_corr.defineHistogram('lqBar_C,fs_rows_C;ToFSiTCorrXC_{0}', type='TH2F', title='LQBar vs FS{0} rows Side C;Train;FS{0} x-pix [50 um]', xbins=20, xmin=-1, xmax=4, ybins=346, ymin=-10.5, ymax=335.5)
    array_corr.defineHistogram('lqBar_A,fs_columns_A;ToFSiTCorrYA_{0}', type='TH2F', title='LQBar vs FS{0} columns Side A;Train;FS{0} y-pix [250 um]', xbins=20, xmin=-1, xmax=4, ybins=85, ymin=-5.5, ymax=79.5)
    array_corr.defineHistogram('lqBar_C,fs_columns_C;ToFSiTCorrYC_{0}', type='TH2F', title='LQBar vs FS{0} columns Side C;Train;FS{0} y-pix [250 um]', xbins=20, xmin=-1, xmax=4, ybins=85, ymin=-5.5, ymax=79.5)


    # Using a map of groups
    layerList = ['P0','P1', 'P2', 'P3'] ## TODO XXX adapt to the enum/xAOD namespace names
    stationList = ['farAside', 'nearAside', 'nearCside', 'farCside']

    array = helper.addArray([stationList,layerList], afpSiLayerAlgorithm, 'AFPSiLayerTool', topPath = 'AFP/SiT/')

    array.defineHistogram('pixelColIDChip', title='Hits per column for station {0}, layer {1};ColID; entries', path='PixelColIDChip/{0}', xbins=80, xmin=0.5, xmax=80.5)
    array.defineHistogram('pixelRowIDChip', title='Hits per row for station {0}, layer {1};RowID; entries', path='PixelRowIDChip/{0}', xbins=336, xmin=0.5, xmax=336.5)
    array.defineHistogram('pixelRowIDChip,pixelColIDChip', title='Hitmap for station {0}, layer {1};RowID;ColID', type='TH2I', path='pixelColRow2D/{0}', xbins=336, xmin=0.5, xmax=336.5, ybins=80, ymin=0.5, ymax=80.5)
    array.defineHistogram('timeOverThreshold', type='TH1I', title='Time over threshold for station {0}, layer {1};timeOverThreshold; entries', path='SiTimeOverThreshold/{0}', xbins=16, xmin=0.5, xmax=16.5)
    
    array.defineHistogram('clusterY,clusterX', title='Cluster position in station {0} Layer {1};x [mm];y [mm]', type='TH2F', path='Cluster/{0}', xbins=336, xmin=0.0, xmax=17.0, ybins=80, ymin=0.0, ymax=20.0)        
    array.defineHistogram('sit_plane_eff_passed, sit_plane_eff_triedY, sit_plane_eff_triedX ;Efficiency', title='Efficiency for station {0} Layer {1};x [mm];y [mm]', type='TEfficiency', path='Efficiency/{0}', xbins=336, xmin=0.0, xmax=17.0, ybins=80, ymin=0.0, ymax=20.0)
    #Sync clusters
    array.defineHistogram('lbClustersPerPlanesAll,clustersPerPlaneAllPP', title='(All) Number of clusters in station {0}, plane {1} per lumiblock divided by <mu> per event;lumiblock;clusters/<mu> per event', type='TProfile', path='Synchronization/Clusters/', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbClustersPerPlanesFront,clustersPerPlaneFrontPP', title='(Front) Number of clusters in station {0}, plane {1} per lumiblock divided by <mu> per event;lumiblock; clusters/<mu> per event', type='TProfile', path='Synchronization/Clusters/Front/', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbClustersPerPlanesMiddle,clustersPerPlaneMiddlePP', title='(Middle) Number of clusters in station {0}, plane {1} per lumiblock divided by <mu> per event; lumiblock;clusters/<mu> per event', type='TProfile', path='Synchronization/Clusters/Middle/', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbClustersPerPlanesEnd,clustersPerPlaneEndPP', title='(End) Number of clusters in station {0}, plane {1} per lumiblock divided by <mu> per event; lumiblock;clusters/<mu> per event', type='TProfile', path='Synchronization/Clusters/End/', xbins=2000, xmin=0.5, xmax=2000.5)

    array.defineHistogram('lbhitsPerPlaneProfile, hitsPerPlaneProfile', title='Number of hits in station {0}, plane {1} per lumiblock per event divided by <mu>;lumiblock; hits/<mu> per event', type='TProfile', path='HitsPerPlanesVsLb/', xbins=2000, xmin=0.5, xmax=2000.5)

    array.defineHistogram('clusterToT', title='Sum of all hits\' ToT in each cluster, station {0}, layer {1};Cluster ToT; Counts', type='TH1I', path='ClusterToT/', xbins=18, xmin=0.5, xmax=18.5)

    array = helper.addArray([stationList], afpSiLayerAlgorithm, 'AFPSiLayerTool', topPath='AFP/SiT/')
    
    array.defineHistogram('planeHits', type='TH1I', title='Number of hits per plane, station {0};plane; hits', path='HitsPerPlanes', xbins=4, xmin=-0.5, xmax=3.5)
    array.defineHistogram('trackY,trackX', title='Number of tracks in AFP station {0};x [mm];y [mm]', type='TH2F', path='Track', xbins=336, xmin=0.0, xmax=17.0, ybins=80, ymin=0.0, ymax=20.0)    
    array.defineHistogram('lbEventsStations', title='Number of events per lumiblock, station {0};lumiblock; events', type='TH1I', path='StationEvents/', xbins=2000, xmin=0.5, xmax=2000.5)
    
    #Sync tracks
    array.defineHistogram('lbTracksAll,Total_tracks_All_profile', title = '(All) Tracks vs lumiblock divided by <mu> per event, station {0};lumiblock;tracks/<mu> per event', type='TProfile', path='Synchronization/Tracks', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbTracksFront,Total_tracks_Front_profile', title = '(Front) Tracks vs lumiblock divided by <mu> per event, station {0}; lumiblock;tracks/<mu> per event', type='TProfile', path='Synchronization/Tracks/Front', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbTracksMiddle,Total_tracks_Middle_profile', title = '(Middle) Tracks vs lumiblock divided by <mu> per event, station {0}; lumiblock;tracks/<mu> per event', type='TProfile', path='Synchronization/Tracks/Middle', xbins=2000, xmin=0.5, xmax=2000.5)
    array.defineHistogram('lbTracksEnd,Total_tracks_End_profile', title = '(End) Tracks vs lumiblock divided by <mu> per event, station {0}; lumiblock;tracks/<mu> per event', type='TProfile', path='Synchronization/Tracks/End', xbins=2000, xmin=0.5, xmax=2000.5)

    arrayToF = helper.addArray([stationList], afpToFAlgorithm, 'AFPToFTool', topPath='AFP/ToF/')
    
    arrayToF.defineHistogram('barInTrainID,trainID', title='ToF hit bar vs train {0};barInTrainID;trainID', type='TH2I', path='HitBarvsTrain/',xbins=4,xmin=-0.5,xmax=3.5,ybins=4,ymin=-0.5,ymax=3.5, xlabels = xLabelsHitBarVsTrain, ylabels = yLabelsHitBarVsTrain)
    
    trainIDListToF = ['T0', 'T1', 'T2', 'T3']
    barInTrainIDListToF = ['A', 'B', 'C', 'D']
    sideListToF = ['sideA', 'sideC']

    # Per bar: per-mu TH1F + per event TProfile
    arrayToFSideBar = helper.addArray([sideListToF, trainIDListToF, barInTrainIDListToF], afpToFAlgorithm, 'AFPToFTool', topPath = 'AFP/ToF/')
    arrayToFSideBar.defineHistogram('lbToFBar', title='ToF hits vs lumiblock divided by <mu> (train {1}, bar {2}), {0}; lb; hits/<mu>', type='TH1F', path='ToFHitsVsLb/{0}/BarsAll', xbins=2000, xmin=0.5, xmax=2000.5, weight = 'lbToFBar_Weight')
    arrayToFSideBar.defineHistogram('lbToFBarPerEvent,hitsPerBarPP', title='ToF hits per event / <mu> (train {1}, bar {2}), {0}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/{0}/BarsAll', xbins=2000, xmin=0.5, xmax=2000.5)

    # Per train: per-mu TH1F + per event TProfile
    arrayToFSideTrain = helper.addArray([sideListToF, trainIDListToF], afpToFAlgorithm, 'AFPToFTool', topPath = 'AFP/ToF/')
    arrayToFSideTrain.defineHistogram('barInTrainIDSide', title='Total hits per bars in {0} {1}; barID; hits', type='TH1I', path='HitsPerBarsInTrain/{0}', xbins=4, xmin=-0.5, xmax=3.5)
    # per-mu TH1F (All/Front/Middle/End)
    arrayToFSideTrain.defineHistogram('lbToFTrainAll', title='(All) Hits per train / <mu>, {0} {1}; lb; hits/<mu>', type='TH1F', path='ToFHitsVsLb/{0}/All/', xbins=2000, xmin=0.5, xmax=2000.5, weight='weightToFTrainAll')
    arrayToFSideTrain.defineHistogram('lbToFTrainFront', title='(Front) Hits per train / <mu>, {0} {1}; lb; hits/<mu>', type='TH1F', path='ToFHitsVsLb/{0}/Front/', xbins=2000, xmin=0.5, xmax=2000.5, weight='weightToFTrainFront')
    arrayToFSideTrain.defineHistogram('lbToFTrainMiddle', title='(Middle) Hits per train / <mu>, {0} {1}; lb; hits/<mu>', type='TH1F', path='ToFHitsVsLb/{0}/Middle/', xbins=2000, xmin=0.5, xmax=2000.5, weight='weightToFTrainMiddle')
    arrayToFSideTrain.defineHistogram('lbToFTrainEnd', title='(End) Hits per train / <mu>, {0} {1}; lb; hits/<mu>', type='TH1F', path='ToFHitsVsLb/{0}/End/', xbins=2000, xmin=0.5, xmax=2000.5, weight='weightToFTrainEnd')
    # per event TProfile (All/Front/Middle/End)
    arrayToFSideTrain.defineHistogram('lbToFPerEvent,hitsPerTrainAllPP', title='(All) Hits per train per event / <mu>, {0} {1}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/{0}/All/', xbins=2000, xmin=0.5, xmax=2000.5)
    arrayToFSideTrain.defineHistogram('lbToFPerEventFront,hitsPerTrainFrontPP', title='(Front) Hits per train per event / <mu>, {0} {1}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/{0}/Front/', xbins=2000, xmin=0.5, xmax=2000.5)
    arrayToFSideTrain.defineHistogram('lbToFPerEventMiddle,hitsPerTrainMiddlePP', title='(Middle) Hits per train per event / <mu>, {0} {1}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/{0}/Middle/', xbins=2000, xmin=0.5, xmax=2000.5)
    arrayToFSideTrain.defineHistogram('lbToFPerEventEnd,hitsPerTrainEndPP', title='(End) Hits per train per event / <mu>, {0} {1}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/{0}/End/', xbins=2000, xmin=0.5, xmax=2000.5)

    # Station-level: per event TProfile
    arrayToFSide = helper.addArray([sideListToF], afpToFAlgorithm, 'AFPToFTool', topPath = 'AFP/ToF/')
    arrayToFSide.defineHistogram('lbToFStationPerEvent,hitsPerStationPP', title='ToF hits per event / <mu>, {0}; lb; hits/<mu> per event', type='TProfile', path='ToFHitsVsLbPerEvent/', xbins=2000, xmin=0.5, xmax=2000.5)
    arrayToFSide.defineHistogram('numberOfHit', title='Number of hit per bar, {0};bar', path='', xbins=4, xmin=-0.5, xmax=3.5)
    arrayToFSide.defineHistogram('barInTrainAll', title='Number of hits in bar, {0}; barInTrain;hits', type='TH1I', path='HitsPerBarsInTrain/', xbins=16, xmin=-0.5, xmax=15.5)
    arrayToFSide.defineHistogram('ToFHits_side', title='ToF hits per lumiblock divided by <mu>, {0}; lb; hits', type='TH1F', path='StationHits/', xbins=2000, xmin=0.5, xmax=2000.5, weight='ToFHits_MU_Weight')
    
    #array for ToF cross-bar delta t
    chan_combinations_list = [  "0AB", "0AC", "0AD", "0BC", "0BD", "0CD", "1AB", "1AC", "1AD", "1BC", "1BD", "1CD", 
                    "2AB", "2AC", "2AD", "2BC", "2BD", "2CD", "3AB", "3AC", "3AD", "3BC", "3BD", "3CD"]
    arrayToFCrossBarDeltaT = helper.addArray([chan_combinations_list], afpToFAlgorithm, 'AFPToFTool', topPath='AFP/ToF/')
    arrayToFCrossBarDeltaT.defineHistogram('crossBarDeltaT_A', title='ToF cross-bar <delta> time (channel combination {0}), side A; <delta> t, [ps]; events', type='TH1D', path='DeltaTime/sideA', xbins=400, xmin=-1500.0, xmax=1500.0)
    arrayToFCrossBarDeltaT.defineHistogram('crossBarDeltaT_C', title='ToF cross-bar <delta> time (channel combination {0}), side C; <delta> t, [ps]; events', type='TH1D', path='DeltaTime/sideC', xbins=400, xmin=-1500.0, xmax=1500.0)
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    result.merge(helper.result())
    return result
    
if __name__=='__main__':

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.Files = ['/eos/home-v/vlysenko/DQ/testing/data25_13p6TeV.00497370.physics_Main.merge.AOD.f1580_m2272._lb0200._0001.1']
    flags.Input.isMC = False
    flags.Output.HISTFileName = 'tof-clean-mon.root'
    

    flags.Concurrency.NumThreads=10
    flags.Concurrency.NumConcurrentEvents=10
    
    
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    
    exampleMonitorAcc = Run3AFPExampleMonitoringConfig(flags)
    cfg.merge(exampleMonitorAcc)

    cfg.run(1000000)