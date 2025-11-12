/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_UT_TAUANALYSISTOOLS_TEST_H
#define TAUANALYSISTOOLS_UT_TAUANALYSISTOOLS_TEST_H 1


#include "AthAnalysisBaseComps/AthAnalysisHelper.h" //tool creation and configuration
#include "POOLRootAccess/TEvent.h" //event looping
#include "GaudiKernel/ToolHandle.h" //for better working with tools

#include "AsgMessaging/MessageCheck.h" //messaging

//ROOT includes
#include "TString.h"
#include "TSystem.h"

// Local include(s):
#include "TauAnalysisTools/TauEfficiencyCorrectionsTool.h"
#include "TauAnalysisTools/TauSelectionTool.h"
#include "TauAnalysisTools/TauSmearingTool.h"
#include "TauAnalysisTools/TauTruthMatchingTool.h"

// EDM include(s):
#include "xAODTau/TauJetContainer.h"
#include "AthContainers/ConstAccessor.h"

#include "CxxUtils/checker_macros.h"

using namespace asg::msgUserCode;  //messaging

int main ATLAS_NOT_THREAD_SAFE ( int argc, char* argv[] )
{
  ANA_CHECK_SET_TYPE (int);

  IAppMgrUI* app = POOL::Init(); //important to do this first!

  // Open the input file:
  TString fileName = "$ASG_TEST_FILE_MC";
  if( argc < 2 )
  {
    ANA_MSG_WARNING( "No file name received, using $ASG_TEST_FILE_MC" );
  }
  else
  {
    fileName = argv[1]; //use the user provided file
  }
  ANA_MSG_INFO("Opening file: " << gSystem->ExpandPathName(fileName.Data()) );

  //Here's an example of how you would create a tool of type ToolType, and set a property on it
  //The preferred way to create and configure the tool is with a ToolHandle:
  //ToolHandle<IToolInterface> myTool("ToolType/myTool");
  //AthAnalysisHelper::setProperty( myTool, "MyProperty", value );
  //myTool.retrieve(); //this will cause the tool to be created and initialized

  // ===========================================================================
  // TauSelectionTool
  // ===========================================================================
  ToolHandle<TauAnalysisTools::ITauSelectionTool> TauSelTool("TauAnalysisTools::TauSelectionTool/TauSelectionTool");
  ANA_CHECK(AthAnalysisHelper::setProperty( TauSelTool, "OutputLevel", MSG::Level::DEBUG ));
  ANA_CHECK(AthAnalysisHelper::setProperty( TauSelTool, "SelectionCuts", int(
    TauAnalysisTools::CutPt |
    TauAnalysisTools::CutAbsEta |
    TauAnalysisTools::CutAbsCharge |
    TauAnalysisTools::CutNTrack |
    TauAnalysisTools::CutJetIDWP |
    TauAnalysisTools::CutMuonOLR
  )));
  ANA_CHECK(AthAnalysisHelper::setProperty( TauSelTool, "JetIDWP", int(TauAnalysisTools::JETIDRNNLOOSE))); 
  ANA_CHECK(AthAnalysisHelper::setProperty( TauSelTool, "MuonOLR", true));
  ANA_CHECK(TauSelTool.retrieve()); //this will cause the tool to be created and initialized

  // ===========================================================================
  // TauSmearingTool
  // ===========================================================================
  ToolHandle<TauAnalysisTools::ITauSmearingTool> TauSmeTool("TauAnalysisTools::TauSmearingTool/TauSmearingTool");
  ANA_CHECK(AthAnalysisHelper::setProperty(TauSmeTool, "Campaign", "mc20"));
  ANA_CHECK(TauSmeTool.retrieve());

  // ===========================================================================
  // TauEfficiencyCorrectionsTool
  // ===========================================================================
  std::vector<int> efficiency_correction_types;
  efficiency_correction_types.push_back(static_cast<int>(TauAnalysisTools::EfficiencyCorrectionType::SFJetIDHadTau));

  ToolHandle<TauAnalysisTools::ITauEfficiencyCorrectionsTool> TauEffCorrTool( "TauAnalysisTools::TauEfficiencyCorrectionsTool/TauEfficiencyCorrectionsTool" );
  ANA_CHECK(AthAnalysisHelper::setProperty( TauEffCorrTool, "JetIDLevel",  static_cast<int>(TauAnalysisTools::JetID::JETIDRNNLOOSE)));
  ANA_CHECK(AthAnalysisHelper::setProperty( TauEffCorrTool, "EfficiencyCorrectionTypes", efficiency_correction_types));
  ANA_CHECK(AthAnalysisHelper::setProperty( TauEffCorrTool, "Campaign", "mc20"));
  ANA_CHECK(TauEffCorrTool.retrieve());

  // ===========================================================================
  // TauTruthMatchingTool
  // ===========================================================================
  ToolHandle<TauAnalysisTools::ITauTruthMatchingTool> T2MT( "TauAnalysisTools::TauTruthMatchingTool/TauTruthMatchingTool");
  ANA_CHECK(AthAnalysisHelper::setProperty(T2MT, "TruthJetContainerName", "AntiKt4TruthDressedWZJets"));
  ANA_CHECK(T2MT.retrieve());

  // defining needed Container
  const xAOD::TauJetContainer* xTauJetContainer = nullptr;

  //loop over input file with POOL
  POOL::TEvent evt;
  ANA_CHECK(evt.readFrom( fileName ));

  // for(int i=0;i < evt.getEntries(); i++) {
  for(int i=0; i < 100; i++)
  {
    if( evt.getEntry(i) < 0)
    {
      ANA_MSG_ERROR("Failed to read event " << i);
      continue;
    }

    ANA_CHECK(evt.retrieve( xTauJetContainer, "TauJets" ));

    for ( auto xTau : *xTauJetContainer )
    {
      // perform truth matching
      auto xTruthTau = T2MT->getTruth(*xTau);

      // Select "good" taus:
      if( ! TauSelTool->accept( *xTau ) ) continue;

      // print some info about the selected tau:
      ANA_MSG_INFO( "Selected tau: pt = " << xTau->pt()
                    << " MeV, eta = " << xTau->eta()
                    << ", phi = " << xTau->phi()
                    << ", prong = " << int(xTau->nTracks())
                    << ", charge = " << int(xTau->charge()));

      static const SG::ConstAccessor<char> accIsTruthMatched ("IsTruthMatched");
      bool avail = accIsTruthMatched.isAvailable(*xTau);
      if (avail && (xTruthTau != nullptr))
      {
        if (xTruthTau->isTau())
        {
          static const SG::ConstAccessor<char> accIsHadronicTau ("IsHadronicTau");
          if (static_cast<bool>(accIsHadronicTau(*xTruthTau))) {
            static const SG::ConstAccessor<size_t> accNumCharged ("numCharged");
            ANA_MSG_INFO( "Tau was matched to a truth hadronic tau, which has " << int(accNumCharged(*xTruthTau))
                          << " prongs and a charge of " << int(xTruthTau->charge()));
          }
          else
            ANA_MSG_INFO( "Tau was matched to a truth leptonic tau, which has a charge of " << int(xTruthTau->charge()));
        }
        else if (xTruthTau->isElectron())
          ANA_MSG_INFO( "Tau was matched to a truth electron");
        else if (xTruthTau->isMuon())
          ANA_MSG_INFO( "Tau was matched to a truth muon");
      }
      else
        ANA_MSG_INFO( "Tau was not matched to truth" );

      typedef ElementLink< xAOD::TruthParticleContainer > Link_t;
      static const SG::ConstAccessor< Link_t > accTruthParticleLink("truthParticleLink");
      if (!accTruthParticleLink.isAvailable(*xTau))
      {
        ANA_MSG_WARNING("link truthParticleLink is not available");
        continue;
      }
      static const SG::ConstAccessor< ElementLink< xAOD::JetContainer > > accTruthJetLink("truthJetLink");
      auto xTruthJetLink = accTruthJetLink(*xTau);
      if (xTruthJetLink.isValid())
      {
        const xAOD::Jet* xTruthJet = *xTruthJetLink;
        ANA_MSG_INFO( "Tau was matched to a truth jet, which has pt = " << xTruthJet->p4().Pt()
                      << ", eta = " << xTruthJet->p4().Eta()
                      << ", phi = " << xTruthJet->p4().Phi()
                      << ", m = " << xTruthJet->p4().M() );
      }
      else
        ANA_MSG_INFO( "Tau was not matched to truth jet" );

      // test the TauEfficiencyCorrectionsTool
      ANA_CHECK(TauEffCorrTool->applyEfficiencyScaleFactor(*xTau));

      // test the TauSmearingTool
      xAOD::TauJet* xTauCopy = nullptr;
      ANA_CHECK(TauSmeTool->correctedCopy(*xTau, xTauCopy));
    }
  }
  ServiceHandle<IProperty> toolSvc("ToolSvc","");
  ANA_CHECK(toolSvc->setProperty("OutputLevel","1"));
  asg::msgToolHandle::setMsgLevel(MSG::Level::DEBUG);

  ANA_CHECK(app->finalize()); //trigger finalization of all services and tools created by the Gaudi Application
  return 0;
}
#endif //> !TAUANALYSISTOOLS_UT_TAUANALYSISTOOLS_TEST_H
