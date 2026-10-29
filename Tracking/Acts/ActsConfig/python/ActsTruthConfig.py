#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Utils.unixtools import find_datafile
from ActsInterop import UnitConstants
from ActsConfig.ActsUtilities import extractChildKwargs

def MapToInDetSimDataWrapSvcCfg(flags,
                                *,
                                collectionName: str) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    AddressRemappingSvc = CompFactory.AddressRemappingSvc(
        TypeKeyOverwriteMaps = [f"InDetSimDataCollection#{collectionName}->InDetSimDataCollectionWrap#{collectionName}"]
        )
    acc.addService(AddressRemappingSvc)
    return acc


def ActsPixelClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsPixelClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkPixelSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'ITkPixelSDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 300) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'ITkPixelClusters')
    kwargs.setdefault('AssociationMapOut', 'ITkPixelClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.PixelClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsStripClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsStripClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'ITkStripSDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 600) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'ITkStripClusters')
    kwargs.setdefault('AssociationMapOut', 'ITkStripClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.StripClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsHgtdClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsHgtdClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='HGTD_SDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'HGTD_SDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 0.) # original inherited value from strips of 600 MeV was too high
    kwargs.setdefault('Measurements', 'HGTD_Clusters')
    kwargs.setdefault('AssociationMapOut', 'HgtdClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.HgtdClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsInDetPixelClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsInDetPixelClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    # Inner Detector configurations
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='PixelSDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'PixelSDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 300) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'PixelClusters')
    kwargs.setdefault('AssociationMapOut', 'PixelClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.PixelClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsInDetStripClusterToTruthAssociationAlgCfg(flags,
                                             name: str = 'ActsInDetStripClusterToTruthAssociationAlg',
                                             **kwargs: dict) -> ComponentAccumulator:
    # Inner Detector configurations
    acc = ComponentAccumulator()
    acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='SCT_SDO_Map') )

    kwargs.setdefault('InputTruthParticleLinks', 'xAODTruthLinks')
    kwargs.setdefault('SimData', 'SCT_SDO_Map')
    kwargs.setdefault('DepositedEnergyMin', 600) # @TODO revise ? From PRD_MultiTruthBuilder.h; should be 1/10 of threshold
    kwargs.setdefault('Measurements', 'SCT_Clusters')
    kwargs.setdefault('AssociationMapOut', 'SCT_ClustersToTruthParticles')
    
    acc.addEventAlgo( CompFactory.ActsTrk.StripClusterToTruthAssociationAlg(name=name, **kwargs) )
    return acc


### Assemble the list of truth association keys for the TruthHitCount and the 
### Track -> truth association algorithm
def _truthAssociationKeysCfg(flags, doInner:bool =True, doMuon: bool = False):
    assocKeys = []
    if doInner and flags.Detector.EnablePixel:
        assocKeys += ['PixelClustersToTruthParticles']
    if doInner and  flags.Detector.EnableSCT:
        assocKeys += [ 'SCT_ClustersToTruthParticles']
    if doInner and  flags.Detector.EnableITkPixel:
        assocKeys += ['ITkPixelClustersToTruthParticles']
    if doInner and flags.Detector.EnableITkStrip:
        assocKeys += ['ITkStripClustersToTruthParticles']
    if doInner and flags.Reco.EnableHGTDExtension and flags.Acts.useHGTDClusterInTrackFinding:
        assocKeys +=['HgtdClustersToTruthParticles']
    if doMuon and flags.Muon.usePhaseIIGeoSetup:
        from MuonConfig.MuonDataPrepConfig import PrimaryMeasContNamesCfg
        assocKeys+=[f"{cont}_TruthAssocMap" for cont in PrimaryMeasContNamesCfg(flags)]
    return assocKeys


def ActsTrackToTruthAssociationAlgCfg(flags,
                                      name: str = 'ActsTracksToTruthAssociationAlg',
                                      doInner:bool = True, doMuon: bool = False,
                                      **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if doInner and flags.Detector.GeometryITk:
        acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )
    elif doInner and flags.Detector.GeometryID:
        acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='SCT_SDO_Map') ) 

    kwargs.setdefault('ACTSTracksLocation','ActsTracks')
    kwargs.setdefault("MeasurementAssocKeys", _truthAssociationKeysCfg(flags, doInner = doInner, doMuon = doMuon))
    kwargs.setdefault('AssociationMapOut','ActsTracksToTruthParticles')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV)
    acc.addEventAlgo( CompFactory.ActsTrk.TrackToTruthAssociationAlg(name=name, **kwargs) )
    return acc

def ActsInDetTrackToTruthAssociationAlgCfg(flags,
                                      name: str = 'ActsInDetTracksToTruthAssociationAlg',
                                      **kwargs: dict) -> ComponentAccumulator:
    return ActsTrackToTruthAssociationAlgCfg(flags, name=name, **kwargs)

def ActsTruthParticleHitCountAlgCfg(flags,
                                    name: str = 'ActsTruthParticleHitCountAlg',
                                    doInner:bool = True, doMuon: bool = False,
                                    **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    if doInner and flags.Detector.GeometryITk:
        acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='ITkStripSDO_Map') )
    elif doInner and flags.Detector.GeometryID:
        acc.merge( MapToInDetSimDataWrapSvcCfg(flags, collectionName='SCT_SDO_Map') ) 
    
    kwargs.setdefault("MeasurementAssocKeys", _truthAssociationKeysCfg(flags, doInner = doInner, doMuon = doMuon))
    kwargs.setdefault('TruthParticleHitCountsOut','TruthParticleHitCounts')
    kwargs.setdefault('MaxEnergyLoss',1e3*UnitConstants.TeV) # @TODO introduce flag and synchronise with TrackToTruthAssociationAlg
    kwargs.setdefault('NHitsMin',4)


    acc.addEventAlgo( CompFactory.ActsTrk.TruthParticleHitCountAlg(name=name, **kwargs) )
    return acc

def ActsInDetTruthParticleHitCountAlgCfg(flags,
                                    name: str = 'ActsInDetTruthParticleHitCountAlg',
                                    **kwargs: dict) -> ComponentAccumulator:
    return ActsTruthParticleHitCountAlgCfg(flags, name=name, **kwargs)

def ActsTruthAssociationAlgCfg(flags,
                              **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if flags.Detector.EnablePixel:
        acc.merge(ActsInDetPixelClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="PixelClusterToTruthAssociationAlg.", **kwargs) ))
        
    if flags.Detector.EnableSCT:
        acc.merge(ActsInDetStripClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="StripClusterToTruthAssociationAlg.", **kwargs) ))
    
    if flags.Detector.EnableITkPixel:
        acc.merge(ActsPixelClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="PixelClusterToTruthAssociationAlg.", **kwargs) ))
        
    if flags.Detector.EnableITkStrip:
        acc.merge(ActsStripClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="StripClusterToTruthAssociationAlg.", **kwargs) ))
    
    if flags.Reco.EnableHGTDExtension and flags.Acts.useHGTDClusterInTrackFinding:
        acc.merge(ActsHgtdClusterToTruthAssociationAlgCfg(flags, **extractChildKwargs(prefix="HgtdClusterToTruthAssociationAlg.", **kwargs) ))
    return acc

def setDefaultTruthMatchingArgs(kwargs) :
    from ROOT.xAOD import UncalibMeasType
    matchWeights = [0. for d in range(UncalibMeasType.nTypes)]
    countWeights = [0. for d in range(UncalibMeasType.nTypes)]
    matchWeights[UncalibMeasType.PixelClusterType] = 10.
    matchWeights[UncalibMeasType.StripClusterType] = 5.

    #TODO: Re-evaluate the weights!
    matchWeights[UncalibMeasType.MdtDriftCircleType] = 1.
    matchWeights[UncalibMeasType.MMClusterType] = 1.
    matchWeights[UncalibMeasType.sTgcStripType] = 1.
    matchWeights[UncalibMeasType.RpcStripType] = 1.
    matchWeights[UncalibMeasType.TgcStripType] = 1.

    countWeights[UncalibMeasType.PixelClusterType] = 1.
    countWeights[UncalibMeasType.StripClusterType] = 1.
    countWeights[UncalibMeasType.MdtDriftCircleType] = 1.
    countWeights[UncalibMeasType.RpcStripType] = 0.5
    countWeights[UncalibMeasType.TgcStripType] = 0.5
    countWeights[UncalibMeasType.MMClusterType] = 0.25
    countWeights[UncalibMeasType.sTgcStripType] = 0.25
    
    kwargs.setdefault('MatchWeights',matchWeights)
    kwargs.setdefault('CountWeights', countWeights)    
    kwargs.setdefault('StatisticPtBins',[1e3,2.5e3,5e3,10e3,100e3])
    kwargs.setdefault('ShowDetailedTables',False)
    kwargs.setdefault('PdgIdCategorisation',False)
    kwargs.setdefault('StatisticEtaBins',[eta/10. for eta in range(5, 40, 5)])

def ActsTrackParticleTruthDecorationAlgCfg(flags,
                                           name: str = 'ActsTrackParticleTruthDecorationAlg',
                                           **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TrackToTruthAssociationMaps','ActsCombinedTracksToTruthParticleAssociation')
    kwargs.setdefault('TrackParticleContainerName','ActsCombinedTracksParticlesAlt')
    kwargs.setdefault('TruthParticleHitCounts','TruthParticleHitCounts')
    # weights used for computing the matching probability and identifying the best match
    setDefaultTruthMatchingArgs(kwargs)
    kwargs.setdefault('TruthLinkedFakeRate', flags.Acts.truthLinkedFakeRate)
    kwargs.setdefault('ComputeTrackRecoEfficiency',False)

    if 'TruthSelectionTool' not in kwargs:
        # should be as tight or looser as the TruthSelectionTool when analysing the truth matches
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
        kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
            InDetRttTruthSelectionToolCfg(flags,
                                          name='RelaxedInDetRttTruthSelectionTool',
                                          requireOnlyPrimary=False,
                                          minPt=500.,
                                          maxEta=4.5
                                          )))

    if 'MCTruthClassifier' not in kwargs:
        from MCTruthClassifier.MCTruthClassifierConfig import MCTruthClassifierCfg
        kwargs.setdefault("MCTruthClassifier", acc.popToolsAndMerge(
            MCTruthClassifierCfg(flags)))

    acc.addEventAlgo( CompFactory.ActsTrk.TrackParticleTruthDecorationAlg(name=name, **kwargs) )
    return acc

def ActsTrackFindingValidationAlgCfg(flags,
                                     name: str = 'ActsTracksValidationAlg',
                                     **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()
    kwargs.setdefault('TruthParticleHitCounts','TruthParticleHitCounts')
    kwargs.setdefault('TrackToTruthAssociationMap','ActsTracksToTruthParticles')
    setDefaultTruthMatchingArgs(kwargs)
    kwargs.setdefault('TruthLinkedFakeRate', flags.Acts.truthLinkedFakeRate)
    kwargs.setdefault('ComputeTrackRecoEfficiency',True)
    
    if 'TruthSelectionTool' not in kwargs:
        from InDetPhysValMonitoring.InDetPhysValMonitoringConfig import InDetRttTruthSelectionToolCfg
        kwargs.setdefault("TruthSelectionTool", acc.popToolsAndMerge(
            InDetRttTruthSelectionToolCfg(flags)))

    acc.addEventAlgo( CompFactory.ActsTrk.TrackFindingValidationAlg(name=name, **kwargs) )
    return acc

def ActsTruthTrackBuilderToolCfg(
        flags,
        name: str = "TruthTrackBuilderTool",
        **kwargs: dict) -> ComponentAccumulator:

    acc = ComponentAccumulator()

    kwargs.setdefault("usePixelClusters", flags.Acts.Gbts.Training.usePixelClusters)
    kwargs.setdefault("useStripClusters", flags.Acts.Gbts.Training.useStripClusters)

    acc.setPrivateTools(
        CompFactory.ActsTrk.TruthTrackBuilderTool(
            name=name,
            **kwargs
        )
    )

    return acc

def ActsGbtsTrainingAlgCfg(flags,
                           name: str = "ActsGbtsTrainingAlg",
                           **kwargs: dict) -> ComponentAccumulator:
    acc = ComponentAccumulator()

    if "TruthTrackBuilderTool" not in kwargs:
        kwargs.setdefault(
            "TruthTrackBuilderTool",
            acc.popToolsAndMerge(
                ActsTruthTrackBuilderToolCfg(flags)
            )
        )

    kwargs.setdefault("geometryFile", find_datafile("gbts_layer_geometry.txt"))
    kwargs.setdefault("outputConnectionTable", flags.Acts.Gbts.Training.outputConnectionTable)
    kwargs.setdefault("doSymmetrization", flags.Acts.Gbts.Training.doSymmetrization)
    kwargs.setdefault("useOldFormatting", flags.Acts.Gbts.Training.useOldFormatting)
    kwargs.setdefault("probThreshold", flags.Acts.Gbts.Training.probThreshold)

    acc.addEventAlgo(
        CompFactory.ActsTrk.GbtsTrainingAlg(name=name, **kwargs)
    )

    return acc
