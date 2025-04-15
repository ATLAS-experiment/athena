/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// ISkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_INTERFACES_IAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_INTERFACES_IAUGMENTATIONTOOL_H 

// Gaudi
#include "GaudiKernel/IAlgTool.h"

namespace DerivationFramework {

  /**
   @class IAugmentationTool
       
   @author James.Catmore-at-cern.ch
   */
     
  class IAugmentationTool : virtual public extend_interfaces<IAlgTool> {
     public:
       DeclareInterfaceID(IAugmentationTool, 1, 0);

       /** Virtual destructor */
       virtual ~IAugmentationTool(){}

       /** Pass the thinning service  */
       virtual StatusCode addBranches() const = 0;  	
  };

} // end of namespace

#endif 
