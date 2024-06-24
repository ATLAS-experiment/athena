/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GLOBALSIM_JJETTOBARRAY_H
#define GLOBALSIM_JJETTOBARRAY_H

#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/DataArrayImpl.h"
#include "L1TopoEvent/jJetTOB.h"

#include "AthenaKernel/CLASS_DEF.h"


namespace GlobalSim {

   class TOBArray;

  class jJetTOBArray : public TCS::InputTOBArray,
		       public TCS::DataArrayImpl<TCS::jJetTOB> {
   public:
    // default constructor
      jJetTOBArray(const std::string & name, unsigned int reserve);

    virtual unsigned int size() const {
      return DataArrayImpl<TCS::jJetTOB>::size();
    }

   private:      
      virtual void print(std::ostream&) const;
   };
   
}

CLASS_DEF( GlobalSim::jJetTOBArray , 17680720 , 1 )

#endif
