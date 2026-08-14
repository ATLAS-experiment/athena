# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Hit-based per-jet z regression (HitZ) for the b-jet trigger.

Runs, inside the jet super-RoI b-tagging step, the hit pipeline the
training-dataset-dumper runs offline: the Si clusters the fast tracking already
built in the view are converted to xAOD, cleaned, decorated with
beamspot-relative coordinates, associated to the jets in a wedge, merged and
truncated, then passed to an ONNX regression.

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
NEG_LOG_SIGMA2_DECOR = 'HitZ_negLogSigma2'

OUTPUT_REMAP = {
    'HitZmdn_TruthJetPVz': Z_DECOR,
    'HitZmdn_TruthJetPVz_stddev': SIGMA_DECOR,
}


def hitClusterCnvCfg(flags):
    """xAOD converters for the in-view Si clusters the fast tracking produced."""
    ca = ComponentAccumulator()

    from InDetConfig.InDetPrepRawDataToxAODConfig import (
        ITkPixelPrepDataToxAODCfg,
        ITkStripPrepDataToxAODCfg,
    )
    ca.merge(ITkPixelPrepDataToxAODCfg(
        flags,
        SiClusterContainer='ITkTrigPixelClusters',
        OutputClusterContainer=PIXEL_HITS,
        WriteNNinformation=False,
        UseTruthInfo=False,
    ))
    ca.merge(ITkStripPrepDataToxAODCfg(
        flags,
        SiClusterContainer='ITkTrigStripClusters',
        # SCT_PrepDataToxAOD spells its xAOD output differently to the pixel one
        SctxAodContainer=STRIP_HITS,
        UseTruthInfo=False,
    ))
    return ca


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
        ))
    return ca


def hitZInferenceCfg(flags, inputJets, inputTracks, nnFile, remap=None):
    """Run the HitZ regression, mirroring the dl2_configs entries next door."""
    ca = ComponentAccumulator()
    ca.addEventAlgo(
        CompFactory.FlavorTagInference.JetTagDecoratorAlg(
            name='HitZJetTagAlg',
            container=inputJets,
            electronContainer='',
            # the network has no track inputs, but an empty constituent key is
            # rejected when the handle is initialised
            constituentContainer=inputTracks,
            decorator=CompFactory.FlavorTagInference.GNNTool(
                name='HitZGNNTool',
                nnFile=nnFile,
                variableRemapping=OUTPUT_REMAP if remap is None else remap,
                defaultZeroTracks=True,
            ),
        ))
    # the hypo condition recovers the variance as exp(-negLogSigma2)
    ca.addEventAlgo(
        CompFactory.FlavorTagDiscriminants.NegLogSigma2DecoratorAlg(
            name='HitZNegLogSigma2Alg',
            jetContainer=inputJets,
            sigmaDecor=SIGMA_DECOR,
            negLogSigma2Decor=NEG_LOG_SIGMA2_DECOR,
        ))
    return ca


def hitZTaggingCfg(flags, inputJets, inputTracks):
    """Cluster conversion, hit association and HitZ inference, in that order."""
    if not flags.Detector.GeometryITk:
        raise ValueError('HitZ is configured for ITk (Run 4) only')

    nnFile = flags.Trigger.Jet.hitZNetwork
    if not nnFile:
        raise ValueError(
            'Trigger.Jet.doHitZ is set but Trigger.Jet.hitZNetwork is empty. '
            'Point it at a HitZ ONNX file resolvable by PathResolver.')

    ca = ComponentAccumulator()
    ca.merge(hitClusterCnvCfg(flags))
    ca.merge(hitAssociationCfg(flags, inputJets))
    ca.merge(hitZInferenceCfg(flags, inputJets, inputTracks, nnFile))
    return ca
