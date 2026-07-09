/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuon/versions/Muon_v1.h"

#define RETURN_CASE(num)   \
    case num: return #num; \


namespace xAOD {
    std::string_view Muon_v1::toString(const Author author) {
        switch (author) {
          using enum Author;
          RETURN_CASE(unknown)
          RETURN_CASE(MuidCo)
          RETURN_CASE(STACO)
          RETURN_CASE(MuTag)
          RETURN_CASE(MuTagIMO)
          RETURN_CASE(MuidSA)
          RETURN_CASE(MuGirl)
          RETURN_CASE(MuGirlLowBeta)
          RETURN_CASE(CaloTag)
          RETURN_CASE(CaloLikelihood)
          RETURN_CASE(CaloScore)
          RETURN_CASE(ExtrapolateMuonToIP)
          RETURN_CASE(NumberOfMuonAuthors)
        }
        return "";
    }
    std::string_view Muon_v1::toString(const MuonType type) {
        switch (type) {
          using enum MuonType;
          RETURN_CASE(Combined)
          RETURN_CASE(MuonStandAlone)
          RETURN_CASE(SegmentTagged)
          RETURN_CASE(CaloTagged)
          RETURN_CASE(SiliconAssociatedForwardMuon)
          RETURN_CASE(ZeroPixelHit)
        }
        return "";
    }
    std::string_view Muon_v1::toString(const ParamDef def) {
        switch (def) {
            using enum ParamDef;
            RETURN_CASE(spectrometerFieldIntegral)
            RETURN_CASE(scatteringCurvatureSignificance)
            RETURN_CASE(scatteringNeighbourSignificance)
            RETURN_CASE(momentumBalanceSignificance)
            RETURN_CASE(segmentDeltaEta)
            RETURN_CASE(segmentDeltaPhi)
            RETURN_CASE(segmentChi2OverDoF)
            RETURN_CASE(ParamEnergyLossSigmaPlus)
            RETURN_CASE(t0)
            RETURN_CASE(beta)
            RETURN_CASE(annBarrel)
            RETURN_CASE(annEndCap)
            RETURN_CASE(innAngle)
            RETURN_CASE(midAngle)
            RETURN_CASE(msInnerMatchChi2)
            RETURN_CASE(msInnerMatchDOF)
            RETURN_CASE(msOuterMatchChi2)
            RETURN_CASE(msOuterMatchDOF)
            RETURN_CASE(meanDeltaADCCountsMDT)
            RETURN_CASE(CaloLRLikelihood)
            RETURN_CASE(CaloMuonIDTag)
            RETURN_CASE(FSR_CandidateEnergy)
            RETURN_CASE(EnergyLoss)
            RETURN_CASE(ParamEnergyLoss)
            RETURN_CASE(MeasEnergyLoss)
            RETURN_CASE(EnergyLossSigma)
            RETURN_CASE(ParamEnergyLossSigmaMinus)
            RETURN_CASE(MeasEnergyLossSigma)
            RETURN_CASE(CaloMuonScore)
      }
      return "";
    }
    std::string_view Muon_v1::toString(const TrackParticleType type) {
        switch (type) {
          using enum TrackParticleType;
          RETURN_CASE(Primary)
          RETURN_CASE(InnerDetectorTrackParticle)
          RETURN_CASE(MuonSpectrometerTrackParticle)
          RETURN_CASE(CombinedTrackParticle)
          RETURN_CASE(ExtrapolatedMuonSpectrometerTrackParticle)
          RETURN_CASE(MSOnlyExtrapolatedMuonSpectrometerTrackParticle)
        }
        return "";
    }
    std::string_view Muon_v1::toString(const EnergyLossType type) {
        switch (type) {
          using enum EnergyLossType;
          RETURN_CASE(Parametrized)
          RETURN_CASE(NotIsolated)
          RETURN_CASE(MOP)
          RETURN_CASE(Tail)
          RETURN_CASE(FSRcandidate)
        }
        return "";
    }
    std::string_view Muon_v1::toString(const Quality qual) {
       switch (qual) {
          using enum Quality;
          RETURN_CASE(Tight)
          RETURN_CASE(Medium)
          RETURN_CASE(Loose)
          RETURN_CASE(VeryLoose)
       }
       return "";
    }
  }


