
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


//#include "GaudiKernel/IToolSvc.h"
#include "AthViews/ViewHelper.h"
#include "HLTEDMCreator.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TrigOutputHandling/TriggerEDMAuxAccessors.h"

#include "xAODTrigger/TrigCompositeAuxContainer.h"
#include "xAODTrigEgamma/TrigElectronAuxContainer.h"
#include "xAODTrigEgamma/TrigPhotonAuxContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "xAODTrigCalo/TrigEMClusterAuxContainer.h"
#include "xAODTrigCalo/TrigCaloClusterAuxContainer.h"

#include "xAODTrigRinger/TrigRingerRingsAuxContainer.h"

#include "xAODTracking/TrackParticleAuxContainer.h"
#include "xAODTrigMissingET/TrigMissingETAuxContainer.h"

#include "xAODTrigMuon/L2StandAloneMuonAuxContainer.h"
#include "xAODTrigMuon/L2CombinedMuonAuxContainer.h"
#include "xAODTrigMuon/L2IsoMuonAuxContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/DiTauJetAuxContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODTracking/VertexAuxContainer.h"
#include "xAODTrigBphys/TrigBphysAuxContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODBTagging/BTagVertexAuxContainer.h"

#include "xAODTrigCalo/CaloClusterTrigAuxContainer.h"

#include "xAODTrigMinBias/TrigT2MbtsBitsAuxContainer.h"

#include "xAODHIEvent/HIEventShapeAuxContainer.h"

#include "xAODTrigRinger/TrigRNNOutputAuxContainer.h"

#include "xAODForward/AFPSiHitsClusterAuxContainer.h"
#include "xAODForward/AFPTrackAuxContainer.h"
#include "xAODForward/AFPToFTrackAuxContainer.h"
#include "xAODForward/AFPProtonAuxContainer.h"
#include "xAODForward/AFPVertexAuxContainer.h"

#include "xAODTrigger/eFexEMRoIAuxContainer.h"
#include "xAODTrigger/eFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexTauRoIAuxContainer.h"
#include "xAODTrigger/jFexFwdElRoIAuxContainer.h"
#include "xAODTrigger/jFexSRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexLRJetRoIAuxContainer.h"
#include "xAODTrigger/jFexMETRoIAuxContainer.h"
#include "xAODTrigger/jFexSumETRoIAuxContainer.h"
#include "xAODTrigger/gFexJetRoIAuxContainer.h"
#include "xAODTrigger/gFexGlobalRoIAuxContainer.h"
#include "xAODTrigger/MuonRoIAuxContainer.h"

#include "xAODCore/ShallowAuxContainer.h"

HLTEDMCreator::HLTEDMCreator( const std::string& type, 
            const std::string& name,
            const IInterface* parent )
  : base_class( type, name, parent ) {}

template<typename T>
StatusCode HLTEDMCreator::initHandles( const HandlesGroup<T>&  handles ) {
  ATH_CHECK( handles.out.initialize() );
  if(m_renounceOutputs) { renounceArray( handles.out ); }
  ATH_CHECK( handles.in.initialize() );
  renounceArray( handles.in );
  ATH_CHECK( handles.views.initialize() );
  renounceArray( handles.views );

  // the case w/o reading from views, both views handles and collection in views should be empty
  if ( handles.views.empty() ) {
    ATH_CHECK( handles.in.empty() );
  } else {
    // the case with views, for every output we expect an input View and an input collection inside that View
    ATH_CHECK( handles.out.size() == handles.in.size() );
    ATH_CHECK( handles.in.size()  == handles.views.size() );
  }
  return StatusCode::SUCCESS;
}

template<typename T>
StatusCode HLTEDMCreator::initAuxKey( const std::vector<SG::VarHandleKey*>& keys ) {
  // Register Aux keys for all handles to avoid hash collisions (ATR-26386).
  for (const SG::VarHandleKey* k : keys) {
    SG::WriteHandleKey<T> auxkey(k->key()+"Aux.");
    ATH_CHECK( auxkey.initialize() );
  }
  return StatusCode::SUCCESS;
}


StatusCode HLTEDMCreator::initialize()
{  
  if ( m_fixLinks.size() > 0 ) {
    // Confirm that m_fixLinks is a sub-set of m_TrigCompositeContainer
    for (const std::string& entry : m_fixLinks) {
      const bool found = std::any_of(m_TrigCompositeContainer.begin(),
                                     m_TrigCompositeContainer.end(), [&](const auto& writeHandleKey) { return writeHandleKey.key() == entry; } );
      if (!found) {
        ATH_MSG_ERROR("FixLinks contains the entry " << entry << ", however this is not one of this EDMCreator tool's managed TrigCompositeContainers.");
        ATH_MSG_ERROR("Configure FixLinks to be a sub-set of TrigCompositeContainer");
        return StatusCode::FAILURE;
      }
    }
    // Set up the write decorate handles to hold the remapped data
    for ( const auto& writeHandleKey: m_TrigCompositeContainer ) {
      const bool doFixLinks = std::any_of(m_fixLinks.begin(), m_fixLinks.end(), [&](const std::string& s) { return s == writeHandleKey.key(); } );
      if (doFixLinks) {
        // This writeHandleKey is being included in the element link remapping
        m_remapLinkColKeys.emplace_back( writeHandleKey.key()+".remap_linkColKeys" );
        m_remapLinkColIndices.emplace_back( writeHandleKey.key()+".remap_linkColIndices" );
      }
    }
    ATH_CHECK( m_remapLinkColKeys.initialize() ) ;
    renounceArray( m_remapLinkColKeys ); // This is not strictly necessary however, since we have many of these collection and no consumer for it we can spare scheduler headache renouncing them
    ATH_CHECK( m_remapLinkColIndices.initialize() );
    renounceArray( m_remapLinkColIndices );
  }

  // this section has to appear after the above initialisation of DecorHandles, else the renounce of TrigComposite does not work as expected

#define INIT(__TYPE) \
  ATH_CHECK( initHandles( HandlesGroup<__TYPE>( m_##__TYPE, m_##__TYPE##InViews, m_##__TYPE##Views ) ) );

#define INIT_XAOD(__TYPE, __STORE_TYPE) \
  ATH_CHECK( initHandles( HandlesGroup<xAOD::__TYPE>( m_##__TYPE, m_##__TYPE##InViews, m_##__TYPE##Views ) ) ); \
  ATH_CHECK( initAuxKey<xAOD::__STORE_TYPE>( m_##__TYPE.keys() ) );

  INIT( TrigRoiDescriptorCollection );
  INIT_XAOD( TrigCompositeContainer, TrigCompositeAuxContainer );
    
  INIT_XAOD( TrigEMClusterContainer, TrigEMClusterAuxContainer );
  INIT_XAOD( TrigCaloClusterContainer, TrigCaloClusterAuxContainer );
  INIT_XAOD( TrigRingerRingsContainer, TrigRingerRingsAuxContainer );
  INIT_XAOD( TrigElectronContainer, TrigElectronAuxContainer );
  INIT_XAOD( ElectronContainer, ElectronAuxContainer );
  INIT_XAOD( PhotonContainer, PhotonAuxContainer );
  INIT_XAOD( TrigPhotonContainer, TrigPhotonAuxContainer );
  INIT_XAOD( TrackParticleContainer, TrackParticleAuxContainer );
  INIT_XAOD( TrigMissingETContainer, TrigMissingETAuxContainer );

  INIT_XAOD( L2StandAloneMuonContainer, L2StandAloneMuonAuxContainer );
  INIT_XAOD( L2CombinedMuonContainer, L2CombinedMuonAuxContainer );
  INIT_XAOD( L2IsoMuonContainer, L2IsoMuonAuxContainer );
  INIT_XAOD( MuonContainer, MuonAuxContainer );
  INIT_XAOD( TauJetContainer, TauJetAuxContainer );
  INIT_XAOD( DiTauJetContainer, DiTauJetAuxContainer );
  INIT_XAOD( TauTrackContainer, TauTrackAuxContainer );
  INIT_XAOD( JetContainer, JetAuxContainer );
  INIT_XAOD( VertexContainer, VertexAuxContainer );
  INIT_XAOD( TrigBphysContainer, TrigBphysAuxContainer );
  INIT_XAOD( BTaggingContainer, BTaggingAuxContainer );
  INIT_XAOD( BTagVertexContainer, BTagVertexAuxContainer );
  INIT_XAOD( CaloClusterContainer, CaloClusterTrigAuxContainer ); // NOTE: Difference in interface and aux
  INIT_XAOD( TrigT2MbtsBitsContainer, TrigT2MbtsBitsAuxContainer );
  INIT_XAOD( HIEventShapeContainer, HIEventShapeAuxContainer );
  INIT_XAOD( TrigRNNOutputContainer, TrigRNNOutputAuxContainer );
  INIT_XAOD( AFPSiHitsClusterContainer, AFPSiHitsClusterAuxContainer );
  INIT_XAOD( AFPTrackContainer, AFPTrackAuxContainer );
  INIT_XAOD( AFPToFTrackContainer, AFPToFTrackAuxContainer );
  INIT_XAOD( AFPProtonContainer, AFPProtonAuxContainer );
  INIT_XAOD( AFPVertexContainer, AFPVertexAuxContainer );

  // Phase-I L1 RoIs EDM
  INIT_XAOD( eFexEMRoIContainer, eFexEMRoIAuxContainer );
  INIT_XAOD( eFexTauRoIContainer, eFexTauRoIAuxContainer );
  INIT_XAOD( jFexTauRoIContainer, jFexTauRoIAuxContainer );
  INIT_XAOD( jFexFwdElRoIContainer, jFexFwdElRoIAuxContainer );
  INIT_XAOD( jFexSRJetRoIContainer, jFexSRJetRoIAuxContainer );
  INIT_XAOD( jFexLRJetRoIContainer, jFexLRJetRoIAuxContainer );
  INIT_XAOD( jFexMETRoIContainer, jFexMETRoIAuxContainer );
  INIT_XAOD( jFexSumETRoIContainer, jFexSumETRoIAuxContainer );
  INIT_XAOD( gFexJetRoIContainer, gFexJetRoIAuxContainer );
  INIT_XAOD( gFexGlobalRoIContainer, gFexGlobalRoIAuxContainer);
  INIT_XAOD( MuonRoIContainer, MuonRoIAuxContainer );

#undef INIT
#undef INIT_XAOD

#define INIT_SHALLOW(__TYPE) \
  ATH_CHECK( m_##__TYPE##ShallowCopy.initialize() ); \
  renounceArray( m_##__TYPE##ShallowCopy ); \
  for ( auto k: m_##__TYPE##ShallowCopy ) \
    m_##__TYPE##ShallowCopyOut.push_back(k.key()); \
  ATH_CHECK( m_##__TYPE##ShallowCopyOut.initialize() ); \
  if(m_renounceOutputs) { renounceArray( m_##__TYPE##ShallowCopyOut ); }

  INIT_SHALLOW( CaloClusterContainer );
  INIT_SHALLOW( JetContainer );

#undef INIT_SHALLOW

  return StatusCode::SUCCESS;
}

template<class T>
struct plainGenerator {
  std::unique_ptr<T> data;
  bool doRecord{true};
  void create( bool create, bool record ) {
    doRecord = record;
    if ( create )
      data = std::make_unique<T>();
  }

  StatusCode record( SG::WriteHandle<T>& h ) {
    if ( doRecord )
      return h.record( std::move( data ) );
    return StatusCode::SUCCESS;
  }
};

template<class T, class STORE>
struct xAODGenerator {
  std::unique_ptr<T> data;
  std::unique_ptr<STORE> store;
  bool doRecord{true};

  void create( bool create, bool record ) {
    doRecord = record;
    if ( create ) {
      data  = std::make_unique<T>();
      store = std::make_unique<STORE>();
      data->setStore( store.get() );
    }
  }

  StatusCode record ( SG::WriteHandle<T>& h ) {
    if ( doRecord )
      return h.record( std::move( data ), std::move( store ) );
    return StatusCode::SUCCESS;
  }
};

template<typename T>
StatusCode  HLTEDMCreator::noMerge( ViewContainer const&, const SG::ReadHandleKey<T>&,
            EventContext const&, T & ) const {
  //  if we are called it means views merging is requested but Type T does not support it (i.e. missing copy c'tor)
  return StatusCode::FAILURE;

}

template<typename T>
StatusCode  HLTEDMCreator::viewsMerge( ViewContainer const& views, const SG::ReadHandleKey<T>& inViewKey,
               EventContext const& context, T & output ) const {
  
  using type_in_container = typename T::base_value_type;
  StoreGateSvc* sg = evtStore().operator->(); // why the get() method is returing a null ptr is a puzzle, we have to use this ugly call to operator instead of it
  ATH_CHECK( sg != nullptr );
  ViewHelper::ViewMerger merger( sg, msg() );
  ATH_CHECK( merger.mergeViewCollection<type_in_container>( views, inViewKey, context, output ) );

  return StatusCode::SUCCESS;
}

 
StatusCode HLTEDMCreator::fixLinks( EventContext const& context ) const {
  if ( m_fixLinks.value().empty() ) {
    ATH_MSG_DEBUG("fixLinks: No collections defined for this tool");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("fixLinks called for " << m_fixLinks.size() << " of " << m_TrigCompositeContainer.size() << " collections");

  // Do the remapping
  int writeHandleArrayIndex = -1;

  // Create a HandleKey that we can re-use during the loop (slightly better performance)
  SG::ReadHandleKey<xAOD::TrigCompositeContainer> readHandleKey("temp");
  ATH_CHECK( readHandleKey.initialize() );

  for ( const auto& writeHandleKey: m_TrigCompositeContainer ) {
    // Check if we are re-mapping this handle
    const bool doFixLinks = std::any_of(m_fixLinks.begin(), m_fixLinks.end(), [&](const std::string& s) { return s == writeHandleKey.key(); } );
    if ( not doFixLinks ) {
      ATH_MSG_DEBUG("Not requested to fix TrigComposite ElementLinks for " << writeHandleKey.key());
      continue;
    }

    // Only increment this index for the sub-set of the TrigComposite collections that we are fixing. Mirror the initialize() logic.
    ++writeHandleArrayIndex;

    ATH_MSG_DEBUG("Fixing links: confirm collection is there: " << writeHandleKey.key() << ", write handle array index: " << writeHandleArrayIndex);
    // Update key name
    readHandleKey = writeHandleKey.key();
    auto readHandle = SG::makeHandle(readHandleKey, context);
    if ( not readHandle.isValid() ) { // object missing, this is now an error as we should have literally just created it
      ATH_MSG_ERROR("Collection is not present. " << readHandleKey.key() << " should have been created by createIfMissing.");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Collection exists with size " << readHandle->size() << " Decision objects" );
    ATH_MSG_DEBUG("Adding decorations: " << m_remapLinkColKeys.at( writeHandleArrayIndex ).key() << " and " << m_remapLinkColIndices.at( writeHandleArrayIndex ).key() );
    
    SG::WriteDecorHandle<xAOD::TrigCompositeContainer, std::vector<SG::sgkey_t> >
      keyDecor(m_remapLinkColKeys.at( writeHandleArrayIndex ), context );
    SG::WriteDecorHandle<xAOD::TrigCompositeContainer, std::vector<xAOD::TrigComposite::index_type> >
      indexDecor( m_remapLinkColIndices.at( writeHandleArrayIndex ), context );

    // Examine each input TC
    int decisionObjectIndex = -1;
    for ( auto inputDecision : *( readHandle.cptr() ) ) {
      ++decisionObjectIndex;

      // Retrieve the link information for remapping
      std::vector< SG::sgkey_t > remappedKeys = inputDecision->linkColKeys(); // Vec copy
      std::vector< xAOD::TrigComposite::index_type > remappedIndexes = inputDecision->linkColIndices(); // Vec copy

      // Search the linked collections for remapping
      size_t const collectionTotal = inputDecision->linkColNames().size();
      ATH_MSG_DEBUG("  Decision object #" << decisionObjectIndex << " has " << collectionTotal << " links");
      for ( size_t elementLinkIndex = 0; elementLinkIndex < collectionTotal; ++elementLinkIndex ) {

        // Load ElementLink identifiers (except for CLID)
        std::string const collectionName = inputDecision->linkColNames().at(elementLinkIndex);
        SG::sgkey_t const collectionKey = remappedKeys.at(elementLinkIndex); //Note: This is the existing before-remap key
        std::string const keyString = *( evtStore()->keyToString( collectionKey ) );
        xAOD::TrigComposite::index_type const collectionIndex = remappedIndexes.at(elementLinkIndex); //Note: This is the existing before-remap index

        // Check for remapping in a merge
        SG::sgkey_t newKey = 0;
        size_t newIndex = 0;
        bool isRemapped = evtStore()->tryELRemap( collectionKey, collectionIndex, newKey, newIndex);
        if ( isRemapped ) {

          ATH_MSG_DEBUG( "    Remap link [" << collectionName <<"] from " << keyString << " to " << *( evtStore()->keyToString( newKey ) ) << ", from index " << collectionIndex << " to index " << newIndex );
          remappedKeys[ elementLinkIndex ] = newKey;
          remappedIndexes[ elementLinkIndex ] = newIndex;

        } else {

          ATH_MSG_DEBUG( "    StoreGate did not remap link [" << collectionName << "] from  " << keyString << " index " << collectionIndex );

        }

      }
      
      // Save the remaps
      keyDecor( *inputDecision ) = remappedKeys;
      indexDecor( *inputDecision ) = remappedIndexes;

    }    
  }  
  
  return StatusCode::SUCCESS;
}


template<typename T, typename STORE, typename G, typename M>
StatusCode HLTEDMCreator::createIfMissing( const EventContext& context, const ConstHandlesGroup<T>& handles, G& generator, M merger ) const {

  // Declare a ReadHandleKey that we can re-use during the loop for reading.
  SG::ReadHandleKey<T> rhk("temp");

  // Same for the Aux store. If there is none (void) this would not compile
  // so we just define a dummy RHK, which will never be used, of type T again.
  using AuxType = std::conditional_t<std::is_void_v<STORE>, T, STORE>;
  SG::ReadHandleKey<AuxType> rhkAux("temp");

  ATH_CHECK( rhk.initialize() && rhkAux.initialize() );

  for (size_t i = 0; i < handles.out.size(); ++i) {
    const SG::WriteHandleKey<T>& whk = handles.out.at(i);
    rhk = whk.key();  // set the RHK to the same key as the WHK

    if ( handles.views.empty() ) { // no merging will be needed
      // Note: This is correct. We are testing if we can read, and if we cannot then we write.
      // What we write will either be a dummy (empty) container, or be populated from N in-View collections.
      auto readHandle = SG::makeHandle( rhk, context );
      if ( readHandle.isValid() ) {
        ATH_MSG_VERBOSE( rhk.key() << " is already present" );
        generator.create(false, false);

        // For xAOD types we need to ensure there is an Aux store. This can happen if the
        // Aux store gets truncated for collections marked with "allowTruncation".
        // The TriggerEDMDeserialiserAlg will already have created a DataLink to the Aux store
        // for the interface container. Now we just need to create an empty Aux store.
        if constexpr (!std::is_void_v<STORE>) {
          rhkAux = rhk.key() + "Aux.";
          auto readAuxHandle = SG::makeHandle(rhkAux, context);
          if ( !readAuxHandle.isValid() ) {
            // This is rare so we just create a WH as needed:
            SG::WriteHandle<STORE> writeAuxHandle( rhkAux.key(), context );
            ATH_MSG_DEBUG("Creating missing Aux store for " << rhk.key());
            ATH_CHECK( writeAuxHandle.record(std::make_unique<STORE>()) );
          }
        }
      } else {
        ATH_MSG_DEBUG( rhk.key() << " is missing, creating it" );
        generator.create(true, true);
      }

    } else {
      // there are views, we assume that in the main store collection of given type#name is absent, else it will not work anyways
      // simplest case, only one set of views is handled first
      // below is handled the cases when the configuration of output keys is for example: A A B C C C D D E
      // which means the first two collections come from first two views and because the names are the same they should end up in the same output collection
      // thefore generators need to instructed to:
      //   - create new collection when a new name is handled (or for the first key)
      //   - and record when it is last identical name in the row (or it is last handled collection)
      if ( handles.out.size() == 1  ) {
      	generator.create(true, true);
      } else  {
        const bool doCreate = i == 0 or handles.out.at(i-1).key() != whk.key();
        const bool doRecord = i == handles.out.size()-1 or handles.out.at(i+1).key() != whk.key();
        ATH_MSG_DEBUG( "Instructing generator " <<  (doCreate ? "to" : "NOT TO") <<  " create collection and " << (doRecord ? "to" : "NOT TO") << " record collection in this iteration");
        generator.create(doCreate, doRecord);
      }

      const SG::ReadHandleKey<ViewContainer>& viewsReadHandleKey = handles.views.at(i);
      ATH_MSG_DEBUG("Will be trying to merge from the " << viewsReadHandleKey.key() << " view container into that output");

      auto viewsHandle = SG::makeHandle( viewsReadHandleKey, context );
      if ( viewsHandle.isValid() ) {
        const SG::ReadHandleKey<T>& inViewReadHandleKey = handles.in.at(i);
        ATH_MSG_DEBUG("Will be merging from " << viewsHandle->size() << " views using in-view key " << inViewReadHandleKey.key() );
        ATH_CHECK( (this->*merger)( *viewsHandle, inViewReadHandleKey , context, *generator.data.get() ) );
      } else {
        ATH_MSG_DEBUG("Views " << viewsReadHandleKey.key() << " are missing. Will leave " << whk.key() << " output collection empty.");
      }

      // Also consider probe variants of each EventView.
      // Not every container will have a corresponding set of (typically) lower-pT probe ROIs, but it's safer to always test.
      const std::string viewsReadHandleKeyProbe = viewsReadHandleKey.key() + "_probe";
      ATH_MSG_VERBOSE("Will try to merge from the " << viewsReadHandleKeyProbe <<  " view container into that output");

      // Falling back to direct SG access here to avoid uninitiated key errors. This is safe to do in the context of the Trigger ControlFlow.
      // I.e. if this collection is to exist in this event, then it is guaranteed to have been produced prior to this alg executing.
      const ViewContainer* viewsContainer_probe = nullptr;
      if (evtStore()->contains<ViewContainer>(viewsReadHandleKeyProbe)) {
        ATH_CHECK(evtStore()->retrieve(viewsContainer_probe, viewsReadHandleKeyProbe));
      }
      if ( viewsContainer_probe ) {
        const SG::ReadHandleKey<T>& inViewReadHandleKey = handles.in.at(i);
        ATH_MSG_DEBUG("Will be merging from " << viewsContainer_probe->size() << " probe views using in-view key " << inViewReadHandleKey.key() );
        ATH_CHECK( (this->*merger)( *viewsContainer_probe, inViewReadHandleKey , context, *generator.data.get() ) );
      } else {
        ATH_MSG_VERBOSE("Probe views " << viewsReadHandleKeyProbe << " are missing.");
      }

    }

    auto writeHandle = SG::makeHandle( whk, context );
    ATH_CHECK( generator.record( writeHandle ) );
  }

  return StatusCode::SUCCESS;
}



StatusCode HLTEDMCreator::createOutput(const EventContext& context) const {
  ATH_MSG_DEBUG("Confirming / Creating this tool's output");
  if ( m_dumpSGBefore )  
    ATH_MSG_DEBUG( evtStore()->dump() );

#define CREATE(__TYPE) \
    {                 \
      plainGenerator<__TYPE> generator;         \
      ATH_CHECK( (createIfMissing<__TYPE, void>( context, ConstHandlesGroup<__TYPE>( m_##__TYPE, m_##__TYPE##InViews, m_##__TYPE##Views ), generator, &HLTEDMCreator::noMerge<__TYPE>)) ); \
    }

  CREATE( TrigRoiDescriptorCollection )

#undef CREATE

#define CREATE_XAOD(__TYPE, __STORE_TYPE) \
  { \
    xAODGenerator<xAOD::__TYPE, xAOD::__STORE_TYPE> generator; \
    ATH_CHECK( (createIfMissing<xAOD::__TYPE, xAOD::__STORE_TYPE>( context, ConstHandlesGroup<xAOD::__TYPE>( m_##__TYPE, m_##__TYPE##InViews, m_##__TYPE##Views ), generator, &HLTEDMCreator::viewsMerge<xAOD::__TYPE>))  ); \
  }


  CREATE_XAOD( TrigCompositeContainer, TrigCompositeAuxContainer );
  CREATE_XAOD( TrigElectronContainer, TrigElectronAuxContainer );
  CREATE_XAOD( ElectronContainer, ElectronAuxContainer );
  CREATE_XAOD( PhotonContainer, PhotonAuxContainer );
  CREATE_XAOD( TrigPhotonContainer, TrigPhotonAuxContainer );
  CREATE_XAOD( TrigEMClusterContainer, TrigEMClusterAuxContainer );
  CREATE_XAOD( TrigCaloClusterContainer, TrigCaloClusterAuxContainer );
  CREATE_XAOD( TrigRingerRingsContainer, TrigRingerRingsAuxContainer );
  CREATE_XAOD( TrackParticleContainer, TrackParticleAuxContainer );
  CREATE_XAOD( TrigMissingETContainer, TrigMissingETAuxContainer );

  CREATE_XAOD( L2StandAloneMuonContainer, L2StandAloneMuonAuxContainer );
  CREATE_XAOD( L2CombinedMuonContainer, L2CombinedMuonAuxContainer );
  CREATE_XAOD( L2IsoMuonContainer, L2IsoMuonAuxContainer );
  CREATE_XAOD( MuonContainer, MuonAuxContainer );
  CREATE_XAOD( TauJetContainer, TauJetAuxContainer );
  CREATE_XAOD( DiTauJetContainer, DiTauJetAuxContainer );
  CREATE_XAOD( TauTrackContainer, TauTrackAuxContainer );
  CREATE_XAOD( CaloClusterContainer, CaloClusterTrigAuxContainer ); // NOTE: Difference in interface and aux
  CREATE_XAOD( JetContainer, JetAuxContainer );
  CREATE_XAOD( VertexContainer,VertexAuxContainer );
  CREATE_XAOD( TrigBphysContainer, TrigBphysAuxContainer );
  CREATE_XAOD( BTaggingContainer,BTaggingAuxContainer );
  CREATE_XAOD( BTagVertexContainer,BTagVertexAuxContainer );
  CREATE_XAOD( TrigT2MbtsBitsContainer, TrigT2MbtsBitsAuxContainer );
  CREATE_XAOD( HIEventShapeContainer, HIEventShapeAuxContainer );
  CREATE_XAOD( TrigRNNOutputContainer, TrigRNNOutputAuxContainer );
  CREATE_XAOD( AFPSiHitsClusterContainer, AFPSiHitsClusterAuxContainer );
  CREATE_XAOD( AFPTrackContainer, AFPTrackAuxContainer );
  CREATE_XAOD( AFPToFTrackContainer, AFPToFTrackAuxContainer );
  CREATE_XAOD( AFPProtonContainer, AFPProtonAuxContainer );
  CREATE_XAOD( AFPVertexContainer, AFPVertexAuxContainer );

  // Phase-I L1 RoIs EDM
  CREATE_XAOD( eFexEMRoIContainer, eFexEMRoIAuxContainer );
  CREATE_XAOD( eFexTauRoIContainer, eFexTauRoIAuxContainer );
  CREATE_XAOD( jFexTauRoIContainer, jFexTauRoIAuxContainer );
  CREATE_XAOD( jFexFwdElRoIContainer, jFexFwdElRoIAuxContainer );
  CREATE_XAOD( jFexSRJetRoIContainer, jFexSRJetRoIAuxContainer );
  CREATE_XAOD( jFexLRJetRoIContainer, jFexLRJetRoIAuxContainer );
  CREATE_XAOD( jFexMETRoIContainer, jFexMETRoIAuxContainer );
  CREATE_XAOD( jFexSumETRoIContainer, jFexSumETRoIAuxContainer );
  CREATE_XAOD( gFexJetRoIContainer, gFexJetRoIAuxContainer );
  CREATE_XAOD( gFexGlobalRoIContainer, gFexGlobalRoIAuxContainer);
  CREATE_XAOD( MuonRoIContainer, MuonRoIAuxContainer );

  // After view collections are merged, need to update collection links
  ATH_CHECK( fixLinks(context) );
  
#undef CREATE_XAOD

  // special cases
  #define CREATE_SHALLOW(__TYPE) \
  { \
    for ( size_t index = 0; index < m_##__TYPE##ShallowCopy.size(); ++index ){ \
      auto readHandle = SG::makeHandle<xAOD::__TYPE> (  m_##__TYPE##ShallowCopy[index], context ); \
      if ( not readHandle.isValid() ) { \
	ATH_MSG_DEBUG( "Creating missing "#__TYPE"ShallowCopy " <<  m_##__TYPE##ShallowCopy[index].key() ); \
	auto writeHandle = SG::makeHandle( m_##__TYPE##ShallowCopyOut[index], context ); \
	ATH_CHECK( writeHandle.record( std::make_unique<xAOD::__TYPE>(), std::make_unique<xAOD::ShallowAuxContainer>() )); \
      } else { \
	ATH_MSG_DEBUG( #__TYPE"ShallowCopy " <<  m_##__TYPE##ShallowCopyOut[index].key() << " present in the event, done nothing"); \
      } \
    } \
  }

  CREATE_SHALLOW( CaloClusterContainer )
  CREATE_SHALLOW( JetContainer )

  #undef CREATE_SHALLOW

  if ( m_dumpSGAfter )  
    ATH_MSG_DEBUG( evtStore()->dump() );

  ATH_MSG_DEBUG("Done");
  return StatusCode::SUCCESS;
}
