# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys

def TrigEgammaRecCfg(flags, name= "trigEgammaRec"):
    acc = ComponentAccumulator()
    TrigEgammaKeys = getTrigEgammaKeys()
    egammaRec = CompFactory.egammaRecBuilder( name = name,
                                                  InputClusterContainerName = TrigEgammaKeys.precisionCaloTopoCollection, # input,
                                                  egammaRecContainer        = TrigEgammaKeys.precisionCaloEgammaRecCollection, # output,
                                                  doTrackMatching           = False,
                                                  # Builder tools
                                                  TrackMatchBuilderTool     = None, # Don't want to use these for trigger....
                                                  ConversionBuilderTool     = None, 
                                                  doConversions             = False)
    acc.addEventAlgo(egammaRec)
    return acc

def TrigEgammaSuperClusterBuilderCfg(flags, name, calibrationType, superClusterCollectionName, superegammaRecCollectionName):
        acc = ComponentAccumulator()
        TrigEgammaKeys = getTrigEgammaKeys()
        from egammaTools.egammaSwToolConfig import egammaSwToolCfg
        from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
        trigMVAfolder = flags.Trigger.egamma.Calib.precCaloMVAVersion
        TrigEgammaSuperClusterBuilder = CompFactory.egammaSuperClusterBuilder( 
                               name = name,
                               InputEgammaRecContainerName  = TrigEgammaKeys.precisionCaloEgammaRecCollection,
                               SuperClusterCollectionName   = superClusterCollectionName,
                               OutputEgammaRecContainerKey = superegammaRecCollectionName, # output,
                               ClusterCorrectionTool        = acc.popToolsAndMerge(egammaSwToolCfg(flags)),   
                               MVACalibSvc                  = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags,name="trigPrecCaloEgammaMVASvc",folder = trigMVAfolder)),
                               CalibrationType              = calibrationType,
                               EtThresholdCut               = 1000,
                               LinkToConstituents           = False)
        acc.addEventAlgo(TrigEgammaSuperClusterBuilder)
        return acc

def TrigCaloClustersInConeToolCfg(flags, ion):
        acc = ComponentAccumulator()
        if ion:
            TrigEgammaKeys = getTrigEgammaKeys(ion =ion)
            name = "TrigCaloClustersInConeToolHI"
        else:
            TrigEgammaKeys = getTrigEgammaKeys()
            name = "TrigCaloClustersInConeTool"
        tool = CompFactory.xAOD.CaloClustersInConeTool(name = name,
                                                       CaloClusterLocation = TrigEgammaKeys.precisionTopoClusterContainer)
        acc.setPrivateTools(tool)
        return acc

def TrigCaloIsoCorrectionToolCfg(flags):
        acc = ComponentAccumulator()
        tool = CompFactory.CP.IsolationCorrectionTool(name = "TrigLeakageCorrTool")
        acc.setPrivateTools(tool)
        return acc

def CaloFillRectangularClusterCfg(flags, **kwargs):
        result = ComponentAccumulator()
        kwargs.setdefault("eta_size", 5)
        kwargs.setdefault("phi_size", 7)
        kwargs.setdefault("cells_name",'CaloCells')
        result.setPrivateTools(CompFactory.CaloFillRectangularCluster(**kwargs))
        return result

def TrigCaloIsolationToolCfg_HI(flags):
        acc = ComponentAccumulator()
        name = "TrigCaloIsolationToolHI"
        from CaloIdentifier import SUBCALO
        tool = CompFactory.xAOD.CaloIsolationTool(name = name,
                                                   CaloFillRectangularClusterTool  = acc.popToolsAndMerge(CaloFillRectangularClusterCfg(flags,name="trigegamma_CaloFillRectangularCluster_HI")),
                                                   ClustersInConeTool              = acc.popToolsAndMerge(TrigCaloClustersInConeToolCfg(flags,ion=True)),
                                                   FlowElementsInConeTool          = None,
                                                   ParticleCaloExtensionTool       = None,
                                                   IsoLeakCorrectionTool           = acc.popToolsAndMerge(TrigCaloIsoCorrectionToolCfg(flags)),
                                                   ParticleCaloCellAssociationTool = None,
                                                   saveOnlyRequestedCorrections    = True,
                                                   InitializeReadHandles           = False,
                                                   doEnergyDensityCorrection       = True,
                                                   UseEMScale                      = True,
                                                   EMCaloNums                      = [SUBCALO.LAREM],
                                                   HadCaloNums                     = [SUBCALO.LARHEC, SUBCALO.TILE])
        acc.setPrivateTools(tool)
        return acc

def TrigCaloIsolationToolCfg(flags):
        acc = ComponentAccumulator()
        name = "TrigCaloIsolationTool"
        from CaloIdentifier import SUBCALO
        tool = CompFactory.xAOD.CaloIsolationTool(name = name,
                                                   CaloFillRectangularClusterTool  = acc.popToolsAndMerge(CaloFillRectangularClusterCfg(flags,name="trigegamma_CaloFillRectangularCluster")),
                                                   ClustersInConeTool              = acc.popToolsAndMerge(TrigCaloClustersInConeToolCfg(flags,ion=False)),
                                                   FlowElementsInConeTool          = None,
                                                   ParticleCaloExtensionTool     = None,
                                                   IsoLeakCorrectionTool           = acc.popToolsAndMerge(TrigCaloIsoCorrectionToolCfg(flags)),
                                                   ParticleCaloCellAssociationTool = None,
                                                   saveOnlyRequestedCorrections    = True,
                                                   InitializeReadHandles           = True,
                                                   TopoClusterEDCentralContainer   = 'TrigIsoEventShape',
                                                   TopoClusterEDForwardContainer   = 'TrigIsoEventShape',
                                                   EFlowEDCentralContainer         = 'TrigIsoEventShape',
                                                   EFlowEDForwardContainer         = 'TrigIsoEventShape',
                                                   EMCaloNums                      = [SUBCALO.LAREM],
                                                   HadCaloNums                     = [SUBCALO.LARHEC, SUBCALO.TILE])
        acc.setPrivateTools(tool)
        return acc


def TrigPhotonIsoBuilderCfg(flags, ion = False):
        acc = ComponentAccumulator()
        if ion:
            name = 'TrigPhotonIsolationBuilderHI'
            TrigEgammaKeys = getTrigEgammaKeys(ion=ion)
            TrigCaloIsolationTool = TrigCaloIsolationToolCfg_HI(flags)
        else:
            name = 'TrigPhotonIsolationBuilder'
            TrigEgammaKeys = getTrigEgammaKeys()
            TrigCaloIsolationTool = TrigCaloIsolationToolCfg(flags)

        from xAODPrimitives.xAODIso import xAODIso as isoPar
        TrigPhotonIsolationBuilder = CompFactory.IsolationBuilder(name                  = name,
                                                                  PhotonCollectionContainerName = TrigEgammaKeys.precisionPhotonContainer,
                                                                  CaloCellIsolationTool = None,
                                                                  CaloTopoIsolationTool = acc.popToolsAndMerge(TrigCaloIsolationTool),
                                                                  PFlowIsolationTool    = None,
                                                                  TrackIsolationTool    = None, 
                                                                  PhIsoTypes            = [[isoPar.topoetcone20, isoPar.topoetcone30, isoPar.topoetcone40]],
                                                                  PhCorTypes            = [[isoPar.core57cells, isoPar.pileupCorrection]],
                                                                  PhCorTypesExtra       = [[]],
                                                                  )
        acc.addEventAlgo(TrigPhotonIsolationBuilder)
        return acc


def TrigEgammaPseudoJetAlgCfg(flags, name='TrigPhotonEgammaPSeudoJetBuilder'):
        acc = ComponentAccumulator()
        # This is to run pseudoJetAlgorithm to compute event density over FullScan TopoClusters
        TrigEgammaPseudoJetAlgBuilder = CompFactory.PseudoJetAlgorithm(name               = name,
                                                                       Label              = "EMTopo",
                                                                       InputContainer     = "HLT_TopoCaloClustersFS", 
                                                                       OutputContainer    = "PseudoJetTrigEMTopo",
                                                                       SkipNegativeEnergy = True)
        acc.addEventAlgo(TrigEgammaPseudoJetAlgBuilder)
        return acc


def TrigConfigEventDensityToolCfg( flags, **kwargs ):
        acc = ComponentAccumulator()
        kwargs.setdefault("JetAlgorithm", "Kt")
        kwargs.setdefault("JetRadius", 0.5)
        kwargs.setdefault("InputContainer", "PseudoJetTrigEMTopo")
        kwargs.setdefault("AbsRapidityMin",0.0)
        kwargs.setdefault("AbsRapidityMax",2.0)
        kwargs.setdefault("AreaDefinition","Voronoi")
        kwargs.setdefault("VoronoiRfact",0.9)
        kwargs.setdefault("OutputContainer",'TrigIsoEventShape')
        acc.setPrivateTools(CompFactory.EventDensityTool(**kwargs))
        return acc


def TrigIsoEventShapeAlgCfg (flags, name = 'TrigPhotonIsoEventShapeAlg'):
        acc = ComponentAccumulator()
        TrigIsoEventShapeAlg = CompFactory.EventDensityAthAlg(name = name,
                                                              EventDensityTool = acc.popToolsAndMerge(TrigConfigEventDensityToolCfg(flags=flags,name="TrigIsoTool")))
        acc.addEventAlgo(TrigIsoEventShapeAlg)
        return acc 


def TrigEgammaFSEventDensitySequenceCfg(flags, name = 'TrigEgammaFSEventDensitySequence'):
        acc = ComponentAccumulator()
        from TrigCaloRec.TrigCaloRecConfig import jetmetTopoClusteringCfg
        acc.merge(jetmetTopoClusteringCfg(flags,
                                          RoIs = '') )
        acc.merge(TrigEgammaPseudoJetAlgCfg(flags))
        acc.merge(TrigIsoEventShapeAlgCfg(flags))
        return acc


