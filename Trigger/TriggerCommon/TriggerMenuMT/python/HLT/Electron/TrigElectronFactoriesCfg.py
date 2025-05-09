#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

__doc__ = "ToolFactories to configure egammaAlgs to be used at the HLT" 

"""
This file defines the factories of the algorithms to be used in an electron trigger sequence in athenaMT
These are inspired by the offline factories, alhtough modified so they reflect the configuration we need for these algorithms at the HLT. 
Offline configurations are available here:
    https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/


"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.Enums import BeamType

def TrigEMTrackMatchBuilderToolCfg(flags,tag,trackparticles):
    acc = ComponentAccumulator()
    name='TrigEMTrackMatchBuilder'+tag
    from egammaTrackTools.egammaTrackToolsConfig import EMExtrapolationToolsCfg
    emExtrapolatorTools = acc.popToolsAndMerge(EMExtrapolationToolsCfg(flags))
    builderTool = CompFactory.EMTrackMatchBuilder(  name, #TODO, this is provate tool, it does not need to be specifically named
                                                    TrackParticlesName = trackparticles,
                                                    ExtrapolationTool  = emExtrapolatorTools,
                                                    broadDeltaEta      = 0.1, #candidate match is done in 2 times this  so +- 0.2
                                                    broadDeltaPhi      = 0.15,  #candidate match is done in 2 times this  so +- 0.3
                                                    useCandidateMatch  = True,
                                                    useScoring         = True,
                                                    SecondPassRescale  = True,
                                                    UseRescaleMetric   = True,
                                                    isCosmics          = flags.Beam.Type is BeamType.Cosmics)
    acc.setPrivateTools(builderTool)
    return acc


def TrigEgammaRecElectronCfg(flags, tag, trackparticles, calocluster, egammaRecContainer):
        acc = ComponentAccumulator()
        name = 'TrigEgammaRecElectron'+tag
        electronRec = CompFactory.egammaRecBuilder( name,
                                                    InputClusterContainerName= calocluster,
                                                    egammaRecContainer= egammaRecContainer,
                                                    doConversions = False,
                                                    TrackMatchBuilderTool = acc.popToolsAndMerge(TrigEMTrackMatchBuilderToolCfg(flags, tag, trackparticles)) )
        acc.addEventAlgo(electronRec)
        return acc


def TrigElectronSuperClusterBuilderCfg(flags, tag, InputEgammaRecContainerName, SuperElectronRecCollectionName,trackparticles):
        acc = ComponentAccumulator()
        from egammaTools.egammaSwToolConfig import egammaSwToolCfg
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        superClusterBuilder = CompFactory.electronSuperClusterBuilder( 'TrigElectronSuperClusterBuilder'+tag,
                                                                        InputEgammaRecContainerName = InputEgammaRecContainerName,
                                                                        OutputEgammaRecContainerKey = SuperElectronRecCollectionName,
                                                                        ClusterCorrectionTool = acc.popToolsAndMerge(egammaSwToolCfg(flags)),
                                                                        MVACalibSvc = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags)),
                                                                        EtThresholdCut = 1000,
                                                                        TrackMatchBuilderTool = acc.popToolsAndMerge(TrigEMTrackMatchBuilderToolCfg(flags, tag, trackparticles)),
                                                                        LinkToConstituents = False)
        acc.addEventAlgo(superClusterBuilder)
        return acc



def TrigEMClusterToolCfg(flags,variant,OutputClusterContainerName):
        acc = ComponentAccumulator()
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        tool = CompFactory.EMClusterTool('TrigEMClusterTool_electron'+variant,
                                            OutputClusterContainerName = OutputClusterContainerName,
                                            MVACalibSvc = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags))
        )        
        acc.setPrivateTools(tool)
        return acc


def TrigTopoEgammaElectronCfg(flags, tag, variant, cellsName, InputElectronRecCollectionName, InputPhotonRecCollectionName, ElectronOutputName, PhotonOutputName, OutputClusterContainerName):
        acc = ComponentAccumulator()
        from egammaTools.EMShowerBuilderConfig import EMShowerBuilderCfg
        builder = CompFactory.xAODEgammaBuilder(name='topoEgammaBuilder_TrigElectrons'+tag,
                                                InputElectronRecCollectionName = InputElectronRecCollectionName,
                                                InputPhotonRecCollectionName = InputPhotonRecCollectionName,
                                                ElectronOutputName = ElectronOutputName,
                                                PhotonOutputName = PhotonOutputName,  
                                                DummyElectronOutputName = "HLT_PrecisionDummyElectron",
                                                AmbiguityTool = CompFactory.EGammaAmbiguityTool(),
                                                EMClusterTool = acc.popToolsAndMerge(TrigEMClusterToolCfg(flags,variant,OutputClusterContainerName)),
                                                EMShowerTool = acc.popToolsAndMerge(EMShowerBuilderCfg(flags, CellsName=cellsName)),
                                                egammaTools = [CompFactory.EMFourMomBuilder()], # TODO use list defined elsewhere
                                                doPhotons = False,
                                                doElectrons = True)
        acc.addEventAlgo(builder)
        return acc


def TrigTrackIsolationToolCfg(flags,tag,trackParticleLocation):
        from InDetConfig.InDetTrackSelectionToolConfig import InDetTrackSelectionTool_Loose_Cfg
        acc = ComponentAccumulator()

        tpicTool = CompFactory.xAOD.TrackParticlesInConeTool(TrackParticleLocation = trackParticleLocation)
        tiTool = CompFactory.xAOD.TrackIsolationTool(name='TrigTrackIsolationTool'+tag,
                                                     TrackParticleLocation = trackParticleLocation,
                                                     VertexLocation = '',
                                                     TracksInConeTool  = tpicTool,
                                                     TrackSelectionTool = acc.popToolsAndMerge(
                                                         InDetTrackSelectionTool_Loose_Cfg(flags, maxZ0SinTheta = 3, minPt = 1000)))
        acc.setPrivateTools(tiTool)
        return acc


def TrigElectronIsoBuilderCfg(flags, tag, TrackParticleLocation, electronCollectionContainerName, useBremAssoc):
        acc = ComponentAccumulator()
        from xAODPrimitives.xAODIso import xAODIso as isoPar
        builder = CompFactory.IsolationBuilder(
                                        name = 'TrigElectronIsolationBuilder'+tag,
                                        ElectronCollectionContainerName = electronCollectionContainerName,
                                        CaloCellIsolationTool = None,
                                        CaloTopoIsolationTool = None,
                                        PFlowIsolationTool    = None,
                                        useBremAssoc          = useBremAssoc,
                                        TrackIsolationTool    = acc.popToolsAndMerge(TrigTrackIsolationToolCfg(flags,tag,TrackParticleLocation)),
                                        ElIsoTypes            = [[isoPar.ptcone30,isoPar.ptcone20]],
                                        ElCorTypes            = [[isoPar.coreTrackPtr]],
                                        ElCorTypesExtra       = [[]],
                                        IsTrigger = True)
        acc.addEventAlgo(builder)
        return acc

def PrecisionElectronTopoMonitorCfg(flags, tag, electronKey, isoVarKeys):
    acc = ComponentAccumulator()
    name = 'PrecisionElectronTopoMonitoring'+tag
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name)
    PrecisionElectronTopoMonitor = CompFactory.egammaMonitorElectronAlgorithm(
                                                                            name = name,
                                                                            ElectronKey = electronKey,
                                                                            IsoVarKeys = isoVarKeys,
                                                                            MonTool = monTool)
    acc.addEventAlgo(PrecisionElectronTopoMonitor) 
    return acc

def PrecisionElectronSuperClusterMonitorCfg(flags, tag, inputEgammaRecContainerName):
    acc = ComponentAccumulator()
    name = 'PrecisionElectronSuperClusterMonitoring'+tag
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorSuperClusterCfg
    monTool = egammaMonitorSuperClusterCfg(flags, name) 
    PrecisionElectronSuperClusterMonitor = CompFactory.egammaMonitorSuperClusterAlgorithm(
                                                                                        name = name,
                                                                                        InputEgammaRecContainerName = inputEgammaRecContainerName,
                                                                                        MonTool = monTool)
    acc.addEventAlgo(PrecisionElectronSuperClusterMonitor)
    return acc

