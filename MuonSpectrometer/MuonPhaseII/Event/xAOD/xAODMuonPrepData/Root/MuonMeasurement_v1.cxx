/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonPrepData/versions/MuonMeasurement_v1.h"
#include "MuonReadoutGeometryR4/MuonReadoutElement.h"
namespace xAOD{
    MuonMeasurement_v1::MuonMeasurement_v1(const MuonMeasurement_v1& other):
        UncalibratedMeasurement_v1{other} {}
    MuonMeasurement_v1& MuonMeasurement_v1::operator=(const  MuonMeasurement_v1& other) {
        if (this != &other) {
            static_cast<UncalibratedMeasurement_v1&>(*this) = other;
        }
        return (*this);
    }
    const Identifier& MuonMeasurement_v1::identify() const {
        if (!m_identifier.isValid()){
            m_identifier.set(readoutElement()->measurementId(measurementHash()));
        }
        return (*m_identifier.ptr());
    }
    const Acts::Surface& MuonMeasurement_v1::surface() const {
        if (!m_surface.isValid()){
            const IdentifierHash hash = 
                type() == UncalibMeasType::MdtDriftCircleType 
                    ? measurementHash() 
                    : layerHash();
            m_surface.set(&readoutElement()->surface(hash));
        }
        return (**m_surface.ptr());
    }
}