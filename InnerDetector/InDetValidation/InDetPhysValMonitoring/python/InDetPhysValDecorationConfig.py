#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def canAddDecorator(flags):
    '''
    check whether the decorator can be added.

    A decorator can be added if a track particle converter alg is in the sequence or
    if ESDs or AODs are read.
    '''

    if not (flags.Detector.GeometryID or flags.Detector.GeometryITk):
        return False

    return (flags.PhysVal.IDPVM.runDecoration and
            ("StreamESD" in flags.Input.ProcessingTags or
             "StreamAOD" in flags.Input.ProcessingTags or
             (len(flags.Input.ProcessingTags) > 0 and
              # Look for substring StreamDAOD in first processing tag to cover
              # all DAOD flavors
              "StreamDAOD" in flags.Input.ProcessingTags[0])))


def InDetPhysHitDecoratorAlgCfg(
        flags, name="InDetPhysHitDecoratorAlg", **kwargs):
    if flags.Detector.GeometryITk:
        return ITkPhysHitDecoratorAlgCfg(flags, name, **kwargs)

    '''
    create decoration algorithm which decorates track particles with the unbiased hit residuals and pulls.
    '''
    acc = ComponentAccumulator()

    if 'InDetTrackHoleSearchTool' not in kwargs:
        from InDetConfig.InDetTrackHoleSearchConfig import (
            InDetTrackHoleSearchToolCfg)
        kwargs.setdefault("InDetTrackHoleSearchTool",
            acc.popToolsAndMerge(InDetTrackHoleSearchToolCfg(flags)))

    if 'Updator' not in kwargs:
        from TrkConfig.TrkMeasurementUpdatorConfig import InDetUpdatorCfg
        kwargs.setdefault("Updator", acc.addPublicTool(acc.popToolsAndMerge(InDetUpdatorCfg(flags))))

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.PixelLorentzAngleConfig import (
            PixelLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            PixelLorentzAngleToolCfg(flags)))

    if 'ResidualPullCalculator' not in kwargs:
        from TrkConfig.TrkResidualPullCalculatorConfig import (
            ResidualPullCalculatorCfg)
        kwargs.setdefault("ResidualPullCalculator", acc.addPublicTool(
            acc.popToolsAndMerge(ResidualPullCalculatorCfg(flags))))

    acc.addEventAlgo(CompFactory.InDetPhysHitDecoratorAlg(name, **kwargs))
    return acc


def ITkPhysHitDecoratorAlgCfg(flags, name="ITkPhysHitDecoratorAlg", **kwargs):
    '''
    create decoration algorithm which decorates track particles with the unbiased hit residuals and pulls.

    '''
    acc = ComponentAccumulator()

    if 'InDetTrackHoleSearchTool' not in kwargs:
        from InDetConfig.InDetTrackHoleSearchConfig import (
            ITkTrackHoleSearchToolCfg)
        ITkTrackHoleSearchTool = acc.popToolsAndMerge(
            ITkTrackHoleSearchToolCfg(flags))
        acc.addPublicTool(ITkTrackHoleSearchTool)
        kwargs.setdefault("InDetTrackHoleSearchTool", ITkTrackHoleSearchTool)

    if 'Updator' not in kwargs:
        from TrkConfig.TrkMeasurementUpdatorConfig import ITkUpdatorCfg
        Updator = acc.popToolsAndMerge(ITkUpdatorCfg(flags))
        acc.addPublicTool(Updator)
        kwargs.setdefault("Updator", Updator)

    if 'LorentzAngleTool' not in kwargs:
        from SiLorentzAngleTool.ITkPixelLorentzAngleConfig import (
            ITkPixelLorentzAngleToolCfg)
        kwargs.setdefault("LorentzAngleTool", acc.popToolsAndMerge(
            ITkPixelLorentzAngleToolCfg(flags)))

    acc.addEventAlgo(CompFactory.InDetPhysHitDecoratorAlg(name, **kwargs))
    return acc


def ParameterErrDecoratorAlgCfg(
        flags, name="ParameterErrDecoratorAlg", **kwargs):
    '''
    create decoration algorithm which decorates track particles with the uncertainties of the track parameters.
    '''
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.ParameterErrDecoratorAlg(name, **kwargs))
    return acc


def InDetPhysValTruthDecoratorAlgCfg(
        flags, name="InDetPhysValTruthDecoratorAlg", **kwargs):
    '''
    create decoration algorithm which decorates truth particles with track parameters at the perigee.
    '''
    acc = ComponentAccumulator()

    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg
    extrapolator = acc.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    acc.addPublicTool(extrapolator)  # TODO: migrate to private?
    kwargs.setdefault("Extrapolator", extrapolator)

    if flags.Detector.GeometryITk:
        kwargs.setdefault("PixelClusterContainerName", "ITkPixelClusters")
        kwargs.setdefault("SCTClusterContainerName", "ITkStripClusters")

    acc.addEventAlgo(CompFactory.InDetPhysValTruthDecoratorAlg(name, **kwargs))
    return acc


def TruthClassDecoratorAlgCfg(flags, name="TruthClassDecoratorAlg", **kwargs):
    '''
    create decoration algorithm which decorates truth particles with origin and type from truth classifier.
    '''
    acc = ComponentAccumulator()
    acc.addEventAlgo(CompFactory.TruthClassDecoratorAlg(name, **kwargs))
    return acc


def TrackDecoratorsCfg(flags, **kwargs):
    '''
    Get track particle decorators needed for the InDetPhysValMonitoring tool
    '''
    acc = ComponentAccumulator()

    if "CombinedInDetTracks" in flags.Input.Collections:
        acc.merge(InDetPhysHitDecoratorAlgCfg(flags, **kwargs))

    acc.merge(ParameterErrDecoratorAlgCfg(flags, **kwargs))

    return acc


def GSFTrackDecoratorsCfg(flags, **kwargs):
    kwargs.setdefault("TrackParticleContainerName", "GSFTrackParticles")
    return TrackDecoratorsCfg(flags, **kwargs)


def AddDecoratorCfg(flags, **kwargs):
    '''
    Add the track particle decoration algorithm to the top sequence.
    The algorithm is to be run on RAW/RDO since it depends on full hit information
    which is generally not available at later stages. The decorations added by this
    algorithm are used by InDetPhysValMonitoring tool.
    '''
    acc = ComponentAccumulator()

    acc.merge(TrackDecoratorsCfg(flags))

    if flags.Input.isMC:
        from BeamSpotConditions.BeamSpotConditionsConfig import (
            BeamSpotCondAlgCfg)
        acc.merge(BeamSpotCondAlgCfg(flags))
        acc.merge(InDetPhysValTruthDecoratorAlgCfg(flags))

    if flags.PhysVal.IDPVM.doValidateGSFTracks:
        acc.merge(AddGSFTrackDecoratorAlgCfg(flags))

    return acc


def AddGSFTrackDecoratorAlgCfg(flags, **kwargs):
    # Search egamma algorithm and add the GSF TrackParticle decorator
    acc = ComponentAccumulator()

    if flags.PhysVal.IDPVM.doValidateGSFTracks:
        acc.merge(GSFTrackDecoratorsCfg(flags))

        for col in flags.PhysVal.IDPVM.validateExtraTrackCollections:
            acc.merge(TrackDecoratorsCfg(
                flags, TrackParticleContainerName=col))


def AddDecoratorIfNeededCfg(flags):
    '''
     Add the InDet decoration algorithm if it has not been ran yet.
    '''

    acc = ComponentAccumulator()

    if not canAddDecorator(flags):
        print('DEBUG addDecoratorIfNeeded ? Stage is too early or too late for running the decoration. Needs reconstructed tracks. Try again during next stage ?')
        return acc

    acc.merge(AddDecoratorCfg(flags))

    return acc
