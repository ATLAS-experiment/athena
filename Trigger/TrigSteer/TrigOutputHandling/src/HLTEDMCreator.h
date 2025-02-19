/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGOUTPUTHANDLING_HLTEDMCREATOR_H
#define TRIGOUTPUTHANDLING_HLTEDMCREATOR_H 1

// STL includes
#include <string>
// FrameWork includes
#include "AthenaBaseComps/AthAlgTool.h"

// OutputHandling includes
#include "DecisionHandling/IHLTOutputTool.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"
#include "AthViews/View.h"

#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "xAODTrigger/TrigCompositeContainer.h"
#include "xAODTrigEgamma/TrigElectronContainer.h"
#include "xAODTrigEgamma/TrigPhotonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTrigCalo/TrigCaloClusterContainer.h"

#include "xAODTrigRinger/TrigRingerRingsContainer.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTrigMissingET/TrigMissingETContainer.h"

#include "xAODTrigMuon/L2StandAloneMuonContainer.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTrigMuon/L2IsoMuonContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODTrigBphys/TrigBphysContainer.h"
#include "xAODBTagging/BTaggingContainer.h"
#include "xAODBTagging/BTagVertexContainer.h"

#include "xAODCaloEvent/CaloClusterContainer.h"

#include "xAODTrigMinBias/TrigT2MbtsBitsContainer.h"

#include "xAODHIEvent/HIEventShapeContainer.h"

#include "xAODTrigRinger/TrigRNNOutputContainer.h"

#include "xAODForward/AFPSiHitsClusterContainer.h"
#include "xAODForward/AFPTrackContainer.h"
#include "xAODForward/AFPToFTrackContainer.h"
#include "xAODForward/AFPProtonContainer.h"
#include "xAODForward/AFPVertexContainer.h"

#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "xAODTrigger/jFexTauRoIContainer.h"
#include "xAODTrigger/jFexFwdElRoIContainer.h"
#include "xAODTrigger/jFexSRJetRoIContainer.h"
#include "xAODTrigger/jFexLRJetRoIContainer.h"
#include "xAODTrigger/jFexMETRoIContainer.h"
#include "xAODTrigger/jFexSumETRoIContainer.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "xAODTrigger/gFexGlobalRoIContainer.h"
#include "xAODTrigger/MuonRoIContainer.h"

/**
 * @class HLTEDMCreator
 * @brief Tool capable of creating collections missing (early rejection) after HLT processing.
 * It declares output handles for all possible collections produced by the trigger
 *
 * to add more classes follow these steps:
 * 1) add EDM pkg in CMakeLists
 * 2) add includes in this file
 * 3) add DEF_*_KEY here
 * 4) initialise it in initialize method (in cxx file)
 * 5) add create in the createOutput
 * 6) test 
 **/

class HLTEDMCreator: public extends<AthAlgTool, IHLTOutputTool>  { 
 public: 
  HLTEDMCreator( const std::string& type,
	     const std::string& name, 
	     const IInterface* parent );

  virtual ~HLTEDMCreator() override = default;
  virtual StatusCode createOutput(const EventContext& context) const override;
  virtual StatusCode initialize() override;


 private: 

  HLTEDMCreator();
  Gaudi::Property<std::vector<std::string>> m_fixLinks{ this, "FixLinks", {}, "Which keys of the TrigCompositeContainer WriteHandleKeyArray might need to have their (e.g. feature) element links re-mapped outside of views"};
  SG::WriteDecorHandleKeyArray<xAOD::TrigCompositeContainer, std::vector<SG::sgkey_t> > m_remapLinkColKeys{ this, "DoNotSet_RemapLinkColKeys", {}, "Do not set, it is configured accordingly to FixLinks & TC output property"};
  SG::WriteDecorHandleKeyArray<xAOD::TrigCompositeContainer, std::vector<xAOD::TrigComposite::index_type> > m_remapLinkColIndices{ this, "DoNotSet_RemapLinkColIndices", {}, "Do not set, it is configured accordingly to FixLinks & TC output property"};

  Gaudi::Property<bool> m_dumpSGBefore{ this, "dumpSGBefore", false, "Dump SG content before the merging"}; // for debugging 
  Gaudi::Property<bool> m_dumpSGAfter { this, "dumpSGAfter", false, "Dump SG content after the merging"};
  Gaudi::Property<bool> m_renounceOutputs { this, "RenounceOutputs", false, "Renounce outputs so that no algs can become dependent on this tool's output. Done in HLT jobs"};

#define DEF_VIEWS(__TYPE) \
  SG::ReadHandleKeyArray< ViewContainer > m_##__TYPE##Views{ this, #__TYPE"Views", {}, "Name  views from where the "#__TYPE" will be read"}



#define DEF_KEY(__TYPE)	\
  SG::WriteHandleKeyArray<__TYPE> m_##__TYPE{ this, #__TYPE, {}, "Required collections of "#__TYPE}; \
  DEF_VIEWS(__TYPE); \
  SG::ReadHandleKeyArray<__TYPE> m_##__TYPE##InViews{ this, #__TYPE"InViews", {}, "Names of "#__TYPE" in respective views"}

#define DEF_XAOD_KEY(__TYPE) \
  SG::WriteHandleKeyArray<xAOD::__TYPE> m_##__TYPE{ this, #__TYPE, {}, "Required collections of xAOD::"#__TYPE};\
  DEF_VIEWS(__TYPE); \
  SG::ReadHandleKeyArray<xAOD::__TYPE> m_##__TYPE##InViews{ this, #__TYPE"InViews", {}, "Names of xAOD::"#__TYPE" in respective views"}

  DEF_KEY( TrigRoiDescriptorCollection );

  DEF_XAOD_KEY( TrigCompositeContainer );
  DEF_XAOD_KEY( TrigEMClusterContainer );
  DEF_XAOD_KEY( TrigCaloClusterContainer );
  DEF_XAOD_KEY( TrigRingerRingsContainer );
  DEF_XAOD_KEY( TrigElectronContainer );
  DEF_XAOD_KEY( ElectronContainer );
  DEF_XAOD_KEY( PhotonContainer );
  DEF_XAOD_KEY( TrigPhotonContainer );
  DEF_XAOD_KEY( TrackParticleContainer );
  DEF_XAOD_KEY( TrigMissingETContainer );
  DEF_XAOD_KEY( L2StandAloneMuonContainer );
  DEF_XAOD_KEY( L2CombinedMuonContainer );
  DEF_XAOD_KEY( L2IsoMuonContainer );
  DEF_XAOD_KEY( MuonContainer );
  DEF_XAOD_KEY( TauJetContainer );
  DEF_XAOD_KEY( TauTrackContainer );
  DEF_XAOD_KEY( CaloClusterContainer );
  DEF_XAOD_KEY( JetContainer );
  DEF_XAOD_KEY( VertexContainer );
  DEF_XAOD_KEY( BTaggingContainer );
  DEF_XAOD_KEY( BTagVertexContainer );
  DEF_XAOD_KEY( TrigBphysContainer );
  DEF_XAOD_KEY( TrigT2MbtsBitsContainer );
  DEF_XAOD_KEY( HIEventShapeContainer );
  DEF_XAOD_KEY( TrigRNNOutputContainer );
  DEF_XAOD_KEY( AFPSiHitsClusterContainer );
  DEF_XAOD_KEY( AFPTrackContainer );
  DEF_XAOD_KEY( AFPToFTrackContainer );
  DEF_XAOD_KEY( AFPProtonContainer );
  DEF_XAOD_KEY( AFPVertexContainer );

  // Phase-I L1 RoIs EDM
  DEF_XAOD_KEY( eFexEMRoIContainer );
  DEF_XAOD_KEY( eFexTauRoIContainer );
  DEF_XAOD_KEY( jFexTauRoIContainer );
  DEF_XAOD_KEY( jFexFwdElRoIContainer );
  DEF_XAOD_KEY( jFexSRJetRoIContainer );
  DEF_XAOD_KEY( jFexLRJetRoIContainer );
  DEF_XAOD_KEY( jFexMETRoIContainer );
  DEF_XAOD_KEY( jFexSumETRoIContainer );
  DEF_XAOD_KEY( gFexJetRoIContainer );
  DEF_XAOD_KEY( gFexGlobalRoIContainer );
  DEF_XAOD_KEY( MuonRoIContainer );

#undef DEF_VIEWS
#undef DEF_KEY
#undef DEF_XAOD_KEY

// special cases with different (ShallowAuxContainer )  (containers not in views)
#define DEF_XAOD_KEY_SHALLOW(__TYPE) \
  SG::ReadHandleKeyArray<xAOD::__TYPE> m_##__TYPE##ShallowCopy{ this, #__TYPE"ShallowCopy", {}, "Required collections with Aux of type ShallowAuxContainerxAOD::"#__TYPE}; \
  SG::WriteHandleKeyArray<xAOD::__TYPE> m_##__TYPE##ShallowCopyOut{ this, #__TYPE"ShallowCopyOut", {}, "Output collections with Aux of type ShallowAuxContainerxAOD::"#__TYPE}

  DEF_XAOD_KEY_SHALLOW( JetContainer );
  DEF_XAOD_KEY_SHALLOW( CaloClusterContainer );


#undef DEF_XAOD_KEY_SHALLOW

  template<typename T>
  struct HandlesGroup {
    HandlesGroup(SG::WriteHandleKeyArray<T>& out_,
		 SG::ReadHandleKeyArray<T>& in_,
		 SG::ReadHandleKeyArray< ViewContainer >& views_)
      : out(out_), in(in_), views(views_) {}

    SG::WriteHandleKeyArray<T>& out;
    SG::ReadHandleKeyArray<T>& in;
    SG::ReadHandleKeyArray< ViewContainer >& views;
  };

  /**
   * Init related handles
   * 
   * Fails when they are setup inconsistently: the only allowed setup is:
   * views and in empty - then out can be of any length - this is mode to fill missing containers
   * views and in not empty, have to be the same size - out has to be 1 - this mode is for merging from several views into one output
   * if merging from several views into several separate output containers is required another instance of that tool can be setup
   **/
  template<typename T>
  StatusCode initHandles( const HandlesGroup<T>&  handles );

  /**
   * Register AuxStore keys for the given keys
   *
   * This is required to avoid hash collisions (ATR-26386).
   */
  template<typename T>
  StatusCode initAuxKey( const std::vector<SG::VarHandleKey*>& keys );

  template<typename T>
  struct ConstHandlesGroup {
    ConstHandlesGroup(const SG::WriteHandleKeyArray<T>& out_,
		      const SG::ReadHandleKeyArray<T>& in_,
		      const SG::ReadHandleKeyArray< ViewContainer >& views_)
      : out(out_), in(in_), views(views_) {}

    const SG::WriteHandleKeyArray<T>& out;
    const SG::ReadHandleKeyArray<T>& in;
    const SG::ReadHandleKeyArray< ViewContainer >& views;
  };

  StatusCode fixLinks() const;

  template<typename T, typename STORE, typename G, typename M >
    StatusCode createIfMissing( const EventContext& context, const ConstHandlesGroup<T>& handles, 
				G& generator, M merger ) const;


  template<typename T>
  StatusCode  viewsMerge( ViewContainer const& views, const SG::ReadHandleKey<T>& inViewKey,
			  EventContext const& context, T & output ) const;
  
  template<typename T>
  StatusCode  noMerge( ViewContainer const& views, const SG::ReadHandleKey<T>& inViewKey,
		       EventContext const& context, T & output ) const;
  
}; 


#endif //> !TRIGOUTPUTHANDLING_HLTEDMCREATOR_H
