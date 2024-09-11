/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"
#include "TrigEventInfoRecorderAlg.h"

class ISvcLocator;

TrigEventInfoRecorderAlg::TrigEventInfoRecorderAlg(const std::string & name, ISvcLocator* pSvcLocator) 
: AthReentrantAlgorithm(name, pSvcLocator) 
{

}

StatusCode TrigEventInfoRecorderAlg::initialize()
{
    ATH_CHECK( m_TrigEventInfoKey.initialize() );

    ATH_CHECK( m_rhoDecor.initialize() );
    ATH_CHECK( m_rhoEMTDecor.initialize() );
    ATH_CHECK( m_muDecor.initialize() );
    ATH_CHECK( m_numPVDecor.initialize() );

    // initialize only the ReadHandleKeys needed by the sequence that called this algorithm
    // initialize(false) for variables not used in the present instantiation
    ATH_CHECK(m_PrimaryVxInputName.initialize(m_decoratePFlowInfo));
    ATH_CHECK(m_rhoKeyPF.initialize(m_decoratePFlowInfo));
    ATH_CHECK(m_rhoKeyEMT.initialize(m_decorateEMTopoInfo));

    if (!m_decorateEMTopoInfo || m_renounceAll) {
        renounce(m_rhoKeyEMT);
    }
    if (!m_decoratePFlowInfo || m_renounceAll) {
        renounce(m_rhoKeyPF);
        renounce(m_PrimaryVxInputName);
    }
    
    
    return StatusCode::SUCCESS;
}


StatusCode TrigEventInfoRecorderAlg::execute(const EventContext& context) const
{
     
     // Create new TrigComposite(Aux)Container and link them together
    auto trigEventInfoContainer = std::make_unique< xAOD::TrigCompositeContainer>();
    auto trigEventInfoContainerAux  = std::make_unique< xAOD::TrigCompositeAuxContainer>();
    
    trigEventInfoContainer->setStore(trigEventInfoContainerAux.get());

    // Create TrigComposite "object" that will serve as the anchor for the decorations
    xAOD::TrigComposite * trigEI = new xAOD::TrigComposite();
    trigEventInfoContainer->push_back(trigEI);
    
    SG::WriteHandle<xAOD::TrigCompositeContainer> trigEventInfoHandle(m_TrigEventInfoKey, context);
    ATH_CHECK(trigEventInfoHandle.record( std::move( trigEventInfoContainer ),
                                          std::move( trigEventInfoContainerAux ) ) );

    // Retrieve event info variables and decorate the TrigComposite object with them
    ATH_CHECK( decorateWithEventInfo(context, trigEI) );

    return StatusCode::SUCCESS;
}

StatusCode TrigEventInfoRecorderAlg::decorateWithEventInfo(const EventContext& context, xAOD::TrigComposite* trigEI) const
{
    // Structure of the function: First retrieve all variables of interest from their respective containers
    // in StoreGate, then decorate the TrigComposite Object passed down from the execute

    // Average Interactions per bunch crossing
    float avgmu = m_lumiBlockMuTool->averageInteractionsPerCrossing(context);
    
    double rho=0, rho_EMT = 0;
    const xAOD::EventShape * eventShape = 0;
    int NPV = 0;

    // pflow jet decorations
    if (m_decoratePFlowInfo) {
        
        SG::ReadHandle<xAOD::EventShape> rhRhoKey_PF(m_rhoKeyPF, context);
        if ( rhRhoKey_PF.isValid() )
        {
            ATH_MSG_VERBOSE("Found event density container "<<m_rhoKeyPF);
            eventShape = rhRhoKey_PF.cptr();

            if ( !eventShape->getDensity( xAOD::EventShape::Density, rho ) ) {
                ATH_MSG_WARNING("Event density not found in handle, but handle found.");
                ATH_MSG_FATAL("Could not retrieve xAOD::EventShape::Density from xAOD::EventShape.");
                return StatusCode::FAILURE;
             }
        }
        else {
            if (!m_renounceAll) ATH_MSG_ERROR("EventShape handle "<<m_rhoKeyPF<<" not found.");
        }
        ATH_MSG_DEBUG("Retrieved Jet Density Rho(PFlow): " << rho);

        // Number of primary vertices: just counting the number of 'good' vertices in HLT_IDVertex_FS container
        SG::ReadHandle<xAOD::VertexContainer> vtxCont(m_PrimaryVxInputName, context);
        const xAOD::Vertex* privtx = nullptr;
        if( vtxCont.isValid() ){
           const xAOD::VertexContainer* vertex_container = vtxCont.get();

           for (unsigned int i_vtx = 0; i_vtx <  vertex_container->size(); i_vtx++)
           {
            privtx = vertex_container->at(i_vtx);
            if ( privtx->nTrackParticles() >= 2)  NPV++;
           }

        }
        else {
           if (!m_renounceAll) ATH_MSG_ERROR( "Couldn't retrieve primary vertex container "<< m_PrimaryVxInputName<<". NPV will be 0" );
        }
    }
    
    if (m_decorateEMTopoInfo) {
        SG::ReadHandle<xAOD::EventShape> rhRhoKey_EMT(m_rhoKeyEMT, context);
        if ( rhRhoKey_EMT.isValid() )
        {
            ATH_MSG_VERBOSE("Found event density container HLT_Kt4EMTopoEventShape");
            eventShape = rhRhoKey_EMT.cptr();
            if ( !eventShape->getDensity( xAOD::EventShape::Density, rho_EMT ) ) {
                ATH_MSG_WARNING("Event density not found in handle, but handle found.");
                ATH_MSG_FATAL("Could not retrieve xAOD::EventShape::Density from xAOD::EventShape.");
                return StatusCode::FAILURE;
            }
        }
        else {
            if (!m_renounceAll) ATH_MSG_ERROR("EventShape handle "<<m_rhoKeyEMT<<" not found.");
        }
        ATH_MSG_DEBUG("Retrieved Jet Density Rho(EMTopo): " << rho_EMT);
    }

    // Now decorate the TrigComposite object with the variables retrieved above
    ATH_MSG_DEBUG("Setting PF JetDensity to " << rho);
    SG::makeHandle<double>(m_rhoDecor, context)(*trigEI) = rho;
    ATH_MSG_DEBUG("Setting EMT JetDensity to " << rho_EMT);
    SG::makeHandle<double>(m_rhoEMTDecor, context)(*trigEI) = rho_EMT;
    ATH_MSG_DEBUG("Setting AverageMu to " << avgmu);
    SG::makeHandle<float>(m_muDecor, context)(*trigEI) = avgmu;
    ATH_MSG_DEBUG("Setting NPV to " << NPV);
    SG::makeHandle<int>(m_numPVDecor, context)(*trigEI) = NPV;

    return StatusCode::SUCCESS;
}
