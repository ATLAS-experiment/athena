/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AsgTools/AsgTool.h"
#include "AsgMessaging/MessageCheck.h"
#include "AsgMessaging/MsgStream.h"
#include <AsgTools/StandaloneToolHandle.h>
#include <xAODBTaggingEfficiency/TruthTagResults.h>
#include "FTagAnalysisInterfaces/IBTaggingTruthTaggingTool.h"
#include "xAODBTaggingEfficiency/TruthTagResults.h"

#include <string>
#include <iomanip>
#include <vector>

ANA_MSG_HEADER(testBTaggingTruthTaggingTester)
ANA_MSG_SOURCE(testBTaggingTruthTaggingTester, "BTaggingTruthTaggingTester")
using namespace testBTaggingTruthTaggingTester;

int test1 ATLAS_NOT_THREAD_SAFE () {
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  asg::StandaloneToolHandle<IBTaggingTruthTaggingTool> tool("BTaggingTruthTaggingTool/BtagTT_Tool");

  //choose working point and CDI file
  ANA_CHECK(tool.setProperty( "TaggerName", "MV2c10" ));
  ANA_CHECK(tool.setProperty( "OperatingPoint", "FixedCutBEff_77" ));
  ANA_CHECK(tool.setProperty( "JetAuthor", "AntiKt4EMTopoJets" ));
  ANA_CHECK(tool.setProperty( "ScaleFactorFileName", "xAODBTaggingEfficiency/13TeV/2017-21-13TeV-MC16-CDI-2018-02-09_v1.root" ));

  //truth tagging settings
  ANA_CHECK(tool.setProperty( "IgnoreScaleFactors", false )); //use the data/MC and MC/MC scale factors when computing event weights
  ANA_CHECK(tool.setProperty( "UseSystematics", true )); // store event weights for all systematic variations in the results object
  ANA_CHECK(tool.setProperty( "MaxNtagged", 3 ));
  ANA_CHECK(tool.setProperty( "UsePermutations", true ));
  ANA_CHECK(tool.setProperty( "UseQuantile", true ));
  ANA_CHECK(tool.initialize());

  std::cout << "Initialization of tool " << tool->name() << " finished." << std::endl;

  //Set the jets to be used in the tool.
  std::vector<float> pt = {44000., 66000., 77000.};
  std::vector<float> eta = {2.2, 1.6, 1.7};
  std::vector<int> flav = {0,4,5};
  std::vector<float> tagw = {0.3, 0.55, 0.99};

  Analysis::TruthTagResults results;

  if(StatusCode::SUCCESS!=tool->CalculateResults( pt,eta,flav,tagw, results) ){
    std::cout << "failed to compute truth tagging results! " << std::endl;
    return -1;
  };

  //direct tagged results:
  std::cout << "Direct tagged results:  "<< std::endl;
  for(unsigned int i=0; i< results.is_tagged.size();i++)
  {
    std::cout << "jet "<< i << " is direct tagged: " << results.is_tagged.at(i) << std::endl;
  }
  //print the results for each of the systematic variations

  for(unsigned int systindex=0; systindex< results.syst_names.size(); systindex++){
      std::string syst_name = results.syst_names.at(systindex);

      std::cout << "direct tag Event SF ( "<< syst_name  <<" ) = " <<results.getEvtDirectTagSF(syst_name) << std::endl;

  }


  //truth tagging results:
  for(int ntags=1; ntags<=3;ntags++){

    std::cout << " for " << ntags << " tagged jets: " << std::endl;
    std::cout << " --------------------------------- " << std::endl;
    //print chosen premutation, tagweight bin assinment and event weight

    std::vector<bool> exclusive_permuation = results.getEventPermutation(ntags,true);
    std::vector<int> exclusive_tagweightBin = results.getEventQuantiles(ntags,true);

    std::vector<bool> inclusive_permuation = results.getEventPermutation(ntags,false);
    std::vector<int> inclusive_tagweightBin = results.getEventQuantiles(ntags,false);

    std::cout << "        exclusive    ||      inclusive" << std::endl;
    std::cout << "      tagged | bin   ||    tagged | bin" << std::endl;
    for(unsigned int i=0; i< exclusive_permuation.size();i++)
      {
        std::cout << "jet "<< i << "       " << exclusive_permuation.at(i) << " " << exclusive_tagweightBin.at(i)
         << "      ||          " <<  inclusive_permuation.at(i) << " " << inclusive_tagweightBin.at(i) << std::endl;
      }

    std::cout << " Nominal event weight exclusive: " << results.getEventWeight(ntags,true,"Nominal") << std::endl;
    std::cout << " Nominal event weight inclusive: " << results.getEventWeight(ntags,false,"Nominal") << std::endl;
    std::cout << " ---------------------------------\n\n " << std::endl;

  }

  return 0;
}


int main ATLAS_NOT_THREAD_SAFE()
{
  try {
    return test1();
  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}
