/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_TRUTHGUIDEDPROTOTRACKCREATOR__H
#define ACTSTRACKRECONSTRUCTION_TRUTHGUIDEDPROTOTRACKCREATOR__H

#include "ActsToolInterfaces/IProtoTrackCreatorTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"


namespace ActsTrk {

  class TruthGuidedProtoTrackCreator :
    public extends<AthAlgTool, ActsTrk::IProtoTrackCreatorTool> {
  public:
    
    TruthGuidedProtoTrackCreator(const std::string& type, 
		const std::string& name,
		const IInterface* parent);
    virtual ~TruthGuidedProtoTrackCreator() = default;
    

    virtual StatusCode  initialize() override;

      /// @brief EF-style pattern recognition to create prototracks 
      /// @param ctx: Event context
      /// @param pixelContainer: pixel cluster 
      /// @param stripContainer: sct cluster 
      /// @param foundProtoTracks: vector to hold the found proto tracks - will be populated by the method.
      /// Method will not discard existing content 
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const override final; 
    protected:
    /// @brief creates a random, dummy set of parameters 
    /// Warning: This is not a real parameter estimate. Should only serve as a placeholder. Use with care
    /// @param truthParticle: Input truth particle to get the parameters from
    /// @return a set of puesdo-dummy params
    std::unique_ptr<Acts::BoundTrackParameters> makeDummyParams (const HepMC::ConstGenParticlePtr & truthParticle) const;

    /// @brief Truth track collection
    SG::ReadHandleKeyArray<PRD_MultiTruthCollection> m_prdMultiTruthCollectionNames{this,"PRD_MultiTruthCollections", {}, "PRD multi truth collection names this builder is working on"};


  };

} // namespace

#endif



