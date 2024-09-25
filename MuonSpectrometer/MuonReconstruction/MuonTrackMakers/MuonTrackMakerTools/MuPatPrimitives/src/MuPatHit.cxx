/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuPatPrimitives/MuPatHit.h"

#include <iostream>
#include <utility>

#include "TrkEventPrimitives/ResidualPull.h"
#include "TrkMeasurementBase/MeasurementBase.h"

namespace Muon {


    // member functions
    MuPatHit::MuPatHit(std::shared_ptr<const Trk::TrackParameters> pars, std::shared_ptr<const Trk::MeasurementBase> presMeas,
                       std::shared_ptr<const Trk::MeasurementBase> broadMeas,  const Info& info) :
        Trk::ObjectCounter<MuPatHit>{},
        m_pars(std::move(pars)), m_precisionMeas(std::move(presMeas)), m_broadMeas(std::move(broadMeas)), m_info(info) {

    }

    MuPatHit::MuPatHit(const MuPatHit& hit):
      Trk::ObjectCounter<MuPatHit>(hit) {
        copy(hit);
    }

    MuPatHit& MuPatHit::operator=(const MuPatHit& hit) {
        if (&hit != this) {
            copy(hit);
        }
        return *this;
    }
    void MuPatHit::copy(const MuPatHit& hit) {
        m_pars = hit.m_pars;
        m_precisionMeas = hit.m_precisionMeas;
        m_broadMeas = hit.m_broadMeas->uniqueClone();
        m_residual = hit.m_residual;
        m_pull = hit.m_pull;
        m_info = hit.m_info;
    }
    const Trk::TrackParameters& MuPatHit::parameters() const { return *m_pars; }

    const Trk::MeasurementBase& MuPatHit::measurement() const {
        if (info().selection == Precise) return *m_precisionMeas;
        return *m_broadMeas;
    }
    const Trk::MeasurementBase& MuPatHit::preciseMeasurement() const { return *m_precisionMeas; }
    const Trk::MeasurementBase& MuPatHit::broadMeasurement() const { return *m_broadMeas; }
    const MuPatHit::Info& MuPatHit::info() const { return m_info; }
    MuPatHit::Info& MuPatHit::info() { return m_info; }
    double MuPatHit::residual() const{return m_residual;}
    double MuPatHit::pull() const{return m_pull;}
    void MuPatHit::setResidual(double residual, double pull) {
        m_residual = residual;
        m_pull = pull;
    }
}  // namespace Muon
