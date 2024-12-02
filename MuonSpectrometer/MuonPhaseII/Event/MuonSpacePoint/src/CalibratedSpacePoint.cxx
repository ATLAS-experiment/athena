/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonSpacePoint/CalibratedSpacePoint.h>

namespace MuonR4{

    using Covariance_t = CalibratedSpacePoint::Covariance_t;
    CalibratedSpacePoint::CalibratedSpacePoint(const SpacePoint* uncalibSpacePoint,
                                               Amg::Vector3D&& posInChamber,
                                               Amg::Vector3D&& dirInChamber,
                                               State st):
        m_parent{uncalibSpacePoint},
        m_posInChamber{posInChamber},
        m_dirInChamber{dirInChamber},
        m_state{st} {
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
    const Covariance_t& CalibratedSpacePoint::covariance() const {
        return m_cov;
    }
    xAOD::UncalibMeasType CalibratedSpacePoint::type() const {
        return m_parent ? m_parent->type() : xAOD::UncalibMeasType::Other;
    }
    double CalibratedSpacePoint::time() const {
        return m_time;
    }
    void CalibratedSpacePoint::setTimeMeasurement(double t) {
        m_time = t;
        m_measuresTime = true;
    }               
    bool CalibratedSpacePoint::measuresTime() const {
        return m_measuresTime;
    }
    bool CalibratedSpacePoint::measuresPhi() const {
        return !m_parent || m_parent->measuresPhi();
    }
    bool CalibratedSpacePoint::measuresEta() const {
        return !m_parent || m_parent->measuresEta();
    }
    CalibratedSpacePoint::State CalibratedSpacePoint::fitState() const {
        return m_state;
    }
    void CalibratedSpacePoint::setFitState(State st) {
        m_state = st;
    }
    unsigned CalibratedSpacePoint::dimension() const {
        return measuresEta() + measuresPhi();
    }
}