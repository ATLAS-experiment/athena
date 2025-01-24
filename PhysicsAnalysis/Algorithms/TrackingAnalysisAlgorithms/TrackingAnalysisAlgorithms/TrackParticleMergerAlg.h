// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef TRACKINGANALYSISALGORITHMS_TRACKPARTICLEMERGERALG_H
#define TRACKINGANALYSISALGORITHMS_TRACKPARTICLEMERGERALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/TrackParticleAuxContainer.h>

#include <AsgTools/PropertyWrapper.h>
#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/ReadHandleKeyArray.h>
#include <AsgDataHandles/ReadDecorHandleKeyArray.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteHandle.h>
#include <AsgDataHandles/ReadHandle.h>

#include <string>

namespace CP {

   /// Algorithm to merge multiple track collections into one
   ///
   /// This algorithm is most commonly used to merge the
   /// InDetTrackParticles and InDetLargeD0TrackParticles
   /// containers. The "CreateViewCollection" will do so
   /// without actually copying the TrackParticle objects.
   ///
   /// @author Jackson Burzynski <jackson.carl.burzynski@cern.ch>
   /// @author Bingxuan Liu <bingxuan.liu@cern.ch>
   ///
   class TrackParticleMergerAlg final : public EL::AnaReentrantAlgorithm {

   public:
      /// Algorithm constructor
      TrackParticleMergerAlg( const std::string& name, ISvcLocator* svcLoc );

      /// @name Function(s) inherited from @c EL::AnaAlgorithm
      /// @{

      /// Function initialising the algorithm
      virtual StatusCode initialize() override final;

      /// Function executing the algorithm
      virtual StatusCode execute(const EventContext& ctx) const override final;

      /// @}

   private:
      /// @name Algorithm properties
      /// @{
      // Declare the algorithm's properties:

      /// Input track collections to be merged
      SG::ReadHandleKeyArray<xAOD::TrackParticleContainer> m_inputTrackParticleLocations {
          this, "InputTrackParticleLocations", {"InDetTrackParticles", "InDetLargeD0TrackParticles"},
           "Input track collections to be merged"
      };

      /// Output collection name
      SG::WriteHandleKey<xAOD::TrackParticleContainer>  m_outputTrackParticleLocationCopy {
          this, "OutputTrackParticleLocationCopy", "InDetWithLRTTrackParticles", "Output collection name"
      };
      SG::WriteHandleKey<ConstDataVector <xAOD::TrackParticleContainer> >  m_outputTrackParticleLocationView {
          this, "OutputTrackParticleLocation", "InDetWithLRTTrackParticles", "Output view collection name"
      };

      /// Option to create a view collection and not deep-copy tracks
      Gaudi::Property<bool>  m_createViewCollection{this, "CreateViewCollection", true};

      /// Extra guard for deep-copy mode
      SG::ReadDecorHandleKeyArray<xAOD::TrackParticleContainer> m_requiredDecorations{
        this, "RequiredDecorations", {}, "Decorations that the algorithm needs to wait for"};


      /// @}

   }; // class TrackParticleMergerAlg

} // namespace CP

#endif // TRACKINGANALYSISALGORITHMS_TRACKPARTICLEMERGERALG_H
