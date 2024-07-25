#include "StripData.h"


std::ostream & operator<<(std::ostream& os,
			  const GlobalSim::StripData& sd) {

  os << "StripData eta: " << sd.m_eta
     << " phi " << sd.m_phi
     << " et " << sd.m_et;
  return os;
}
  
