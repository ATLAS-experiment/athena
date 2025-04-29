/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPatternEvent/Segment.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

namespace MuonR4{
    Segment::Segment(Amg::Vector3D&& globPos, Amg::Vector3D&& globDir,
                     const SegmentSeed* parent, MeasVec&& constMeas,
                     double chi2, unsigned int nDoF):
        m_globPos{std::move(globPos)},
        m_globDir{std::move(globDir)},
        m_parent{parent},
        m_measurements{std::move(constMeas)},
        m_chi2{chi2}, 
        m_nDoF{nDoF}{
        
        for (const MeasType& meas : m_measurements) {
            switch(meas->type()) {
                case xAOD::UncalibMeasType::MMClusterType:
                case xAOD::UncalibMeasType::MdtDriftCircleType:
                    m_summary.nPrecHits += meas->fitState() == CalibratedSpacePoint::State::Valid;
                    m_summary.nPrecOutlier += (meas->fitState() != CalibratedSpacePoint::State::Valid);
                    m_summary.tech = meas->type();
                    break;
                case xAOD::UncalibMeasType::Other:
                case xAOD::UncalibMeasType::RpcStripType:
                case xAOD::UncalibMeasType::TgcStripType:
                    m_summary.nEtaTrigHits += meas->measuresEta();
                    m_summary.nPhiHits += meas->measuresPhi();
                    break;
                case xAOD::UncalibMeasType::sTgcStripType: {
                    auto* prd = static_cast<const xAOD::sTgcMeasurement*>(meas->spacePoint()->primaryMeasurement());
                    switch (prd->channelType()) {
                        case sTgcIdHelper::sTgcChannelTypes::Strip:
                            m_summary.nPrecHits += meas->fitState() == CalibratedSpacePoint::State::Valid;
                            m_summary.nPrecOutlier += (meas->fitState() != CalibratedSpacePoint::State::Valid);
                            m_summary.tech = meas->type();
                            break;
                        case sTgcIdHelper::sTgcChannelTypes::Pad:
                            ++m_summary.nEtaTrigHits;
                            ++m_summary.nPhiHits;
                            break;
                        case sTgcIdHelper::sTgcChannelTypes::Wire:
                            ++m_summary.nPhiHits;
                            break;
                    }
                }
                default:
                    break;
            }
        }
    }

    /** @brief Sets the fitted segment time */
    void Segment::setSegmentT0(double t0) {
        m_t0 = std::make_optional<double>(t0);             
    }
    /** @brief Set how many iteration the fitter needed to reach convergence */
    void Segment::setCallsToConverge(unsigned int nCalls) {
        m_nCalls = nCalls;
    }
    /** @brief Set the uncertainties from the fit */
    void Segment::setParUncertainties(SegmentFit::Covariance&& cov){
        m_cov = std::move(cov);
    }
    

}