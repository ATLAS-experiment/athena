# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ******************************************************************************
# DO NOT USE THE FUNCTIONS OF THIS MODULE IN PRODUCTION.
#
# The functions of this module change the offline ACTS reconstruction for
# technical studies, for example timing studies of a trigger-like tracking.
# The output is not a valid offline reconstruction.
# ******************************************************************************

from AthenaCommon.Logging import logging
import AthenaCommon.SystemOfUnits as Units

_log = logging.getLogger(__name__)


def _warnDoNotUseInProduction(name: str) -> None:
    _log.warning("*" * 78)
    _log.warning(f"{__name__}.{name} is active.")
    _log.warning("This configuration is for technical studies only. DO NOT USE IT IN PRODUCTION.")
    _log.warning("*" * 78)


def hltLikeTrackingFlags(flags) -> None:
    """preInclude: ACTS offline primary pass only, with HLT-like settings

    The primary pass uses the cuts of the main (fast) pass of 2025-06-23
    (commit 2a23404e2e0), which the HLT ACTS tracking used at that time.
    All secondary passes, the strip space point formation, the strip seeding,
    the brem recovery, HGTD, the xAOD to Trk track conversion, the ACTS GSF
    refit and the truth are off.

    The job does the tracking only: this preInclude applies
    InDetConfig.ConfigurationHelpers.OnlyTrackingPreInclude first.

    If you need the truth (for example for IDPVM), set Tracking.doTruth=True
    and ITk.doTruth=True in a preExec.
    """
    _warnDoNotUseInProduction("hltLikeTrackingFlags")

    # Tracking only: no calo, no muons, no egamma, no jets, ...
    from InDetConfig.ConfigurationHelpers import OnlyTrackingPreInclude
    OnlyTrackingPreInclude(flags)

    # Secondary passes: ACTS and legacy switches
    flags.Acts.doLargeRadius = False
    flags.Acts.doLowPt = False
    flags.Acts.doITkConversion = False
    flags.Tracking.doLargeD0 = False
    flags.Tracking.doLowPtLargeD0 = False
    flags.Tracking.doLowPt = False
    flags.Tracking.doITkConversion = False

    # No strip space points and no strip seeds. The CKF still uses the strip clusters.
    flags.Acts.SpacePoints.doStrip = False
    flags.Tracking.ITkActsPass.useITkStripSeeding = False

    # The HLT does not use brem recovery and does not run the ambiguity resolution
    flags.Tracking.ITkActsPass.doBremRecoverySi = False
    flags.Acts.doAmbiguityResolution = False

    # HGTD off
    flags.Reco.EnableHGTDExtension = False
    flags.HGTD.doActs = False
    flags.Acts.useHGTDClusterInTrackFinding = False

    # No conversion to Trk tracks and no ACTS GSF refit (both are offline only)
    flags.Acts.doxAODToTrkConversion = False
    flags.Acts.GsfRefitActs = False

    # Truth off. Turn it on in a preExec if you need it.
    flags.Tracking.doTruth = False
    flags.ITk.doTruth = False

    # Cuts of the main pass on 2025-06-23
    flags.Tracking.ITkActsPass.minPT = lambda pcf: [
        0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale,
        0.4 * Units.GeV * pcf.BField.configuredSolenoidFieldScale]
    flags.Tracking.ITkActsPass.minPTSeed = lambda pcf: (
        0.9 * Units.GeV * pcf.BField.configuredSolenoidFieldScale)
    flags.Tracking.ITkActsPass.maxPrimaryImpactSeed = 2.0 * Units.mm
    flags.Tracking.ITkActsPass.maxPrimaryImpact = [2.0 * Units.mm, 2.0 * Units.mm, 10.0 * Units.mm]
    flags.Tracking.ITkActsPass.maxZImpactSeed = 150.0 * Units.mm
    flags.Tracking.ITkActsPass.maxZImpact = [150.0 * Units.mm]
    # Hit and hole cuts, per eta bin where there are three values
    flags.Tracking.ITkActsPass.minClusters = [9, 8, 7]
    flags.Tracking.ITkActsPass.minPixel = [3]
    flags.Tracking.ITkActsPass.maxHoles = [1]

    # Chi2 cuts of the track finding (measurement, outlier) of the fast pass from 2025-07-09
    # (e9a24a7e5db): the chi2 values 50 and 100 for all measurement dimensions.
    # Now the cuts are in sigma of a 1D Gaussian. The sigma below gives the same p-value as the
    # chi2 cut for pixels (2D): erfc(n/sqrt(2)) = exp(-chi2/2), so n = 6.76 and 9.75.
    # For strips (1D) the equivalent chi2 cuts are n^2 = 45.7 and 95.0, which is a little tighter.
    from math import exp
    from statistics import NormalDist

    def sigmaOf2DChi2(chi2: float) -> float:
        return -NormalDist().inv_cdf(exp(-chi2 / 2) / 2)

    flags.Tracking.ITkActsPass.Xi2maxSigma = [sigmaOf2DChi2(50.0)]
    flags.Tracking.ITkActsPass.Xi2maxNoAddSigma = [sigmaOf2DChi2(100.0)]

    # Digital pixel clusters: no ToT to charge conversion in the clusterization,
    # no charge-weighted position and no on-track calibration in the CKF
    from ActsConfig.ActsConfigFlags import PixelCalibrationStrategy, StripCalibrationStrategy
    flags.Tracking.doPixelDigitalClustering = True
    flags.Acts.Clusters.RetrieveChargeInformation = False
    flags.Acts.Clusters.UseWeightedPosition = False
    flags.Acts.PixelCalibrationStrategy = PixelCalibrationStrategy.Uncalibrated
    flags.Acts.StripCalibrationStrategy = StripCalibrationStrategy.Uncalibrated


def rdoWarmUpClusteringCfg(flags, cfg) -> None:
    """postInclude: add a warm-up copy of the pixel and strip clusterization

    The first algorithm that reads the RDO containers pays the read, the
    decompression and the conversion of the RDOs. The warm-up copy reads the
    RDOs first, and nothing reads its clusters. Then the time of the real
    clusterization algorithms is only the clustering and the output EDM.
    The real algorithms get the warm-up clusters as an extra input, so the
    scheduler always runs the warm-up copy first.
    """
    _warnDoNotUseInProduction("rdoWarmUpClusteringCfg")

    from ActsConfig.ActsClusterizationConfig import (
        ActsClusterizationInputPrefix,
        ActsPixelClusterizationAlgCfg,
        ActsStripClusterizationAlgCfg,
    )
    from InDetConfig.ITkActsHelpers import primaryPassExtension

    if flags.Acts.useCache:
        raise ValueError("rdoWarmUpClusteringCfg does not support Acts.useCache=True: "
                         "the real clusterization would find the warm-up clusters in the cache and skip the clustering")

    prefix = ActsClusterizationInputPrefix(flags)
    extension = primaryPassExtension(flags)
    rois = f"{extension}RegionOfInterest"

    detectors = []
    if flags.Detector.EnableITkPixel:
        detectors.append(("Pixel", ActsPixelClusterizationAlgCfg, "xAOD::PixelClusterContainer"))
    if flags.Detector.EnableITkStrip:
        detectors.append(("Strip", ActsStripClusterizationAlgCfg, "xAOD::StripClusterContainer"))

    for detector, algCfg, clusterType in detectors:
        realName = f"{prefix}{extension}{detector}ClusterizationAlg"
        realAlg = cfg.getEventAlgo(realName)
        if realAlg is None:
            raise ValueError(f"rdoWarmUpClusteringCfg: no algorithm {realName} in the configuration")

        warmUpClusters = f"ITk{detector}Clusters_RDOWarmUp"
        cfg.merge(algCfg(flags,
                         name=f"{extension}{detector}RDOWarmUpClusterizationAlg",
                         RoIs=rois,
                         ClustersKey=warmUpClusters,
                         MonTool=""))
        realAlg.ExtraInputs |= {(clusterType, f"StoreGateSvc+{warmUpClusters}")}

        # ExtraInputs only orders the algorithms under the MT scheduler. A serial
        # job (--multithreaded False) runs the sequence in its order, and cfg.merge
        # appended the warm-up copy at the end. Move it right before the real one.
        _moveBefore(cfg, f"{extension}{detector}RDOWarmUpClusterizationAlg", realName)


def _moveBefore(cfg, name: str, before: str) -> None:
    from AthenaCommon.CFElements import findOwningSequence
    top = cfg.getSequence()
    alg = cfg.getEventAlgo(name)
    fromSeq = findOwningSequence(top, name)
    toSeq = findOwningSequence(top, before)
    if fromSeq is None or toSeq is None:
        raise ValueError(f"rdoWarmUpClusteringCfg: cannot find the sequences of {name} and {before}")
    fromSeq.Members = [m for m in fromSeq.Members if m.getName() != name]
    members = list(toSeq.Members)
    index = [m.getName() for m in members].index(before)
    toSeq.Members = members[:index] + [alg] + members[index:]
