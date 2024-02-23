#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetAlgorithmConfig.py
@author M. Aparo
@date 02-10-2023
@brief CA-based python configurations for the event algorithms in this package
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthHitDecoratorAlgCfg( flags, name="TruthHitDecoratorAlg", **kwargs ):
    '''
    create decoration algorithm which decorates
    truth particles with track parameters at the perigee.
    '''
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge( AtlasExtrapolatorCfg(flags) )
    acc.addPublicTool( extrapolator )
    kwargs.setdefault( "Extrapolator", extrapolator )

    acc.addEventAlgo( CompFactory.IDTPM.TruthHitDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineElectronDecoratorAlgCfg( flags, name="OfflineElectronDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    acc.addEventAlgo( CompFactory.IDTPM.OfflineElectronDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineElectronGSFDecoratorAlgCfg( flags, name="OfflineElectronGSFDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "OfflineTrkParticleContainerName", "GSFTrackParticles" )
    kwargs.setdefault( "useGSF", True )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineElectronDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineMuonDecoratorAlgCfg( flags, name="OfflineMuonDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    acc.addEventAlgo( CompFactory.IDTPM.OfflineMuonDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineMuonCombDecoratorAlgCfg( flags, name="OfflineMuonDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "OfflineTrkParticleContainerName", "CombinedMuonTrackParticles" )
    kwargs.setdefault( "useCombinedMuonTracks", True )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineMuonDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineTauBDT1prongDecoratorAlgCfg( flags, name="OfflineTauBDT1prongDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "Prefix", "LinkedTauBDT1prong_" )
    kwargs.setdefault( "TauType", "BDT" )
    kwargs.setdefault( "TauNprongs", 1 )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineTauDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineTauBDT3prongDecoratorAlgCfg( flags, name="OfflineTauBDT3prongDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "Prefix", "LinkedTauBDT3prong_" )
    kwargs.setdefault( "TauType", "BDT" )
    kwargs.setdefault( "TauNprongs", 3 )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineTauDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineTauRNN1prongDecoratorAlgCfg( flags, name="OfflineTauRNN1prongDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "Prefix", "LinkedTauRNN1prong_" )
    kwargs.setdefault( "TauType", "RNN" )
    kwargs.setdefault( "TauNprongs", 1 )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineTauDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineTauRNN3prongDecoratorAlgCfg( flags, name="OfflineTauRNN3prongDecoratorAlg", **kwargs ):
    acc = ComponentAccumulator()
    kwargs.setdefault( "Prefix", "LinkedTauRNN3prong_" )
    kwargs.setdefault( "TauType", "RNN" )
    kwargs.setdefault( "TauNprongs", 3 )
    acc.addEventAlgo( CompFactory.IDTPM.OfflineTauDecoratorAlg( name, **kwargs ) )
    return acc


def OfflineObjectDecoratorAlgCfg( flags, name="OfflineObjectDecoratorAlg", **kwargs ):
    '''
    create decoration algorithm(s) to decorate offline tracks with a link to
    the offline object they correspond to in the event reconstruction
    '''
    acc = ComponentAccumulator()

    objStrList = []
    tauTypeList = []
    for trkAnaName in flags.PhysVal.IDTPM.trkAnaNames:
        objStr = getattr( flags.PhysVal.IDTPM, trkAnaName+".SelectOfflineObject" )
        if objStr : objStrList.append( objStr )
        tauType = getattr( flags.PhysVal.IDTPM, trkAnaName+".TauType" )
        if tauType : tauTypeList.append( tauType )

    if "Electron" in objStrList:
        acc.merge( OfflineElectronDecoratorAlgCfg(flags) )

    if "ElectronGSF" in objStrList:
        acc.merge( OfflineElectronGSFDecoratorAlgCfg(flags) )

    if "Muon" in objStrList:
        acc.merge( OfflineMuonDecoratorAlgCfg(flags) )

    if "MuonComb" in objStrList:
        acc.merge( OfflineMuonCombDecoratorAlgCfg(flags) )

    if "Tau" in objStrList:
        if "BDT" in tauTypeList:
            acc.merge( OfflineTauBDT1prongDecoratorAlgCfg(flags) )
            acc.merge( OfflineTauBDT3prongDecoratorAlgCfg(flags) )
        if "RNN" in tauTypeList:
            acc.merge( OfflineTauRNN1prongDecoratorAlgCfg(flags) )
            acc.merge( OfflineTauRNN3prongDecoratorAlgCfg(flags) )

    return acc
