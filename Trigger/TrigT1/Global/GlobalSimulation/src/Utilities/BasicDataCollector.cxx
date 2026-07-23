/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicDataCollector.h"

#include <algorithm>
#include <sstream>

namespace GlobalSim {

  using namespace std::chrono;

  BasicDataCollector::BasicDataCollector():
    m_t0{high_resolution_clock::now()}{
  }
  
  void BasicDataCollector::collect(const IAlgTool& tool,
				   const std::string& msg){
    const auto& label = tool.name();
    auto iter = 
      std::find_if(m_msgs.begin(),
		   m_msgs.end(),
		   [&label](const std::pair<std::string,
			    std::vector<std::string>>& p){
		     return label == p.first;
		   });
    auto d =  high_resolution_clock::now() - m_t0;

    auto ss = std::stringstream();
    ss << duration_cast<microseconds>(d).count() << " (us): " << msg;
    
    if (iter == m_msgs.cend()){
      m_msgs.push_back(std::pair(label, std::vector<std::string>{ss.str()}));
    } else {
      (iter->second).push_back(ss.str());
    }
  }

  
  std::string BasicDataCollector::to_string() const{
    auto result = std::string();
    for (const auto& p : m_msgs){
      result += p.first + '\n';;
      for (const auto& m : p.second){
	result += "    "  + m + '\n';
      }
    }
    return result;
  }

  std::ostream& operator << (std::ostream& os, const BasicDataCollector& bdc){
    os << bdc.to_string();
    return os;
  }
}
