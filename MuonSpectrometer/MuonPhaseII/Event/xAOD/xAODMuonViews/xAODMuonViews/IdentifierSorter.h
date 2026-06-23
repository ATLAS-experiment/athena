/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODMUONVIEWS_IDENTIFIERSORTER_H
#define XAODMUONVIEWS_IDENTIFIERSORTER_H

#include "xAODMuonPrepData/MuonMeasurement.h"

namespace MuonR4{
    /** @brief Helper struct to establish a sorting of the muon measurements based on their
     *         Identifier. Measurements are first sorted by their associated readout element,
     *         then by the measurement layer. Precision measurements are then ordered before
     *         the non-precision measurements */
    class IdentifierSorter{
        public:
            /** @brief default constructor */
            IdentifierSorter() = default;
            /** @brief Sorting operator */
            bool operator()(const xAOD::MuonMeasurement* a, const xAOD::MuonMeasurement* b) const;
    };
}

#endif