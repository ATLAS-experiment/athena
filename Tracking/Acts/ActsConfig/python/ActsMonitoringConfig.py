# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsMonitoringHistSvcCfg(flags) -> ComponentAccumulator:
    acc = ComponentAccumulator()
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
                            xbins=100, xmin=0, xmax=10000)
    
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
                            xbins=100, xmin=0, xmax=10000)
    
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
    monTool.defineHistogram('numStripSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Space Points',
                            xbins=100, xmin=0, xmax=1000000)
    monTool.defineHistogram('numStripOverlapSpacePoints', path='EXPERT', type='TH1I', title='Number of Strip Overlap Space Points',
                            xbins=100, xmin=0, xmax=100000)
    
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
