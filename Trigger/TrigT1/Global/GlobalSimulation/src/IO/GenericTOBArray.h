/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  TOBArray.h
//  TopoCore
//  Created by Joerg Stelzer on 11/17/12.
//
// Adapted for GlobalL1TopoSim

#ifndef GLOBALSIM_GENERICTOBARRAY_H
#define GLOBALSIM_GENERICTOBARRAY_H

#include "L1TopoEvent/DataArrayImpl.h"
#include "L1TopoEvent/GenericTOB.h"

#include "AthenaKernel/CLASS_DEF.h"

#include <ostream>
#include <string>

namespace TCS {

  class GenericTOB;
  class CompositeTOB;
}

namespace GlobalSim {
			
  typedef bool (sort_fnc)(TCS::GenericTOB* tob1, TCS::GenericTOB* tob2);
  
  class GenericTOBArray: virtual public TCS::DataArray,
			 virtual public TCS::DataArrayImpl<TCS::GenericTOB>{
    
  public:
  
    // default constructor
    GenericTOBArray(const std::string& name): DataArray{name}{
    };

    using DataArrayImpl<TCS::GenericTOB>::push_back;
    void push_back(const TCS::CompositeTOB& tob);
    
    void sort(sort_fnc);
    
    bool ambiguityFlag() const {return m_ambiguityFlag;}
    
    void setAmbiguityFlag(bool ambiguityFlag) {
      m_ambiguityFlag = ambiguityFlag;
    }

    
    virtual void print(std::ostream& os) const override;

  private:

    friend std::ostream& operator << (std::ostream&,
				      const GlobalSim::GenericTOBArray&);

    bool m_ambiguityFlag = false;
  };
  
}

CLASS_DEF( GlobalSim::GenericTOBArray , 186602265 , 1 )


#endif
