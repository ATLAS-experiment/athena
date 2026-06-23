/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_GENPARTICLE_H
#define ATLASHEPMC_GENPARTICLE_H
#include "HepMC3/GenParticle.h"
#include "HepMC3/PrintStreams.h"
#include "AtlasHepMC/Barcode.h"
#include "AtlasHepMC/Polarization.h"
#include "AtlasHepMC/Flow.h"
namespace HepMC3 {
/// @brief Print one-line info with idiomatic C++ printing
/// @note More generic printing methods from HepMC3::Print should be preffered - move to PrintStreams.h?
inline std::ostream& operator << (std::ostream& os,  GenParticlePtr p) {ConstGenParticlePtr cp = p; Print::line(os, std::move(cp)); return os; }
}
namespace HepMC {
typedef HepMC3::GenParticlePtr GenParticlePtr;
typedef HepMC3::ConstGenParticlePtr ConstGenParticlePtr;
inline GenParticlePtr newGenParticlePtr(const HepMC3::FourVector &mom = HepMC3::FourVector::ZERO_VECTOR(), int pid = 0, int status = 0) {
    return std::make_shared<HepMC3::GenParticle>(mom, pid, status);
}
inline ConstGenParticlePtr newConstGenParticlePtr(const HepMC3::FourVector &mom = HepMC3::FourVector::ZERO_VECTOR(), int pid = 0, int status = 0) {
    return std::make_shared<const HepMC3::GenParticle>(mom, pid, status);
}
inline int barcode_or_id(const ConstGenParticlePtr& p) { return p->id(); }

using HepMC3::GenParticle;
}
#endif
