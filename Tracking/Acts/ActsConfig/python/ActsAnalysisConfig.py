# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrackAnalysisAlgCfg(flags,
                            name: str = "ActsTrackAnalysisAlg",
                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    kwargs.setdefault('TracksLocation', 'ActsTracks')
    kwargs.setdefault("MonGroupName", kwargs['TracksLocation'])

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, kwargs['TracksLocation'] + 'AnalysisAlgCfg')

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.TrackAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    monitoringGroup.defineHistogram('Ntracks', title='Number of Tracks;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=500, xmin=0, xmax=20000)
    import math
    monitoringGroup.defineHistogram('theta', title='Track polar angle;#theta;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=128, xmin=0, xmax=math.pi)
    monitoringGroup.defineHistogram('phi', title='Track azimuthal angle;#phi;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=128, xmin=-math.pi, xmax=math.pi)
    monitoringGroup.defineHistogram('qoverp', title='track inverse momentum;q/p [1/GeV];Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=200, xmin=-1.2, xmax=1.2)
    monitoringGroup.defineHistogram('chi2OverNdof', title='fit chi2 / ndof;#chi^{2}/nDoF;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=100, xmin=0, xmax=20)
    monitoringGroup.defineHistogram('nStates', title='Number of states / track;# states;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=60, xmin=0, xmax=60)
    monitoringGroup.defineHistogram('nMeasurements', title='Number of measurements / track;# measurements;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=35, xmin=0, xmax=35)
    monitoringGroup.defineHistogram('nPixelHits', title='Number of pixel hits / track;# pixel hits;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=35, xmin=0, xmax=35)
    monitoringGroup.defineHistogram('nStripHits', title='Number of strip hits / track;# strip hits;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=35, xmin=0, xmax=35)
    monitoringGroup.defineHistogram('surfaceType', title='type of reference surface;type;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=35, xmin=0, xmax=35)


    acc.merge(helper.result())
    return acc

def ActsTrackParticleAnalysisAlgCfg(flags,
                                    name: str = 'ActsTrackParticleAnalysisAlg',
                                    **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('MonitorTrackStateCounts',True)
    kwargs.setdefault('TrackParticleLocation', 'ActsTrackParticles')

    kwargs.setdefault('MonGroupName', kwargs['TrackParticleLocation'])

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TrackParticleAnalysisAlgCfg')
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.TrackParticleAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/'+name+'/')

    monitoringGroup.defineHistogram('pt', title='TrackParticle pt;pt (MeV);Entries', type='TH1F', path=kwargs['MonGroupName'],
                                    xbins=100, xmin=0, xmax=10e3)
    monitoringGroup.defineHistogram('eta', title='TrackParticle eta;eta;Entries', type='TH1F', path=kwargs['MonGroupName'],
                                    xbins=50, xmin=-4, xmax=4)

    # hit counts
    monitoringGroup.defineHistogram('pixelHits', title='Number of pixel hits;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=50, xmin=-1, xmax=49)
    monitoringGroup.defineHistogram('innermostHits', title='Number of innermost pixel hits;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=6, xmin=-1, xmax=5)
    monitoringGroup.defineHistogram('nextToInnermostHits', title='Number of next-to-innermost pixel hits;N;Entries', type='TH1I',
                                    path=kwargs['MonGroupName'], xbins=6, xmin=-1, xmax=5)
    monitoringGroup.defineHistogram('expectInnermostHit', title='Innermost pixel hit expected;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                    xbins=3, xmin=-1, xmax=2)
    monitoringGroup.defineHistogram('expectNextToInnermostHit', title='Next-to-innermost pixel hit expected;N;Entries', type='TH1I',
                                    path=kwargs['MonGroupName'],xbins=3, xmin=-1, xmax=2)

    if kwargs['MonitorTrackStateCounts'] :
        # have to add artifical dependency, because the ActsTracks are created by
        # a special reader algorithm from the various component branches, but the
        # element links pointing to the ActsTracks would not add this dependency
        # by themselves.
        if 'ExtraInputs' not in kwargs :
            tracks_name = kwargs['TrackParticleLocation']
            if tracks_name[-len('TrackParticles'):] == 'TrackParticles' :
                kwargs.setdefault('ExtraInputs',{('ActsTrk::TrackContainer',tracks_name[:-len('TrackParticles')]+'Tracks')})

        monitoringGroup.defineHistogram('States', title='Number of states on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('Measurements', title='Number of measurements on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('Parameters', title='Number of parameters on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('Outliers', title='Number of outliers on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('Holes', title='Number of holes on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('SharedHits', title='Number of shared hits on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
        monitoringGroup.defineHistogram('MaterialStates', title='Number of material states on track;N;Entries', type='TH1I', path=kwargs['MonGroupName'],
                                        xbins=50, xmin=0, xmax=50)
    acc.merge(helper.result())

    acc.addEventAlgo( CompFactory.ActsTrk.TrackParticleAnalysisAlg(name, **kwargs) )
    return acc

def ActsHgtdClusterAnalysisAlgCfg(flags,
                                  name: str = "ActsHgtdClusterAnalysisAlg",
                                  **kwargs) -> ComponentAccumulator:
    if flags.HGTD.Geometry.useGeoModelXml:
        from HGTD_GeoModelXml.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    else:
        from HGTD_GeoModel.HGTD_GeoModelConfig import HGTD_ReadoutGeometryCfg
    acc = HGTD_ReadoutGeometryCfg(flags)

    kwargs.setdefault("MonGroupName", "ActsHgtdClusters")
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'ActsHgtdClusterAnalysisAlgCfg')

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.HgtdClusterAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    path = "ActsHgtdClusters"
    monitoringGroup.defineHistogram('localX,localY;h_localXY', title="h_localXY; x [mm]; y [mm]", type="TH2F", path=path,
                                    xbins=20, xmin=-30, xmax=30,
                                    ybins=20, ymin=-30, ymax=30)
    monitoringGroup.defineHistogram('globalX,globalY;h_globalXY', title="h_globalXY; x [mm]; y [mm]", type="TH2F", path=path,
                                    xbins=100, xmin=-750, xmax=750,
                                    ybins=100, ymin=-750, ymax=750)
    monitoringGroup.defineHistogram('globalZ,globalR;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path=path,
                                    xbins=100, xmin=-3600, xmax=3600,
                                    ybins=100, ymin=0, ymax=800)
    monitoringGroup.defineTree('localX,localY,localT,localCovXX,localCovYY,localCovTT,globalX,globalY,globalZ,globalR,eta;HgtdClusters',
                               path='ntuples',
                               treedef='localX/vector<float>:localY/vector<float>:localT/vector<float>:localCovXX/vector<float>:localCovYY/vector<float>:localCovTT/vector<float>:globalX/vector<float>:globalY/vector<float>:globalZ/vector<float>:globalR/vector<float>:eta/vector<float>')
    
    acc.merge(helper.result())
    return acc

def ActsPixelClusterAnalysisAlgCfg(flags,
                                   name: str = "ActsPixelClusterAnalysisAlg",
                                   extension: str = "Acts",
                                   **kwargs) -> ComponentAccumulator:
    path = extension.replace("Acts", "") + "PixelClusters"

    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc = ITkPixelReadoutGeometryCfg(flags)

    kwargs.setdefault("MonGroupName", extension + "ClusterAnalysisAlg")
        
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, extension + 'ClusterAnalysisAlgCfg')
    
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.PixelClusterAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path=path,
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=0, ymax=400)
    monitoringGroup.defineHistogram('globalX,globalY;h_globalXY', title="h_globalXY; x [mm]; y [mm]", type="TH2F", path=path,
                                    xbins=800, xmin=-400, xmax=400,
                                    ybins=800, ymin=-400, ymax=400)
    monitoringGroup.defineHistogram('eta;h_etaCluster', title="h_etaCluster; cluster #eta", type="TH1F", path=path,
                                    xbins=100, xmin=-5, xmax=5)
    
    monitoringGroup.defineTree('barrelEndcap,layerDisk,phiModule,etaModule,isInnermost,isNextToInnermost,eta,globalX,globalY,globalZ,perp,localX,localY,localCovXX,localCovYY,sizeX,sizeY,widthY;PixelClusters',
                               path='ntuples',
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:isInnermost/vector<int>:isNextToInnermost/vector<int>:eta/vector<double>:globalX/vector<float>:globalY/vector<float>:globalZ/vector<float>:perp/vector<float>:localX/vector<float>:localY/vector<float>:localCovXX/vector<float>:localCovYY/vector<float>:sizeX/vector<int>:sizeY/vector<int>:widthY/vector<float>')

    acc.merge(helper.result())
    return acc


def ActsStripClusterAnalysisAlgCfg(flags,
                                   name: str = "ActsStripClusterAnalysisAlg",
                                   extension: str = "Acts",
                                   **kwargs) -> ComponentAccumulator:
    path = extension.replace("Acts", "") + "StripClusters"
    
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("MonGroupName", extension + "ClusterAnalysisAlg")

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, extension + "ClusterAnalysisAlgCfg")

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.StripClusterAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path=path,
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=300, ymax=1100)    
    monitoringGroup.defineHistogram('globalX,globalY;h_globalXY', title="h_globalXY; x [mm]; y [mm]", type="TH2F", path=path,
                                    xbins=1600, xmin=-1100, xmax=1100,
                                    ybins=1600, ymin=-1100, ymax=1100)
    monitoringGroup.defineHistogram('eta;h_etaCluster', title="h_etaCluster; cluster #eta", type="TH1F", path=path,
                                    xbins=100, xmin=-5, xmax=5)

    monitoringGroup.defineTree(f'barrelEndcap,layerDisk,phiModule,etaModule,sideModule,eta,globalX,globalY,globalZ,perp,localX,localCovXX,sizeX;{path}',
                               path='ntuples', 
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:sideModule/vector<int>:eta/vector<double>:globalX/vector<float>:globalY/vector<float>:globalZ/vector<float>:perp/vector<float>:localX/vector<float>:localCovXX/vector<float>:sizeX/vector<int>')

    acc.merge(helper.result())
    return acc

def ActsBaseSpacePointAnalysisAlgCfg(flags,
                                     name: str = "",
                                     extension: str = "Acts",
                                     histoPath = "",
                                     ntupleName = "",
                                     **kwargs) -> ComponentAccumulator:
    isPixel = 'Pixel' in name
    perp_min = 0 if isPixel else 300
    perp_max = 400 if isPixel else 1100

    acc = ComponentAccumulator()
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, extension + 'SpacePointAnalysisAlgCfg')

    kwargs.setdefault("MonGroupName", extension + "SpacePointAnalysisAlg")
    
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SpacePointAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')
    

    monitoringGroup.defineHistogram('Nsp;h_Nsp', title="Number of Space Points;N;Entries", type="TH1I", path=f"{histoPath}",
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineHistogram('globalX,globalY;h_globalXY', title="h_globalXY; x [mm]; y [mm]", type="TH2F", path=f"{histoPath}",
                                    xbins=800, xmin=-perp_max, xmax=perp_max,
                                    ybins=800, ymin=-perp_max, ymax=perp_max)
    monitoringGroup.defineHistogram('globalZ,perp;h_globalZR', title="h_globalZR; z [mm]; r [mm]", type="TH2F", path=f"{histoPath}",
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('eta;h_etaSpacePoint', title="h_etaSpacePoint; space point #eta", type="TH1F", path=f"{histoPath}",
                                    xbins=100, xmin=-5, xmax=5)

    monitoringGroup.defineTree(f'barrelEndcap,layerDisk,phiModule,etaModule,sideModule,isInnermost,isNextToInnermost,isOverlap,eta,globalX,globalY,globalZ,perp,globalCovR,globalCovZ;{ntupleName}',
                               path='ntuples',
                               treedef='barrelEndcap/vector<int>:layerDisk/vector<int>:phiModule/vector<int>:etaModule/vector<int>:sideModule/vector<int>:isInnermost/vector<int>:isNextToInnermost/vector<int>:isOverlap/vector<int>:eta/vector<double>:globalX/vector<double>:globalY/vector<double>:globalZ/vector<double>:perp/vector<double>:globalCovR/vector<double>:globalCovZ/vector<double>')

    acc.merge(helper.result())
    return acc

def ActsPixelSpacePointAnalysisAlgCfg(flags,
                                      name: str = "ActsPixelSpacePointAnalysisAlg",
                                      extension: str = "Acts",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from PixelGeoModelXml.ITkPixelGeoModelConfig import ITkPixelReadoutGeometryCfg
    acc.merge(ITkPixelReadoutGeometryCfg(flags))

    kwargs.setdefault("SpacePointContainerKey", "ITkPixelSpacePoints")
    kwargs.setdefault("UsePixel", True)
    kwargs.setdefault("UseOverlap", False)

    acc.merge(ActsBaseSpacePointAnalysisAlgCfg(flags, 
                                               name = name,
                                               extension = extension,
                                               histoPath = extension.replace("Acts", "") + "PixelSpacePoints",
                                               ntupleName = extension.replace("Acts", "") + "PixelSpacePoints",
                                               **kwargs))
    return acc



def ActsStripSpacePointAnalysisAlgCfg(flags,
                                      name: str = "ActsStripSpacePointAnalysisAlg",
                                      extension: str = "Acts",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc.merge(ITkStripReadoutGeometryCfg(flags))

    kwargs.setdefault("SpacePointContainerKey", "ITkStripSpacePoints")
    kwargs.setdefault("UsePixel", False)
    kwargs.setdefault("UseOverlap", False)

    acc.merge(ActsBaseSpacePointAnalysisAlgCfg(flags,
                                               name = name,
                                               extension = extension,
                                               histoPath = extension.replace("Acts", "") + "StripSpacePoints",
                                               ntupleName = extension.replace("Acts", "") + "StripSpacePoints",
                                               **kwargs))
    return acc


def ActsStripOverlapSpacePointAnalysisAlgCfg(flags,
                                             name: str = "ActsStripOverlapSpacePointAnalysisAlg",
                                             extension: str = "Acts",
                                             **kwargs) -> ComponentAccumulator:
    from StripGeoModelXml.ITkStripGeoModelConfig import ITkStripReadoutGeometryCfg
    acc = ITkStripReadoutGeometryCfg(flags)

    kwargs.setdefault("SpacePointContainerKey", "ITkStripOverlapSpacePoints")
    kwargs.setdefault("UsePixel", False)
    kwargs.setdefault("UseOverlap", True)

    acc.merge(ActsBaseSpacePointAnalysisAlgCfg(flags,
                                               name = name,
                                               extension = extension,
                                               histoPath = extension.replace("Acts", "") + "StripOverlapSpacePoints",
                                               ntupleName = extension.replace("Acts", "") + "StripOverlapSpacePoints",
                                               **kwargs))
    return acc


def ActsBaseSeedAnalysisAlgCfg(flags, 
                               name: str = "",
                               extension: str = "Acts",
                               histoPath: str = "",
                               ntupleName: str = "",
                               **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    isPixel = 'Pixel' in name
    perp_min = 0 if isPixel else 300
    perp_max = 400 if isPixel else 1100

    kwargs.setdefault('MonGroupName', extension + 'SeedAnalysisAlg')
    
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, extension + 'SeedAnalysisAlgCfg')

    from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
    geoTool = acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags))
    acc.addPublicTool(geoTool)
    
    # ATLAS Converter Tool
    from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
    converterTool = acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags))
    
    # Track Param Estimation Tool
    from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
    trackEstimationTool = acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags))
    
    kwargs.setdefault('TrackingGeometryTool', acc.getPublicTool(geoTool.name)) # PublicToolHandle
    kwargs.setdefault('ATLASConverterTool', converterTool)
    kwargs.setdefault('TrackParamsEstimationTool', trackEstimationTool)

    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SeedAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    monitoringGroup.defineHistogram('Nseed', title='Number of Seeds;N;Entries', type='TH1I', path=f'{histoPath}',
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineHistogram('z1,r1;zr1', title='Bottom SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('z2,r2;zr2', title='Middle SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)
    monitoringGroup.defineHistogram('z3,r3;zr3', title='Top SP - Z coordinate vs R;z [mm];r [mm]', type='TH2F', path=f'{histoPath}',
                                    xbins=1500, xmin=-3000, xmax=3000,
                                    ybins=400, ymin=perp_min, ymax=perp_max)

    monitoringGroup.defineHistogram('x1;x1', title='Bottom SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y1;y1', title='Bottom SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z1;z1', title='Bottom SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r1;r1', title='Bottom SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    monitoringGroup.defineHistogram('x2;x2', title='Middle SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y2;y2', title='Middle SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z2;z2', title='Middle SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r2;r2', title='Middle SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    monitoringGroup.defineHistogram('x3;x3', title='Top SP - x coordinate;x [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('y3;y3', title='Top SP - y coordinate;y [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-perp_max, xmax=perp_max)
    monitoringGroup.defineHistogram('z3;z3', title='Top SP - z coordinate;z [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=-3000, xmax=3000)
    monitoringGroup.defineHistogram('r3;r3', title='Top SP - radius coordinate;r [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                    xbins=100, xmin=perp_min, xmax=perp_max)
    
    if 'PixelSeeds' in ntupleName:
        monitoringGroup.defineHistogram('pt;pT', title='Pt;Pt;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=100, xmin=0, xmax=100)
        monitoringGroup.defineHistogram('d0;d0', title='d0;d0 [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=2)
        monitoringGroup.defineHistogram('eta;Eta', title='Pseudo-Rapidity;Pseudo-Rapidity;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=4.5)
        monitoringGroup.defineHistogram('theta;Theta', title='Theta;Theta;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=1.6)
        monitoringGroup.defineHistogram('penalty;Penalty', title='Penalty;Penalty;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=200)
        monitoringGroup.defineHistogram('dzdr_b;dzdr_b', title='dzdr_b;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-30, xmax=30)
        monitoringGroup.defineHistogram('dzdr_t;dzdr_t', title='dzdr_t;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-30, xmax=30)
    elif 'StripSeeds' in ntupleName:
        monitoringGroup.defineHistogram('pt;pT', title='Pt;Pt;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=100, xmin=0, xmax=2300)
        monitoringGroup.defineHistogram('d0;d0', title='d0;d0 [mm];Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=120)
        monitoringGroup.defineHistogram('eta;Eta', title='Pseudo-Rapidity;Pseudo-Rapidity;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=4.5)
        monitoringGroup.defineHistogram('theta;Theta', title='Theta;Theta;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=1.6)
        monitoringGroup.defineHistogram('penalty;Penalty', title='Penalty;Penalty;Entries;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=0, xmax=20000)
        monitoringGroup.defineHistogram('dzdr_b;dzdr_b', title='dzdr_b;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-6.5, xmax=6.5)
        monitoringGroup.defineHistogram('dzdr_t;dzdr_t', title='dzdr_t;;;', type='TH1F', path=f'{histoPath}',
                                        xbins=50, xmin=-6.5, xmax=6.5)
                
    if flags.Tracking.doTruth:
        monitoringGroup.defineHistogram('passed,estimated_eta;EfficiencyEta', title='Efficiency vs eta;eta;Efficiency', type='TEfficiency', path=f'{histoPath}',
                                        xbins=50, xmin=-5, xmax=5)
        monitoringGroup.defineHistogram('passed,estimated_pt;EfficiencyPt', title='Efficiency vs pT;pT [GeV];Efficiency', type='TEfficiency', path=f'{histoPath}',
                                        xbins=30, xmin=0, xmax=120)

    # Tree
    list_variables = "x1,y1,z1,r1,x2,y2,z2,r2,x3,y3,z3,r3,pt,theta,eta,d0,dzdr_b,dzdr_t,penalty,event_number,actual_mu"
    tree_def = "x1/vector<double>:y1/vector<double>:z1/vector<double>:r1/vector<double>:x2/vector<double>:y2/vector<double>:z2/vector<double>:r2/vector<double>:x3/vector<double>:y3/vector<double>:z3/vector<double>:r3/vector<double>\
:pt/vector<float>:theta/vector<float>:eta/vector<float>:d0/vector<float>:dzdr_b/vector<float>:dzdr_t/vector<float>:penalty/vector<float>:event_number/l:actual_mu/F"
    if flags.Tracking.doTruth:
        list_variables += ",truth_barcode,truth_prob"
        tree_def += ":truth_barcode/vector<int>:truth_prob/vector<double>"

    monitoringGroup.defineTree(f'{list_variables};{ntupleName}',
                               path='ntuples',
                               treedef=tree_def )

    acc.merge(helper.result())
    return acc



def ActsPixelSeedAnalysisAlgCfg(flags,
                                name: str = "ActsPixelSeedAnalysisAlg",
                                extension: str = "Acts",
                                **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputSeedCollection', 'ActsPixelSeeds')

    if flags.Tracking.doTruth:
        kwargs.setdefault('DetectorElements', 'ITkPixelDetectorElementCollection')
        kwargs.setdefault('ITkClustersTruth', '') #PRD_MultiTruthITkPixel')

    return ActsBaseSeedAnalysisAlgCfg(flags,
                                      name,
                                      extension,
                                      histoPath = extension.replace("Acts", "") + 'PixelSeeds',
                                      ntupleName = extension.replace("Acts", "") + 'PixelSeeds',
                                      **kwargs)


def ActsStripSeedAnalysisAlgCfg(flags,
                                name: str = "ActsStripSeedAnalysisAlg",
                                extension: str = "Acts",
                                **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputSeedCollection', 'ActsStripSeeds')
    kwargs.setdefault('UsePixel', False)

    if flags.Tracking.doTruth:
        kwargs.setdefault('DetectorElements', 'ITkStripDetectorElementCollection')
        kwargs.setdefault('ITkClustersTruth', '') #PRD_MultiTruthITkStrip')

    return ActsBaseSeedAnalysisAlgCfg(flags,
                                      name,
                                      extension,
                                      histoPath = extension.replace("Acts", "") + 'StripSeeds',
                                      ntupleName = extension.replace("Acts", "") + 'StripSeeds',
                                      **kwargs)


def ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags,
                                               name: str = "",
                                               extension: str = "Acts", 
                                               histoPath: str = "",
                                               ntupleName: str = "",
                                               **kwargs) -> ComponentAccumulator:
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,extension + 'EstimatedTrackParamsAnalysisAlgCfg')

    kwargs.setdefault('MonGroupName', extension + 'SeedAnalysisAlg')
    
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.EstimatedTrackParamsAnalysisAlg, name, **kwargs)
    monitoringGroup = helper.addGroup(monitoringAlgorithm, kwargs['MonGroupName'], '/ActsAnalysis/')

    monitoringGroup.defineHistogram('Nparams', title='Number of Estimated Parameters from Seeds;N;Entries', type='TH1I', path=f'{histoPath}',
                                    xbins=100, xmin=0, xmax=0)

    monitoringGroup.defineTree(f"track_param_pt,track_param_eta,track_param_phi,track_param_loc0,track_param_loc1,track_param_theta,track_param_qoverp,track_param_time,track_param_charge;{ntupleName}",
                               path="ntuples",
                               treedef="track_param_pt/vector<double>:track_param_eta/vector<double>:track_param_phi/vector<double>:track_param_loc0/vector<double>:track_param_loc1/vector<double>:track_param_theta/vector<double>:track_param_qoverp/vector<double>:track_param_time/vector<double>:track_param_charge/vector<int>")

    return helper.result()

def ActsSeedingAlgorithmAnalysisAlgCfg(flags,
                                       name: str = "ActsSeedingAlgorithmAnalysis",
                                       **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    MonitoringGroupNames = []

    if "SeedingTools" not in kwargs:
        from InDetConfig.SiSpacePointsSeedToolConfig import ITkSiSpacePointsSeedMakerCfg
        ITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(ITkSiSpacePointsSeedMakerCfg(flags))
        ITkSiSpacePointsSeedMaker.maxSize = 1e8
        MonitoringGroupNames.append("ITkSiSpacePointSeedMaker")

        from ActsConfig.ActsSeedingConfig import ActsSiSpacePointsSeedMakerToolCfg
        # The default Acts pixel seeding tool performs by default a seed selection after the seed finding
        # We have to disable it or a fair comparison with the other seed computations
        from ActsConfig.ActsSeedingConfig import ActsPixelSeedingToolCfg
        seedToolPixel = acc.popToolsAndMerge(ActsPixelSeedingToolCfg(flags, doSeedQualitySelection=False))
        # We then override the pixel seeding tool inside the ActsSiSpacePointsSeedMakerToolCfg so that we pick this one
        ActsITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(ActsSiSpacePointsSeedMakerToolCfg(flags, SeedToolPixel=seedToolPixel))
        ActsITkSiSpacePointsSeedMaker.doSeedConversion = False
        MonitoringGroupNames.append("ActsITkSiSpacePointSeedMaker")

        from ActsConfig.ActsSeedingConfig import ActsPixelGbtsSeedingToolCfg
        gbtsSeedToolPixel = acc.popToolsAndMerge(ActsPixelGbtsSeedingToolCfg(flags))
        # We then override the pixel seeding tool inside the ActsSiSpacePointsSeedMakerToolCfg so that we pick this one
        # Strip will not be Gbts ... so we ignore it
        ActsGbtsITkSiSpacePointsSeedMaker = acc.popToolsAndMerge(ActsSiSpacePointsSeedMakerToolCfg(flags,
                                                                                                   name="ActsSiSpacePointsSeedMakerGbts",
                                                                                                   SeedToolPixel=gbtsSeedToolPixel))
        ActsGbtsITkSiSpacePointsSeedMaker.doSeedConversion = False
        MonitoringGroupNames.append("ActsGbtsITkSiSpacePointSeedMaker")
        
        from ActsConfig.ActsSeedingConfig import ActsPixelOrthogonalSeedingToolCfg, ActsStripOrthogonalSeedingToolCfg
        pixel_orthogonal_seeding_tool = acc.popToolsAndMerge(ActsPixelOrthogonalSeedingToolCfg(flags))
        strip_orthogonal_seeding_tool = acc.popToolsAndMerge(ActsStripOrthogonalSeedingToolCfg(flags))
        ActsITkSiSpacePointsSeedMakerOrthogonal = \
          acc.popToolsAndMerge(ActsSiSpacePointsSeedMakerToolCfg(flags,
                                                                 name="ActsSiSpacePointsSeedMakerOrthogonal",
                                                                 SeedToolPixel=pixel_orthogonal_seeding_tool,
                                                                 SeedToolStrip=strip_orthogonal_seeding_tool))
        ActsITkSiSpacePointsSeedMakerOrthogonal.doSeedConversion = False
        MonitoringGroupNames.append("ActsOrthogonalITkSiSpacePointSeedMaker")

        from GaudiKernel.GaudiHandles import PrivateToolHandleArray
        kwargs.setdefault("SeedingTools",
                          PrivateToolHandleArray([ITkSiSpacePointsSeedMaker,
                                                  ActsITkSiSpacePointsSeedMaker,
                                                  ActsGbtsITkSiSpacePointsSeedMaker,
                                                  ActsITkSiSpacePointsSeedMakerOrthogonal]))

    kwargs.setdefault("MonitorNames", MonitoringGroupNames)

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'SeedingAlgorithmAnalysisAlgCfg')
    monitoringAlgorithm = helper.addAlgorithm(CompFactory.ActsTrk.SeedingAlgorithmAnalysisAlg, name, **kwargs)

    for groupName in MonitoringGroupNames:
      monitoringGroup = helper.addGroup(monitoringAlgorithm, groupName, '/'+groupName+'/')
      monitoringGroup.defineTree('eventNumber,stripSeedInitialisationTime,stripSeedProductionTime,pixelSeedInitialisationTime,pixelSeedProductionTime,numberPixelSpacePoints,numberStripSpacePoints,numberPixelSeeds,numberStripSeeds;seedInformation',
                                 path='ntuples',
                                 treedef='eventNumber/I:stripSeedInitialisationTime/F:stripSeedProductionTime/F:pixelSeedInitialisationTime/F:pixelSeedProductionTime/F:numberPixelSpacePoints/I:numberStripSpacePoints/I:numberPixelSeeds/I:numberStripSeeds/I')

    acc.merge(helper.result())
    return acc


def ActsPixelEstimatedTrackParamsAnalysisAlgCfg(flags,
                                                name: str = 'ActsPixelEstimatedTrackParamsAnalysisAlg',
                                                extension: str = "Acts",
                                                **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputTrackParamsCollection', 'ActsPixelEstimatedTrackParams')
    return ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags,
                                                      name,
                                                      extension,
                                                      histoPath = extension.replace("Acts", "") + 'PixelEstimatedTrackParams',
                                                      ntupleName = extension.replace("Acts", "") + 'PixelEstimatedTrackParams',
                                                      **kwargs)


def ActsStripEstimatedTrackParamsAnalysisAlgCfg(flags,
                                                name: str = 'ActsStripEstimatedTrackParamsAnalysisAlg',
                                                extension: str = "Acts",
                                                **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputTrackParamsCollection', 'ActsStripEstimatedTrackParams')
    return ActsBaseEstimatedTrackParamsAnalysisAlgCfg(flags,
                                                      name,
                                                      extension,
                                                      histoPath = extension.replace("Acts", "") + 'StripEstimatedTrackParams',
                                                      ntupleName = extension.replace("Acts", "") + 'StripEstimatedTrackParams',
                                                      **kwargs)


def ActsBaseSeedsToTrackParamsAlgCfg(flags,
                                     name: str = "",
                                     **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if 'ExtrapolationTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsExtrapolationToolCfg
        kwargs.setdefault(
            'ExtrapolationTool',
            acc.popToolsAndMerge(ActsExtrapolationToolCfg(flags, MaxSteps=10000)),
        )

    if 'TrackingGeometryTool' not in kwargs:
        from ActsConfig.ActsGeometryConfig import ActsTrackingGeometryToolCfg
        kwargs.setdefault(
            'TrackingGeometryTool',
            acc.popToolsAndMerge(ActsTrackingGeometryToolCfg(flags)),
        )

    if 'ATLASConverterTool' not in kwargs:
        from ActsConfig.ActsEventCnvConfig import ActsToTrkConverterToolCfg
        kwargs.setdefault('ATLASConverterTool', acc.popToolsAndMerge(ActsToTrkConverterToolCfg(flags)))

    if 'TrackParamsEstimationTool' not in kwargs:
        from ActsConfig.ActsTrackParamsEstimationConfig import ActsTrackParamsEstimationToolCfg
        kwargs.setdefault('TrackParamsEstimationTool', acc.popToolsAndMerge(ActsTrackParamsEstimationToolCfg(flags)))

    acc.addEventAlgo(CompFactory.ActsTrk.SeedsToTrackParamsAlg(name, **kwargs))
    return acc

def ActsPixelSeedsToTrackParamsAlgCfg(flags,
                                      name: str = 'ActsPixelSeedsToTrackParamsAlg',
                                      extension: str = "Acts",
                                      **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputSeedContainerKey', 'ActsPixelSeeds')
    kwargs.setdefault('DetectorElementsKey', 'ITkPixelDetectorElementCollection')
    kwargs.setdefault('OutputTrackParamsCollectionKey', 'ActsPixelEstimatedTrackParams')
    return ActsBaseSeedsToTrackParamsAlgCfg(flags,
                                            name,
                                            **kwargs)


def ActsStripSeedsToTrackParamsAlgCfg(flags,
                                      name: str = 'ActsStripSeedsToTrackParamsAlgCfg',
                                      extension: str = "Acts",
                                      **kwargs) -> ComponentAccumulator:
    kwargs.setdefault('InputSeedContainerKey', 'ActsStripSeeds')
    kwargs.setdefault('DetectorElementsKey', 'ITkStripDetectorElementCollection')
    kwargs.setdefault('OutputTrackParamsCollectionKey', 'ActsStripEstimatedTrackParams')
    return ActsBaseSeedsToTrackParamsAlgCfg(flags,
                                            name,
                                            **kwargs)


def PhysValActsCfg(flags,
                   name: str = 'PhysValActs',
                   **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    # deduce what to analyse from the collectiosn in the input file
    typedCollections = flags.Input.TypedCollections
    for col in typedCollections:
        col_type, col_name = col.split("#")
        if col_type == "xAOD::PixelClusterContainer":
            kwargs.setdefault("doPixelClusters", True)
        elif col_type == "xAOD::StripClusterContainer":
            kwargs.setdefault("doStripClusters", True)
        elif col_type == "xAOD::HGTDClusterContainer":
            kwargs.setdefault("doHgtdClusters", flags.PhysVal.IDPVM.doHGTD)
        elif col_type == "xAOD::SpacePointContainer":
            if 'Pixel' in col_name:
                kwargs.setdefault("doPixelSpacePoints", True)
            elif 'Strip' in col_name:
                kwargs.setdefault("doStripSpacePoints", True)
                kwargs.setdefault("doStripOverlapSpacePoints", True)

    acc.setPrivateTools(CompFactory.ActsTrk.PhysValTool(name=name,
                                                        **kwargs))
    return acc
    
def ActsSeedAnalysisCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSeedAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripSeedAnalysisAlgCfg(flags))
    return acc


def ActsEstimatedTrackParamsAnalysisCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelEstimatedTrackParamsAnalysisAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripEstimatedTrackParamsAnalysisAlgCfg(flags))
    return acc


def ActsSeedsToTrackParamsCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelSeedsToTrackParamsAlgCfg(flags))
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripSeedsToTrackParamsAlgCfg(flags))
    return acc
