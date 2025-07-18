/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODAuxillaryMeasurement/versions/AuxillaryMeasurement_v1.h"
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "ActsGeoUtils/SurfaceEncoding.h"

namespace {
   using SurfLink_t = xAOD::AuxillaryMeasurement_v1::SurfLink_t;
   using ProjectorType = xAOD::AuxillaryMeasurement_v1::ProjectorType;
   static const SG::Accessor<SurfLink_t> acc_surfLink{"surfaceLink"};
}

namespace xAOD{
   AUXSTORE_PRIMITIVE_GETTER_WITH_CAST(AuxillaryMeasurement_v1, char, ProjectorType, calibProjector)
   AUXSTORE_PRIMITIVE_SETTER_WITH_CAST(AuxillaryMeasurement_v1, char, ProjectorType, calibProjector, setProjector)

   unsigned AuxillaryMeasurement_v1::numDimensions() const{
      switch (calibProjector()){
         using enum ProjectorType;
         case e1DimNoTime:
         case e1DimRotNoTime:
            return 1;
         case e2DimNoTime:
         case e1DimWithTime:
         case e1DimRotWithTime:
            return 2;
         case e2DimWithTime:
            return 3;
      };
      return 0;
   }

   const SurfLink_t& AuxillaryMeasurement_v1::surfaceLink() const {
      return acc_surfLink(*this);
   }
   void AuxillaryMeasurement_v1::setSurface(const SurfacePtr_t& surfPtr,
                                            SurfLink_t&& link) {
      m_surface.reset();
      acc_surfLink(*this) = std::move(link);
      m_surface.set(surfPtr);
   }
   const AuxillaryMeasurement_v1::SurfacePtr_t& 
      AuxillaryMeasurement_v1::surface() const {
      if (!m_surface.isValid()) {
         SurfLink_t assocLink = surfaceLink();
         assert(assocLink.isValid());
         /// Surfaces associated with pseudo measurements are never alignable
         m_surface.set(ActsTrk::decodeSurface(*assocLink));
      }
      return *m_surface.ptr();
   }

}