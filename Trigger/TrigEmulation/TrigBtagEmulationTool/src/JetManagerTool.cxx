/*
Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration 
*/

#include "src/JetManagerTool.h"
#include "AsgDataHandles/ReadHandle.h"
#ifdef XAOD_STANDALONE
#include "CxxUtils/crc64.h"
#else
#include "SGTools/StringPool.h"
#endif

#include <format>

namespace {
  // StoreGate's key of a name. AnalysisBase has no function for it (xAOD::Event::stringToKey
  // hashes names it does not know with std::hash), so there it is a copy of SG::StringPool::stringToKey.
  SG::sgkey_t storeGateKey(const std::string& name, CLID clid) {
#ifdef XAOD_STANDALONE
    constexpr SG::sgkey_t keyMask = (SG::sgkey_t{1} << 30) - 1;
    return CxxUtils::crc64addint(CxxUtils::crc64(name), clid) & keyMask;
#else
    SG::StringPool pool;
    return pool.stringToKey(name, clid);
#endif
  }
}

namespace Trig {

//**********************************************************************

JetManagerTool::JetManagerTool(const std::string& name) 
  : asg::AsgTool(name)
{}
  
StatusCode JetManagerTool::initialize() {
  ATH_MSG_DEBUG( "Initializing " << name() );

  m_jetInputKey = m_jetcontainer.value();

  ATH_CHECK(m_jetInputKey.initialize( SG::AllowEmpty ));

  m_bjetInputKey = m_jetcontainer.value() + "_bJets";
  ATH_CHECK(m_bjetInputKey.initialize( !m_jetInputKey.key().empty() and m_LHCPeriod == 3 ));

  ATH_CHECK(m_btagInputKey.initialize(m_LHCPeriod == 2));

  const bool isRun2GSC = m_LHCPeriod == 2 and m_jetInputKey.key().find("GSC") != std::string::npos;
  ATH_CHECK(m_splitJetInputKey.initialize(isRun2GSC));
  if (m_LHCPeriod == 2) {
    m_onlineJetIndex = onlineJetIndex(m_jetInputKey.key());
    if (isRun2GSC) m_onlineSplitJetIndex = onlineJetIndex(m_splitJetInputKey.key());
  }

  return StatusCode::SUCCESS;
}
  
StatusCode JetManagerTool::retrieveByContainer(const EventContext& ctx,
					       EmulContext& emulCtx) const
{
  ATH_MSG_DEBUG( "Retrieving via Container ..." );

  auto outputJets = std::make_unique<std::vector<TrigBtagEmulationJet>>();
  auto sortedPreselJets = std::make_unique<std::vector<TrigBtagEmulationJet>>();

  // Get Jet Objects
  // We are retrieving xAOD::Jet object
  // This will first retrieve the jets in the event given the jet input key <jet-key>
  // then it retrieves the corresponding b-jet collection <jet-key>_bJets
  // replacing the non-b-jets with the b-jets

  // Retrieve jets
  ATH_MSG_DEBUG("Retrieving jet collection: " << m_jetInputKey.key());
  SG::ReadHandle< xAOD::JetContainer > jetContainerHandle = SG::makeHandle( m_jetInputKey, ctx );
  ATH_CHECK(jetContainerHandle.isValid());
  const xAOD::JetContainer* theJetContainer = jetContainerHandle.cptr();
  // Put in storage    
  outputJets->reserve(theJetContainer->size());
  sortedPreselJets->reserve(theJetContainer->size());

  if (m_LHCPeriod == 3) {
    // Retrieve jets
    for ( const xAOD::Jet *jet : *theJetContainer ) {
      TrigBtagEmulationJet toAdd(*jet, m_btagging_link.value());
      outputJets->push_back( toAdd ); 
    }

    // Retrieve b-Jets
    ATH_MSG_DEBUG("Retrieving b-jet collection: " << m_bjetInputKey.key());
    SG::ReadHandle< xAOD::JetContainer > bjetContainerHandle = SG::makeHandle( m_bjetInputKey, ctx );
    // if not valid, it means there is no corresponding b-jet collection
    // this happens for instance for presel jets
    // so this is ok
    if ( bjetContainerHandle.isValid() ) { 
      ATH_CHECK( bjetContainerHandle.isValid() );
      const xAOD::JetContainer* theBJetContainer = bjetContainerHandle.cptr();
  
      // Replace jets with b-jets
      // - loop on bjets
      for ( const xAOD::Jet *bjet : *theBJetContainer ) {
        // - loop on stored jets
        for (std::size_t ijet(0); ijet < outputJets->size(); ijet++) {
          const auto &emuljet = outputJets->at(ijet);
          const xAOD::Jet* jet = emuljet.jet();

          // To-Do: find better way
          if (bjet->pt() == jet->pt() && bjet->eta() == jet->eta() &&
              bjet->phi() == jet->phi()) {
            outputJets->at(ijet) = TrigBtagEmulationJet(*bjet, m_btagging_link.value());
            break;
          }
        }
      }
    } // is valid
  
    // Prepare presel jets
    for ( const auto& jet : *outputJets.get() ) {
      const xAOD::Jet *theJet = jet.jet();
      sortedPreselJets->push_back( TrigBtagEmulationJet(*theJet) );
    }


    // Sort presel jets
    sort(sortedPreselJets->begin(), sortedPreselJets->end(), 
         [] (const auto& lhs, const auto& rhs) -> bool
         { return lhs.pt() > rhs.pt(); }
         );

    ATH_MSG_DEBUG( " - Ten largest jets:");
    for(unsigned int i = 0; i < 10 and i < sortedPreselJets->size(); i++) {
      ATH_MSG_DEBUG( " - pt=" << (sortedPreselJets->at(i).pt() * 0.001) << " eta=" << sortedPreselJets->at(i).eta() );
    }
  }
  else if (m_LHCPeriod == 2) {
    if (jetContainerName().find("a4tcemsubjes") != std::string::npos) {
      ATH_MSG_DEBUG(jetContainerName() << " is not a b-jet collection. Do not retrieve b-tagging information.");
      for ( const xAOD::Jet *jet : *theJetContainer ) {
        TrigBtagEmulationJet toAdd(*jet, nullptr);
        outputJets->push_back( toAdd ); 
      }

    } else {

      // Retrieve b-tagging information
      // based on https://gitlab.cern.ch/atlas-trigger/b-jet/TrigBtagEmulationTool/-/blob/21.2/Root/JetManager.cxx#L423
      ATH_MSG_DEBUG("Retrieving b-tagging collection: " << m_btagInputKey.key());
      SG::ReadHandle< xAOD::BTaggingContainer > btagContainerHandle = SG::makeHandle( m_btagInputKey, ctx );
      ATH_CHECK(btagContainerHandle.isValid());
      const xAOD::BTaggingContainer *theBTagContainer = btagContainerHandle.cptr();
      ATH_MSG_DEBUG("jet container size: " << theJetContainer->size() << ", btag container size: " << theBTagContainer->size());

      if (msgLvl(MSG::DEBUG)) {
        for (const xAOD::Jet *jet : *theJetContainer) {
          ATH_MSG_DEBUG("Jet pt=" << (jet->pt() * 0.001) << " eta=" << jet->eta());
        }
      }

      bool isGSCchain = jetContainerName().find("GSC")!=std::string::npos;

      // for GSC chains the b-tagged jet can be a split jet
      const xAOD::JetContainer *theSplitJetContainer = nullptr;
      if (isGSCchain) {
        SG::ReadHandle< xAOD::JetContainer > splitJetContainerHandle = SG::makeHandle( m_splitJetInputKey, ctx );
        if (splitJetContainerHandle.isValid()) theSplitJetContainer = splitJetContainerHandle.cptr();
      }

      for ( const xAOD::BTagging *btag : *theBTagContainer ) {
        //coverity[UNNECESSARY_STRING_COPY:FALSE]
        static const SG::ConstAccessor<std::vector<ElementLink<xAOD::IParticleContainer>>> btagToJetAccessor("BTagBtagToJetAssociator");
        if (btagToJetAccessor.isAvailable(*btag)) {
          const auto &jetLink = btagToJetAccessor(*btag);
          if (jetLink.size() > 0) {
            const xAOD::Jet *btaggedJet = nullptr;
            if (jetLink.at(0).isValid()) {
              btaggedJet = static_cast<const xAOD::Jet *>(*jetLink.at(0));
            } else {
              // data: the link still points to the per-RoI jet collection of the HLT
              btaggedJet = onlineJet(m_onlineJetIndex, jetLink.at(0).persKey(), theJetContainer);
              if (btaggedJet == nullptr)
                btaggedJet = onlineJet(m_onlineSplitJetIndex, jetLink.at(0).persKey(), theSplitJetContainer);
            }
            if (btaggedJet == nullptr) {
              ATH_MSG_DEBUG("BTagging object without a jet in " << jetContainerName());
              continue;
            }
            double mv2c20_score, mv2c10_score;
            btag->MVx_discriminant("MV2c20", mv2c20_score);
            btag->MVx_discriminant("MV2c10", mv2c10_score);
            ATH_MSG_DEBUG("BTagging jet link index "
                          << jetLink.at(0).index() << " pt=" << (btaggedJet->pt() * 0.001) << " eta=" << btaggedJet->eta()
                          << ", mv2c20=" << mv2c20_score << ", mv2c10=" << mv2c10_score);

            // First check if the btagged jet is present in the jets retrieved by // the container
            const xAOD::Jet *matchedJet = nullptr;

            bool isJetPresent = false;
            for (const xAOD::Jet *theJet : *theJetContainer) {

              if ((!isGSCchain && btaggedJet == theJet) || // For non-GSC chains check if the btagged and container jets are the same
                  (isGSCchain && matchedSPLITjet( btaggedJet, theJet))) { // For GSC chains check if the container jets satisfy the dR matching with the btagged jet
                matchedJet = theJet;
                isJetPresent = true;
                break;
              }
            }
            if (matchedJet == nullptr) {
              ATH_MSG_DEBUG("Matched jet pointer is invalid...");
              continue;
            } else {
              ATH_MSG_DEBUG("Matched jet found: " << matchedJet);
            }

            // Check if the linked Jet has already been found
            bool isJetUnique = true;
            for (TrigBtagEmulationJet &j : *outputJets.get())
              if (matchedJet->p4().Et() == j.et() &&
                  matchedJet->eta() == j.eta() && matchedJet->phi() == j.phi()) {
                isJetUnique = false;
              }

            // Save Jet and BTagging objects if jet is found and unique
            if (isJetPresent && isJetUnique) {
              outputJets->push_back(TrigBtagEmulationJet(*matchedJet, btag));
            }
          }
        }
      }
    }
  }

  // Store objects
  const std::string storage_name = m_jetInputKey.key();
  emulCtx.store( storage_name, std::move(outputJets) );
  if (m_LHCPeriod == 3) {
    emulCtx.store( storage_name + "_presel", std::move(sortedPreselJets) );
  }

  return StatusCode::SUCCESS;
}

const std::vector<TrigBtagEmulationJet>& JetManagerTool::getJets(const EmulContext& emulCtx) const 
{
  return *emulCtx.get<std::vector<TrigBtagEmulationJet>>(m_jetcontainer.value());
}
const std::vector<TrigBtagEmulationJet>& JetManagerTool::getSortedPreselJets(const EmulContext& emulCtx) const 
{ 
  return *emulCtx.get<std::vector<TrigBtagEmulationJet>>(m_jetcontainer.value() + "_presel");
}

// In Run 2 data the b-tagging objects still link to the per-RoI jet collections of the HLT,
// "HLTAutoKey_<label>_<CLID>_<sub-type>_to_<n>" (HLT::IHolder::generateAliasKey), which are
// not in the file. A link keeps only the StoreGate key of that name, and jet n of it is jet n
// of the merged HLT_xAOD__JetContainer_<label>, so we hash all such names once and look it up.
JetManagerTool::OnlineJetIndex JetManagerTool::onlineJetIndex(const std::string& jetContainerName) {
  const CLID clid = ClassID_traits<xAOD::JetContainer>::ID();
  static const std::string containerPrefix = "HLT_xAOD__JetContainer_";
  OnlineJetIndex index;
  if (jetContainerName.rfind(containerPrefix, 0) != 0) return index;
  const std::string label = jetContainerName.substr(containerPrefix.size());
  // the sub-type differs between events
  for (unsigned int subType = 0; subType < 64; subType++) {
    for (std::size_t n = 0; n < 64; n++) {
      index.emplace(storeGateKey(std::format("HLTAutoKey_{}_{}_{}_to_{}", label, clid, subType, n), clid), n);
    }
  }
  return index;
}

const xAOD::Jet* JetManagerTool::onlineJet(const OnlineJetIndex& index, SG::sgkey_t linkKey,
                                           const xAOD::JetContainer* jets) {
  if (jets == nullptr) return nullptr;
  const auto found = index.find(linkKey);
  if (found == index.end() or found->second >= jets->size()) return nullptr;
  return jets->at(found->second);
}

bool JetManagerTool::matchedSPLITjet(const xAOD::Jet *splitJet,
                                 const xAOD::Jet *gscJet) const {
  return splitJet->p4().DeltaR( gscJet->p4() ) < 0.05;
}


}

//**********************************************************************
