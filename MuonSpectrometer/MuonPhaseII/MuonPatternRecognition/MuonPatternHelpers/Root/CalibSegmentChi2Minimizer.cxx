/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternHelpers/CalibSegmentChi2Minimizer.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"



namespace MuonR4{
    using HitType = CalibSegmentChi2Minimizer::HitType;
    using HitVec  = CalibSegmentChi2Minimizer::HitVec;
    using namespace SegmentFit;

    CalibSegmentChi2Minimizer::CalibSegmentChi2Minimizer(const std::string& name,
                                                         HitVec&& hits,
                                                         CalibratorFunc calibrator,
                                                         bool doT0Fit):
            AthMessaging{name},
            m_name{name},
            m_hits{std::move(hits)},
            m_calibrator{calibrator},
            m_doT0Fit{doT0Fit} {
        
        for (const HitType& hit : m_hits) {
            /// Beam spot constraint
            if (!hit->spacePoint()) {
                m_nMeas +=2;
                continue;
            }
            m_nMeas += hit->spacePoint()->measuresEta();
            m_nMeas += hit->spacePoint()->measuresPhi();
            m_hasPhi |= hit->spacePoint()->measuresPhi();
            if (m_doT0Fit) {
                if (hit->type() == xAOD::UncalibMeasType::RpcStripType) {
                    ++m_nMeas;
                }
            }
        }
        std::sort(m_hits.begin(), m_hits.end(), 
                  [](const HitType& a, const HitType& b) { 
                    return a->positionInChamber().z() < b->positionInChamber().z();
                  });
    }

    CalibSegmentChi2Minimizer* CalibSegmentChi2Minimizer::Clone() const {
        HitVec copyHits{};
        for (const HitType& copyMe : m_hits) {
            copyHits.emplace_back(std::make_unique<CalibratedSpacePoint>(*copyMe));
        }
        return new CalibSegmentChi2Minimizer(m_name, std::move(copyHits), m_calibrator, m_doT0Fit);
    }
    unsigned int CalibSegmentChi2Minimizer::NDim() const {
        return toInt(AxisDefs::nPars);
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
        HitVec released{};
        std::vector<const SpacePoint*> uncalibSP{};
        for(HitType& hit : m_hits) {
            /// Beam spot constraint does not have a space point
            if (!hit->spacePoint()) {
                released.push_back(std::move(hit));
            } else {
                uncalibSP.push_back(hit->spacePoint());
            }        
        }
        const Amg::Vector3D segPos{pars[toInt(AxisDefs::x0)],
                                   pars[toInt(AxisDefs::y0)], 0.};

        const Amg::Vector3D segDir = Amg::Vector3D(pars[toInt(AxisDefs::tanPhi)],
                                                   pars[toInt(AxisDefs::tanTheta)], 1.).unit();
        
        const double segT0 = pars[toInt(AxisDefs::time)];
        HitVec reCalibSP = m_calibrator(uncalibSP, segPos, segDir, segT0);
        released.insert(released.end(),
                        std::make_move_iterator(reCalibSP.begin()),
                        std::make_move_iterator(reCalibSP.end()));
        m_hits.clear();
        return released;
    }
    double CalibSegmentChi2Minimizer::DoEval(const double* pars) const {
      
        /// 
        const Amg::Vector3D segPos{pars[toInt(AxisDefs::x0)],
                                   pars[toInt(AxisDefs::y0)], 0.};

        const Amg::Vector3D segDir = Amg::Vector3D(pars[toInt(AxisDefs::tanPhi)],
                                                   pars[toInt(AxisDefs::tanTheta)], 1.).unit();
        
        const double segT0 = pars[toInt(AxisDefs::time)];
        ATH_MSG_VERBOSE("Starting parameters  position: "<<Amg::toString(segPos)<<", direction: "<<Amg::toString(segDir)
                       <<", t0: "<<segT0<<".");

        
        std::vector<const SpacePoint*> uncalibSP{};
        uncalibSP.reserve(m_hits.size());
        std::vector<const CalibratedSpacePoint*> calibHits{};
        for(const HitType& hit : m_hits) {
            /// Beam spot constraint does not have a space point
            if (!hit->spacePoint()) {
                calibHits.push_back(hit.get());
                continue;
            }
            uncalibSP.push_back(hit->spacePoint());        
        }
        HitVec garbage = m_calibrator(uncalibSP, segPos, segDir, segT0);
        for (const HitType& hit: garbage) {
            calibHits.push_back(hit.get());
        }
        double chi2{0.};
        std::optional<double> t = m_doT0Fit ? std::make_optional<double>(segT0) : std::nullopt;

        for (const CalibratedSpacePoint* hit : calibHits) {
            chi2 += SegmentFitHelpers::chiSqTerm(segPos,segDir, t, *hit, msg());
        }
        ATH_MSG_VERBOSE("Final chi2: "<<chi2);
        return chi2;
    }



}
