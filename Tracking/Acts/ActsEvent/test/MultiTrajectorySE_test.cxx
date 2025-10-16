/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#undef NDEBUG
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MultiTrajectorySE_test

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include "Acts/EventData/MeasurementHelpers.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/SourceLink.hpp"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include "ActsEvent/MultiTrajectory.h"
#include "CommonHelpers/GenerateParameters.hpp"
#include "CommonHelpers/TestSourceLink.hpp"
#include "xAODTracking/TrackJacobianAuxContainer.h"
#include "xAODTracking/TrackMeasurementAuxContainer.h"
#include "xAODTracking/TrackParametersAuxContainer.h"
#include "xAODTracking/TrackStateAuxContainer.h"

#include "xAODTracking/TrackSurface.h"
#include "xAODTracking/TrackSurfaceContainer.h"
#include "xAODTracking/TrackSurfaceAuxContainer.h"

#include "ActsGeoUtils/SurfaceEncoding.h"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/ConeSurface.hpp"
#include "Acts/Surfaces/CylinderSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/DiscSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/StrawSurface.hpp"



BOOST_AUTO_TEST_SUITE(EventDataMultiTrajectorySE)


template<typename surfType>
void testSurface(surfType surf, std::shared_ptr<const Acts::Surface> outSurf, const Acts::GeometryContext& gctx) {
    BOOST_CHECK_EQUAL(int(surf->type()), int(outSurf->type()));
    BOOST_CHECK_EQUAL(surf->center(gctx), outSurf->center(gctx));
    BOOST_CHECK_EQUAL(surf->transform(gctx).rotation().eulerAngles(2, 1, 0), 
                  outSurf->transform(gctx).rotation().eulerAngles(2, 1, 0));  
    BOOST_CHECK_EQUAL(size(surf->bounds().values()), size(outSurf->bounds().values()));
    for (unsigned int i=0; i<size(surf->bounds().values()); i++)  {  
      BOOST_TEST(surf->bounds().values()[i] == outSurf->bounds().values()[i], boost::test_tools::tolerance(0.001));
    }      
}  

// Surface encoding/decoding test
BOOST_AUTO_TEST_CASE(InsertRefSurface) {

  const Acts::GeometryContext gctx{};

  float layerZ = 30.;
  Acts::Transform3 transform(Acts::Translation3(0., 0., -layerZ));
  float rotation[3] = {2.1, 1.2, 0.4};
  transform *= Acts::AngleAxis3(rotation[0], Acts::Vector3(0., 0., 1.));  //rotZ
  transform *= Acts::AngleAxis3(rotation[1], Acts::Vector3(0., 1., 0.));  //rotY
  transform *= Acts::AngleAxis3(rotation[2], Acts::Vector3(1., 0., 0.));  //rotX


  xAOD::TrackSurfaceContainer backend;
  xAOD::TrackSurfaceAuxContainer aux;
  backend.setStore(&aux);
  
  auto surfBackend = new xAOD::TrackSurface();
  backend.push_back(surfBackend);

  for (int i=0; i<7; i++){

    switch (i)
    {
      case 0:  // ConeSurface
        {
        double alpha(M_PI/4.), minZ(5.), maxZ(25), halfPhi(M_PI);
        auto surf = Acts::Surface::makeShared<Acts::ConeSurface>(
              transform, alpha, minZ, maxZ, halfPhi);                   
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }     

      case 1:  // CylinderSurface
        {
        double layerR(20.) , layerHalfZ(30.);   
        auto surf = Acts::Surface::makeShared<Acts::CylinderSurface>(
              transform, layerR, layerHalfZ);                 
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }

      case 2:  // DiscSurface
        {
        double rMin(1.0), rMax(5.0), halfPhiSector(M_PI / 8.);  
        auto surf = Acts::Surface::makeShared<Acts::DiscSurface>(
              transform, rMin, rMax, halfPhiSector);                    
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }  

      case 3:  // PerigeeSurface
        {
        auto surf = Acts::Surface::makeShared<Acts::PerigeeSurface>(
              transform);                    
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }  

      case 4:  // PlaneSurface
        {
        Acts::Vector2 min(0.,1.), max(5.,6.);
        auto rBounds = std::make_shared<const Acts::RectangleBounds>(min, max); 
        auto surf = Acts::Surface::makeShared<Acts::PlaneSurface>(
              transform, rBounds);   
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }  

      case 5:  // StrawSurface
        {
        double radius(10.), halfZ(20.);
        auto surf = Acts::Surface::makeShared<Acts::StrawSurface>(
              transform, radius, halfZ);   
        ActsTrk::encodeSurface(surfBackend, surf.get(), gctx);
        auto outSurf = ActsTrk::decodeSurface(surfBackend);
        testSurface(std::move(surf), std::move(outSurf), gctx);
        break;
        }  
             
      default:
        continue;
        break;
    }  
  }
}

BOOST_AUTO_TEST_SUITE_END()