/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_ATTRIBUTENAMES_H
#define ATLASHEPMC_ATTRIBUTENAMES_H
#include <string>

// Declare the attribute variables.
// ATLASHEPMC_ATTRIBNAME can be defined before including this file
// to make the definitions.
#ifndef ATLASHEPMC_ATTRIBNAME
# define ATLASHEPMC_ATTRIBNAME(N) extern const std::string N
#endif
namespace HepMCStr {
ATLASHEPMC_ATTRIBNAME(BunchCrossingTime);      // Bunch crossing time offset.
ATLASHEPMC_ATTRIBNAME(LHERecord);              // Original Les Houches Event record.
ATLASHEPMC_ATTRIBNAME(PileUpType);             // ATLAS pile-up type classification.
ATLASHEPMC_ATTRIBNAME(ShadowParticle);         // Marks a shadow particle.
ATLASHEPMC_ATTRIBNAME(ShadowParticleId);       // Original particle identifier for a shadow particle.
ATLASHEPMC_ATTRIBNAME(alphaQCD);               // Strong coupling constant.
ATLASHEPMC_ATTRIBNAME(alphaQED);               // Electromagnetic coupling constant.
ATLASHEPMC_ATTRIBNAME(barcode);                // Particle or vertex barcode.
ATLASHEPMC_ATTRIBNAME(barcodes);               // Collection of barcodes.
ATLASHEPMC_ATTRIBNAME(cycles);                 // Generator-specific cycle information. Set to 1 if the cycles(loops) are present.
ATLASHEPMC_ATTRIBNAME(event_scale);            // Event hard-process scale.
ATLASHEPMC_ATTRIBNAME(filterHT);               // Generator-level HT used for filtering. Typically from POWHEG.
ATLASHEPMC_ATTRIBNAME(filterMET);              // Generator-level missing transverse energy used for filtering.  Typically from POWHEG.
ATLASHEPMC_ATTRIBNAME(filterWeight);           // Event filter weight.
ATLASHEPMC_ATTRIBNAME(flow);                   // Particle flow information.
ATLASHEPMC_ATTRIBNAME(flow1);                  // First flow index.
ATLASHEPMC_ATTRIBNAME(flow2);                  // Second flow index.
ATLASHEPMC_ATTRIBNAME(flow3);                  // Third flow index.
ATLASHEPMC_ATTRIBNAME(flows);                  // Collection of flow indices.
ATLASHEPMC_ATTRIBNAME(long_long_event_number); // 64-bit event number.
ATLASHEPMC_ATTRIBNAME(mpi);                    // Number of multiple parton interactions.
ATLASHEPMC_ATTRIBNAME(phi);                    // Azimuthal angle for particle polarization.
ATLASHEPMC_ATTRIBNAME(random_states);          // Random number generator states.
ATLASHEPMC_ATTRIBNAME(signal_process_id);      // Signal process identifier.
ATLASHEPMC_ATTRIBNAME(signal_process_vertex);  // Signal process vertex.
ATLASHEPMC_ATTRIBNAME(signal_vertex_id);       // Signal vertex identifier.
ATLASHEPMC_ATTRIBNAME(theta);                  // Polar angle for particle polarization.
ATLASHEPMC_ATTRIBNAME(weights);                // Event weights.
}

#undef ATLASHEPMC_ATTRIBNAME
#endif
