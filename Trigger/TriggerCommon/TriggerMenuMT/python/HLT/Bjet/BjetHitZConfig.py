# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Hit-based per-jet z regression (HitZ) for the b-jet trigger.

Runs, in a step of its own over the jet super-RoI and before any tracking,
the hit pipeline the training-dataset-dumper runs offline: the Si clusters are
built in the view, converted to xAOD, cleaned, decorated with beamspot-relative
coordinates, associated to the jets in a wedge, merged and truncated, then
passed to an ONNX regression.

Unlike the taggers in BjetFlavourTaggingConfig, nothing here consumes tracks or
a vertex.
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# xAOD containers the cluster converters write into
PIXEL_HITS = 'PixelClusters'
STRIP_HITS = 'StripClusters'

# HitsLoader in FlavorTagInference resolves the hits by this decoration name and
# does not honour variableRemapping, so it is not free to choose
HIT_ASSOCIATION = 'hitsAssociatedWithJet'

# PixelPrepDataToxAOD writes these through the AUXDATA macro rather than a
# WriteDecorHandleKey, so the scheduler cannot see them without a nudge
PIXEL_GOODNESS_DECORS = ['isFake', 'hasBSError', 'DCSState']

# The wedge and the hit budget have to match what the network was trained with
MAX_HITS = 700
DPHI_HIT_TO_JET = 0.1
DETA_HIT_TO_JET = 0.1
DZ_HIT_TO_VERTEX = 180

# HitZ runs before the menu's JetViewAlg, so it applies its own acceptance
MIN_JET_PT = 20e3
MAX_ABS_JET_ETA = 4.0

# Map the MDN output names onto the (z, sigma) pair the menu side expects
Z_DECOR = 'HitZ_z0'
SIGMA_DECOR = 'HitZ_z0_sigma'

OUTPUT_REMAP = {
    'HitZV01_TruthJetPVz': Z_DECOR,
    'HitZV01_TruthJetPVz_stddev': SIGMA_DECOR,
}


def hitClusterCnvCfg(flags):
    """xAOD converters for the in-view Si clusters."""
    ca = ComponentAccumulator()

    from InDetConfig.InDetPrepRawDataToxAODConfig import (
        ITkPixelPrepDataToxAODCfg,
        ITkStripPrepDataToxAODCfg,
    )
    # Both converters write far more than HitZ reads. All the pipeline needs is
    # the global position (the network eats beamspot-relative coordinates built
    # from it), bec (the barrel/endcap split in the association) and, on the
    # pixel side, the three goodness flags. The rest is switched off:
    #   - WriteRDOinformation pulls the per-RDO charge calibration on the pixel
    #     side and builds an Identifier->RDO map over the whole strip RDO
    #     container on the strip side, once per event.
    #   - WriteExtendedPRDinformation adds four module-geometry lookups per
    #     cluster.
    # WriteModuleStatus keeps hasBSError/DCSState, which used to be reachable
    # only through WriteRDOinformation.
    pixelSlimming = {}
    stripSlimming = {}
    if flags.Trigger.Jet.hitZSlimConverters:
        pixelSlimming = dict(WriteRDOinformation=False,
                             WriteExtendedPRDinformation=False,
                             WriteModuleStatus=True)
        stripSlimming = dict(WriteRDOinformation=False)

    ca.merge(ITkPixelPrepDataToxAODCfg(
        flags,
        SiClusterContainer='ITkTrigPixelClusters',
        OutputClusterContainer=PIXEL_HITS,
        WriteNNinformation=False,
        UseTruthInfo=False,
        **pixelSlimming,
    ))
    ca.merge(ITkStripPrepDataToxAODCfg(
        flags,
        SiClusterContainer='ITkTrigStripClusters',
        # SCT_PrepDataToxAOD spells its xAOD output differently to the pixel one
        SctxAodContainer=STRIP_HITS,
        UseTruthInfo=False,
        **stripSlimming,
    ))

    ca.getEventAlgo('ITkPixelPrepDataToxAOD').ExtraOutputs = [
        ('xAOD::TrackMeasurementValidationContainer',
         f'StoreGateSvc+{PIXEL_HITS}.{decor}')
        for decor in PIXEL_GOODNESS_DECORS
    ]
    return ca


def hitAssociationDataObjID(inputJets):
    """The merged association, as the scheduler has to be told to see it."""
    return ('xAOD::BaseContainer',
            f'StoreGateSvc+{inputJets}.{HIT_ASSOCIATION}')


def hitAssociationCfg(flags, inputJets, maxHits=MAX_HITS,
                      dphi=DPHI_HIT_TO_JET, deta=DETA_HIT_TO_JET,
                      dz=DZ_HIT_TO_VERTEX, minJetPt=MIN_JET_PT,
                      maxAbsJetEta=MAX_ABS_JET_ETA):
    """Clean, decorate and associate the Pixel and Strip hits to the jets."""
    ca = ComponentAccumulator()

    from BeamSpotConditions.BeamSpotConditionsConfig import BeamSpotCondAlgCfg
    ca.merge(BeamSpotCondAlgCfg(flags))

    for label, hits, isSCT in [('Pixel', PIXEL_HITS, False),
                               ('Strip', STRIP_HITS, True)]:
        ca.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.CleanHitDecoratorAlg(
                name=f'HitZCleanHit{label}',
                hitContainer=hits,
                isSCT=isSCT,
                DoOverlapRemoval=False,
            ))
        ca.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.HitBeamSpotDataDecoratorAlg(
                name=f'HitZBeamSpotDeco{label}',
                hitContainer=hits,
                beamSpotKey='BeamSpotData',
            ))
        ca.addEventAlgo(
            CompFactory.FlavorTagDiscriminants.JetHitAssociationAlg(
                name=f'HitZJetHitAssociation{label}',
                jetContainer=inputJets,
                hitContainer=hits,
                hitAssociation=f'{HIT_ASSOCIATION}{"SCT" if isSCT else "Pixel"}',
                dphiHitToJet=dphi,
                detaHitToJet=deta,
                dzHitToVertex=dz,
                minJetPt=minJetPt,
                maxAbsJetEta=maxAbsJetEta,
                # truncation happens downstream, on the merged collection
                maxHits=0,
            ))

    ca.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.HitsSelectorAlg(
            name='HitZHitsSelector',
            jetContainer=inputJets,
            # declared against jetContainer as parent, so no container prefix
            PixelAssociation=f'{HIT_ASSOCIATION}Pixel',
            SCTAssociation=f'{HIT_ASSOCIATION}SCT',
            hitAssociation=HIT_ASSOCIATION,
            pixelHitContainer=PIXEL_HITS,
            SCTHitContainer=STRIP_HITS,
            maxHits=maxHits,
            # HitsLoader reads this back through a bare ConstAccessor, so the
            # scheduler needs the edge to the tagger drawn by hand
            ExtraOutputs=[hitAssociationDataObjID(inputJets)],
        ))
    return ca


def hitZInferenceCfg(flags, inputJets, nnFile, remap=None):
    """Run the HitZ regression, mirroring the dl2_configs entries next door."""
    ca = ComponentAccumulator()

    # GNNTool builds its own CPU session unless it is given a sharing service;
    # the service is the only place the execution provider can be chosen
    nnSvc = CompFactory.FlavorTagInference.NNSharingOnnxSvc(
        'HitZNNSharingOnnxSvc',
        executionProvider=flags.Trigger.Jet.hitZExecutionProvider,
        deviceId=flags.Trigger.Jet.hitZDeviceId,
        useTF32=flags.Trigger.Jet.hitZUseTF32,
    )
    ca.addService(nnSvc)

    ca.addEventAlgo(
        CompFactory.FlavorTagInference.JetTagDecoratorAlg(
            name='HitZJetTagAlg',
            container=inputJets,
            electronContainer='',
            # the network reads hits, not tracks, so it declares no track
            # inputs and needs no constituent container
            constituentContainer='',
            decorator=CompFactory.FlavorTagInference.GNNTool(
                name='HitZGNNTool',
                nnFile=nnFile,
                variableRemapping=OUTPUT_REMAP if remap is None else remap,
                defaultZeroTracks=True,
                nnSharingService=nnSvc,
            ),
            ExtraInputs=[hitAssociationDataObjID(inputJets)],
        ))
    return ca


def hitZTaggingCfg(flags, inputJets):
    """Cluster conversion, hit association and HitZ inference, in that order."""
    if not flags.Detector.GeometryITk:
        raise ValueError('HitZ is configured for ITk (Run 4) only')
    if flags.Trigger.useActsTracking:
        raise ValueError('HitZ reads the InDet trigger clusters, which '
                         'Trigger.useActsTracking does not produce')

    nnFile = flags.Trigger.Jet.hitZNetwork
    if not nnFile:
        raise ValueError(
            'An HZ chain is configured but Trigger.Jet.hitZNetwork is empty. '
            'Point it at a HitZ ONNX file resolvable by PathResolver.')

    ca = ComponentAccumulator()
    ca.merge(hitClusterCnvCfg(flags))
    ca.merge(hitAssociationCfg(flags, inputJets))
    ca.merge(hitZInferenceCfg(flags, inputJets, nnFile))
    return ca


def hitZRoICfg(flags, inputJets, RoIs):
    """Everything that runs in the HitZ super-RoI view: Si clusterisation, then HitZ."""
    ca = ComponentAccumulator()

    # the clusterisation algs carry the signature in their name, so a
    # signature of its own keeps them apart from the RoI FTF step's
    signature = 'jetSuperHitZ'
    from AthenaCommon.Logging import logging
    from TrigInDetConfig.utils import getFlagsForActiveConfig
    from TrigInDetConfig.ITkTrigSequence import ITkTrigSequence
    trkFlags = getFlagsForActiveConfig(flags, 'jetSuper', logging.getLogger(__name__))
    ca.merge(ITkTrigSequence(trkFlags, signature, RoIs, 'VDVHitZ')
             .viewDataVerifier('VDVHitZ'))
    ca.addEventAlgo(CompFactory.AthViews.ViewDataVerifier(
        name='VDVHitZJets',
        DataObjects={('xAOD::JetContainer', f'StoreGateSvc+{inputJets}')},
    ))

    from InDetConfig.InDetPrepRawDataFormationConfig import (
        ITkTrigPixelClusterizationCfg,
        ITkTrigStripClusterizationCfg,
    )
    ca.merge(ITkTrigPixelClusterizationCfg(trkFlags, roisKey=RoIs, signature=signature))
    ca.merge(ITkTrigStripClusterizationCfg(trkFlags, roisKey=RoIs, signature=signature))

    ca.merge(hitZTaggingCfg(flags, inputJets))
    return ca
