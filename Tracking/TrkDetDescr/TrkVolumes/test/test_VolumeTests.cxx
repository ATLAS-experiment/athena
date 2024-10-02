/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#include "TrkVolumes/TrapezoidVolumeBounds.h"
#include "TrkSurfaces/Surface.h"

#include <array>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>


void   test_TrapezoidVolumeBounds() {
   Trk::TrapezoidVolumeBounds bounds(271.3050000, // minhalex,
                                     666.0000000, //haley,
                                     24.6700000,  //halez,
                                     1.8151425,   //alpha
                                     1.8151425);  //beta);
   Amg::RotationMatrix3D rotation_matrix = Amg::RotationMatrix3D::Identity();
   rotation_matrix.col(0) = Amg::Vector3D( 0.707107, -0.707107, 0.);
   rotation_matrix.col(1) = Amg::Vector3D(-0.707107, -0.707107, 0.);
   rotation_matrix.col(2) = Amg::Vector3D( 0.,        0.,      -1.);
   Amg::Vector3D center(-1127.84, -1127.84, -7474);
   //Amg::Vector3D center = Amg::Vector3D::Zero();
   Amg::Transform3D translation(center);
   Amg::Transform3D rotation( rotation_matrix);
   Amg::Transform3D transform = translation * rotation;

   const std::vector<const Trk::Surface*>*
      bound_surfaces=bounds.decomposeToSurfaces(transform);
   BOOST_CHECK( bound_surfaces != nullptr);
   BOOST_CHECK( bound_surfaces->size() == 6 );
   // reference
   double gamma = (bounds.alpha() - 0.5 * M_PI);
   double hy = bounds.halflengthY();
   double hz = bounds.halflengthZ();

   std::array<float,2> hx {static_cast<float>(bounds.minHalflengthX()),
                           static_cast<float>(bounds.minHalflengthX() + (2. * hy) * std::tan(gamma)) };
   std::array<float,2> scale {-1.f,1.f};
   std::array<Amg::Vector3D, 8> corner;
   for (unsigned int corner_i=0; corner_i<8; ++corner_i) {
      corner.at(corner_i) = transform * Amg::Vector3D{ scale[(corner_i & (1<<0)) != 0] * hx[(corner_i & (1<<1))!=0 ],
                                                       scale[(corner_i & (1<<1)) != 0] * hy,
                                                       scale[(corner_i & (1<<2)) != 0] * hz};
   }

   for (const Amg::Vector3D &a_corner : corner) {
      BOOST_TEST_MESSAGE( "Corner: " << a_corner[0] << " " << a_corner[1] << " " << a_corner[2] );
      unsigned int counter=0;
      unsigned int surf_i=0;
      for(const Trk::Surface* a_surface : *bound_surfaces) {
         bool on_surface  = a_surface->isOnSurface(a_corner, true /* BoundaryCheck */, 1e-3, 1e-3);
         if (on_surface) {
            BOOST_TEST_MESSAGE( "On surface " << surf_i );
         }
         counter += on_surface;
         ++surf_i;
      }
      BOOST_CHECK( counter == 3 );
   }
}

BOOST_AUTO_TEST_CASE(VolumeTests, *boost::unit_test::tolerance(1e-10)) {
   test_TrapezoidVolumeBounds();
}
