/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  GLOBALSIM_EEMSELECT_H
#define  GLOBALSIM_EEMSELECT_H

// Converts selected eEmTOBs to GenericTOBS

#include <string>

class StatusCode;

namespace GlobalSim{
  
  class eEmTOBArray;
  class GenericTOBArray;
  
  class eEmSelect {
  public:

    eEmSelect(const std::string& name,
			 unsigned int InputWidth,
			 unsigned int MinET,
			 unsigned int REtaMin,
			 unsigned int RHadMin,
			 unsigned int WsTotMin);
    
    virtual ~eEmSelect();
    
    StatusCode run(const eEmTOBArray& input,
		   GenericTOBArray& output) const;

    std::string toString() const;
    
  private:
    std::string m_name {};
    unsigned int m_InputWidth{0}; // max number output objs
    unsigned int m_MinET{0};
    unsigned int m_REtaMin{0};
    unsigned int m_RHadMin{0};
    unsigned int m_WsTotMin{0};
    
  };
  
}

#endif
