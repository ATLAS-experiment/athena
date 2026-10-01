/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   MuonContainerMergingAlg   
//   author Maria Mironova, Simone Pagan Griso, Kehang Bai
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Generic algorithm for merging a list of muon containers
///////////////////////////////////////////////////////////////////
#include "MuonAnalysisAlgorithms/MuonContainerMergingAlg.h"
#include "xAODMuon/MuonContainer.h"
#include <xAODMuon/MuonAuxContainer.h>
#include <AthContainers/ConstDataVector.h>
#include <AsgDataHandles/WriteHandle.h>
#include <xAODBase/IParticleHelpers.h>

namespace CP {
    MuonContainerMergingAlg::MuonContainerMergingAlg( const std::string& name, ISvcLocator* svcLoc )
    : EL::AnaReentrantAlgorithm( name, svcLoc ){
        //nothing to do here
    }

    StatusCode MuonContainerMergingAlg::initialize() {

        // initialize list of muon containters
        ATH_MSG_DEBUG("Initializing MuonContainerMergingAlg");
        ATH_CHECK( m_inputMuonContainers.initialize() );
       
        // Default m_outMuonLocationCopy key to the OutputMuonLocation key
        if ( !m_createViewCollection.value() && m_outMuonLocationCopy.key().empty() ) { m_outMuonLocationCopy = m_outMuonLocationView.key(); }
        
        // initialize output muon containters
        ATH_CHECK( m_outMuonLocationCopy.initialize(!m_createViewCollection.value()) );
        ATH_CHECK( m_outMuonLocationView.initialize(m_createViewCollection.value()) );

        // Return gracefully:
        return StatusCode::SUCCESS;
    }

    StatusCode MuonContainerMergingAlg::execute(const EventContext &ctx) const  {

        // Setup containers for output, to avoid const conversions setup two different kind of containers
        std::unique_ptr<ConstDataVector<xAOD::MuonContainer>> outputViewCol;
        std::unique_ptr<xAOD::MuonContainer> outputCol;
        std::unique_ptr<xAOD::MuonAuxContainer> outputAuxCol;
        if(m_createViewCollection) {
            outputViewCol = std::make_unique<ConstDataVector<xAOD::MuonContainer>>(SG::VIEW_ELEMENTS);
        } else {
            outputCol = std::make_unique<xAOD::MuonContainer>();
            outputAuxCol = std::make_unique<xAOD::MuonAuxContainer>();
            outputCol->setStore(outputAuxCol.get());
        }

        // retrieve muons from StoreGate
        std::vector<const xAOD::MuonContainer*> muonCollections;
        muonCollections.reserve(m_inputMuonContainers.size());
        size_t muonCountSum = 0;
        for (const auto& mcname : m_inputMuonContainers){
          // Retrieve tracks from StoreGate
          SG::ReadHandle<xAOD::MuonContainer> muCol (mcname, ctx);
          ATH_CHECK(muCol.isValid());
          muonCollections.push_back(muCol.cptr());
          muonCountSum += muCol->size();
        }
        if (m_createViewCollection) {
          outputViewCol->reserve(muonCountSum);  
        } else {
          outputCol->reserve(muonCountSum);
        }

        /// Loop over input list of collections
        for (auto& mciter : muonCollections) {
          if (mciter and !mciter->empty()) {
            ATH_MSG_DEBUG("Size of muon collection " << mciter->size());
            /// Loop over elements of each muon collection
            for (const auto* const muon : *mciter) {
              /// Write them in the output (view) collection
              if (m_createViewCollection) {
                outputViewCol->push_back(muon);
              } else {
                auto newMuon = std::make_unique<xAOD::Muon>(*muon);
                setOriginalObjectLink(*muon, *newMuon);
                outputCol->push_back(std::move(newMuon));
              }
            }
          }
        }

        // write output to SG
        if (m_createViewCollection) {
          SG::WriteHandle< ConstDataVector<xAOD::MuonContainer> > h_write(m_outMuonLocationView, ctx);
          ATH_CHECK(h_write.record(std::move(outputViewCol)));
        }
        else {
          SG::WriteHandle<xAOD::MuonContainer> h_write(m_outMuonLocationCopy, ctx);
          ATH_CHECK(h_write.record(std::move(outputCol), std::move(outputAuxCol)));
        }

        return StatusCode::SUCCESS;

    }

}
