/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ISkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_INTERFACES_ISKIMMINGTOOL_H
#define DERIVATIONFRAMEWORK_INTERFACES_ISKIMMINGTOOL_H 

// Gaudi
#include "GaudiKernel/IAlgTool.h"

namespace DerivationFramework {

  /**
   @class ISkimmingTool
       
   @author James.Catmore-at-cern.ch
    (based on ISF equivalent, J. Chapman)
   */
     
  class ISkimmingTool : virtual public extend_interfaces<IAlgTool> {
     public:
       DeclareInterfaceID(ISkimmingTool, 1, 0);

       /** Virtual destructor */
       virtual ~ISkimmingTool(){}

       /** Check that the current event passes this filter */
       virtual bool eventPassesFilter() const = 0;
  };

} // end of namespace

#endif 
