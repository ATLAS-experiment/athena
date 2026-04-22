/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):

#include "xAODMuonPrepData/versions/CombinedMuonStrip_v1.h"

#include "AthLinks/ElementLink.h"
#include "xAODMuonPrepData/MuonMeasurementContainer.h"
namespace {
    using Link_t = ElementLink<xAOD::MuonMeasurementContainer>;
    static const SG::Accessor<Link_t> acc_primLink{"MuonStripLink1"};
    static const SG::Accessor<Link_t> acc_secondLink{"MuonStripLink2"};

}
namespace xAOD {
    xAOD::UncalibMeasType CombinedMuonStrip_v1::type() const {
        if (const auto* prim = primaryStrip(); prim != nullptr) {
            return prim->type();
        }
        if (const auto* second = secondaryStrip() ; second != nullptr) {
            return second->type();
        }
        return xAOD::UncalibMeasType::nTypes;
    }
    const xAOD::MuonMeasurement* CombinedMuonStrip_v1::primaryStrip() const {
        if (acc_primLink.isAvailable(*this) && acc_primLink(*this).isValid()) {
              return *acc_primLink(*this);
        }
        return nullptr;
    }
     const xAOD::MuonMeasurement* CombinedMuonStrip_v1::secondaryStrip() const {

        if (acc_secondLink.isAvailable(*this) && acc_secondLink(*this).isValid()) {
              return *acc_secondLink(*this);
        }
        return nullptr;
    }

    void CombinedMuonStrip_v1::setPrimaryStrip(const xAOD::MuonMeasurement* meas) {
        assert(meas != nullptr);
        const auto* cont = static_cast<const xAOD::MuonMeasurementContainer*>(meas->container());
        acc_primLink(*this) = Link_t{*cont, meas->index()};
    }
    void CombinedMuonStrip_v1::setSecondaryStrip(const xAOD::MuonMeasurement* meas){
        assert(meas != nullptr);
        const auto* cont = static_cast<const xAOD::MuonMeasurementContainer*>(meas->container());
        acc_secondLink(*this) = Link_t{*cont, meas->index()};
    }
}