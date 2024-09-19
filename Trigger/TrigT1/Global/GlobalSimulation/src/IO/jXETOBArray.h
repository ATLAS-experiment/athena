// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_jXETOBARRAY_H
#define GLOBALSIM_jXETOBARRAY_H

#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/DataArrayImpl.h"
#include "L1TopoEvent/jXETOB.h"

#include "AthenaKernel/CLASS_DEF.h"

#include <vector>

namespace GlobalSim {
   
  class jXETOBArray : public TCS::InputTOBArray,
		      public TCS::DataArrayImpl<TCS::jXETOB> {
   public:
      
      // default constructor
      jXETOBArray(const std::string & name, unsigned int reserve);

    virtual unsigned int size() const {
      return TCS::DataArrayImpl<TCS::jXETOB>::size();
    }
      
   private:
      virtual void print(std::ostream&) const;
   };   
}

CLASS_DEF( GlobalSim::jXETOBArray , 196929881 , 1 )


#endif
