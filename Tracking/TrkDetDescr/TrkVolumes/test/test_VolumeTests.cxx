/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#include "TrkVolumes/TrapezoidVolumeBounds.h"
#include "TrkSurfaces/Surface.h"
#include "TrkVolumes/CylinderVolumeBounds.h"
#include "TrkVolumes/DoubleTrapezoidVolumeBounds.h"
#include "TrkVolumes/Volume.h"

#include "TrkVolumes/VolumeBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkSurfaces/EllipseBounds.h"
#include "TrkSurfaces/RotatedTrapezoidBounds.h"
#include "TrkSurfaces/TrapezoidBounds.h"
#include "TrkSurfaces/CylinderBounds.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/DiamondBounds.h"

#include <array>
#include <typeinfo>

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

constexpr double relative_tolerance = 1e-5;
constexpr double abs_dist_tolerance = 1e-3; //mm

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

   std::vector<std::unique_ptr<Trk::Surface>> bound_surfaces=bounds.decomposeToSurfaces(transform);
   BOOST_CHECK( bound_surfaces.size() == 6 );
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

   Amg::Vector3D center_of_gravity=Amg::Vector3D::Zero();
   for (const Amg::Vector3D &a_corner : corner) {
      center_of_gravity += a_corner;
   }
   center_of_gravity /= corner.size();

   BOOST_TEST_MESSAGE( "center-of-gravity "
                       << center_of_gravity[0] << " " << center_of_gravity[1] << " " << center_of_gravity[2]);
   unsigned int surf_i=0;
   for(const std::unique_ptr<Trk::Surface>& a_surface : bound_surfaces) {
      unsigned int counter=0;
      Amg::Vector3D surface_center=Amg::Vector3D::Zero();
      unsigned int corner_i=0;
      for (const Amg::Vector3D &a_corner : corner) {
         bool on_surface  = a_surface->isOnSurface(a_corner, true /* BoundaryCheck */, abs_dist_tolerance, abs_dist_tolerance);
         if (on_surface) {
            surface_center += a_corner;
            BOOST_TEST_MESSAGE( "On surface " << surf_i << " corner " << corner_i << " "
                                << a_corner[0] << " " << a_corner[1] << " " << a_corner[2]);
         }
         counter += on_surface;
         ++corner_i;
      }
      BOOST_CHECK( counter == 4 );
      if (counter>0) {
         surface_center /= counter;
      }
      BOOST_TEST_MESSAGE( "surface_center  " << surf_i << " "
                          << surface_center[0] << " " << surface_center[1] << " " << surface_center[2]);
      BOOST_TEST_MESSAGE( a_surface->normal().dot( surface_center - center_of_gravity ));
      BOOST_CHECK( a_surface->normal().dot( surface_center - center_of_gravity )>0.);
      ++surf_i;
   }
}

namespace {
   void createArcVertices(double phi_start, double  phi_end, double radius_x, double radius_y,  double offset_z,
                          unsigned int n, std::vector<Amg::Vector3D> &bound_out) {
      bound_out.reserve( bound_out.size() + n+1 );
      float phi_step = (phi_end - phi_start) / n;
      float phi=phi_start;
      for (unsigned int i=0; i<=n; ++i) {
         bound_out.emplace_back(radius_x * cos(phi),
                                              radius_y * sin(phi),
                                              offset_z);
         phi += phi_step;
      }
   }

   using AvVector = std::pair<Amg::Vector3D, unsigned int>;

   bool  checkVertices(const Trk::Volume &volume, const Trk::Surface &surface, const std::vector<Amg::Vector3D> &vertices,
                       AvVector *vertex_sum) {
      bool contained = true;
      for (const Amg::Vector3D &a_vertex : vertices) {
         Amg::Vector3D pos = surface.transform() * a_vertex;
         if (vertex_sum) {
            vertex_sum->first += pos;
            ++vertex_sum->second;
         }
         bool inside = volume.inside(pos,1e-3);
         contained &= inside;
      }
      return contained;
   }

   template <class T_BoundType>
   bool checkBounds(const Trk::Volume &volume, const Trk::Surface &surface, const T_BoundType& bounds, AvVector *vertex_sum);

   template <>
   bool checkBounds<Trk::RectangleBounds>(const Trk::Volume &volume, const Trk::Surface &surface,
                                          const Trk::RectangleBounds& bounds, AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      vertices.reserve(4);
      double delta_x = std::min(bounds.halflengthX()*relative_tolerance,abs_dist_tolerance);
      double delta_y = std::min(bounds.halflengthY()*relative_tolerance,abs_dist_tolerance);
      vertices.emplace_back(-bounds.halflengthX()+delta_x, -bounds.halflengthY()+delta_y,0. );
      vertices.emplace_back(-bounds.halflengthX()+delta_x,  bounds.halflengthY()-delta_y,0. );
      vertices.emplace_back( bounds.halflengthX()-delta_x,  bounds.halflengthY()-delta_y,0. );
      vertices.emplace_back( bounds.halflengthX()-delta_x, -bounds.halflengthY()+delta_y,0. );
      return checkVertices(volume, surface, vertices, vertex_sum );
   }
   template <>
   bool checkBounds<Trk::TrapezoidBounds>(const Trk::Volume &volume,
                                          const Trk::Surface &surface,
                                          const Trk::TrapezoidBounds& bounds,
                                          AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      vertices.reserve(4);
      double delta_minx = std::min(bounds.minHalflengthX()*relative_tolerance,abs_dist_tolerance);
      double delta_maxx = std::min(bounds.maxHalflengthX()*relative_tolerance,abs_dist_tolerance);
      double delta_y = std::min(bounds.halflengthY()*relative_tolerance,abs_dist_tolerance);
      vertices.emplace_back(-bounds.minHalflengthX()+delta_minx, -bounds.halflengthY()+delta_y,0. );
      vertices.emplace_back(-bounds.maxHalflengthX()+delta_maxx,  bounds.halflengthY()-delta_y,0. );
      vertices.emplace_back( bounds.maxHalflengthX()-delta_maxx,  bounds.halflengthY()-delta_y,0. );
      vertices.emplace_back( bounds.minHalflengthX()-delta_minx, -bounds.halflengthY()+delta_y,0. );

      return checkVertices(volume, surface, vertices, vertex_sum);

   }
   template <>
   bool checkBounds<Trk::RotatedTrapezoidBounds>(const Trk::Volume &volume,
                                                 const Trk::Surface &surface,
                                                 const Trk::RotatedTrapezoidBounds& bounds,
                                                 AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      double delta_x = std::min(bounds.halflengthX()*relative_tolerance,abs_dist_tolerance);
      double delta_miny = std::min(bounds.minHalflengthY()*relative_tolerance,abs_dist_tolerance);
      double delta_maxy = std::min(bounds.maxHalflengthY()*relative_tolerance,abs_dist_tolerance);
      vertices.reserve(4);
      vertices.emplace_back(-bounds.halflengthX()+delta_x, -bounds.minHalflengthY()+delta_miny,0. );
      vertices.emplace_back(-bounds.halflengthX()+delta_x,  bounds.minHalflengthY()-delta_miny,0. );
      vertices.emplace_back( bounds.halflengthX()-delta_x,  bounds.maxHalflengthY()-delta_maxy,0. );
      vertices.emplace_back( bounds.halflengthX()-delta_x, -bounds.maxHalflengthY()+delta_maxy,0. );

      return checkVertices(volume, surface, vertices,vertex_sum);

   }
   template <>
   bool checkBounds<Trk::DiamondBounds>(const Trk::Volume &volume,
                                        const Trk::Surface &surface,
                                        const Trk::DiamondBounds& bounds,
                                        AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      vertices.reserve(6);
      double delta_minx = std::min(bounds.minHalflengthX()*1e-5,1e-3);
      double delta_maxx = std::min(bounds.maxHalflengthX()*1e-5,1e-3);
      double delta_y = std::min(std::min(bounds.halflengthY1(),bounds.halflengthY2())*1e-5,1e-3);
      vertices.emplace_back(-bounds.minHalflengthX()+delta_minx, -2*bounds.halflengthY1()+delta_y,0. );
      vertices.emplace_back(-bounds.medHalflengthX()+delta_maxx,  0.,0. );
      vertices.emplace_back(-bounds.maxHalflengthX()+delta_maxx,  2*bounds.halflengthY2()-delta_y,0. );
      vertices.emplace_back( bounds.maxHalflengthX()-delta_maxx,  2*bounds.halflengthY2()-delta_y,0. );
      vertices.emplace_back( bounds.medHalflengthX()-delta_maxx,  0.,0. );
      vertices.emplace_back( bounds.minHalflengthX()-delta_minx, -2*bounds.halflengthY1()+delta_y,0. );

      return checkVertices(volume, surface, vertices,vertex_sum);
    }

   template <>
   bool checkBounds<Trk::DiscBounds>(const Trk::Volume &volume, const Trk::Surface &surface, const Trk::DiscBounds& bounds,
                                     AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      unsigned int outer_n = 6;
      unsigned int inner_n = 6;
      vertices.reserve( outer_n + inner_n);
      double phi_start = bounds.averagePhi() - std::abs(bounds.halfPhiSector()) + std::abs(bounds.halfPhiSector()) *abs_dist_tolerance;
      double phi_end = bounds.averagePhi() + std::abs(bounds.halfPhiSector()) - std::abs(bounds.halfPhiSector()) *abs_dist_tolerance;
      double delta_r = std::min((bounds.rMax()-bounds.rMin()) * relative_tolerance, abs_dist_tolerance);
      createArcVertices(phi_start, phi_end, bounds.rMin()+delta_r, bounds.rMin()+delta_r, 0., inner_n, vertices);
      createArcVertices(phi_start, phi_end, bounds.rMax()-delta_r, bounds.rMax()-delta_r, 0., outer_n, vertices);
      return checkVertices(volume, surface, vertices,vertex_sum );
   }
   template <>
   bool checkBounds<Trk::CylinderBounds>(const Trk::Volume &volume, const Trk::Surface &surface, const Trk::CylinderBounds& bounds,
                                         AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      unsigned int outer_n = 6;
      unsigned int inner_n = 6;
      vertices.clear();
      vertices.reserve( outer_n + inner_n);

      double phi_start = bounds.averagePhi() - std::abs(bounds.halfPhiSector()) + std::abs(bounds.halfPhiSector()) *abs_dist_tolerance;
      double phi_end = bounds.averagePhi() + std::abs(bounds.halfPhiSector()) - std::abs(bounds.halfPhiSector()) *abs_dist_tolerance;
      double delta_r = 0.;
      double delta_z = std::min( bounds.halflengthZ()* relative_tolerance, abs_dist_tolerance);
      createArcVertices(phi_start, phi_end, bounds.r()-delta_r, bounds.r()-delta_r,  bounds.halflengthZ()-delta_z, outer_n,vertices);
      createArcVertices(phi_start, phi_end, bounds.r()-delta_r, bounds.r()-delta_r, -bounds.halflengthZ()+delta_z, outer_n,vertices);
      return checkVertices(volume, surface, vertices, vertex_sum );
   }
   template <>
   bool checkBounds<Trk::EllipseBounds>(const Trk::Volume &volume,
                                        const Trk::Surface &surface,
                                        const Trk::EllipseBounds& bounds,
                                        AvVector *vertex_sum) {
      std::vector<Amg::Vector3D>  vertices;
      unsigned int outer_n = 6;
      unsigned int inner_n = 6;
      vertices.reserve( outer_n + inner_n);
      double phi_start = bounds.averagePhi() - bounds.halfPhiSector() + bounds.halfPhiSector() *relative_tolerance;
      double phi_end = bounds.averagePhi() + bounds.halfPhiSector() - bounds.halfPhiSector() *relative_tolerance;
      double delta_rx = 0.;
      double delta_ry = 0.;

      createArcVertices(phi_start, phi_end, bounds.rMinX()+delta_rx, bounds.rMinY()+delta_ry, 0., inner_n,  vertices);
      createArcVertices(phi_start, phi_end, bounds.rMaxX()-delta_rx, bounds.rMaxY()-delta_ry, 0., outer_n,  vertices);
      return checkVertices(volume, surface, vertices, vertex_sum);
   }

   template <class T_BoundType>
   std::pair<bool,bool> checkSpecificBounds(const Trk::Volume &volume, const Trk::Surface &surface, const Trk::SurfaceBounds& bounds,
                                            AvVector *vertex_sum) {
      const T_BoundType *specific_bounds = dynamic_cast<const T_BoundType *>(&bounds);
      if (specific_bounds) {
         return std::make_pair(true, checkBounds<T_BoundType>(volume, surface, *specific_bounds, vertex_sum));
      }
      else {
         return std::make_pair(false,true);
      }
   }

   bool checkSurface(const Trk::Volume *volume, const Trk::Surface &surface, AvVector *vertex_sum) {
      if (!volume) return true;
      const Trk::SurfaceBounds& bounds = surface.bounds();

      if (auto [used, contained] = checkSpecificBounds<Trk::RectangleBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::DiscBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::CylinderBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::RotatedTrapezoidBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::TrapezoidBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::DiamondBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }
      if (auto [used, contained] = checkSpecificBounds<Trk::EllipseBounds>(*volume,surface,bounds, vertex_sum); used) { return contained; }

      return false;
   }
   bool checkSurface(const Trk::Volume *volume, const Trk::Surface *surface, AvVector *vertex_sum) {
      if (!surface) return true;
      return checkSurface(volume, *surface, vertex_sum);
   }

   void dumpSurface(const Trk::Surface *surface, std::ostream *out_ptr=nullptr, const Trk::Volume *volume=nullptr) {
      if (surface) {
         std::stringstream out;
         if (!out_ptr) {
            out_ptr = &out;
         }
         (*out_ptr)
            << "--begin: Trk::Surface " << typeid(*surface).name()
            << (volume ? (checkSurface(volume, *surface, nullptr) ? " contained" :  " NOT-CONTAINED") : "")
            << std::endl
            << *surface << std::endl
            << "--end: Trk::Surface" << std::endl;

         if (out_ptr == &out) {
            std::cout << out.str() << std::flush;
         }
      }
   }

}

template <typename T>
void   test_volumeBounds(const T &volume_bounds, const Amg::RotationMatrix3D &rotation, const Amg::Vector3D &center) {
   Amg::Transform3D translation(center);
   Amg::Transform3D transform( translation * Amg::Transform3D(rotation));

   Trk::Volume volume(std::make_unique<Amg::Transform3D>(transform),
                      std::make_shared<T>(volume_bounds));

   std::vector<std::unique_ptr<Trk::Surface>> surfaces =
      volume.volumeBounds().decomposeToSurfaces(volume.transform());

   AvVector vertex_sum=std::make_pair(Amg::Vector3D::Zero(), 0u);
   std::vector<std::pair<Amg::Vector3D, unsigned int> > surface_vertex_sum;
   surface_vertex_sum.resize(surfaces.size());
   std::fill(surface_vertex_sum.begin(),surface_vertex_sum.end(),vertex_sum);

   bool all_passed=true;
   // test that surfaces are contained in volume
   unsigned int surf_i=0;
   --surf_i;
   for (const std::unique_ptr<Trk::Surface>& a_surface : surfaces) {
      ++surf_i;
      bool passed = checkSurface(&volume, a_surface.get(), &surface_vertex_sum.at(surf_i));
      vertex_sum.first += surface_vertex_sum.at(surf_i).first;
      vertex_sum.second += surface_vertex_sum.at(surf_i).second;

      all_passed &= passed;
      BOOST_CHECK( passed  );
   }

      Amg::Vector3D center_of_gravity = Amg::Vector3D::Zero();
   if (vertex_sum.second>0)  {
      center_of_gravity = vertex_sum.first / vertex_sum.second;
      BOOST_TEST_MESSAGE( "center_of_gravity "
                          << center_of_gravity[0] << " " << center_of_gravity[1] << " " << center_of_gravity[2]);
   }

   // test that normals point outwards
   surf_i=0u;
   --surf_i;
   for (const std::unique_ptr<Trk::Surface>& a_surface : surfaces) {
      ++surf_i;
      if (!a_surface
          || dynamic_cast<const Trk::CylinderSurface *>(a_surface.get())
          || surface_vertex_sum.at(surf_i).second==0u) continue;

      Amg::Vector3D surface_center = surface_vertex_sum.at(surf_i).first/surface_vertex_sum.at(surf_i).second;
      BOOST_TEST_MESSAGE( "surface_center  " << surf_i << " "
                          << surface_center[0] << " " << surface_center[1] << " " << surface_center[2]);
      BOOST_TEST_MESSAGE( a_surface->transform().linear().col(2).dot( surface_center - center_of_gravity ));
      bool normal_direction_outwards = a_surface->transform().linear().col(2).dot( surface_center - center_of_gravity )>0.;
      BOOST_CHECK( normal_direction_outwards);
      all_passed &= normal_direction_outwards;
   }

   if (!all_passed) {
      std::cout << volume << std::endl;
      for (const std::unique_ptr<Trk::Surface>& a_surface : surfaces) {
         dumpSurface(a_surface.get(), nullptr, &volume);
      }
   }
}
void   test_CylinderVolumeBounds(Trk::CylinderVolumeBounds &cylinder_volume_bounds, const Amg::RotationMatrix3D &rotation, const Amg::Vector3D &center) {
   test_volumeBounds(cylinder_volume_bounds, rotation, center);
}

void   test_CylinderVolumeBounds() {
   {
      Trk::CylinderVolumeBounds cylinder_volume_bounds(0.00, 1500.00, 1.57, 19.00);
      Amg::RotationMatrix3D rotation;
      rotation.col(0) = Amg::Vector3D(-1.83697e-16, -1, 0);
      rotation.col(1) = Amg::Vector3D(-1, -1.83697e-16, 0);
      rotation.col(2) = Amg::Vector3D(0, 0, 1);
      Amg::Vector3D center(0, 0, -12900.2);
      test_CylinderVolumeBounds(cylinder_volume_bounds, rotation, center);
   }

   {
      Trk::CylinderVolumeBounds cylinder_volume_bounds(1285.00, 1480.00, M_PI , 522.50);
      Amg::RotationMatrix3D rotation;
      rotation.col(0) = Amg::Vector3D(1, 0, 0);
      rotation.col(1) = Amg::Vector3D(0, -1, 0);
      rotation.col(2) = Amg::Vector3D(0, 0, -1);
      Amg::Vector3D center(1.05424e-12, 0, -21522.5);
      test_CylinderVolumeBounds(cylinder_volume_bounds, rotation, center);
   }

   {
      Trk::CylinderVolumeBounds cylinder_volume_bounds(70.00, 279.00, 3.14, 2073.00);
      Amg::Vector3D center(0, 0, -23973);
      Amg::RotationMatrix3D rotation;
      rotation.col(0) = Amg::Vector3D(1, 0, 0);
      rotation.col(1) = Amg::Vector3D(0, 1, 0);
      rotation.col(2) = Amg::Vector3D(0, 0, 1);
      test_CylinderVolumeBounds(cylinder_volume_bounds, rotation, center);
   }

   }

void  test_TrapezoidVolumeBounds2() {
   Trk::TrapezoidVolumeBounds trapezoid_bounds(1596.7500305, 728.3492804, 48.5280521, 1.7195152, 1.7195152);
   Amg::Vector3D center(-3872.24, -9350.77, -21713);
   Amg::RotationMatrix3D rotation;
   rotation.col(0) = Amg::Vector3D(0.923608, -0.383337, -0.000407933);
   rotation.col(1) = Amg::Vector3D(-0.383337, -0.923608, -0.00131771);
   rotation.col(2) = Amg::Vector3D(0.000128358, 0.00137342, -0.999999);
   test_volumeBounds(trapezoid_bounds, rotation, center);
}

void test_DoubleTrapezoidVolumeBounds() {
   {
      Trk::DoubleTrapezoidVolumeBounds
         double_trapezoid_volume_bounds(40.0000000, 40.0000000, 40.0000000, 786.1100000, 7.8900000, 637.4000000);

      Amg::Vector3D  center(-2769.45, -8390.84, 12080);
      Amg::RotationMatrix3D rotation;
      rotation.col(0) = Amg::Vector3D(1, 0, 0);
      rotation.col(1) = Amg::Vector3D(0, 1, 0);
      rotation.col(2) = Amg::Vector3D(0, 0, 1);
      test_volumeBounds(double_trapezoid_volume_bounds, rotation, center);
   }
   {
      Trk::DoubleTrapezoidVolumeBounds
         double_trapezoid_volume_bounds(906.8400000, 1055.5000000, 1055.5000000, 296.3000000, 280.2000000, 24.6700000);
      Amg::Vector3D  center(2878.35, 2878.35, 7474);
      Amg::RotationMatrix3D rotation;
      rotation.col(0) = Amg::Vector3D(0.707107, -0.707107, 0);
      rotation.col(1) = Amg::Vector3D(0.707107, 0.707107, 0);
      rotation.col(2) = Amg::Vector3D(0, 0, 1);
      test_volumeBounds(double_trapezoid_volume_bounds, rotation, center);
   }
}

BOOST_AUTO_TEST_CASE(VolumeTests, *boost::unit_test::tolerance(1e-10)) {
   test_TrapezoidVolumeBounds();
   test_TrapezoidVolumeBounds2();
}

BOOST_AUTO_TEST_CASE(CylinderVolumeBounds, *boost::unit_test::tolerance(1e-10)) {
   test_CylinderVolumeBounds();
}

BOOST_AUTO_TEST_CASE(DoubleTrapezoidVolumeBounds, *boost::unit_test::tolerance(1e-10)) {
   test_DoubleTrapezoidVolumeBounds();
}
