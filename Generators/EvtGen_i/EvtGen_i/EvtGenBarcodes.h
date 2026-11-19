/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EVTGEN_I_EVTGENBARCODES_H
#define EVTGEN_I_EVTGENBARCODES_H

#include "AtlasHepMC/Barcode.h"
#include "AtlasHepMC/GenEvent.h"

#include <algorithm>

namespace EvtGenBarcodes {

  /// Finds the largest particle barcode and the lowest vertex barcode of an event.
  inline void findBarcodeRange(const HepMC::GenEvent& event, int& maxParticleBarcode, int& minVertexBarcode) {
    maxParticleBarcode = 0;
    minVertexBarcode = 0;
    for (const auto& p : event.particles()) {
      maxParticleBarcode = std::max(maxParticleBarcode, HepMC::barcode(p));
    }
    for (const auto& v : event.vertices()) {
      minVertexBarcode = std::min(minVertexBarcode, HepMC::barcode(v));
    }
  }

}

#endif
