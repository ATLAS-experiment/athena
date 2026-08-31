/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// TrkDetDescrTPCnvTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUNITTESTS_TRKDETDESCRTPCNVTEST_H
#define TRKDETDESCRUNITTESTS_TRKDETDESCRTPCNVTEST_H

// Trk
#include "TrkDetDescrUnitTests/TrkDetDescrUnitTestBase.h"
// Athena & Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

namespace Trk {
             
    /** @class TrkDetDescrTPCnvTest
       
        Test the new TP Converter for writing and reading
        
        @author Andreas.Salzburger@cern.ch       
      */
      
    class TrkDetDescrTPCnvTest : public TrkDetDescrUnitTestBase  {
     public:

       /** Standard Athena-Algorithm Constructor */
       using Trk::TrkDetDescrUnitTestBase::TrkDetDescrUnitTestBase;

       /* specify the test here */
       StatusCode runTest();
       
     private:
         Gaudi::Property<bool> m_writeMode{this, "WriteMode", true};
         
         Gaudi::Property<std::string> m_materialStepCollectionName
	   {this, "MaterialStepCollection", "RandomMaterialSteps"};
         Gaudi::Property<std::string> m_layerMaterialCollectionName
	   {this, "LayerMaterialMap", "RandomLayerMaterialMap"};
         Gaudi::Property<std::string> m_elementTableName
	   {this, "ElementTable", "RandomElementTable"};

   };
}

#endif
