/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMuonViews/IdentifierSorter.h"

#include "Acts/Utilities/Helpers.hpp"

namespace MuonR4{

bool IdentifierSorter::operator()(const xAOD::MuonMeasurement* a, const xAOD::MuonMeasurement* b) const {
    if (a->type() != b->type()) {
        return Acts::toUnderlying(a->type()) < Acts::toUnderlying(b->type());
    }
    if (a->identifierHash() != b->identifierHash()) {
        return a->identifierHash() < b->identifierHash();
    }
    if (a->layerHash() < b->layerHash()) {
        return a->layerHash() < b->layerHash();
    }
    if (a->measuresPhi() < b->measuresPhi()) {
        return b->measuresPhi();
    }
    return a->measurementHash() < b->measurementHash();
}

}