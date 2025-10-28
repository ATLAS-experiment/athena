# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMonitoringHistSvcCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    #check that acts is not part of the trigger, if it is then the trigger will be setting up the histogramming service
    if not flags.Trigger.useActsTracking:
        histSvc = CompFactory.THistSvc(Output = ["EXPERT DATAFILE='acts-expert-monitoring.root', OPT='RECREATE'"])
        acc.addService(histSvc)
    return acc

def ActsITkPixelClusterizationMonitoringToolCfg(flags,
                                                name: str = "ActsITkPixelClusterizationMonitoringTool",
                                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('TIME_readingRDOs', path='EXPERT', type='TH1F', title='Time for reading RDOs',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('TIME_clusterization', path='EXPERT', type='TH1F', title='Time for clustering:',
                            xbins=100, xmin=0, xmax=1000)

    monTool.defineHistogram('NClustersCreated', path='EXPERT', type='TH1F', title='Number of clusters produced',
                            xbins=100, xmin=0, xmax=5000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsITkStripClusterizationMonitoringToolCfg(flags,
                                                name: str = "ActsITkStripClusterizationMonitoringTool",
                                                **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('TIME_readingRDOs', path='EXPERT', type='TH1F', title='Time for reading RDOs',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('TIME_clusterization', path='EXPERT', type='TH1F', title='Time for clustering:',
                            xbins=100, xmin=0, xmax=1000)

    monTool.defineHistogram('NClustersCreated', path='EXPERT', type='TH1F', title='Number of clusters produced',
                            xbins=100, xmin=0, xmax=5000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsDataPreparationMonitoringToolCfg(flags,
                                         name: str = "ActsDataPreparationMonitoringTool",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)

    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=100)

    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsHgtdClusterizationMonitoringToolCfg(flags,
                                            name: str = "ActsHgtdClusterizationMonitoringTool",
                                            **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)

    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=50, xmin=0, xmax=400)

    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsHGTDTrackExtensionMonitoringCfg(flags,
                                        name: str = "ActsHGTDTrackExtensionMonitoringTool",
                                        **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)

    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)
    monTool.defineHistogram('track_n_measurements', path='EXPERT', type='TH1I', title='Number of track measurements',
                            xbins=50, xmin=0, xmax=50)
    monTool.defineHistogram('track_n_holes', path='EXPERT', type='TH1I', title='Number of track holes',
                            xbins=50, xmin=0, xmax=50)
    monTool.defineHistogram('track_n_outliers', path='EXPERT', type='TH1I', title='Number of track outliers',
                            xbins=50, xmin=0, xmax=50)
    monTool.defineHistogram('track_chi2_ndf', path='EXPERT', type='TH1F', title='Track chi2/ndf',
                            xbins=10, xmin=0, xmax=100)
    monTool.defineHistogram('cluster_x', path='EXPERT', type='TH1F', title='Cluster x position',
                            xbins=100, xmin=-1000, xmax=1000)
    monTool.defineHistogram('cluster_y', path='EXPERT', type='TH1F', title='Cluster y position',
                            xbins=100, xmin=-1000, xmax=1000)
    monTool.defineHistogram('cluster_z', path='EXPERT', type='TH1F', title='Cluster z position',
                            xbins=1000, xmin=-4000, xmax=4000)
    monTool.defineHistogram('cluster_t', path='EXPERT', type='TH1F', title='Cluster time',
                            xbins=500, xmin=0, xmax=50)
    monTool.defineHistogram('n_hgtd_clusters', path='EXPERT', type='TH1I', title='Number of HGTD clusters',
                            xbins=30, xmin=0, xmax
                            =30)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc


def ActsPixelSpacePointFormationMonitoringToolCfg(flags,
                                                  name: str = "ActsPixelSpacePointFormatioMonitoringTool",
                                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numPixSpacePoints', path='EXPERT', type='TH1I', title='Number of Pixel Space Points',
                            xbins=100, xmin=0, xmax=1000000)

    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsStripSpacePointFormationMonitoringToolCfg(flags,
                                                  name: str = "ActsStripSpacePointFormationMonitoringTool",
                                                  **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('TIME_containerAccessor', path='EXPERT', type='TH1F', title='Time for execute of containerAccessor',
                            xbins=100, xmin=0, xmax=1000)
    monTool.defineHistogram('numStripSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Space Points',
                            xbins=100, xmin=0, xmax=1000000)
    monTool.defineHistogram('numStripOverlapSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Overlap Space Points',
                            xbins=100, xmin=0, xmax=100000)
    
    monTool.defineHistogram('nCachedIdHashes', path='EXPERT', type='TH1I', title='Number of cached ID hashes which have been inserted',
                            xbins=100, xmin=0, xmax=5000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsITkPixelSeedingMonitoringToolCfg(flags,
                                         name: str = "ActsITkPixelSeedingMonitoringTool",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_seedCreation', path='EXPERT', type='TH1F', title='Time for seed creation',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_parameterEstimation', path='EXPERT', type='TH1F', title='Time for parameter estimation',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('nSeeds', path='EXPERT', type='TH1I', title='Number of seeds',
                            xbins=100, xmin=0, xmax=100000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))     
    return acc
    
def ActsITkStripSeedingMonitoringToolCfg(flags,
                                         name: str = "ActsITkStripSeedingMonitoringTool",
                                         **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_seedCreation', path='EXPERT', type='TH1F', title='Time for seed creation',
                            xbins=50, xmin=0, xmax=10000)
    monTool.defineHistogram('TIME_parameterEstimation', path='EXPERT', type='TH1F', title='Time for parameter estimation',
                            xbins=50, xmin=0, xmax=10000)
    
    monTool.defineHistogram('nSeeds', path='EXPERT', type='TH1I', title='Number of seeds',
                            xbins=100, xmin=0, xmax=100000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))     
    return acc

def ActsTrackFindingMonitoringToolCfg(flags,
                                      name: str = "ActsTrackFindingMonitoringTool",
                                      **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)
    
    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title="Time for execute",
                            xbins=100, xmin=0, xmax=70000)
    monTool.defineHistogram('nTracks', path='EXPERT', type='TH1I', title='Number of tracks',
                            xbins=100, xmin=0, xmax=100000)
    
    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc

def ActsAmbiguityResolutionMonitoringToolCfg(flags,
                                             name: str = "ActsAmbiguityResolutionMonitoringTool",
                                             **kwargs) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, name)

    monTool.defineHistogram('TIME_execute', path='EXPERT', type='TH1F', title='Time for execute',
                            xbins=100, xmin=0, xmax=10000)

    acc.setPrivateTools(monTool)
    acc.merge(ActsMonitoringHistSvcCfg(flags))
    return acc
