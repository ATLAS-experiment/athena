/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonSpacePoint/CalibratedSpacePoint.h>

namespace MuonR4{
    CalibratedSpacePoint::CalibratedSpacePoint(const SpacePoint* uncalibSpacePoint,
                                               Amg::Vector3D&& posInChamber,
                                               Amg::Vector3D&& dirInChamber):
        m_parent{uncalibSpacePoint},
        m_posInChamber{posInChamber},
        m_dirInChamber{dirInChamber} {
    }
    const SpacePoint* CalibratedSpacePoint::spacePoint() const{
        return m_parent;
    }
    const Amg::Vector3D& CalibratedSpacePoint::positionInChamber() const {
        return m_posInChamber;
    }
    const Amg::Vector3D& CalibratedSpacePoint::directionInChamber() const{
        return m_dirInChamber;
    }
    double CalibratedSpacePoint::driftRadius() const {
        return m_driftRadius;
    }
    void CalibratedSpacePoint::setDriftRadius(const double r) {
        m_driftRadius = r;
    }    
    const AmgSymMatrix(2)& CalibratedSpacePoint::covariance() const {
        return m_cov;
    }
    void CalibratedSpacePoint::setCovariance(AmgSymMatrix(2)&& cov) {
        m_cov = std::move(cov);
    }
    xAOD::UncalibMeasType CalibratedSpacePoint::type() const {
        return m_parent ? m_parent->type() : xAOD::UncalibMeasType::Other;
    }
    double CalibratedSpacePoint::time() const {
        return m_time;
    }
    double CalibratedSpacePoint::timeCovariance() const {
        return m_timeCov;
    }
    void CalibratedSpacePoint::setTimeMeasurement(double t, double errT) {
        m_time = t;
        m_timeCov = errT;
    }

}