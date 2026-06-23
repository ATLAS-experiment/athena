/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/* Author: Andrii Verbytskyi andrii.verbytskyi@mpp.mpg.de */

#ifndef ATLASHEPMC_ATTRIBUTENAMES_H
#define ATLASHEPMC_ATTRIBUTENAMES_H
#include <string>
namespace HepMCStr {
 inline const std::string BunchCrossingTime{"BunchCrossingTime"};
 inline const std::string LHERecord{"LHERecord"};
 inline const std::string PileUpType{"PileUpType"};
 inline const std::string ShadowParticle{"ShadowParticle"};
 inline const std::string ShadowParticleId{"ShadowParticleId"};
 inline const std::string alphaQCD{"alphaQCD"};
 inline const std::string alphaQED{"alphaQED"};
 inline const std::string barcode{"barcode"};
 inline const std::string barcodes{"barcodes"};
 inline const std::string cycles{"cycles"};
 inline const std::string event_scale{"event_scale"};
 inline const std::string filterHT{"filterHT"};
 inline const std::string filterMET{"filterMET"};
 inline const std::string filterWeight{"filterWeight"};
 inline const std::string flow{"flow"};
 inline const std::string flow1{"flow1"};
 inline const std::string flow2{"flow2"};
 inline const std::string flow3{"flow3"};
 inline const std::string flows{"flows"};
 inline const std::string long_long_event_number{"long_long_event_number"};
 inline const std::string mpi{"mpi"};
 inline const std::string phi{"phi"};
 inline const std::string random_states{"random_states"};
 inline const std::string signal_process_id{"signal_process_id"};
 inline const std::string signal_process_vertex{"signal_process_vertex"};
 inline const std::string signal_vertex_id{"signal_vertex_id"};
 inline const std::string theta{"theta"};
 inline const std::string weights{"weights"};
}
#endif
