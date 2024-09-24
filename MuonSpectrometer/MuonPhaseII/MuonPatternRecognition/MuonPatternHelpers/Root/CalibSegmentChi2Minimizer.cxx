/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternHelpers/CalibSegmentChi2Minimizer.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonSpacePointCalibrator/ISpacePointCalibrator.h"
#include "GaudiKernel/PhysicalConstants.h"
namespace {
    constexpr double c_inv{1./ Gaudi::Units::c_light};
}


namespace MuonR4{
    using HitType = CalibSegmentChi2Minimizer::HitType;
    using HitVec  = CalibSegmentChi2Minimizer::HitVec;
    using namespace SegmentFit;

    CalibSegmentChi2Minimizer::CalibSegmentChi2Minimizer(const std::string& name,
                                                        const EventContext& ctx,
                                                         const Amg::Transform3D& locToGlobTrf,
                                                         HitVec&& hits,
                                                         const ISpacePointCalibrator* calibrator,
                                                         bool doT0Fit):
            AthMessaging{name},
            m_name{name},
             m_ctx{ctx},
            m_locToGlob{locToGlobTrf},
            m_hits{std::move(hits)},
            m_calibrator{calibrator},
            m_doT0Fit{doT0Fit} {
        
        
        for (const HitType& hit : m_hits) {
            if (hit->fitState() != CalibratedSpacePoint::State::Valid){
                continue;
            }
            /// Beam spot constraint
            if (hit->type() == xAOD::UncalibMeasType::Other) {
                m_nMeas +=2;
                m_hasPhi = true;
                continue;
            }
            m_nMeas += hit->spacePoint()->measuresEta();
            m_nMeas += hit->spacePoint()->measuresPhi();
            m_hasPhi |= hit->spacePoint()->measuresPhi();
            if (m_doT0Fit && hit->measuresTime() && hit->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
                ++m_nMeas;
            }
        }
        if (m_nMeas - 2 - (m_hasPhi ? 2 : 0) <= 1) {
            m_doT0Fit = false;
        }
        std::ranges::sort(m_hits, 
                  [](const HitType& a, const HitType& b) { 
                    return a->positionInChamber().z() < b->positionInChamber().z();
                  });
    }

    CalibSegmentChi2Minimizer* CalibSegmentChi2Minimizer::Clone() const {
        HitVec copyHits{};
        for (const HitType& copyMe : m_hits) {
            copyHits.emplace_back(std::make_unique<CalibratedSpacePoint>(*copyMe));
        }
        return new CalibSegmentChi2Minimizer(m_name, m_ctx, m_locToGlob, std::move(copyHits), m_calibrator, m_doT0Fit);
    }
    const Amg::Transform3D& CalibSegmentChi2Minimizer::localToGlobTrans() const {
        return m_locToGlob;
    }
    unsigned int CalibSegmentChi2Minimizer::NDim() const {
        return toInt(ParamDefs::nPars);
    }
    bool CalibSegmentChi2Minimizer::doTimeFit() const {
        return m_doT0Fit;
    }
    int CalibSegmentChi2Minimizer::nDoF() const {
        return m_nMeas - 2 - (m_hasPhi ? 2 : 0) - m_doT0Fit;
    }
    bool CalibSegmentChi2Minimizer::hasPhiMeas() const {
        return m_hasPhi;
    }
    const HitVec& CalibSegmentChi2Minimizer::measurements() const {
        return m_hits;
    }
    HitVec CalibSegmentChi2Minimizer::release(const double* pars) {
        
        Parameters parVec{Parameters::Zero()};
        for (int p = 0 ; p < toInt(ParamDefs::nPars); ++p) {
            parVec[p] = pars[p];
        }
        const auto[segPos, segDir] = makeLine(parVec);
        const double timeDelay = pars[toInt(ParamDefs::time)];
        HitVec released = m_calibrator->calibrate(m_ctx, std::move(m_hits), segPos, segDir, timeDelay);
        m_hits.clear();
        return released;
    }
    double CalibSegmentChi2Minimizer::DoEval(const double* pars) const {
      
        Parameters parVec{Parameters::Zero()};
        for (int p = 0 ; p < toInt(ParamDefs::nPars); ++p) {
            parVec[p] = pars[p];
        }
        const auto[segPos, segDir] = makeLine(parVec);
   
        const double timeDelay = pars[toInt(ParamDefs::time)];
        ATH_MSG_VERBOSE("Starting parameters  position: "<<Amg::toString(segPos)<<", direction: "<<Amg::toString(segDir)
                       <<", timeDelay: "<<timeDelay<<".");
   
        m_hits = m_calibrator->calibrate(m_ctx, std::move(m_hits), segPos, segDir, timeDelay);
     
        double chi2{0.};
        std::optional<double> arrivalTime = std::nullopt;
        if (m_doT0Fit) {
            arrivalTime = std::make_optional<double>( (m_locToGlob *segPos).mag() * c_inv / timeDelay);
        }
        for (const HitType& hit : m_hits) {            
            chi2 += SegmentFitHelpers::chiSqTerm(segPos,segDir, timeDelay, arrivalTime, *hit, msg());
        }
        ATH_MSG_VERBOSE("Final chi2: "<<chi2);
        return chi2;
    }
}
