/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h> 
namespace MuonR4{

    CalibratedSpacePoint::CalibratedSpacePoint(const SpacePoint* uncalibSpacePoint,
                                               Amg::Vector3D&& posInChamber,
                                               State st):
        m_parent{uncalibSpacePoint},
        m_posInChamber{posInChamber},
        m_state{st} {
    }
    const SpacePoint* CalibratedSpacePoint::spacePoint() const { return m_parent; }
    const Amg::Vector3D& CalibratedSpacePoint::localPosition() const { return m_posInChamber; }
    const Amg::Vector3D& CalibratedSpacePoint::sensorDirection() const {
        static const Amg::Vector3D s_Dir{Amg::Vector3D::UnitX()};
        return m_parent? m_parent->sensorDirection() : s_Dir;
    }
    const Amg::Vector3D& CalibratedSpacePoint::toNextSensor() const {
        static const Amg::Vector3D s_Dir{Amg::Vector3D::UnitY()};
        return m_parent? m_parent->toNextSensor() : s_Dir;
    }
    const Amg::Vector3D& CalibratedSpacePoint::planeNormal() const {
        static const Amg::Vector3D s_Dir{Amg::Vector3D::UnitZ()};
        return m_parent? m_parent->planeNormal() : s_Dir;
    }
    const CalibratedSpacePoint::Cov_t& 
        CalibratedSpacePoint::covariance() const {
        return m_cov;
    }
    bool CalibratedSpacePoint::hasTime() const { return m_measuresTime; }
    bool CalibratedSpacePoint::measuresLoc0() const { return measuresPhi(); }
    bool CalibratedSpacePoint::measuresLoc1() const { return measuresEta(); } 
    bool CalibratedSpacePoint::isStraw() const { return type() == xAOD::UncalibMeasType::MdtDriftCircleType; }
    double CalibratedSpacePoint::driftRadius() const {
        return m_driftRadius;
    }
    void CalibratedSpacePoint::setDriftRadius(const double r) { m_driftRadius = r; }    
    xAOD::UncalibMeasType CalibratedSpacePoint::type() const {
        return m_parent ? m_parent->type() : xAOD::UncalibMeasType::Other;
    }
    double CalibratedSpacePoint::time() const { return m_time; }
    void CalibratedSpacePoint::setTimeMeasurement(double t) {
        m_time = t;
        m_measuresTime = true;
    }               
    bool CalibratedSpacePoint::measuresPhi() const { return !m_parent || m_parent->measuresPhi(); }
    bool CalibratedSpacePoint::measuresEta() const { return !m_parent || m_parent->measuresEta(); }
    CalibratedSpacePoint::State CalibratedSpacePoint::fitState() const { return m_state; }
    void CalibratedSpacePoint::setFitState(State st) { m_state = st; }
    unsigned CalibratedSpacePoint::dimension() const { return measuresEta() + measuresPhi(); }
    void CalibratedSpacePoint::setCovariance(const Cov_t& cov) {m_cov = cov;}

    void CalibratedSpacePoint::print(std::ostream& ostr) const {
        if (type() != xAOD::UncalibMeasType::Other) {
            ostr<<"Calibrated SP "<<spacePoint()->msSector()->idHelperSvc()->toString(spacePoint()->identify());
        } else {
            ostr<<"Auxiliary measurement";
        }
        ostr<<" @ "<<Amg::toString(localPosition());
        if (type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            ostr<<", wire: "<<Amg::toString(sensorDirection())<<", drift R: "<<driftRadius();
        } else {
            ostr<<", (dir/toNext/planeNormal): "<<Amg::toString(sensorDirection())
                <<"/"<<Amg::toString(toNextSensor())<<"/"<<Amg::toString(planeNormal());
        }
        auto boolToStr = [](const bool B) -> std::string {
            return B ? "yay" : "nay";
        };
        if (hasTime()) {
            ostr<<", time: "<<time();
        }
        ostr<<", measures eta/phi/time: "<<boolToStr(measuresEta())
            <<"/"<<boolToStr(measuresPhi())<<"/"<<boolToStr(hasTime());
        ostr<<", covariance (eta/phi/time): ("<<m_cov[Acts::toUnderlying(CovIdx::etaCov)]<<", "
             <<m_cov[Acts::toUnderlying(CovIdx::phiCov)]<<", "<<m_cov[Acts::toUnderlying(CovIdx::timeCov)]<<")";
    }
}