// Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_CTAUTOBARRAY_H
#define GLOBALSIM_CTAUTOBARRAY_H

#include <ostream>
#include "L1TopoEvent/InputTOBArray.h"
#include "L1TopoEvent/DataArrayImpl.h"
#include "L1TopoEvent/cTauTOB.h"

#include "AthenaKernel/CLASS_DEF.h"


namespace GlobalSim {
   
  class cTauTOBArray : public TCS::InputTOBArray,
		       public TCS::DataArrayImpl<TCS::cTauTOB> {
  public:
      
      cTauTOBArray(const std::string & name, unsigned int reserve);

      virtual unsigned int size() const {
	return DataArrayImpl<TCS::cTauTOB>::size();
      }

   private:
      virtual void print(std::ostream&) const;
   };
}

CLASS_DEF( GlobalSim::cTauTOBArray , 239876180 , 1 )

#endif
