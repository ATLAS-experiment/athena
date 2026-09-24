/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/******************************************************************************
Name:        EventCleaningTool

Author:      Julia Gonski
Created:     Nov 2016

Description: Class for selecting events that pass recommended jet cleaning procedure
******************************************************************************/

// This class header and package headers
#include "JetSelectorTools/EventCleaningTool.h"
#include "JetSelectorTools/JetCleaningTool.h"
#include "JetSelectorTools/Helpers.h"

// The xAOD jet type
#include "xAODJet/Jet.h"
#include "xAODEventInfo/EventInfo.h"

// xAOD/ASG includes
#include "AsgMessaging/AsgMessaging.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"


// STL includes
#include <iostream>
#include <cmath>
#include <cfloat>

// ROOT includes
#include "TEnv.h"

namespace ECUtils {

//=============================================================================
// Constructors
//=============================================================================
EventCleaningTool::EventCleaningTool(const std::string& name)
  : asg::AsgTool(name)
  , m_jetCleaningTool("JetCleaningTool/JetCleaningTool")
{
  m_jetCleaningTool.declarePropertyFor(this, "JetCleaningTool");
}


//=============================================================================
// Destructor
//=============================================================================
EventCleaningTool::~EventCleaningTool() {}

//=============================================================================
// Initialize
//=============================================================================
StatusCode EventCleaningTool::initialize()
{
  if(m_passJvtKey.key() == "" || m_passORKey.key() == ""){
    ATH_MSG_ERROR( "Tool initialized with unknown decorator names." );
    return StatusCode::FAILURE;
  }
  if (m_cleaningLevel == ""){
    ATH_MSG_ERROR( "Tool initialized with unknown cleaning level." );
    return StatusCode::FAILURE;
  }
  if(m_jetContainerName == ""){
    ATH_MSG_ERROR( "Tool initialized with no jet container name." );
    return StatusCode::FAILURE;
  }

  // initialize jet cleaning tool if not empty.
  if (!m_jetCleaningTool.empty()) {
    ATH_CHECK(m_jetCleaningTool.setProperty("JetContainer", m_jetContainerName ));
    ATH_CHECK(m_jetCleaningTool.setProperty("CutLevel", m_cleaningLevel ));
    ATH_CHECK(m_jetCleaningTool.setProperty("UseDecorations", m_useDecorations ));  //for AODs we can't use decorations
    ATH_CHECK(m_jetCleaningTool.retrieve());
    ATH_MSG_INFO( "Event cleaning tool configured with cut level " << m_cleaningLevel  );
  }

  m_passJvtKey = m_jetContainerName + "." + m_prefix + m_passJvtKey.key();
  m_passORKey = m_jetContainerName + "." + m_prefix + m_passORKey.key();
  m_jetCleanKey = m_jetContainerName + "." + m_prefix + "jetClean_" + m_cleaningLevel;

  ATH_CHECK(m_passJvtKey.initialize());
  ATH_CHECK(m_passORKey.initialize());
  ATH_CHECK(m_jetCleanKey.initialize(m_decorate));

#ifndef XAOD_STANDALONE
  if(m_suppressInputDeps){
    // The user has promised that this will be produced by the same alg.
    // Tell the scheduler to ignore it to avoid circular dependencies.
    renounce(m_passJvtKey);
    renounce(m_passORKey);
  }
  if(m_suppressOutputDeps) {
    renounce(m_jetCleanKey);
  }
#endif

  return StatusCode::SUCCESS;
}

bool EventCleaningTool::acceptEvent(const xAOD::JetContainer* jets) const
{
	bool pass_pt = 0;
	bool pass_eta = 0;
	bool pass_accept = 0;
	int jvtDecision = 0;
	int orDecision = 0;
	bool isThisJetGood = 0;
	bool isEventAllGood = 1;

  SG::ReadDecorHandle<xAOD::JetContainer, char> jvtHandle(m_passJvtKey);
  SG::ReadDecorHandle<xAOD::JetContainer, char> orHandle(m_passORKey);
  SG::WriteDecorHandle<xAOD::JetContainer, char> jetCleanHandle(m_jetCleanKey);

	for (auto thisJet : *jets){  //loop over decorated jet collection
		pass_pt = thisJet->pt() > m_pt;
		pass_eta = fabs(thisJet->eta()) < m_eta;
		pass_accept = keepJet(*thisJet);
		jvtDecision = jvtHandle(*thisJet);
		orDecision = !(orHandle(*thisJet));  //recall, passOR==0 means that the jet is not an overlap and should be kept!

		ATH_MSG_DEBUG("Jet info: pT: " << pass_pt << ", eta: " << pass_eta << ", accept? " << pass_accept << ", jvt: " << jvtDecision << ", or: " << orDecision);
		if(pass_pt && pass_eta && jvtDecision && orDecision){//only consider jets for cleaning if they pass these requirements.
			isThisJetGood = pass_accept;
			isEventAllGood = isEventAllGood && isThisJetGood; //any event with a bad jet is rejected
		}
		else isThisJetGood = pass_accept;     //if it fails any one of these, it shouldn't be able to kill the whole event, but we still need to know cleaning
		ATH_MSG_DEBUG("Is jet good? " << isThisJetGood);
		if(m_decorate) jetCleanHandle(*thisJet) = isThisJetGood;
 	}
	ATH_MSG_DEBUG("Is event good? " << isEventAllGood);
	return isEventAllGood;
}

int EventCleaningTool::keepJet(const xAOD::Jet& jet) const
{
	return m_jetCleaningTool->keep(jet);
}

bool EventCleaningTool::passHSTPFilter(const xAOD::JetContainer* jets, const xAOD::JetContainer* puJets, const double jetThreshold) const
{
  /*
    QCD multijet (dijet) simulations face an ambiguity between HS and pileup jets, 
    since both originate from the same physics process. Combined with JZ sample slicing, 
    large in-time pileup in low-pT slices can cause events to leak into higher kinematic regimes, 
    leading to unphysical normalization in the detector-level jet spectrum. 
    The Hard-Scatter Softer Than Pile-up filter requires the HS jet to have higher pT than all pileup jets, 
    restoring a physical reconstructed jet pT spectrum.
    See: https://atlas-jetetmiss.docs.cern.ch/users/QCD-samples/#hard-scatter-softer-than-pileup-hstp-filter
  */

  // In the rare case of no HS truth jets in the event, assume it is close to the jetThreshold (default 5000 MeV)
  double maxHsJetPt = jetThreshold; 

  // Assume unsorted jetContainer, find leading jet
  for (const auto thisJet : *jets){
    if (thisJet->pt() > maxHsJetPt){
      maxHsJetPt = thisJet->pt();
    }
  }

  // Reject the event if any PU jet has a larger pT.
  // Assume unsorted jetContainer, compare if leading jet 
  for (const auto thisPUJet : *puJets){ 
    if (thisPUJet->pt() > maxHsJetPt){
      return false;
    }
  } 

  return true;
}
}//ECUtils


