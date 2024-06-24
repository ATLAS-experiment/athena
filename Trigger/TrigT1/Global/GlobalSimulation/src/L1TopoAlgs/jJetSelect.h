/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  GLOBALSIM_JJETSELECT_H
#define  GLOBALSIM_JJETSELECT_H

// convert selected jJetTOBs to GenericTOBs
#include <string>

class StatusCode;

namespace GlobalSim {

  class jJetTOBArray;
  class GenericTOBArray;

  class jJetSelect{
  public:

    jJetSelect(const std::string& name,
	       unsigned int InputWidth,
	       unsigned int MinET,
	       unsigned int MinEta,
	       unsigned int MaxEta);
    
    virtual ~jJetSelect(){}

    StatusCode run(const jJetTOBArray&, GenericTOBArray&) const;

    std::string toString() const;
  private:

    std::string m_name{};

    using parType_t = unsigned int;
    
    unsigned int m_InputWidth;
    unsigned int m_MinET{0u};
    unsigned int m_MinEta{0u};
    unsigned int m_MaxEta{0u};
  };
  
}

#endif
