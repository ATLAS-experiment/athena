/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
  */
#ifndef TRACKINGSURFACEHELPER_H
#define TRACKINGSURFACEHELPER_H

#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "Acts/Surfaces/Surface.hpp"

#include  <stdexcept>

/** Simple helper class which allows to access the tracking surface associated to a certain (Si-)measurement.
 */
class TrackingSurfaceHelper
{
public:
   static constexpr unsigned int s_NMeasTypes = 4;  // depends on definition in UncalibMeasType
   TrackingSurfaceHelper() {}
   TrackingSurfaceHelper(std::array<std::vector< const Acts::Surface *>, s_NMeasTypes>  &&acts_surfaces)
      : m_actsSurfaces(std::move(acts_surfaces))
   {}
   // return mutable reference to allow filling the vector by element.
   std::vector< const Acts::Surface *> &actsSurfaces(xAOD::UncalibMeasType type) {
      assert(static_cast<std::size_t>(type) < m_actsSurfaces.size());
      return m_actsSurfaces[static_cast<std::size_t>(type)];
   }
   void setSiDetectorElements(xAOD::UncalibMeasType type, const InDetDD::SiDetectorElementCollection *det_element_collection) {
      assert(static_cast<std::size_t>(type) < m_siDetectorElements.size() );
      m_siDetectorElements[static_cast<unsigned int>(type)] = det_element_collection;
   }
   const Acts::Surface &associatedActsSurface(const xAOD::UncalibratedMeasurement &measurement) const {
      assert(static_cast<std::size_t>(measurement.type()) < m_actsSurfaces.size()
             && measurement.identifierHash() < m_actsSurfaces[ static_cast<unsigned int>(measurement.type()) ].size() );
      const Acts::Surface *acts_surface = m_actsSurfaces[ static_cast<unsigned int>(measurement.type()) ].at( measurement.identifierHash() );
      assert( acts_surface);
      return *acts_surface;
   }
   const Trk::Surface &associatedSurface(const xAOD::UncalibratedMeasurement &measurement) const {
      // @TODO always do a runtime check ?
      assert(static_cast<std::size_t>(measurement.type()) < m_siDetectorElements.size()
             && m_siDetectorElements[ static_cast<unsigned int>(measurement.type()) ]);
      const InDetDD::SiDetectorElement *
         element = m_siDetectorElements[ static_cast<unsigned int>( measurement.type()) ]
                                   ->getDetectorElement(measurement.identifierHash());

      const Trk::Surface *surface = element ? &(element->surface()) : nullptr;
      if (!surface) throw std::runtime_error("No associated surface for xAOD::UncalibratedMeasurement");
      return *surface;
   }
private:
   std::array<const InDetDD::SiDetectorElementCollection *, s_NMeasTypes> m_siDetectorElements {};
   std::array<std::vector< const Acts::Surface *>, s_NMeasTypes> m_actsSurfaces;
};
#endif
