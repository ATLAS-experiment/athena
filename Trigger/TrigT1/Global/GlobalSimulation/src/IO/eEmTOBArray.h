// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_EEMTOBARRAY_H
#define GLOBALSIM_EEMTOBARRAY_H

#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/DataArrayImpl.h"
#include "L1TopoEvent/eEmTOB.h"

#include "AthenaKernel/CLASS_DEF.h"

namespace TCS {
  class InputTOBArray;
  class eEmTOB;
}

namespace GlobalSim {
   
  class eEmTOBArray: public TCS::InputTOBArray,
		     public TCS::DataArrayImpl<TCS::eEmTOB> {
   public:
      
      eEmTOBArray(const std::string & name, unsigned int reserve);

    virtual unsigned int size() const {
      return TCS::DataArrayImpl<TCS::eEmTOB>::size();
    }

   private:
      virtual void print(std::ostream&) const;
   };
}

CLASS_DEF( GlobalSim::eEmTOBArray , 13700225 , 1 )

#endif
