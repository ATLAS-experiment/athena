# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

"""
This file defines the factories of the algorithms to be used in a photon trigger sequence in athenaMT
These are inspired by the offline factories, alhtough modified so they reflect the configuration we need for these algorithms at the HLT. 
Offline configurations are available here:
  https://gitlab.cern.ch/atlas/athena/blob/master/Reconstruction/egamma/egammaAlgs/python/
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def PrecisionPhotonCaloIsoMonitorCfg(flags, name = 'PrecisionPhotonCaloIsoEgammaBuilderMon', ion=False):
    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags, ion = ion)
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name)
    collectionIn = TrigEgammaKeys.precisionPhotonContainer
    PrecisionPhotonCaloIsoMonitor = CompFactory.egammaMonitorPhotonAlgorithm(
                    name = name,
                    PhotonKey = TrigEgammaKeys.precisionPhotonContainer,
                    IsoVarKeys = [ '%s.topoetcone20' % collectionIn, '%s.topoetcone40' % collectionIn],
                    MonTool = monTool)
    acc.addEventAlgo(PrecisionPhotonCaloIsoMonitor)
    return acc

def TrigEMClusterToolCfg(flags, ion=False):
    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags, ion = ion)
    from egammaMVACalib.egammaMVACalibConfig import egammaMVASvcCfg
    tool = CompFactory.EMClusterTool('TrigEMClusterTool_photon',
                                      OutputClusterContainerName = TrigEgammaKeys.precisionPhotonEMClusterContainer,
                                      MVACalibSvc = acc.getPrimaryAndMerge(egammaMVASvcCfg(flags,name = "TrigEgammaMVASvc")))
    acc.setPrivateTools(tool)
    return acc

def TrigTopoEgammaPhotonCfg(flags):
    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags)
    from egammaTools.EMShowerBuilderConfig import EMShowerBuilderCfg
    from egammaTools.EMPIDBuilderConfig import EMPIDBuilderPhotonCfg
    TrigTopoEgammaPhotons = CompFactory.xAODEgammaBuilder( name = 'TrigTopoEgammaPhotons',
                                                           InputElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterRecCollection,
                                                           InputPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterCollection,
                                                           ElectronOutputName = TrigEgammaKeys.precisionElectronContainer,
                                                           PhotonOutputName = TrigEgammaKeys.precisionPhotonContainer,
                                                           EMClusterTool = acc.popToolsAndMerge(TrigEMClusterToolCfg(flags, ion=False)),
                                                           EMShowerTool = acc.popToolsAndMerge(EMShowerBuilderCfg(flags,name='TrigEMShowerBuilder',CellsName="CaloCells")),
                                                           PhotonTools = [acc.popToolsAndMerge(EMPIDBuilderPhotonCfg(flags,name='TrigEMPIDBuilderPhotonCfg'))],
                                                           doPhotons = True,
                                                           doElectrons = False,
                                                          )
    acc.addEventAlgo(TrigTopoEgammaPhotons)
    return acc

def TrigTopoEgammaPhotonCfg_HI(flags):
    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags,ion=True)
    from egammaTools.EMShowerBuilderConfig import EMShowerBuilderCfg
    from egammaTools.EMPIDBuilderConfig import EMPIDBuilderPhotonCfg
    TrigTopoEgammaPhotons = CompFactory.xAODEgammaBuilder( name = 'TrigTopoEgammaPhotons_HI',
                                                           InputElectronRecCollectionName = TrigEgammaKeys.precisionElectronSuperClusterRecCollection,
                                                           InputPhotonRecCollectionName = TrigEgammaKeys.precisionPhotonSuperClusterCollection,
                                                           ElectronOutputName = TrigEgammaKeys.precisionElectronContainer,
                                                           PhotonOutputName = TrigEgammaKeys.precisionPhotonContainer,
                                                           EMClusterTool = acc.popToolsAndMerge(TrigEMClusterToolCfg(flags, ion=True)),
                                                           EMShowerTool = acc.popToolsAndMerge(EMShowerBuilderCfg(flags,name='TrigEMShowerBuilder_HI',CellsName="CorrectedRoICaloCells")),
                                                           PhotonTools = [acc.popToolsAndMerge(EMPIDBuilderPhotonCfg(flags,name='TrigEMPIDBuilderPhotonCfg_HI'))],
                                                           doPhotons = True,
                                                           doElectrons = False,
                                                          )
    acc.addEventAlgo(TrigTopoEgammaPhotons)
    return acc

def PrecisionPhotonTopoMonitorCfg(flags, ion=False,name = 'PrecisionPhotonTopoMonitoring'):

    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags,ion=ion)
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorPrecisionCfg
    monTool = egammaMonitorPrecisionCfg(flags, name+('HI' if ion is True else ''))

    PrecisionPhotonTopoMonitor = CompFactory.egammaMonitorPhotonAlgorithm(
            name = name+('HI' if ion is True else ''),
            PhotonKey = TrigEgammaKeys.precisionPhotonContainer,
            MonTool = monTool
            )

    acc.addEventAlgo(PrecisionPhotonTopoMonitor)
    return acc

def PrecisionPhotonSuperClusterMonitorCfg(flags, ion = False, name ='PrecisionPhotonSuperClusterMonitoring'):

    acc = ComponentAccumulator()
    from TriggerMenuMT.HLT.Egamma.TrigEgammaKeys import getTrigEgammaKeys
    TrigEgammaKeys = getTrigEgammaKeys(flags, ion=ion)
    from TrigEgammaMonitoring.egammaMonitorPrecisionConfig import egammaMonitorSuperClusterCfg
    monTool = egammaMonitorSuperClusterCfg(flags, name+('HI' if ion is True else ''))

    PrecisionPhotonSuperClusterMonitor = CompFactory.egammaMonitorSuperClusterAlgorithm(
            name = name+('HI' if ion is True else ''),
            InputEgammaRecContainerName = TrigEgammaKeys.precisionPhotonSuperClusterCollection,
            MonTool = monTool
            )

    acc.addEventAlgo(PrecisionPhotonSuperClusterMonitor)
    return acc
