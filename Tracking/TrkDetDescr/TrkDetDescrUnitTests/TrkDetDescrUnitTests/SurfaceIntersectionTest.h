/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// IntersectionTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUNITTESTS_SURFACEINTERSECTIONTEST_H
#define TRKDETDESCRUNITTESTS_SURFACEINTERSECTIONTEST_H

// Athena & Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
// Trk includes
#include "TrkDetDescrUnitTests/TrkDetDescrUnitTestBase.h"
#include "TrkDetDescrInterfaces/ISurfaceBuilder.h"


namespace Trk {
             
    /** @class IntersectionTest
       
        Test calling the internal surface intersection methods 
        
        @author Andreas.Salzburger@cern.ch       
      */
      
    class SurfaceIntersectionTest : public TrkDetDescrUnitTestBase  {
     public:

       /** Standard Athena-Algorithm Constructor */
       using TrkDetDescrUnitTestBase::TrkDetDescrUnitTestBase;
        
       /* finalize */
       StatusCode finalize();

       /* specify the test here */
       StatusCode runTest();
       
       /** initialize the test, i.e. retrieve the TrackingGeometry Svc */
       StatusCode initializeTest();
       
      private:
       
       ToolHandle<ISurfaceBuilder> m_surfaceBuilder{this, "SurfaceBuilder", ""};

       Gaudi::Property<size_t> m_testsPerSurface
	 {this, "NumberOfTestsPerSurface", 100};
       size_t                          m_eventCounter = 0;

       std::vector<size_t>             m_surfaces = std::vector<size_t>(6, 0);
       std::vector<size_t>             m_surfaceAttempts = std::vector<size_t>(6, 0);
       std::vector<size_t>             m_surfaceSuccessful = std::vector<size_t>(6, 0);
       std::vector<std::string>        m_surfaceNames = std::vector<std::string>(6, "");
       std::vector<bool>               m_surfaceNamesSet = std::vector<bool>(6, false);

       Gaudi::Property<bool> m_writeTTree{this, "WriteTTree", true};
       std::string m_treeFolder = "/val/";

   };
}

#endif
