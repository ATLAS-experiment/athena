/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_GENVERTEX_H
#define ATLASHEPMC_GENVERTEX_H
#include "HepMC3/GenVertex.h"
#include "HepMC3/PrintStreams.h"
#include "AtlasHepMC/Barcode.h"
#include "AtlasHepMC/AttributeNames.h"
namespace HepMC3 {
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  begin(const HepMC3::GenVertex& v) { return v.particles_out().begin(); }
inline std::vector<HepMC3::ConstGenParticlePtr>::const_iterator  end(const HepMC3::GenVertex& v) { return v.particles_out().end(); }
inline std::vector<HepMC3::GenParticlePtr>::const_iterator  begin(HepMC3::GenVertex& v) { return v.particles_out().begin(); }
inline std::vector<HepMC3::GenParticlePtr>::const_iterator  end(HepMC3::GenVertex& v) { return v.particles_out().end(); }

/// @brief Print one-line info with idiomatic C++ printing
/// @note More generic printing methods from HepMC3::Print should be preffered - move to PrintStreams.h?
inline std::ostream& operator << (std::ostream& os,  GenVertexPtr v) { ConstGenVertexPtr cv = v; Print::line(os,std::move(cv)); return os; }
}
namespace HepMC {
typedef HepMC3::GenVertexPtr GenVertexPtr;
typedef HepMC3::ConstGenVertexPtr ConstGenVertexPtr;
inline GenVertexPtr newGenVertexPtr(const HepMC3::FourVector& pos = HepMC3::FourVector::ZERO_VECTOR(), const int i=0) {
    GenVertexPtr v = std::make_shared<HepMC3::GenVertex>(pos);
    v->set_status(i);
    return v;
}
using HepMC3::GenVertex;
}
#endif
