/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PU1SuppPortsOut.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const GlobalSim::PU1SuppPortsOut& out) {
    os << "PU1SuppPortsOut:\n";
    os << "Multiplicity = " << out.m_outputMultiplicity << "\n";
    for (std::size_t i = 0; i < out.m_outputTobs.size(); ++i) {
        os << "  TOB[" << i << "] = " << out.m_outputTobs[i] << "\n";
    }
    return os;
}

