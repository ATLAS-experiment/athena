/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "LArStripNeighborhoodDumper.h"

#include <fstream>

namespace GlobalSim {
  LArStripNeighborhoodDumper::LArStripNeighborhoodDumper(){
  }

  StatusCode
  LArStripNeighborhoodDumper::dump(const std::string& name,
				   const xAOD::EventInfo& eventInfo,
				   const LArStripNeighborhoodContainer& neighborhoods) const {

    std::ofstream out(name + "_" +
                      std::to_string(eventInfo.eventNumber()) +
                      ".log");

    out << "run " << eventInfo.runNumber()
        << " evt " <<  eventInfo.eventNumber()
        << " is simulation " << std::boolalpha
        << eventInfo.eventType(xAOD::EventInfo::IS_SIMULATION)
        << " weight " << eventInfo.mcEventWeight() << '\n';

    for (const auto& nbhd : neighborhoods) {
      out << *nbhd << '\n';
    }

    out.close();

    return StatusCode::SUCCESS;
  }

  void dump_stripdataVector(const StripDataVector& sdv, std::ostream& os) {

    for(const auto& sd : sdv) {
      os << sd.m_eta << ' ';
    }
    os << '\n';


    for(const auto& sd : sdv) {
      os << sd.m_phi << ' ';
    }
    os << '\n';

    for(const auto & sd : sdv) {
      os << sd.m_e << ' ';
    }
    os << '\n';
    os << '\n';
  }

  void dump_n(const LArStripNeighborhood* n,
              std::ostream& os){
    dump_stripdataVector(n->phi_low(), os);
    dump_stripdataVector(n->phi_center(), os);
    dump_stripdataVector(n->phi_high(), os);
  }

  StatusCode
  LArStripNeighborhoodDumper::dumpTerse(const std::string& name,
					const xAOD::EventInfo& eventInfo,
					const LArStripNeighborhoodContainer& neighborhoods) const {

    std::ofstream out(name + "_" +
                      std::to_string(eventInfo.eventNumber()) +
                      "_terse.log");
    out << "run " << eventInfo.runNumber()
	<< " evt " <<  eventInfo.eventNumber()
	<< " is simulation " << std::boolalpha
	<< eventInfo.eventType(xAOD::EventInfo::IS_SIMULATION)
	<< " weight " << eventInfo.mcEventWeight() << '\n';

    for (const auto& n : neighborhoods) {dump_n(n, out);}


    out.close();

    return StatusCode::SUCCESS;
  }
}    


