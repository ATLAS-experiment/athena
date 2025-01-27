/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s)
#include "TauAnalysisTools/BuildTruthTaus.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "AthContainers/Accessor.h"

// EDM include(s):
#include "xAODTruth/TruthVertex.h"
#include "xAODEventInfo/EventInfo.h"

#include "TruthUtils/MagicNumbers.h"

// Tool include(s)
#include "MCTruthClassifier/MCTruthClassifier.h"

using namespace TauAnalysisTools;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
BuildTruthTaus::BuildTruthTaus( const std::string& name )
  : AsgMetadataTool(name)
  , m_tMCTruthClassifier("MCTruthClassifier", this)
{
  declareProperty( "WriteInvisibleFourMomentum", m_bWriteInvisibleFourMomentum = false);
  declareProperty( "WriteVisibleChargedFourMomentum", m_bWriteVisibleChargedFourMomentum = false);
  declareProperty( "WriteVisibleNeutralFourMomentum", m_bWriteVisibleNeutralFourMomentum = false);
  declareProperty( "WriteDecayModeVector", m_bWriteDecayModeVector = true);
  declareProperty( "WriteVertices", m_bWriteVertices = true); 
}

//______________________________________________________________________________
StatusCode BuildTruthTaus::initialize()
{
  if (!m_truthMatchingMode) {
    ATH_MSG_INFO( "Initializing BuildTruthTaus, will generate " <<  m_truthTauOutputContainer.key() << " from " << m_truthParticleContainer.key() << " container" );
  }
  else {
    ATH_MSG_INFO( "Initializing BuildTruthTaus in truth matching mode, using input container " << m_truthTauInputContainer.key() );
  }

  // input containers
  ATH_CHECK( m_truthTauInputContainer.initialize(m_truthMatchingMode) );
  ATH_CHECK( m_truthParticleContainer.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_truthElectronContainer.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_truthMuonContainer.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_truthJetContainer.initialize(SG::AllowEmpty) );

  // in truth matching mode, truth electron/muon/jet containers are optional but expected
  if (m_truthMatchingMode) {
    if (m_truthElectronContainer.empty()) {
      ATH_MSG_WARNING("Truth electron container is not available, won't perform matching to truth electrons");
    }
    if (m_truthMuonContainer.empty()) {
      ATH_MSG_WARNING("Truth muon container is not available, won't perform matching to truth muons");
    }
    if (m_truthJetContainer.empty()) {
      ATH_MSG_WARNING("Truth jet container is not available, won't perform matching to truth jets");
    }
  }

  // output container
  ATH_CHECK( m_truthTauOutputContainer.initialize(!m_truthMatchingMode) );

  // The following properties are only available in athena
#ifndef XAOD_ANALYSIS
  ATH_CHECK(m_tMCTruthClassifier.setProperty("ParticleCaloExtensionTool", ""));
  ATH_CHECK(m_tMCTruthClassifier.setProperty("TruthInConeTool", ""));
#endif
  
  ATH_CHECK(ASG_MAKE_ANA_TOOL(m_tMCTruthClassifier, MCTruthClassifier));
  ATH_CHECK(m_tMCTruthClassifier.initialize());

  // drop at earliest occasion
  m_bTruthTauAvailable = !m_truthTauInputContainer.empty();

  return StatusCode::SUCCESS;
}


StatusCode BuildTruthTaus::retrieveTruthTaus()
{
  return retrieveTruthTaus (m_truthTausEvent);
}


StatusCode BuildTruthTaus::retrieveTruthTaus(ITruthTausEvent& truthTausEvent) const
{
  return retrieveTruthTaus (dynamic_cast<TruthTausEvent&> (truthTausEvent));
}


StatusCode BuildTruthTaus::retrieveTruthTaus(TruthTausEvent& truthTausEvent) const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // truth matching mode
  if (m_truthMatchingMode) {
    if (!m_truthElectronContainer.empty()) {
      SG::ReadHandle<xAOD::TruthParticleContainer> truthElectronsHandle(m_truthElectronContainer, ctx);
      if (!truthElectronsHandle.isValid()) {
	ATH_MSG_ERROR ("Could not retrieve " << truthElectronsHandle.key());
	return StatusCode::FAILURE;
      }
      truthTausEvent.m_xTruthElectronContainerConst = truthElectronsHandle.cptr();
    }

    if (!m_truthMuonContainer.empty()) {
      SG::ReadHandle<xAOD::TruthParticleContainer> truthMuonsHandle(m_truthMuonContainer, ctx);
      if (!truthMuonsHandle.isValid()) {
	ATH_MSG_ERROR ("Could not retrieve " << truthMuonsHandle.key());
	return StatusCode::FAILURE;
      }
      truthTausEvent.m_xTruthMuonContainerConst = truthMuonsHandle.cptr();
    }

    if (!m_truthJetContainer.empty()) {
      SG::ReadHandle<xAOD::JetContainer> truthJetsHandle(m_truthJetContainer, ctx);
      if (!truthJetsHandle.isValid()) {
	ATH_MSG_ERROR ("Could not retrieve " << truthJetsHandle.key());
	return StatusCode::FAILURE;
      }
      truthTausEvent.m_xTruthJetContainerConst = truthJetsHandle.cptr();
    }

    SG::ReadHandle<xAOD::TruthParticleContainer> truthTausHandle(m_truthTauInputContainer, ctx);
    if (!truthTausHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve " << truthTausHandle.key());
      return StatusCode::FAILURE;
    }
    truthTausEvent.m_xTruthTauContainerConst = truthTausHandle.cptr();
  }
  // truth tau building mode
  else {
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticlesHandle(m_truthParticleContainer, ctx);
    if (!truthParticlesHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve " << truthParticlesHandle.key());
      return StatusCode::FAILURE;
    }
    truthTausEvent.m_xTruthParticleContainer = truthParticlesHandle.cptr();

    auto truthTausOutput = std::make_unique<xAOD::TruthParticleContainer>();
    auto truthTausOutputAux = std::make_unique<xAOD::TruthParticleAuxContainer>();
    truthTausOutput->setStore(truthTausOutputAux.get());
    truthTausEvent.m_xTruthTauContainer = truthTausOutput.get();

    ATH_CHECK( buildTruthTausFromTruthParticles(truthTausEvent) );

    auto writeHandle = SG::makeHandle(m_truthTauOutputContainer, ctx);
    ATH_CHECK(writeHandle.record(std::move(truthTausOutput), std::move(truthTausOutputAux)));
  }

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
//______________________________________________________________________________
//______________________________________________________________________________
StatusCode
BuildTruthTaus::buildTruthTausFromTruthParticles(TruthTausEvent& truthTausEvent) const
{
  static const SG::Accessor<char> dressedPhotonAcc ("dressedPhoton");

  bool copyDressedPhotons = false;

  for (auto xTruthParticle : *truthTausEvent.m_xTruthParticleContainer)
  {
    if ( xTruthParticle->isTau() )
    {
      auto xTruthTau = std::make_unique<xAOD::TruthParticle>();
      xTruthTau->makePrivateStore( *xTruthParticle );

      if ( examineTruthTau(*xTruthTau).isFailure() )
      {
        continue;
      }

      // The dressedPhoton decoration will likely be unlocked and thus will
      // not be copied by the above.  Copy it explicitly in that case.
      // See ATLASRECTS-8008.
      // First time through the loop we check to see if the decoration
      // needs to be copied and remember for subsequent iterations
      // (during which the destination decoration will have already
      // been created).
      if (truthTausEvent.m_xTruthTauContainer->empty() &&
          dressedPhotonAcc.isAvailable ( *xTruthParticle ) &&
          !dressedPhotonAcc.isAvailable ( *xTruthTau ))
      {
        copyDressedPhotons = true;
      }
      if (copyDressedPhotons)
      {
        dressedPhotonAcc( *xTruthTau ) = dressedPhotonAcc( *xTruthParticle );
      }

      // Run classification
      auto pClassification = m_tMCTruthClassifier->particleTruthClassifier(xTruthTau.get());
      static const SG::Accessor<unsigned int> decClassifierParticleType("classifierParticleType");
      static const SG::Accessor<unsigned int> decClassifierParticleOrigin("classifierParticleOrigin");
      decClassifierParticleType(*xTruthTau) = pClassification.first;
      decClassifierParticleOrigin(*xTruthTau) = pClassification.second;

      // create link to the original TruthParticle
      ElementLink < xAOD::TruthParticleContainer > lTruthParticleLink(xTruthParticle, *truthTausEvent.m_xTruthParticleContainer);
      static const SG::Accessor<ElementLink< xAOD::TruthParticleContainer > > accOriginalTruthParticle("originalTruthParticle");
      accOriginalTruthParticle(*xTruthTau) = lTruthParticleLink;

      truthTausEvent.m_xTruthTauContainer->push_back(std::move(xTruthTau));
    }
  }
  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
StatusCode BuildTruthTaus::examineTruthTau(const xAOD::TruthParticle& xTruthParticle) const
{
  // skip this tau if it has no decay vertex, should not happen
  if ( !xTruthParticle.hasDecayVtx() )
    return StatusCode::FAILURE;

  ATH_MSG_VERBOSE("looking for charged daughters of a truth tau");

  TauTruthInfo truthInfo;

  const xAOD::TruthVertex* xDecayVertex = xTruthParticle.decayVtx();
  if (xDecayVertex == nullptr)
    return StatusCode::FAILURE;
  for ( size_t iOutgoingParticle = 0; iOutgoingParticle < xDecayVertex->nOutgoingParticles(); ++iOutgoingParticle )
  {
    const xAOD::TruthParticle* xTruthDaughter = xDecayVertex->outgoingParticle(iOutgoingParticle);
    if (xTruthDaughter == nullptr)
    {
      ATH_MSG_ERROR("Truth daughter of tau decay was not found in "<< m_truthParticleContainer.key() <<" container. Please ensure that this container has the full tau decay information or produce the TruthTaus container in AtlasDerivation.");
      return StatusCode::FAILURE;
    }

    // if tau decays into tau this is not a proper tau decay
    if ( xTruthDaughter->isTau() )
    {
      ATH_MSG_VERBOSE("Tau decays into a tau itself -> skip this decay");
      return StatusCode::FAILURE;
    }
  }

  examineTruthTauDecay(xTruthParticle, truthInfo).ignore();

  if (truthInfo.m_bIsHadronicTau)
    ATH_MSG_VERBOSE(truthInfo.m_iNChargedDaughters << " prong hadronic truth tau was found with uniqueID "<<HepMC::uniqueID(xTruthParticle));
  else
    ATH_MSG_VERBOSE(truthInfo.m_iNChargedDaughters << " prong leptonic truth tau was found with uniqueID "<<HepMC::uniqueID(xTruthParticle));
  if ( truthInfo.m_iNChargedDaughters%2 == 0 )
  {
    ATH_MSG_WARNING("found tau with even multiplicity: " << truthInfo.m_iNChargedDaughters);
    printDecay(xTruthParticle);
  }

  static const SG::Decorator<double> decPtVis("pt_vis");
  static const SG::Decorator<double> decEtaVis("eta_vis");
  static const SG::Decorator<double> decPhiVis("phi_vis");
  static const SG::Decorator<double> decMVis("m_vis");

  static const SG::Decorator<size_t> decNumCharged("numCharged");
  static const SG::Decorator<size_t> decNumChargedPion("numChargedPion");
  static const SG::Decorator<size_t> decNumNeutral("numNeutral");
  static const SG::Decorator<size_t> decNumNeutralPion("numNeutralPion");

  decPtVis(xTruthParticle) = truthInfo.m_vTruthVisTLV.Pt();
  decEtaVis(xTruthParticle) = truthInfo.m_vTruthVisTLV.Eta();
  decPhiVis(xTruthParticle) = truthInfo.m_vTruthVisTLV.Phi();
  decMVis(xTruthParticle) = truthInfo.m_vTruthVisTLV.M();

  decNumCharged(xTruthParticle) = truthInfo.m_iNChargedDaughters;
  decNumChargedPion(xTruthParticle) = truthInfo.m_iNChargedPions;
  decNumNeutral(xTruthParticle) = truthInfo.m_iNNeutralPions+truthInfo.m_iNNeutralOthers;
  decNumNeutralPion(xTruthParticle) = truthInfo.m_iNNeutralPions;

  static const SG::Decorator<char> decIsHadronicTau("IsHadronicTau");
  decIsHadronicTau(xTruthParticle) = (char)truthInfo.m_bIsHadronicTau;

  if ( m_bWriteInvisibleFourMomentum )
  {
    TLorentzVector vTruthInvisTLV = xTruthParticle.p4() - truthInfo.m_vTruthVisTLV;
    static const SG::Decorator<double> decPtInvis("pt_invis");
    static const SG::Decorator<double> decEtaInvis("eta_invis");
    static const SG::Decorator<double> decPhiInvis("phi_invis");
    static const SG::Decorator<double> decMInvis("m_invis");
    decPtInvis(xTruthParticle)  = vTruthInvisTLV.Pt();
    decEtaInvis(xTruthParticle) = vTruthInvisTLV.Eta();
    decPhiInvis(xTruthParticle) = vTruthInvisTLV.Phi();
    decMInvis(xTruthParticle)   = vTruthInvisTLV.M();
  }

  if ( m_bWriteVisibleChargedFourMomentum )
  {
    static const SG::Decorator<double> decPtVisCharged("pt_vis_charged");
    static const SG::Decorator<double> decEtaVisCharged("eta_vis_charged");
    static const SG::Decorator<double> decPhiVisCharged("phi_vis_charged");
    static const SG::Decorator<double> decMVisCharged("m_vis_charged");
    decPtVisCharged(xTruthParticle)  = truthInfo.m_vTruthVisTLVCharged.Pt();
    decEtaVisCharged(xTruthParticle) = truthInfo.m_vTruthVisTLVCharged.Eta();
    decPhiVisCharged(xTruthParticle) = truthInfo.m_vTruthVisTLVCharged.Phi();
    decMVisCharged(xTruthParticle)   = truthInfo.m_vTruthVisTLVCharged.M();
  }

  if ( m_bWriteVisibleNeutralFourMomentum )
  {
    static const SG::Decorator<double> decPtVisNeutral("pt_vis_neutral");
    static const SG::Decorator<double> decEtaVisNeutral("eta_vis_neutral");
    static const SG::Decorator<double> decPhiVisNeutral("phi_vis_neutral");
    static const SG::Decorator<double> decMVisNeutral("m_vis_neutral");
    decPtVisNeutral(xTruthParticle)  = truthInfo.m_vTruthVisTLVNeutral.Pt();
    decEtaVisNeutral(xTruthParticle) = truthInfo.m_vTruthVisTLVNeutral.Eta();
    decPhiVisNeutral(xTruthParticle) = truthInfo.m_vTruthVisTLVNeutral.Phi();
    decMVisNeutral(xTruthParticle)   = truthInfo.m_vTruthVisTLVNeutral.M();
  }

  if ( m_bWriteDecayModeVector )
  {
    static const SG::Decorator<std::vector<int> > decDecayModeVector("DecayModeVector");
    decDecayModeVector(xTruthParticle) = truthInfo.m_vDecayMode;
  }

  if ( m_bWriteVertices )
  {
    // tau decay vertex
    static const SG::Decorator<float> decDecayVertexX("decay_vertex_x");
    static const SG::Decorator<float> decDecayVertexY("decay_vertex_y");
    static const SG::Decorator<float> decDecayVertexZ("decay_vertex_z"); 
   
    decDecayVertexX(xTruthParticle) = truthInfo.m_vDecayVertex.X();
    decDecayVertexY(xTruthParticle) = truthInfo.m_vDecayVertex.Y();
    decDecayVertexZ(xTruthParticle) = truthInfo.m_vDecayVertex.Z();
 
    // tau production vertex
    static const SG::Decorator<float> decProdVertexX("prod_vertex_x");
    static const SG::Decorator<float> decProdVertexY("prod_vertex_y");
    static const SG::Decorator<float> decProdVertexZ("prod_vertex_z");
    
    decProdVertexX(xTruthParticle) = truthInfo.m_vProdVertex.X();
    decProdVertexY(xTruthParticle) = truthInfo.m_vProdVertex.Y();
    decProdVertexZ(xTruthParticle) = truthInfo.m_vProdVertex.Z();
  }

  return StatusCode::SUCCESS;
}

//______________________________________________________________________________
StatusCode BuildTruthTaus::examineTruthTauDecay (const xAOD::TruthParticle& xTruthParticle,
                                                 TauTruthInfo& truthInfo) const
{
  // get vertex and check if it exists
  const xAOD::TruthVertex* xDecayVertex = xTruthParticle.decayVtx();
  if (!xDecayVertex)
    return StatusCode::SUCCESS;

  truthInfo.m_vDecayVertex.SetXYZ(xDecayVertex->x(),xDecayVertex->y(),xDecayVertex->z());

  if (xTruthParticle.hasProdVtx() ) {
     const xAOD::TruthVertex* xProdVertex = xTruthParticle.prodVtx();
     truthInfo.m_vProdVertex.SetXYZ(xProdVertex->x(),xProdVertex->y(),xProdVertex->z());
  } else {
     truthInfo.m_vProdVertex.SetXYZ(-1234,-1234,-1234);
  }  

  for ( size_t iOutgoingParticle = 0; iOutgoingParticle < xDecayVertex->nOutgoingParticles(); ++iOutgoingParticle )
  {
    const xAOD::TruthParticle* xTruthDaughter = xDecayVertex->outgoingParticle(iOutgoingParticle);
    if (xTruthDaughter == nullptr)
    {
      ATH_MSG_ERROR("Truth daughter of tau decay was not found in "<< m_truthParticleContainer.key() <<" container. Please ensure that this container has the full tau decay information or produce the TruthTaus container in AtlasDerivation.");
      return StatusCode::FAILURE;
    }

    int iAbsPdgId = xTruthDaughter->absPdgId();
    int iPdgId = xTruthDaughter->pdgId();

    // look at decay of unstable particles
    if (MC::isDecayed(xTruthDaughter)  || !MC::isPhysical(xTruthDaughter))
    {
      if ( iAbsPdgId != 111 && iAbsPdgId != 311 && iAbsPdgId != 310 && iAbsPdgId != 130 )
      {
        examineTruthTauDecay(*xTruthDaughter, truthInfo).ignore();
        continue;
      }
    }

    // only process stable particles
    if (!MC::isStable(xTruthDaughter) && !MC::isDecayed(xTruthDaughter))
      continue;

    // add pdgID to vector for decay mode classification
    truthInfo.m_vDecayMode.push_back(iPdgId);

    // if tau decays leptonically, indicated by an electron/muon neutrino then
    // it is not a hadronic decay
    if ( xTruthDaughter->isHadron() )
      truthInfo.m_bIsHadronicTau = true;

    // ignore neutrinos for further progress
    if ( xTruthDaughter->isNeutrino() )
    {
      ATH_MSG_VERBOSE("found neutrino decay particle with PdgId "<<iPdgId);
      continue;
    }

    // add momentum of non-neutrino particle to visible momentum
    truthInfo.m_vTruthVisTLV += xTruthDaughter->p4();
    if ( m_bWriteVisibleChargedFourMomentum )
      if ( xTruthDaughter->isCharged() )
        truthInfo.m_vTruthVisTLVCharged += xTruthDaughter->p4();
    if ( m_bWriteVisibleNeutralFourMomentum )
      if ( xTruthDaughter->isNeutral() )
        truthInfo.m_vTruthVisTLVNeutral += xTruthDaughter->p4();

    // only count charged decay particles
    if ( xTruthDaughter->isCharged() )
    {
      ATH_MSG_VERBOSE("found charged decay particle with PdgId "<<iPdgId);
      truthInfo.m_iNChargedDaughters++;
      // count charged pions
      if (iAbsPdgId==211) truthInfo.m_iNChargedPions++;
      else truthInfo.m_iNChargedOthers++;
    }
    else
    {
      ATH_MSG_VERBOSE("found neutral decay particle with PdgId "<<iPdgId);
      // count neutral pions
      if (iAbsPdgId==111) truthInfo.m_iNNeutralPions++;
      else truthInfo.m_iNNeutralOthers++;
    }
  }
  return StatusCode::SUCCESS;
}

void BuildTruthTaus::printDecay(const xAOD::TruthParticle& xTruthParticle, int depth) const
{
  // loop over all decay particles, print their kinematic and other properties

  const xAOD::TruthVertex* xDecayVertex = xTruthParticle.decayVtx();
  if (xDecayVertex == nullptr)
    return;

  for ( size_t iOutgoingParticle = 0; iOutgoingParticle < xDecayVertex->nOutgoingParticles(); ++iOutgoingParticle )
  {
    const xAOD::TruthParticle* xTruthDaughter = xDecayVertex->outgoingParticle(iOutgoingParticle);
    if (xTruthDaughter == nullptr)
    {
      ATH_MSG_WARNING("Truth daughter of tau decay was not found in "<< m_truthParticleContainer.key() <<" container. Please ensure that this container has the full tau decay information or produce the TruthTaus container in AtlasDerivation.");
      return;
    }
    ATH_MSG_WARNING("depth "<<depth
                    <<" e "<<xTruthDaughter->e()
                    <<" eta "<<xTruthDaughter->p4().Eta()
                    <<" phi "<<xTruthDaughter->p4().Phi()
                    <<" pdgid "<<xTruthDaughter->pdgId()
                    <<" status "<<xTruthDaughter->status()
                    <<" uniqueID "<<HepMC::uniqueID(xTruthDaughter));
    printDecay(*xTruthDaughter, depth+1);
  }
}
