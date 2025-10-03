/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/TEvent.h"
#else
#include "POOLRootAccess/TEvent.h"
#endif

#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "CalibrationDataInterface/CDIReader.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

#include <string>
#include <iomanip>
#include "TFile.h"


using CP::CorrectionCode;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// This executable cycles through the different calibrated flavour taggers                             //
//// in a CDI file, and tests out both the SFEigen and SFGlobalEigen systematic strategies              //
//// to compare their output and performance.                                                         //
//// Use this in unison with the 'validate_reduction' function in CalibrationDataEigenVariations.cxx//
//// to plot the bin-to-bin correlations - both for the "true" covariance matrix, and the         //
//// approximate covariance matrix constructed **after** the PCA reduction takes place          //
////////////////////////////////////////////////////////////////////////////////////////////////

// Further information on usage:
// This tool can be configured to test also the response, given a CDI file and a DAOD file.
// In this case, the CDI file must contain information for taggers which can operate on the objects in the DAOD file, 
// outdated CDI files cannot be used on newer DAODs and vice versa! This code will fail otherwise - to avoid this, you
// can filter down what combinations to study but adding some simple statements of the form "if(...) continue;", below

#ifdef XAOD_STANDALONE
xAOD::TEvent event(xAOD::TEvent::kClassAccess);
#else
POOL::TEvent event(POOL::TEvent::kClassAccess);
#endif


int main ATLAS_NOT_THREAD_SAFE () {
  bool retval = true;
  bool perform_jet_validation = false; // flag for turning on the 
  // systematic strategies to compare
  std::vector<std::string> strats;
  strats.push_back("SFEigen");
  strats.push_back("SFGlobalEigen");
  std::cout << "Starting up the SystematicStrategyComparison . . ." << std::endl;
  // set your CDI file path here
  std::string CDIfile = "/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/xAODBTaggingEfficiency/13TeV/2022-22-13TeV-MC20-CDI-2022-07-28_v1.root";
  
  if(perform_jet_validation){
    // set your own DAOD file (ideally a sample with many jets, like ttH), and select a suitable event below
    std::string DAODpath = "/DAODs/Example.pool.root.1"; // path to an example DAOD file, from which we can retrieve jets to test the BTagging tools
      //load some jets to show how to use the tool
    TFile* root_file = TFile::Open(DAODpath.c_str(),"read");
    if(!event.readFrom(root_file).isSuccess()){
      std::cout << "failed to load file" << std::endl;
      return -1;
    }
    event.getEntry(42);
  }
  
  Analysis::CDIReader Reader(CDIfile);
  for(const std::string& tag : Reader.getTaggers()){
      for(const std::string& jeta : Reader.getJetCollections(tag)){
        for(const std::string& wp : Reader.getWorkingPoints(tag, jeta)){
          for(const std::string& strat : strats ){
            asg::StandaloneToolHandle<IBTaggingEfficiencyTool> tool("BTaggingEfficiencyTool/SysStratTest");
            StatusCode code1 = tool.setProperty("ScaleFactorFileName", CDIfile); // set your CDI file here
            StatusCode code2 = tool.setProperty("TaggerName",    tag  );
            StatusCode code3 = tool.setProperty("OperatingPoint", wp);
            StatusCode code4 = tool.setProperty("JetAuthor",     jeta);
            StatusCode code5 = tool.setProperty("MinPt", 1);
            StatusCode code6 = tool.setProperty("SystematicsStrategy", strat);//either "SFEigen" or "SFGlobalEigen"
            StatusCode code7 = tool.setProperty("useFlexibleConfig", true);

            StatusCode code_init = tool.initialize();
            if (code_init != StatusCode::SUCCESS || code1 != StatusCode::SUCCESS || code2 != StatusCode::SUCCESS 
                 || code3 != StatusCode::SUCCESS || code4 != StatusCode::SUCCESS || code5 != StatusCode::SUCCESS  
                 || code6 != StatusCode::SUCCESS || code7 != StatusCode::SUCCESS)
              {
              std::cout << "Initialization of tool " << tool->name() << " failed! " << std::endl;
              return -1;
            }
            else {
              std::cout << "Initialization of tool " << tool->name() << " finished." << std::endl;
            }

            // select your efficiency map based on the DSID of your sample:
            unsigned int sample_dsid = 410464;

            tool->setMapIndex(sample_dsid);


            std::cout << "-----------------------------------------------------" << std::endl;
            const std::map<CP::SystematicVariation, std::vector<std::string> > allowed_variations = tool->listSystematics();
            std::cout << "Allowed systematics variations for tool " << tool->name() << ":" << std::endl;
            for (const auto& var : allowed_variations) {
              std::cout << std::setw(40) << std::left << var.first.name() << ":";
              for (const auto& flv : var.second) std::cout << " " << flv;
              std::cout << std::endl;
            }
            std::cout << "-----------------------------------------------------" << std::endl;

            if(perform_jet_validation){
              // retrieve the "real jets" of the jet collection in question
              const xAOD::JetContainer* jets = nullptr;
              if (!event.retrieve(jets, jeta).isSuccess()){ std::cout << " error retrieving jets " << std::endl; return -1;}

              // test with the jet!
              int jet_index = 0;
              for(const xAOD::Jet* jet : *jets){
                if(jet->pt() < 20000 or std::abs(jet->eta()) > 2.4) break; // any lower than this and you start seeing failed SF/Eff retrieval
                int truthlabel = -999;
                jet->getAttribute("HadronConeExclTruthLabelID",truthlabel);
                std::cout << "\n- - - - - - - - - - - - - - -  Jet " << jet_index << " - - - - - - - - - - - - - - - -" << std::endl;
                std::cout << " |- Jet index " << jet_index <<  " px = " << jet->px() << " py = " << jet->py() << " pz = " << jet->pz() << " e " << jet->e() << std::endl;
                std::cout << " \\_ pt = " << jet->pt() << " eta = " << jet->eta() << " phi = " << jet->phi() << " m " << jet->m() << std::endl;
                
                // Storage for sf/eff values
                float sf=0;
                float eff=0;
                CorrectionCode result;
                std::cout << "Testing function calls without systematics..." << std::endl;
                result = tool->getEfficiency(*jet,eff);
                if( result!=CorrectionCode::Ok) { std::cout << "b jet get efficiency failed"<<std::endl; retval=false;}
                else {
                  std::cout << "b jet get efficiency succeeded: " << eff << std::endl;
                }
                result = tool->getScaleFactor(*jet,sf);
                if( result!=CorrectionCode::Ok) { std::cout << "b jet get scale factor failed"<<std::endl; retval=false;}
                else {
                  std::cout << "b jet get scale factor succeeded: " << sf << std::endl;
                }

                std::cout << "Testing function calls with systematics..." << std::endl;
                const CP::SystematicSet& systs = tool->affectingSystematics();
                for(const auto& var : systs){
                  CP::SystematicSet set;
                  set.insert(var);
                  StatusCode sresult = tool->applySystematicVariation(set);
                  if( sresult !=StatusCode::SUCCESS) {
                    std::cout << var.name() << " apply systematic variation FAILED " << std::endl;
                  }
                  result = tool->getScaleFactor(*jet,sf);
                  if( result!=CorrectionCode::Ok) {
                    std::cout << var.name() << " getScaleFactor FAILED" << std::endl;
                  } else {
                    std::cout << var.name() << ": scale-factor = " << sf << std::endl;
                  }
                }
                // don't forget to switch back off the systematics...
                CP::SystematicSet defaultSet;
                StatusCode dummyResult = tool->applySystematicVariation(defaultSet);
                if (dummyResult != StatusCode::SUCCESS) std::cout << "problem disabling systematics setting!" << std::endl;
                
                jet_index += 1;
              }
            }
          } // end strats loop
        } // end wps loop
      } // end jets loop
    } // end taggers loop
  std::cout << " Great, now the SystematicStrategyComparison is finished! " << std::endl;

  return retval;
}
