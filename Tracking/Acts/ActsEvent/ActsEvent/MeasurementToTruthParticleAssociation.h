/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#ifndef MEASUREMENTTOGENPARTICLEASSOCIATION_H
#define MEASUREMENTTOGENPARTICLEASSOCIATION_H
#include "boost/container/small_vector.hpp"
#include "xAODTruth/TruthParticle.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "AthLinks/DataLink.h"
#include <vector>

namespace ActsTrk
{
   constexpr unsigned int NTruthParticlesPerMeasurement = 5;  // chosen to make the small_vector fit into 164 bytes
                                                            // a tiny fraction of measurements will have more than
                                                            // 5 associated GenParticles
   using ParticleVector = boost::container::small_vector<const xAOD::TruthParticle *, NTruthParticlesPerMeasurement>;
   class MeasurementToTruthParticleAssociation : public std::vector<ParticleVector> {
   public:
      using std::vector<ParticleVector>::vector ;

      void setSourceContainer(DataLink<xAOD::UncalibratedMeasurementContainer> &&source) {
         m_sourceMeasurements = std::move(source);
      }
      const xAOD::UncalibratedMeasurementContainer *sourceContainer() const {
         return  m_sourceMeasurements.getDataPtr();
      }
      bool isCompatibleWith(const xAOD::UncalibratedMeasurementContainer *container) const {
         
         if (!container) return false;

         if (container == m_sourceMeasurements.getDataPtr()) return true;
         
         return false;
      }

      // convenience methods to convert "data-links" to derived measurement container
      template <class T_MeasurementContainer>
      void setSourceContainer(const T_MeasurementContainer &source, const EventContext &ctx) {
         m_sourceMeasurements = DataLink<xAOD::UncalibratedMeasurementContainer>(source, ctx );
      }
      template <class T_MeasurementContainer>
      void setSourceContainer(const DataLink<T_MeasurementContainer> &source) {
          m_sourceMeasurements = DataLink<xAOD::UncalibratedMeasurementContainer>(source.getDataPtr(), source.source() );
      }

   private:
      DataLink<xAOD::UncalibratedMeasurementContainer> m_sourceMeasurements;
   };

}

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::MeasurementToTruthParticleAssociation , 151157774 , 1 )

#endif
