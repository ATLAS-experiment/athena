/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// MuonClusterHypoAlg
// (see header for history/etc..)
//
#include "MuonClusterHypoAlg.h"

#include <cmath>
#include <algorithm>
#include <sstream>
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ITHistSvc.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "xAODTrigger/TrigComposite.h"
#include "xAODTrigger/TrigCompositeContainer.h"

#include "CLHEP/Units/SystemOfUnits.h"

class ISvcLocator;

using TrigCompositeUtils::createAndStore;
using TrigCompositeUtils::Decision;
using TrigCompositeUtils::DecisionContainer;
using TrigCompositeUtils::DecisionAuxContainer;
using TrigCompositeUtils::DecisionIDContainer;
using TrigCompositeUtils::decisionIDs;
using TrigCompositeUtils::newDecisionIn;
using TrigCompositeUtils::linkToPrevious;
using TrigCompositeUtils::viewString;
using TrigCompositeUtils::featureString;
using TrigCompositeUtils::findLink;
using TrigCompositeUtils::LinkInfo;
using TrigCompositeUtils::hypoAlgNodeName;
using TrigCompositeUtils::allFailed;

MuonClusterHypoAlg::MuonClusterHypoAlg(const std::string & name, ISvcLocator* pSvcLocator):
                          HypoBase(name, pSvcLocator){
}

MuonClusterHypoAlg::~MuonClusterHypoAlg(){
}

StatusCode MuonClusterHypoAlg::initialize()
{

    ATH_MSG_DEBUG("in initialize()");
    ATH_CHECK( m_outputCompositesKey.initialize() );

    if (!m_hypoTools.empty()) ATH_CHECK(m_hypoTools.retrieve());
    if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());


    ATH_MSG_INFO("initialize() success");
    return StatusCode::SUCCESS;
}

StatusCode MuonClusterHypoAlg::execute(const EventContext& ctx) const
{

    ATH_MSG_DEBUG("in execute()");

    auto CluNum = Monitored::Scalar("NumClu", 3);

    auto acceptAll     = Monitored::Scalar<bool>("AcceptAll", false);
    auto nRoIEndCap    = Monitored::Scalar<int>("nRoIEndCap", 4);
    auto nRoIBarrel    = Monitored::Scalar<int>("nRoIBarrel", 3);
    auto etaMax        = Monitored::Scalar<float>("maxEta", 2.5);
    auto etaMid        = Monitored::Scalar<float>("midEta", 1.0);
    auto isPass        = Monitored::Scalar<bool>("isPass", 0);

    auto t1            = Monitored::Timer("TIME_HypoAlg");
    auto t2            = Monitored::Timer("TIME_HypoAlg_DecisionLoop");

    t1.start();
    
    auto mon = Monitored::Group(m_monTool, acceptAll, nRoIEndCap, nRoIBarrel, etaMax, etaMid, CluNum, isPass, t1, t2);

    if(acceptAll) {
        ATH_MSG_DEBUG("Accepting all the events with no cuts!");
    } else {
        ATH_MSG_DEBUG("Selecting barrel muon RoI clusters with |eta|< " << etaMid << " and >= " << nRoIBarrel << " RoIs");
        ATH_MSG_DEBUG("and endcap muon RoI clusters with |eta| > : " << etaMid << "and >= " <<nRoIEndCap << " RoIs");
    }

    // Obtain incoming Decision Objects (one per L1 muon ROI)
    auto previousDecisionsHandle = SG::makeHandle( decisionInput(), ctx );
    ATH_CHECK( previousDecisionsHandle.isValid() );
    ATH_MSG_DEBUG( "Running with "<< previousDecisionsHandle->size() <<" previous decisions");

    // New output trigger decisions container
    SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), ctx);
    auto decisions = outputHandle.ptr();

    if (previousDecisionsHandle->size() == 0) {
        ATH_MSG_DEBUG( "No incoming decision objects, nothing to do");
        return StatusCode::SUCCESS;
    }

    // Get the TrigCompositeContainer linked to the ReadHandleKey
    // it contains the Muon RoI Cluster information
    auto compContHdl = SG::makeHandle(m_outputCompositesKey, ctx);
    auto compCont = compContHdl.get();

    if (compCont->size() == 0) {
        ATH_MSG_DEBUG( "No reconstructed muon-roi-cluster summary object, nothing to do");
        return StatusCode::SUCCESS;
    } else if (compCont->size() > 1) {
        ATH_MSG_ERROR( "This HypoAlg is only expecting exactly one reconstructed muon cluster object, but MuonCluster reconstruction has output " << compCont->size() << ". Cannot currently deal with this.");
        return StatusCode::FAILURE;
    }

    // Creating a DecisionIDContainer to hold all active chain IDs
    DecisionIDContainer prev;

    // Create new output decision object
    // as we have only one output physics object ("muCluster") and many inputs (L1 muon ROI) we link multiple parents
    Decision* d = newDecisionIn(decisions, hypoAlgNodeName());

    // Link our physics object ("muCluster")
    d->setObjectLink( featureString(), ElementLink<xAOD::TrigCompositeContainer>(*compCont, 0, ctx) );

    // Link our parents (L1 muon ROI)
    for (const Decision* previousDecision : *previousDecisionsHandle) {
        linkToPrevious(d, previousDecision, ctx);
        decisionIDs(previousDecision, prev);  // Collate active chains on this L1 ROI (previousDecision) into prev
    }
   
    // Creating the DecisionInfo struct to pass to the HypoTool
    MuonClusterHypoTool::DecisionInfo tool_info{d, compCont, std::move(prev)};

    t2.start();
    for ( auto& tool: m_hypoTools ){
        ATH_CHECK( tool->decide(tool_info) );
    }
    t2.stop();

    // We only have 1 decision. If more decisions are needed in the future, wrap 'if' in a loop over the decision container.
    if (!allFailed(d)) {isPass = 1;}    // allFailed returns true is d is empty, else returns false.
    
    ATH_CHECK(hypoBaseOutputProcessing(outputHandle));

    return StatusCode::SUCCESS;

}
