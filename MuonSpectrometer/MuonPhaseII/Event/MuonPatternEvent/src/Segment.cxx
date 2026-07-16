/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternEvent/Segment.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonSpacePoint/SpacePointHelpers.h"

namespace MuonR4{
    using namespace Muon::MuonStationIndex;
    Segment::Segment(Amg::Vector3D&& globPos, Amg::Vector3D&& globDir,
                     const SegmentSeed* parent, MeasVec&& constMeas,
                     double chi2, unsigned int nDoF):
        m_globPos{std::move(globPos)},
        m_globDir{std::move(globDir)},
        m_parent{parent},
        m_measurements{std::move(constMeas)},
        m_chi2{chi2}, 
        m_nDoF{nDoF}{}

    void Segment::setSegmentT0(double t0) {
        m_t0 = std::make_optional<double>(t0);             
    }

    void Segment::setCallsToConverge(unsigned int nCalls) {
        m_nCalls = nCalls;
    }

    void Segment::setParUncertainties(SegmentFit::Covariance&& cov){
        m_cov = std::move(cov);
    }

    TechnologyIndex Segment::technology() const { 
        for (const MeasType& meas : m_measurements) {
            if (isPrecisionHit(*meas)) {
                return xAOD::toTechnologyIndex(meas->type());
            }
        }
        return TechnologyIndex::TechnologyUnknown;
    }

}