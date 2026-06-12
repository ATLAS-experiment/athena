///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "METRemappingAlg.h"

#include <memory>

namespace DerivationFramework {

  METRemappingAlg::METRemappingAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthAlgorithm(name, pSvcLocator)
  {

  }

  StatusCode METRemappingAlg::initialize()
  {
    ATH_MSG_VERBOSE("METRemappingAlg::initialize()");

    ATH_CHECK( m_jetContKey.initialize() );
    ATH_CHECK( m_photonContKey.initialize() );
    ATH_CHECK( m_electronContKey.initialize() );
    ATH_CHECK( m_muonContKey.initialize() );
    ATH_CHECK( m_tauContKey.initialize() );

    ATH_CHECK( m_inputMapKey.initialize() );
    ATH_CHECK( m_inputCoreKey.initialize() );
    ATH_CHECK( m_outputMapKey.initialize() );
    ATH_CHECK( m_outputCoreKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode METRemappingAlg::execute(const EventContext& ctx)
  {
    ATH_MSG_VERBOSE("METRemappingAlg::execute()");


    SG::ReadHandle<xAOD::JetContainer> jetContHandle(m_jetContKey, ctx);
    if( !jetContHandle.isValid() ) {
      ATH_MSG_ERROR("Unable to retrieve input jet container " << m_jetContKey.key());
      return StatusCode::FAILURE;
    }

    // first iterate through the AnalysisJets container and populate a map
    // that links original Jet objects to their calibrated counterparts
    std::map<const xAOD::Jet*, ElementLink<xAOD::JetContainer> > jetLinkMap;
    for( const xAOD::IParticle *j : *jetContHandle ) {
      if( !m_accOriginalObject.isAvailable(*j) ) {
        ATH_MSG_ERROR("originalObjectLink not available!");
        return StatusCode::FAILURE;
      }
      const xAOD::IParticle *orig = *m_accOriginalObject(*j);
      ElementLink<xAOD::JetContainer> link(*jetContHandle, j->index());
      jetLinkMap.try_emplace(
        static_cast<const xAOD::Jet*>(orig),
        link
      );
    }

    // repeat for Photon/Electron/Muon/Tau containers
    linkMap_t objectLinkMap;
    SG::ReadHandle<xAOD::PhotonContainer> photonContHandle(m_photonContKey, ctx);
    ATH_CHECK( fillLinkMap(objectLinkMap, photonContHandle) );

    SG::ReadHandle<xAOD::ElectronContainer> electronContHandle(m_electronContKey, ctx);
    ATH_CHECK( fillLinkMap(objectLinkMap, electronContHandle) );

    SG::ReadHandle<xAOD::MuonContainer> muonContHandle(m_muonContKey, ctx);
    ATH_CHECK( fillLinkMap(objectLinkMap, muonContHandle) );

    SG::ReadHandle<xAOD::TauJetContainer> tauContHandle(m_tauContKey, ctx);
    ATH_CHECK( fillLinkMap(objectLinkMap, tauContHandle) );

    // now retrieve and iterate through the METmap from PHYS and
    // use its contents as a baseline to populate our own
    SG::ReadHandle<xAOD::MissingETAssociationMap> inputMapHandle(m_inputMapKey, ctx);
    if( !inputMapHandle.isValid() ) {
      ATH_MSG_ERROR("Unable to retrieve input MissingETAssociationMap " << m_inputMapKey.key());
      return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::MissingETAssociationMap> outputMapHandle = SG::makeHandle(m_outputMapKey, ctx);
    ATH_CHECK( outputMapHandle.record(
                 std::make_unique<xAOD::MissingETAssociationMap>(),
                 std::make_unique<xAOD::MissingETAuxAssociationMap>()
                                      ));
    ATH_CHECK( outputMapHandle.isValid() );

    const ElementLink<xAOD::IParticleContainer> invalidLink;
    for( const xAOD::MissingETAssociation *el : *inputMapHandle ) {
      // copy constructor creates a deep copy
      auto assoc = outputMapHandle->push_back(new xAOD::MissingETAssociation(*el));

      if( !assoc->isMisc() ) {
        // check if the reference jet has a calibrated equivalent that should be linked to instead
        std::map<const xAOD::Jet*, ElementLink<xAOD::JetContainer> >::const_iterator jet_it = jetLinkMap.find(assoc->refJet());
        if( jet_it != jetLinkMap.end() ) {
          // relink to calibrated jet
          assoc->setJetLink(jet_it->second);
          
          // update objectLinks for this association
          MissingETBase::Types::objlink_vector_t objectLinks;
          for( const ElementLink<xAOD::IParticleContainer> &link : assoc->objectLinks() ) {
            if( !link.isValid() ) {
              objectLinks.push_back(invalidLink);
              continue;
            }

            linkMap_t::const_iterator obj_it = objectLinkMap.find(*link);
            if( obj_it != objectLinkMap.end() ) {
              objectLinks.emplace_back(obj_it->second);
            } else {
              // objects that aren't found in the map were selected away,
              // but we should leave an invalid link to maintain index order
              objectLinks.push_back(invalidLink);
            }
          }
          assoc->setObjectLinks(objectLinks);

        } else { // jet_it == jetLinkMap.end()
          // jet was selected away - this case should not happen, just give an error for now
          ATH_MSG_ERROR("Jet not found!");
          return StatusCode::FAILURE;
        }
      } else { // assoc->isMisc() == true
        // update links in the misc association
        MissingETBase::Types::objlink_vector_t miscObjectLinks;
        for( const ElementLink<xAOD::IParticleContainer> &link : assoc->objectLinks() ) {
          if( !link.isValid() ) {
            miscObjectLinks.push_back(invalidLink);
            continue;
          }
          
          linkMap_t::const_iterator obj_it = objectLinkMap.find(*link);
          if( obj_it != objectLinkMap.end() ) {
            miscObjectLinks.emplace_back(obj_it->second);
          } else {
            miscObjectLinks.push_back(invalidLink);
          }
        }
        assoc->setObjectLinks(miscObjectLinks);
      }
    } //> end loop over METmap

    // copy over the MET core container
    SG::ReadHandle<xAOD::MissingETContainer> inputCoreHandle(m_inputCoreKey, ctx);
    if( !inputCoreHandle.isValid() ) {
      ATH_MSG_ERROR("Unable to retrieve input MET core container " << m_inputCoreKey.key());
      return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::MissingETContainer> outputCoreHandle = SG::makeHandle(m_outputCoreKey, ctx);
    ATH_CHECK( outputCoreHandle.record(
                 std::make_unique<xAOD::MissingETContainer>(),
                 std::make_unique<xAOD::MissingETAuxContainer>()
                                       ));
    ATH_CHECK( outputCoreHandle.isValid() );

    for( const xAOD::MissingET *el : *inputCoreHandle ) {
      outputCoreHandle->push_back(new xAOD::MissingET(*el));
    }

    return StatusCode::SUCCESS;
  }

  template<typename handle_t> 
  StatusCode METRemappingAlg::fillLinkMap(linkMap_t &map, handle_t &handle)
  {
    if( !handle.isValid() ) {
      ATH_MSG_ERROR("Unable to retrieve " << handle.key());
      return StatusCode::FAILURE;
    }

    for( const xAOD::IParticle *obj : *handle ) {
      if( !m_accOriginalObject.isAvailable(*obj) ) {
        ATH_MSG_ERROR("originalObjectLink not available!");
        return StatusCode::FAILURE;
      }
      const xAOD::IParticle *orig = *m_accOriginalObject(*obj);
      ElementLink<xAOD::IParticleContainer> link(*handle,obj->index());
      map.try_emplace(orig, link);
    }
    return StatusCode::SUCCESS;
  }

} //> end namespace DerivationFramework
