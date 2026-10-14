/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/TrackingPrimitives.h"

#define ENUM_CASE(name) case name: return #name;

namespace xAOD {

std::string_view toString(const TrackFitter obj) {
     switch (obj) {
          using enum TrackFitter;
          ENUM_CASE(Unknown)
          ENUM_CASE(iPatTrackFitter)
          ENUM_CASE(xKalman)
          ENUM_CASE(KalmanFitter)
          ENUM_CASE(GaussianSumFilter)
          ENUM_CASE(GlobalChi2Fitter)
          ENUM_CASE(DistributedKalmanFilter)
          ENUM_CASE(DeterministicAnnealingFilter)
          ENUM_CASE(KalmanDNAFitter)
          ENUM_CASE(MuonboyFitter)
          ENUM_CASE(NumberOfTrackFitters)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const TrackProperties obj) {
     switch (obj) {
          using enum TrackProperties;
          ENUM_CASE(BremFit)
          ENUM_CASE(BremFitSuccessful)
          ENUM_CASE(StraightTrack)
          ENUM_CASE(SlimmedTrack)
          ENUM_CASE(HardScatterOrKink)
          ENUM_CASE(LowPtTrack)
          ENUM_CASE(NumberOfTrackProperties)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const TrackPatternRecoInfo obj) {
     switch (obj) {
          using enum TrackPatternRecoInfo;
          ENUM_CASE(SiSPSeededFinder)
          ENUM_CASE(SiCTBTracking)
          ENUM_CASE(InDetAmbiguitySolver)
          ENUM_CASE(InDetExtensionProcessor)
          ENUM_CASE(TRTSeededTrackFinder)
          ENUM_CASE(Muonboy)
          ENUM_CASE(MuGirlUnrefitted)
          ENUM_CASE(STACO)
          ENUM_CASE(StacoLowPt)
          ENUM_CASE(MuTag)
          ENUM_CASE(MooreToTrackTool)
          ENUM_CASE(TrigIDSCAN)
          ENUM_CASE(TrigSiTrack)
          ENUM_CASE(TrigTRTxK)
          ENUM_CASE(TrigTRTLUT)
          ENUM_CASE(Fatras)
          ENUM_CASE(iPatLegacyCnv)
          ENUM_CASE(xKalmanLegacyCnv)
          ENUM_CASE(SimpleAmbiguityProcessorTool)
          ENUM_CASE(InDetAmbiTrackSelectionTool)
          ENUM_CASE(TRTStandalone)
          ENUM_CASE(MuidStandAlone)
          ENUM_CASE(TRTSeededSingleSpTrackFinder)
          ENUM_CASE(MooreLegacyCnv)
          ENUM_CASE(MuidComb)
          ENUM_CASE(Moore)
          ENUM_CASE(MuidCombined)
          ENUM_CASE(MuidVertexAssociator)
          ENUM_CASE(MuGirl)
          ENUM_CASE(iPatRec)
          ENUM_CASE(MuGirlLowBeta)
          ENUM_CASE(FatrasSimulation)
          ENUM_CASE(ReverseOrderedTrack)
          ENUM_CASE(MuonNotHittingTileVolume)
          ENUM_CASE(SiSpacePointsSeedMaker_Cosmic)
          ENUM_CASE(SiSpacePointsSeedMaker_HeavyIon)
          ENUM_CASE(SiSpacePointsSeedMaker_LowMomentum)
          ENUM_CASE(SiSpacePointsSeedMaker_BeamGas)
          ENUM_CASE(SiSpacePointsSeedMaker_VeryLowMomentum)
          ENUM_CASE(MuidMuonRecoveryTool)
          ENUM_CASE(MuidStandaloneRefit)
          ENUM_CASE(TrackInCaloROI)
          ENUM_CASE(SiSpacePointsSeedMaker_ForwardTracks)
          ENUM_CASE(strategyA)
          ENUM_CASE(strategyB)
          ENUM_CASE(strategyC)
          ENUM_CASE(FTK)
          ENUM_CASE(FastTrackFinderSeed)
          ENUM_CASE(SiSPSeededFinderSimple)
          ENUM_CASE(SiSpacePointsSeedMaker_LargeD0)
          ENUM_CASE(SiSpacePointsSeedMaker_ITkConversionTracks)
          ENUM_CASE(Pseudotracking)
          ENUM_CASE(NumberOfTrackRecoInfo)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const ParticleHypothesis obj) {
     switch (obj) {
          using enum ParticleHypothesis;
          ENUM_CASE(nonInteracting)
          ENUM_CASE(electron)
          ENUM_CASE(muon)
          ENUM_CASE(pion)
          ENUM_CASE(kaon)
          ENUM_CASE(proton)
          ENUM_CASE(photon)
          ENUM_CASE(neutron)
          ENUM_CASE(pi0)
          ENUM_CASE(k0)
          ENUM_CASE(nonInteractingMuon)
          ENUM_CASE(noHypothesis)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const ParameterPosition obj) {
     switch (obj) {
          using enum ParameterPosition;
          ENUM_CASE(BeamLine)
          ENUM_CASE(FirstMeasurement)
          ENUM_CASE(LastMeasurement)
          ENUM_CASE(CalorimeterEntrance)
          ENUM_CASE(CalorimeterExit)
          ENUM_CASE(MuonSpectrometerEntrance)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const SummaryType obj) {
     switch (obj) {
          using enum SummaryType;
          ENUM_CASE(numberOfContribPixelLayers)
          ENUM_CASE(numberOfBLayerHits)
          ENUM_CASE(numberOfBLayerOutliers)
          ENUM_CASE(numberOfBLayerSharedHits)
          ENUM_CASE(numberOfBLayerSplitHits)
          ENUM_CASE(expectBLayerHit)
          ENUM_CASE(expectInnermostPixelLayerHit)
          ENUM_CASE(numberOfInnermostPixelLayerHits)
          ENUM_CASE(numberOfInnermostPixelLayerOutliers)
          ENUM_CASE(numberOfInnermostPixelLayerSharedHits)
          ENUM_CASE(numberOfInnermostPixelLayerSplitHits)
          ENUM_CASE(numberOfInnermostPixelLayerEndcapHits)
          ENUM_CASE(numberOfInnermostPixelLayerEndcapOutliers)
          ENUM_CASE(numberOfInnermostPixelLayerSharedEndcapHits)
          ENUM_CASE(numberOfInnermostPixelLayerSplitEndcapHits)
          ENUM_CASE(expectNextToInnermostPixelLayerHit)
          ENUM_CASE(numberOfNextToInnermostPixelLayerHits)
          ENUM_CASE(numberOfNextToInnermostPixelLayerOutliers)
          ENUM_CASE(numberOfNextToInnermostPixelLayerSharedHits)
          ENUM_CASE(numberOfNextToInnermostPixelLayerSplitHits)
          ENUM_CASE(numberOfNextToInnermostPixelLayerEndcapHits)
          ENUM_CASE(numberOfNextToInnermostPixelLayerEndcapOutliers)
          ENUM_CASE(numberOfNextToInnermostPixelLayerSharedEndcapHits)
          ENUM_CASE(numberOfNextToInnermostPixelLayerSplitEndcapHits)
          ENUM_CASE(numberOfDBMHits)
          ENUM_CASE(numberOfPixelHits)
          ENUM_CASE(numberOfPixelOutliers)
          ENUM_CASE(numberOfPixelHoles)
          ENUM_CASE(numberOfPixelSharedHits)
          ENUM_CASE(numberOfPixelSplitHits)
          ENUM_CASE(numberOfGangedPixels)
          ENUM_CASE(numberOfGangedFlaggedFakes)
          ENUM_CASE(numberOfPixelDeadSensors)
          ENUM_CASE(numberOfPixelSpoiltHits)
          ENUM_CASE(numberOfSCTHits)
          ENUM_CASE(numberOfSCTOutliers)
          ENUM_CASE(numberOfSCTHoles)
          ENUM_CASE(numberOfSCTDoubleHoles)
          ENUM_CASE(numberOfSCTSharedHits)
          ENUM_CASE(numberOfSCTDeadSensors)
          ENUM_CASE(numberOfSCTSpoiltHits)
          ENUM_CASE(numberOfTRTHits)
          ENUM_CASE(numberOfTRTOutliers)
          ENUM_CASE(numberOfTRTHoles)
          ENUM_CASE(numberOfTRTHighThresholdHits)
          ENUM_CASE(numberOfTRTHighThresholdHitsTotal)
          ENUM_CASE(numberOfTRTHighThresholdOutliers)
          ENUM_CASE(numberOfTRTDeadStraws)
          ENUM_CASE(numberOfTRTTubeHits)
          ENUM_CASE(numberOfTRTXenonHits)
          ENUM_CASE(numberOfTRTSharedHits)
          ENUM_CASE(numberOfHGTDHits)
          ENUM_CASE(numberOfHGTDOutliers)
          ENUM_CASE(numberOfHGTDHoles)
          ENUM_CASE(numberOfHGTDSharedHits)
          ENUM_CASE(numberOfPrecisionLayers)
          ENUM_CASE(numberOfPrecisionHoleLayers)
          ENUM_CASE(numberOfPhiLayers)
          ENUM_CASE(numberOfPhiHoleLayers)
          ENUM_CASE(numberOfTriggerEtaLayers)
          ENUM_CASE(numberOfTriggerEtaHoleLayers)
          ENUM_CASE(numberOfGoodPrecisionLayers)
          ENUM_CASE(numberOfOutliersOnTrack)
          ENUM_CASE(standardDeviationOfChi2OS)
          ENUM_CASE(eProbabilityComb)
          ENUM_CASE(eProbabilityHT)
          ENUM_CASE(pixeldEdx)
          ENUM_CASE(TRTTrackOccupancy)
          ENUM_CASE(numberOfContribPixelBarrelLayers)
          ENUM_CASE(numberOfContribPixelEndcap)
          ENUM_CASE(numberOfPixelBarrelHits)
          ENUM_CASE(numberOfPixelEndcapHits)
          ENUM_CASE(numberOfContribPixelBarrelFlatLayers)
          ENUM_CASE(numberOfContribPixelBarrelInclinedLayers)
          ENUM_CASE(numberOfPixelBarrelFlatHits)
          ENUM_CASE(numberOfPixelBarrelInclinedHits)
          ENUM_CASE(numberOfTrackSummaryTypes)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const MuonSummaryType obj) {
     switch (obj) {
          using enum MuonSummaryType;
          ENUM_CASE(primarySector)
          ENUM_CASE(secondarySector)
          ENUM_CASE(innerSmallHits)
          ENUM_CASE(innerLargeHits)
          ENUM_CASE(middleSmallHits)
          ENUM_CASE(middleLargeHits)
          ENUM_CASE(outerSmallHits)
          ENUM_CASE(outerLargeHits)
          ENUM_CASE(extendedSmallHits)
          ENUM_CASE(extendedLargeHits)
          ENUM_CASE(innerSmallHoles)
          ENUM_CASE(innerLargeHoles)
          ENUM_CASE(middleSmallHoles)
          ENUM_CASE(middleLargeHoles)
          ENUM_CASE(outerSmallHoles)
          ENUM_CASE(outerLargeHoles)
          ENUM_CASE(extendedSmallHoles)
          ENUM_CASE(extendedLargeHoles)
          ENUM_CASE(phiLayer1Hits)
          ENUM_CASE(phiLayer2Hits)
          ENUM_CASE(phiLayer3Hits)
          ENUM_CASE(phiLayer4Hits)
          ENUM_CASE(etaLayer1Hits)
          ENUM_CASE(etaLayer2Hits)
          ENUM_CASE(etaLayer3Hits)
          ENUM_CASE(etaLayer4Hits)
          ENUM_CASE(phiLayer1Holes)
          ENUM_CASE(phiLayer2Holes)
          ENUM_CASE(phiLayer3Holes)
          ENUM_CASE(phiLayer4Holes)
          ENUM_CASE(etaLayer1Holes)
          ENUM_CASE(etaLayer2Holes)
          ENUM_CASE(etaLayer3Holes)
          ENUM_CASE(etaLayer4Holes)
          ENUM_CASE(innerClosePrecisionHits)
          ENUM_CASE(middleClosePrecisionHits)
          ENUM_CASE(outerClosePrecisionHits)
          ENUM_CASE(extendedClosePrecisionHits)
          ENUM_CASE(innerOutBoundsPrecisionHits)
          ENUM_CASE(middleOutBoundsPrecisionHits)
          ENUM_CASE(outerOutBoundsPrecisionHits)
          ENUM_CASE(extendedOutBoundsPrecisionHits)
          ENUM_CASE(combinedTrackOutBoundsPrecisionHits)
          ENUM_CASE(isEndcapGoodLayers)
          ENUM_CASE(isSmallGoodSectors)
          ENUM_CASE(phiLayer1RPCHits)
          ENUM_CASE(phiLayer2RPCHits)
          ENUM_CASE(phiLayer3RPCHits)
          ENUM_CASE(phiLayer4RPCHits)
          ENUM_CASE(etaLayer1RPCHits)
          ENUM_CASE(etaLayer2RPCHits)
          ENUM_CASE(etaLayer3RPCHits)
          ENUM_CASE(etaLayer4RPCHits)
          ENUM_CASE(phiLayer1RPCHoles)
          ENUM_CASE(phiLayer2RPCHoles)
          ENUM_CASE(phiLayer3RPCHoles)
          ENUM_CASE(phiLayer4RPCHoles)
          ENUM_CASE(etaLayer1RPCHoles)
          ENUM_CASE(etaLayer2RPCHoles)
          ENUM_CASE(etaLayer3RPCHoles)
          ENUM_CASE(etaLayer4RPCHoles)
          ENUM_CASE(phiLayer1TGCHits)
          ENUM_CASE(phiLayer2TGCHits)
          ENUM_CASE(phiLayer3TGCHits)
          ENUM_CASE(phiLayer4TGCHits)
          ENUM_CASE(etaLayer1TGCHits)
          ENUM_CASE(etaLayer2TGCHits)
          ENUM_CASE(etaLayer3TGCHits)
          ENUM_CASE(etaLayer4TGCHits)
          ENUM_CASE(phiLayer1TGCHoles)
          ENUM_CASE(phiLayer2TGCHoles)
          ENUM_CASE(phiLayer3TGCHoles)
          ENUM_CASE(phiLayer4TGCHoles)
          ENUM_CASE(etaLayer1TGCHoles)
          ENUM_CASE(etaLayer2TGCHoles)
          ENUM_CASE(etaLayer3TGCHoles)
          ENUM_CASE(etaLayer4TGCHoles)
          ENUM_CASE(cscEtaHits)
          ENUM_CASE(cscUnspoiledEtaHits)
          ENUM_CASE(phiLayer1STGCHits)
          ENUM_CASE(phiLayer2STGCHits)
          ENUM_CASE(etaLayer1STGCHits)
          ENUM_CASE(etaLayer2STGCHits)
          ENUM_CASE(phiLayer1STGCHoles)
          ENUM_CASE(phiLayer2STGCHoles)
          ENUM_CASE(etaLayer1STGCHoles)
          ENUM_CASE(etaLayer2STGCHoles)
          ENUM_CASE(MMHits)
          ENUM_CASE(MMHoles)
          ENUM_CASE(innerTriggerEtaHits)
          ENUM_CASE(innerTriggerPhiHits)
          ENUM_CASE(middleTriggerEtaHits)
          ENUM_CASE(middleTriggerPhiHits)
          ENUM_CASE(outerTriggerEtaHits)
          ENUM_CASE(outerTriggerPhiHits)
          ENUM_CASE(innerTriggerEtaHoles)
          ENUM_CASE(innerTriggerPhiHoles)
          ENUM_CASE(middleTriggerEtaHoles)
          ENUM_CASE(middleTriggerPhiHoles)
          ENUM_CASE(outerTriggerEtaHoles)
          ENUM_CASE(outerTriggerPhiHoles)
          ENUM_CASE(sTgcPadHits)
          ENUM_CASE(numberOfMuonSummaryTypes)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const RejectionStep obj) {
     switch (obj) {
          using enum RejectionStep;
          ENUM_CASE(solveTracks)
          ENUM_CASE(addNewTracks)
          ENUM_CASE(refitTrack)
          ENUM_CASE(addTrack)
          ENUM_CASE(decideWhichHitsToKeep)
          ENUM_CASE(getCleanedOutTrack)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const RejectionReason obj) {
     switch (obj) {
          using enum RejectionReason;
          ENUM_CASE(acceptedTrack)
          ENUM_CASE(stillBeingProcessed)
          ENUM_CASE(trackScoreZero)
          ENUM_CASE(duplicateTrack)
          ENUM_CASE(subtrackCreated)
          ENUM_CASE(refitFailed)
          ENUM_CASE(bremRefitFailed)
          ENUM_CASE(bremRefitSubtrackCreated)
          ENUM_CASE(bremRefitTrackScoreZero)
          ENUM_CASE(refitTrackScoreZero)
          ENUM_CASE(TSOSRejectedHit)
          ENUM_CASE(TSOSOutlierShared)
          ENUM_CASE(pixelSplitButTooManyShared2Ptc)
          ENUM_CASE(pixelSplitButTooManyShared3Ptc)
          ENUM_CASE(tooManySharedRecoverable)
          ENUM_CASE(tooManySharedNonRecoverable)
          ENUM_CASE(sharedSCT)
          ENUM_CASE(sharedHitsBadChi2)
          ENUM_CASE(sharedHitsNotEnoughUniqueHits)
          ENUM_CASE(firstHitSharedAndPixIBL)
          ENUM_CASE(firstHitSharedAndExtraShared)
          ENUM_CASE(sharedHitsNotEnoughUniqueSiHits)
          ENUM_CASE(sharedIBLSharedWithNoIBLTrack)
          ENUM_CASE(sharedPixelSharedWithDifferentIBLTrack)
          ENUM_CASE(tooManySharedAfterIncreasingShared)
          ENUM_CASE(notEnoughSiHits)
          ENUM_CASE(notEnoughTRTHits)
          ENUM_CASE(notEnoughUniqueSiHits)
          ENUM_CASE(tooFewHits)
          ENUM_CASE(failedSubtrackCreation)
          ENUM_CASE(subtrackCreatedWithRecoveredShared)
          ENUM_CASE(other)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const ObserverToolIndex obj) {
     switch (obj) {
          using enum ObserverToolIndex;
          ENUM_CASE(track)
          ENUM_CASE(score)
          ENUM_CASE(rejectStep)
          ENUM_CASE(rejectReason)
          ENUM_CASE(parentId)
          ENUM_CASE(numPixelHoles)
          ENUM_CASE(numSCTHoles)
          ENUM_CASE(numSplitSharedPixel)
          ENUM_CASE(numSplitSharedSCT)
          ENUM_CASE(numSharedOrSplit)
          ENUM_CASE(numSharedOrSplitPixels)
          ENUM_CASE(numShared)
          ENUM_CASE(isPatternTrack)
          ENUM_CASE(totalSiHits)
          ENUM_CASE(inROI)
          ENUM_CASE(hasIBLHit)
          ENUM_CASE(hasSharedIBLHit)
          ENUM_CASE(hasSharedPixel)
          ENUM_CASE(firstPixIsShared)
          ENUM_CASE(numPixelDeadSensor)
          ENUM_CASE(numSCTDeadSensor)
          ENUM_CASE(numPixelHits)
          ENUM_CASE(numSCTHits)
          ENUM_CASE(numUnused)
          ENUM_CASE(numTRT_Unused)
          ENUM_CASE(numSCT_Unused)
          ENUM_CASE(numPseudo)
          ENUM_CASE(averageSplit1)
          ENUM_CASE(averageSplit2)
          ENUM_CASE(numWeightedShared)
          ENUM_CASE(rejectStep_full)
          ENUM_CASE(rejectReason_full)
     }
     return "NOT IMPLEMENTED";
}

std::string_view toString(const SurfaceType obj) {
     switch (obj) {
          using enum SurfaceType;
          ENUM_CASE(Cone)
          ENUM_CASE(Cylinder)
          ENUM_CASE(Disc)
          ENUM_CASE(Perigee)
          ENUM_CASE(Plane)
          ENUM_CASE(Straw)
          ENUM_CASE(Curvilinear)
          ENUM_CASE(Other)
     }
     return "NOT IMPLEMENTED";
}

namespace VxType {
std::string_view toString(const VertexType obj) {
     switch (obj) {
          using enum VertexType;
          ENUM_CASE(NoVtx)
          ENUM_CASE(PriVtx)
          ENUM_CASE(SecVtx)
          ENUM_CASE(PileUp)
          ENUM_CASE(ConvVtx)
          ENUM_CASE(V0Vtx)
          ENUM_CASE(KinkVtx)
          ENUM_CASE(NotSpecified)
     }
     return "NOT IMPLEMENTED";
}
} // namespace VxType

} // namespace xAOD

#undef ENUM_CASE