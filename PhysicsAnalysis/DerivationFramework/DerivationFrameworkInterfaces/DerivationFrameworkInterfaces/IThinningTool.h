/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ISkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_INTERFACES_ITHINNINGTOOL_H
#define DERIVATIONFRAMEWORK_INTERFACES_ITHINNINGTOOL_H 

// Gaudi
#include "GaudiKernel/IAlgTool.h"

namespace DerivationFramework {

  /**
   @class IThinningTool
       
   @author James.Catmore-at-cern.ch
   */
     
  class IThinningTool : virtual public extend_interfaces<IAlgTool> {
     public:
       DeclareInterfaceID(IThinningTool, 1, 0);

       /** Virtual destructor */
       virtual ~IThinningTool(){}

       /** Pass the thinning service  */
       virtual StatusCode doThinning() const = 0;  	
  };

} // end of namespace

#endif 
