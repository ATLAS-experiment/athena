#include "DataCollector.h"

std::ostream& operator << (std::ostream& os,
			   const GlobalSim::DataCollector& col) {
  
  os << "DataCollector\nints:[" << col.m_ints.size() << "]\n\n";

  for (const auto& p : col.m_ints) {
    os << p.first << ":\n"
       << p.second <<'\n';
  }

  os << "\nsize_t containers:\n\n";

  for (const auto& p : col.m_sz_ts) {
    os << p.first << ":\n"
       << p.second << '\n';

    os <<'\n';
  }
 
 
  os << "\neEmTob containers:\n\n";

  for (const auto& p : col.m_eEmTobContainers) {
    os << p.first << " [" << p.second.size() << "]\n";
    os << p.second;

  }

  os << "\n vector of eEmTob containers:\n\n";

  for (const auto& p : col.m_vec_eEmTobContainers) {
    os << p.first << " [" << p.second.size() << "]\n"
       << p.second << '\n';
   
  }

 
  os << "\n vector of GenericTob containers:\n\n";

  for (const auto& p : col.m_vec_GenericTobContainers) {
    os << '\n' << p.first << " [" << p.second.size() << "]\n"
       << p.second<<'\n';
  }
 
  return os;
}      
