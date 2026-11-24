#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgTools/ToolHandle.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include "xAODBTaggingEfficiency/BTaggingGridBoundsHandler.h"
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
using json = nlohmann::ordered_json;

// Define alias for the TEvent class 
// which is not the same for AnalysisBase and AthAnalysis
#ifdef XAOD_STANDALONE
// Those lines are only included if using AnalysisBase
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
using TEVENT = xAOD::TEvent;
#else
// Those lines are only included if using AthAnalysis or Athena
#include "POOLRootAccess/TEvent.h"
using TEVENT = POOL::TEvent;
#endif


ANA_MSG_HEADER(testBTaggingGridBoundsHandlerTester)
ANA_MSG_SOURCE(testBTaggingGridBoundsHandlerTester, "BTaggingGridBoundsHandlerTester")
using namespace testBTaggingGridBoundsHandlerTester;


float getJetQuantity(const xAOD::Jet & jet, const BTaggingToolUtil::varType vType){
 if (vType == BTaggingToolUtil::varType::pT) {
    return BTaggingToolUtil::MeVToGeV( jet.pt() ); 
  } else if (vType == BTaggingToolUtil::varType::mass) {
    return  BTaggingToolUtil::MeVToGeV( jet.m() );
  }  else if (vType == BTaggingToolUtil::varType::eta) {
    return jet.eta();
  } else if (vType == BTaggingToolUtil::varType::abseta) {
    return std::abs(jet.eta());
  } else { 
    std::cout << "Unsupported jet variable quantity requested: '" + BTaggingToolUtil::getVariableName(vType) + "'" << std::endl;
    throw std::runtime_error("getJetQuantity unsupported jet variable: '" + BTaggingToolUtil::getVariableName(vType) + "'");
  }
}

std::string getJetInfo(const xAOD::Jet & jet){
  // Print the pT and the mass in GeV 
  std::ostringstream os;
  os << "Jet(pT, eta, phi, m)=" 
     << "(" << jet.pt()/1000. << " GeV, " 
     << jet.eta() << ", "
     << jet.phi() << ", "
     << jet.m()/1000. << " GeV)";
  return os.str();
}

std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>> getGriBoundsHandler(
  const json &jsonConfig, 
  BTaggingBoundsHandler::JetQuantityGetter *funcGetJetQuantity, 
  const bool errorExpected = false, const std::string &errorMsgExpected =""){
  // Make sure that the json config contains the key listBins 
  if (!jsonConfig.contains("listBins")){
    throw std::runtime_error("json config does not contain key listBins");
  }

  // Make sure that listBins is a list 
  const auto &listBins = jsonConfig["listBins"];
  if (!listBins.is_array()) {
    throw std::runtime_error("listBins must be an array");
  }
  // Loop over listBins entry and initialize the corresponding bound handler 
  std::vector<BTaggingBoundsHandler> vecBoundsHandlers;

  for (const auto &binConfig : listBins) {
    try {
      vecBoundsHandlers.push_back(BTaggingBoundsHandler(binConfig, funcGetJetQuantity));
      // Useful for debugging
      // Get the latest handler that was just added 
      // const BTaggingBoundsHandler &handler = vecBoundsHandlers[vecBoundsHandlers.size() -1];
      // // Print handler content 
      // std::cout << "handler=" << handler << std::endl;
    } catch (const std::exception& e) {
      throw std::runtime_error("Failed to initialize handler: " + std::string(e.what()));
    }
  }

  std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>> gridBoundHandlerPtr;

  const std::string context = "GridBoundsHandler initialization";
  // Finally add the grid of handlers to the map of MC-MC handlers
  // If any exception is thrown return failure 
  bool errorFound = false;
  try { 
    gridBoundHandlerPtr = std::make_unique<BTaggingGridBoundsHandler<BTaggingBoundsHandler>>(context, std::move(vecBoundsHandlers));
  } catch (const std::exception& e) {
    // Error found 
    errorFound = true;
    // Assign null pointer in that case 
    gridBoundHandlerPtr = nullptr;

    if (errorExpected) { 
      // The error was expected now checking if the error message is the one we expected 
      // If that's the case no error is raised otherwise raise an error 
      if ( errorMsgExpected == std::string(e.what())){
        
      }
      else { 
        throw std::runtime_error(
          "GridBoundsHandler initialization failure expected but the error message and message expected are differing: "
          "errorMsgExpected='" + errorMsgExpected + "' "
          "versus errorMsg='" + std::string(e.what()) + "'"
        );
      }
    }
    else { 
      // If no error is expected then raise the error 
      throw std::runtime_error("Failed GridBoundsHandler initialization handler: " + std::string(e.what()));
    }
  }

  // Make sure that whether an error was found or not 
  // both are agreeing 
  if (errorExpected != errorFound){
    throw std::runtime_error(
      "errorExpected=" + std::to_string(errorExpected) + 
      " and errorFound=" + std::to_string(errorFound) + 
      " are different --> Logical issue in the test"
    );
  }

  // Finally return gridBoundHandler pointer if everything was successful 
  return gridBoundHandlerPtr;
}

std::string makeRegular2DBinsGridpTMass(const std::vector<double>& pT, const std::vector<double>& mass){
  std::string out;
  for (size_t i = 0; i < pT.size() - 1 ; ++i){
    for (size_t j = 0; j < mass.size() - 1; ++j){
      if (!out.empty()) { 
        out += ",\n";
      }
      out += "{ \"binVariables\": { ";
      out += "\"pT\": [" + std::to_string(pT[i]) + ", " + std::to_string(pT[i + 1]) + "], ";
      out += "\"mass\": [" + std::to_string(mass[j]) + ", " + std::to_string(mass[j + 1]) + "] } }";
    }
  }
  return out;
}

std::string makeRegular3DBinsGridpTMassAbseta(const std::vector<double>& pT, const std::vector<double>& mass, const std::vector<double>& abseta){
  std::string out;
  for (size_t i = 0; i < pT.size() - 1 ; ++i) {
    for (size_t j = 0; j < mass.size() - 1; ++j) {
      for (size_t k = 0; k < abseta.size() - 1; ++k) {
        if (!out.empty()) { 
          out += ",\n";
        }
        out += "{ \"binVariables\": { ";
        out += "\"pT\": [" + std::to_string(pT[i]) + ", " + std::to_string(pT[i + 1]) + "], ";
        out += "\"mass\": [" + std::to_string(mass[j]) + ", " + std::to_string(mass[j + 1]) + "],";
        out += "\"abseta\": [" + std::to_string(abseta[k]) + ", " + std::to_string(abseta[k + 1]) + "] } }";
      }
    }
  }
  return out;
}

int test1DGrid ATLAS_NOT_THREAD_SAFE (int /* argc */ , char* /* argv */ [] ) {
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  #ifdef XAOD_STANDALONE
  // Important to do this first!
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis or Athena
  POOL::Init();
  #endif

  std::cout << "\n\n\n\n" << std::endl;
  ANA_MSG_INFO("Starting test1DGrid");

  // The handlers only store a pointer to the jet quantity getter
  // so it must outlive all the handlers: it is created here and kept alive until the end of the test
  // (it also ensures that all the handlers share the same getter as required to compare them)
  std::unique_ptr<BTaggingBoundsHandler::JetQuantityGetter> funcGetJetQuantity = std::make_unique<BTaggingBoundsHandler::JetQuantityGetter>(&getJetQuantity);
  
  json jsonGridContiguousRegularBinning = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [20   ,   30    ] } },
      { "binVariables": { "pT": [30   ,   50    ] } },
      { "binVariables": { "pT": [50   ,   75    ] } },
      { "binVariables": { "pT": [75   ,   100   ] } },
      { "binVariables": { "pT": [100  ,   250   ] } },
      { "binVariables": { "pT": [250  ,   500   ] } },
      { "binVariables": { "pT": [500  ,   1000  ] } },
      { "binVariables": { "pT": [1000 , "+fmax" ] } }
    ]})###"
  );

  // Define an alias 
  using GridBoundsHandlerUniquePtr = std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>>;

  // Normal retrieval no error expected 
  GridBoundsHandlerUniquePtr gridHandlerContiguousRegularBinning = getGriBoundsHandler( jsonGridContiguousRegularBinning, funcGetJetQuantity.get() );

  std::vector<float> pTVec = { 
    10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 25, 27.5, 35, 40, 45, 55, 70, 80, 90, 110, 150, 200, 400, 600, 800, 
    1500, 10000, 1.e6, 1.e10
  };

  std::vector<float> pTSpecialValuesVec = {
    std::numeric_limits<float>::max(),     // Still a valid float (it's the largest finite float number possible 
    std::numeric_limits<float>::infinity(), // A value larger than any finite number 
    std::numeric_limits<float>::quiet_NaN() // NaN value
  };

  // Min and Max values where the grid should be defined 
  // Outside of the range no handler should be found 
  const double pTMinGridValid = 20.;
  const double pTMaxGridValid = std::numeric_limits<float>::max();

  // Fix eta and phi of the jet 
  const double etaJet = 0.;
  const double phiJet = 0.;
  const double massJet = 5;

  xAOD::Jet jet;
  jet.makePrivateStore();
  
  // Test first the normal values 
  for (const float &pTJet : pTVec){
    // Change the jet 4-momentum
    // Reminder units should be MeV hence multiplying by 1000. the mass and the pT
    jet.setJetP4(xAOD::JetFourMom_t(pTJet*1000., etaJet, phiJet, massJet*1000.));

    // Print jet inforamtion 
    ANA_MSG_INFO("test1DGrid checking bounds handlers for jet=" << getJetInfo(jet));
    
    // Retrieve the corresponding handlers
    const BTaggingBoundsHandler* handlerContiguousRegularBinning = gridHandlerContiguousRegularBinning->getHandlerPtr(jet);
    // // For debugging 
    // if (handlerContiguousRegularBinning){
    //   std::cout << "  --> handlerContiguousRegularBinning=" << (*handlerContiguousRegularBinning) << std::endl;
    // }
    // else { 
    //   std::cout << "  --> No handlerContiguousRegularBinning for current jet" << std::endl;
    // }

    // The jet is within the valid grid i.e. if that boolean is true the handlers should NOT be null pointers
    // except for handlerHole is the jet falls in the hole of the gridHandlerdHole
    bool jetIsWithinValidGrid = ((jet.pt()/1000. >= pTMinGridValid) && (jet.pt()/1000 < pTMaxGridValid)); 

    if (!handlerContiguousRegularBinning && jetIsWithinValidGrid){
      // The handler is a null pointer
      // If the handler is a null pointer while the jet is within the valid grid 
      // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
      throw std::runtime_error("handlerContiguousRegularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
    }
    
    if (handlerContiguousRegularBinning && !jetIsWithinValidGrid){
      // The handler is a null pointer
      // If the handler is a null pointer while the jet is within the valid grid 
      // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
      throw std::runtime_error("handlerContiguousRegularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string());
    }
  
    // Check that the jet is indeed falling in the handler that was returned by the grid 
    // If that's not the case there is a logical in the implementation of the BTaggingGridBoundsHandler class 
    if (handlerContiguousRegularBinning && !handlerContiguousRegularBinning->isJetWithinBounds(jet)){
      throw std::runtime_error( "jet and handlerContiguousRegularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string() );
    }
  }


  // Now check the very peculiar values 
  for (const float &pTJet : pTSpecialValuesVec ){
    // Change the jet 4-momentum
    // NB: here opposed to as the loop above do not multiply by 1000 as dealing with special values  
    jet.setJetP4(xAOD::JetFourMom_t(pTJet, etaJet, phiJet, massJet));

    // Print jet inforamtion 
    ANA_MSG_INFO("test1DGrid checking bounds handlers for SPECIAL jet=" << getJetInfo(jet));

    // Retrieve the corresponding handlers
    const BTaggingBoundsHandler* handlerContiguousRegularBinning = gridHandlerContiguousRegularBinning->getHandlerPtr(jet);
    // // For debugging 
    // if (handlerContiguousRegularBinning){
    //   std::cout << "  --> handlerContiguousRegularBinning=" << (*handlerContiguousRegularBinning) << std::endl;
    // }
    // else { 
    //   std::cout << "  --> No handlerContiguousRegularBinning for current jet" << std::endl;
    // }

    // NB: again here opposed to as the loop above do not divide by 1000 the jet pT as dealing with special values  
    bool jetIsWithinValidGrid = std::isfinite(jet.pt());

    if (!handlerContiguousRegularBinning && jetIsWithinValidGrid){
      // The handler is a null pointer
      // If the handler is a null pointer while the jet is within the valid grid 
      // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
      throw std::runtime_error("handlerContiguousRegularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
    }
    
    if (handlerContiguousRegularBinning && !jetIsWithinValidGrid){
      // The handler is a null pointer
      // If the handler is a null pointer while the jet is within the valid grid 
      // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
      throw std::runtime_error("handlerContiguousRegularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string());
    }
    // Check that the jet is indeed falling in the handler that was returned by the grid 
    // If that's not the case there is a logical in the implementation of the BTaggingGridBoundsHandler class 
    if (handlerContiguousRegularBinning && !handlerContiguousRegularBinning->isJetWithinBounds(jet)){
      throw std::runtime_error( "jet and handlerContiguousRegularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string() );
    }
  }

  return 0;
}

int test2DGrid ATLAS_NOT_THREAD_SAFE (int /* argc */ , char* /* argv */ [] ) {
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  #ifdef XAOD_STANDALONE
  // Important to do this first!
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis or Athena
  POOL::Init();
  #endif

  std::cout << "\n\n\n\n" << std::endl;
  ANA_MSG_INFO("Starting test2DGrid");

  // The handlers only store a pointer to the jet quantity getter
  // so it must outlive all the handlers: it is created here and kept alive until the end of the test
  // (it also ensures that all the handlers share the same getter as required to compare them)
  std::unique_ptr<BTaggingBoundsHandler::JetQuantityGetter> funcGetJetQuantity = std::make_unique<BTaggingBoundsHandler::JetQuantityGetter>(&getJetQuantity);

  // To visualize the grids below use an AI agent to schematize it 
  // or refer to the MR: https://gitlab.cern.ch/atlas/athena/-/merge_requests/92159
  // NB: the grid are not at all representative of 
  // any binning used within the FTAG group 
  // it is ONLY for testing purposes 

  // The generated grid is regular between pT=250-1000 GeV and mass=50-250 GeV in the sense it can be defined with 
  // a pT and mass vectors 
  json jsonGridContiguousRegularBinning = json::parse(
    "{ \"listBins\": [" + 
    makeRegular2DBinsGridpTMass(
      {250, 270, 290, 310, 330, 350, 370, 400, 500, 600, 700, 800, 900, 1000},
      {50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250}
    ) + "]}");

  // The grid below is an irregular grid it covers the full phase space 
  // for pT = 250-1000 GeV and mass = 50-250 GeV ranges
  // i.e. it does not have any hole and no overlapping bins 
  json jsonGridContiguousIrregularBinning = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 270 ], "mass": [50,  75 ] } },
      { "binVariables": { "pT": [270, 300 ], "mass": [50,  100] } },
      { "binVariables": { "pT": [300, 500 ], "mass": [50,  100] } },
      { "binVariables": { "pT": [500, 1000], "mass": [50,  80 ] } },
      { "binVariables": { "pT": [500, 750 ], "mass": [80,  100] } },
      { "binVariables": { "pT": [750, 1000], "mass": [80,  100] } },
      { "binVariables": { "pT": [250, 270 ], "mass": [75,  100] } },
      { "binVariables": { "pT": [250, 280 ], "mass": [100, 175] } },
      { "binVariables": { "pT": [280, 500 ], "mass": [100, 150] } },
      { "binVariables": { "pT": [500, 1000], "mass": [100, 150] } },
      { "binVariables": { "pT": [280, 400 ], "mass": [150, 175] } },
      { "binVariables": { "pT": [400, 500 ], "mass": [150, 175] } },
      { "binVariables": { "pT": [250, 500 ], "mass": [175, 200] } },
      { "binVariables": { "pT": [250, 300 ], "mass": [200, 250] } },
      { "binVariables": { "pT": [300, 500 ], "mass": [200, 250] } },
      { "binVariables": { "pT": [500, 1000], "mass": [150, 250] } }
    ]})###"
  );

  // The grid has an hole for pT 275–285 GeV, mass 100–150 GeV
  // this is a configuration that is fine the code will not raise an issue 
  // it's up to when the JSON file is created to be careful of such issue 
  // But below we check that jets falling in the hole are not associated with 
  // an handler (since they should not) 
  const double pTMinHole = 275.;
  const double pTMaxHole = 285.;
  const double massMinHole = 100.;
  const double massMaxHole = 150.;
  json jsonGridHole = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 275 ] , "mass": [50 , 75 ] } },
      { "binVariables": { "pT": [275, 300 ] , "mass": [50 , 100] } },
      { "binVariables": { "pT": [300, 500 ] , "mass": [50 , 100] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [50 , 80 ] } },
      { "binVariables": { "pT": [500, 750 ] , "mass": [80 , 100] } },
      { "binVariables": { "pT": [750, 1000] , "mass": [80 , 100] } },
      { "binVariables": { "pT": [250, 275 ] , "mass": [75 , 100] } },
      { "binVariables": { "pT": [250, 275 ] , "mass": [100, 175] } },
      { "binVariables": { "pT": [285, 500 ] , "mass": [100, 150] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [100, 150] } },
      { "binVariables": { "pT": [275, 400 ] , "mass": [150, 175] } },
      { "binVariables": { "pT": [400, 500 ] , "mass": [150, 175] } },
      { "binVariables": { "pT": [250, 500 ] , "mass": [175, 200] } },
      { "binVariables": { "pT": [250, 300 ] , "mass": [200, 250] } },
      { "binVariables": { "pT": [300, 500 ] , "mass": [200, 250] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [150, 250] } }
    ]})###"
  );

  // The grid has an overlap 
  // The bin (pT: 250–285, mass 100–175) and bin (pT 275–400, mass 150–175) 
  // share the region pT: 275–285 GeV, mass 150–175 GeV.
  json jsonGridOverlap = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 275 ] , "mass": [50  , 75 ] } },
      { "binVariables": { "pT": [275, 300 ] , "mass": [50  , 100] } },
      { "binVariables": { "pT": [300, 500 ] , "mass": [50  , 100] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [50  , 80 ] } },
      { "binVariables": { "pT": [500, 750 ] , "mass": [80  , 100] } },
      { "binVariables": { "pT": [750, 1000] , "mass": [80  , 100] } },
      { "binVariables": { "pT": [250, 275 ] , "mass": [75  , 100] } },
      { "binVariables": { "pT": [250, 285 ] , "mass": [100 , 175] } },
      { "binVariables": { "pT": [285, 500 ] , "mass": [100 , 150] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [100 , 150] } },
      { "binVariables": { "pT": [275, 400 ] , "mass": [150 , 175] } },
      { "binVariables": { "pT": [400, 500 ] , "mass": [150 , 175] } },
      { "binVariables": { "pT": [250, 500 ] , "mass": [175 , 200] } },
      { "binVariables": { "pT": [250, 300 ] , "mass": [200 , 250] } },
      { "binVariables": { "pT": [300, 500 ] , "mass": [200 , 250] } },
      { "binVariables": { "pT": [500, 1000] , "mass": [150 , 250] } }
    ]})###"
  );

  // Grid with one bin with an additional variable 
  // invalid the overlap cannot be checked
  // each bin should have the exact same variables 
  json jsonGridAdditionalVarForOneBin = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 270 ], "mass": [50,  75 ] } },
      { "binVariables": { "pT": [270, 300 ], "mass": [50,  100] } },
      { "binVariables": { "pT": [300, 500 ], "mass": [50,  100] } },
      { "binVariables": { "pT": [500, 1000], "mass": [50,  80 ] } },
      { "binVariables": { "pT": [500, 750 ], "mass": [80,  100] } },
      { "binVariables": { "pT": [750, 1000], "mass": [80,  100] , "abseta": [ 0.0 , 2.5] } },
      { "binVariables": { "pT": [250, 270 ], "mass": [75,  100] } },
      { "binVariables": { "pT": [250, 280 ], "mass": [100, 175] } },
      { "binVariables": { "pT": [280, 500 ], "mass": [100, 150] } },
      { "binVariables": { "pT": [500, 1000], "mass": [100, 150] } },
      { "binVariables": { "pT": [280, 400 ], "mass": [150, 175] } },
      { "binVariables": { "pT": [400, 500 ], "mass": [150, 175] } },
      { "binVariables": { "pT": [250, 500 ], "mass": [175, 200] } },
      { "binVariables": { "pT": [250, 300 ], "mass": [200, 250] } },
      { "binVariables": { "pT": [300, 500 ], "mass": [200, 250] } },
      { "binVariables": { "pT": [500, 1000], "mass": [150, 250] } }
    ]})###"
  );

  // Define an alias 
  using GridBoundsHandlerUniquePtr = std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>>;

  // Normal retrieval no error expected 
  GridBoundsHandlerUniquePtr gridHandlerContiguousRegularBinning = getGriBoundsHandler( jsonGridContiguousRegularBinning, funcGetJetQuantity.get() );
  GridBoundsHandlerUniquePtr gridHandlerContiguousIrregularBinning = getGriBoundsHandler( jsonGridContiguousIrregularBinning, funcGetJetQuantity.get() );
  GridBoundsHandlerUniquePtr gridHandlerdHole = getGriBoundsHandler( jsonGridHole, funcGetJetQuantity.get() );

  // Issue expected 
  const std::string errorMsgExpectedGridOverlap = "Handler overlap issue found, Overlapping bins for GridBoundsHandler initialization: BTaggingBoundsHandler(pT: [250, 285], mass: [100, 175]) and BTaggingBoundsHandler(pT: [275, 400], mass: [150, 175])";
  GridBoundsHandlerUniquePtr gridHandlerOverlap = getGriBoundsHandler( jsonGridOverlap, funcGetJetQuantity.get(), true, errorMsgExpectedGridOverlap);

  const std::string errorMsgExpectedGridAdditionalVarForOneBin = "Handler overlap issue found, Cannot check overlap of handlers with a different number of variables: BTaggingBoundsHandler(pT: [250, 270], mass: [50, 75]) versus BTaggingBoundsHandler(pT: [750, 1000], mass: [80, 100], abseta: [0, 2.5])";
  GridBoundsHandlerUniquePtr gridAdditionalVarForOneBin = getGriBoundsHandler( jsonGridAdditionalVarForOneBin, funcGetJetQuantity.get(), true, errorMsgExpectedGridAdditionalVarForOneBin);

  // Make sure the valid pointers are not null pointers 
  if (!gridHandlerContiguousRegularBinning){
    throw std::runtime_error("gridHandlerContiguousRegularBinning should never be a null pointer as the grid is invalid");
  }
  if (!gridHandlerContiguousIrregularBinning){
    throw std::runtime_error("gridHandlerContiguousIrregularBinning should never be a null pointer as the grid is invalid");
  }
  if (!gridHandlerdHole){
    throw std::runtime_error("gridHandlerdHole should never be a null pointer as the grid is invalid");
  }

  // Make sure the problematic pointers should be null pointers 
  if (gridHandlerOverlap){ 
    throw std::runtime_error("gridHandlerOverlap should be a null pointer as the grid is invalid");
  }
  if (gridAdditionalVarForOneBin){ 
    throw std::runtime_error("gridAdditionalVarForOneBin should be a null pointer as the grid is invalid");
  }

  // Now only use the valid handler and check that the jet indeed falls into the the handlers returned by the grid handlers 
  // In this way it ensures that the implementation of the BTaggingGridBoundsHandlerTester is valid 

  // The pT and mass ranges are larger than the bins of the grid on purpose 
  const double pTMin = 200.; 
  const double pTMax = 1100.; 
  const double pTStep = 2.;
  const double massMin = 20.;
  const double massMax = 300.;
  const double massStep = 2.;
  const int nPT   = static_cast<int>(std::lround((pTMax   - pTMin)   / pTStep));
  const int nMass = static_cast<int>(std::lround((massMax - massMin) / massStep));

  // Min and Max values where the grid should be defined 
  // Outside of the range no handler should be found 
  const double pTMinGridValid = 250.;
  const double pTMaxGridValid = 1000.;
  const double massMinGridValid = 50.;
  const double massMaxGridValid = 250.;

  // Fix eta and phi of the jet 
  const double etaJet = 0.;
  const double phiJet = 0.;

  xAOD::Jet jet;
  jet.makePrivateStore();

  // Loop over pT and eta and create a new jet then check if the handler retrieved is the one we expect 
  for (int iPT = 0; iPT < nPT; ++iPT) {
    // Use 0.5 to be at the center 
    const double pTJet = pTMin + (iPT + 0.5) * pTStep; 
    for (int iMass = 0; iMass < nMass; ++iMass) {
      const double massJet = massMin + (iMass + 0.5) * massStep; 

      // Change the jet 4-momentum
      // Reminder units should be MeV hence multiplying by 1000. the mass and the pT
      jet.setJetP4(xAOD::JetFourMom_t(pTJet*1000., etaJet, phiJet, massJet*1000.));
      
      // Print jet inforamtion 
      ANA_MSG_INFO("test2DGrid checking bounds handlers for jet=" << getJetInfo(jet));

      // Retrieve the corresponding handlers
      const BTaggingBoundsHandler* handlerContiguousRegularBinning = gridHandlerContiguousRegularBinning->getHandlerPtr(jet);
      const BTaggingBoundsHandler* handlerContiguousIrregularBinning = gridHandlerContiguousIrregularBinning->getHandlerPtr(jet);
      const BTaggingBoundsHandler* handlerHole = gridHandlerdHole->getHandlerPtr(jet);

      // The jet is within the valid grid i.e. if that boolean is true the handlers should NOT be null pointers
      // except for handlerHole is the jet falls in the hole of the gridHandlerdHole
      bool jetIsWithinValidGrid = ((jet.pt()/1000. >= pTMinGridValid) && (jet.pt()/1000 < pTMaxGridValid) && (jet.m()/1000 >= massMinGridValid) && (jet.m()/1000 < massMaxGridValid)); 
      // The jet falls in the hole of the gridHandlerdHole i.e. if that boolean is true then the handlerHole is expected to be a null pointer 
      bool jetIsWithinHoleOfGridHandlerHole = (jet.pt()/1000. >= pTMinHole) && (jet.pt()/1000 < pTMaxHole) && (jet.m()/1000 >= massMinHole) && (jet.m()/1000 < massMaxHole);

      if (!handlerContiguousRegularBinning && jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousRegularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
      }
      if (!handlerContiguousIrregularBinning && jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousIrregularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
      }
      if (!handlerHole && jetIsWithinValidGrid && !jetIsWithinHoleOfGridHandlerHole){
        // The handler is a null pointer
        // Here the jet is not in the hole of the grid so the handlerHole should not be a null pointer 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerHole should never be a null pointer for a jet within the valid grid except if the jet falls in the hole of that grid: jet=" + getJetInfo(jet));
      }

      if (handlerContiguousRegularBinning && !jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousRegularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string());
      }
      if (handlerContiguousIrregularBinning && !jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousIrregularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousIrregularBinning=" + handlerContiguousIrregularBinning->to_string());
      }
      if (handlerHole && !jetIsWithinValidGrid){
        // The handler is a not null pointer while jet is outside of the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerHole should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerHole=" + handlerHole->to_string());
      }
      if (handlerHole && jetIsWithinHoleOfGridHandlerHole){
        // The jet falls in the hole of the grid so the handler should be a null pointer 
        // but if entering in this if it means the handlerHole is NOT a null pointer... 
        // implying there is a logical issue in the implementation of the BTaggingGridBoundsHandler class 
        throw std::runtime_error("handlerHole expected to be a null pointer but it is not a null pointer for jet=" + getJetInfo(jet) + " as the jet is falling in the hole of the gridHandlerdHole: handlerHole=" + handlerHole->to_string() );
      }

      // Check that the jet is indeed falling in the handler that was returned by the grid 
      // If that's not the case there is a logical in the implementation of the BTaggingGridBoundsHandler class 
      if (handlerContiguousRegularBinning && !handlerContiguousRegularBinning->isJetWithinBounds(jet)){
        throw std::runtime_error( "jet and handlerContiguousRegularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string() );
      }
      if (handlerContiguousIrregularBinning && !handlerContiguousIrregularBinning->isJetWithinBounds(jet)){
        throw std::runtime_error( "jet and handlerContiguousIrregularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousIrregularBinning=" + handlerContiguousIrregularBinning->to_string() );
      }
      if (handlerHole && !handlerHole->isJetWithinBounds(jet)){
        throw std::runtime_error( "jet and handlerContiguousIrregularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerHole=" + handlerHole->to_string() );
      }
    }
  }

  return 0;
}


int test3DGrid ATLAS_NOT_THREAD_SAFE (int /* argc */ , char* /* argv */ [] ) {
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  #ifdef XAOD_STANDALONE
  // Important to do this first!
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis or Athena
  POOL::Init();
  #endif

  std::cout << "\n\n\n\n" << std::endl;
  ANA_MSG_INFO("Starting test3DGrid");

  // The handlers only store a pointer to the jet quantity getter
  // so it must outlive all the handlers: it is created here and kept alive until the end of the test
  // (it also ensures that all the handlers share the same getter as required to compare them)
  std::unique_ptr<BTaggingBoundsHandler::JetQuantityGetter> funcGetJetQuantity = std::make_unique<BTaggingBoundsHandler::JetQuantityGetter>(&getJetQuantity);

  // To visualize the grids below use an AI agent to schematize it 
  // or refer to the MR: https://gitlab.cern.ch/atlas/athena/-/merge_requests/92159
  // NB: the grid are not at all representative of 
  // any binning used within the FTAG group 
  // it is ONLY for testing purposes 

  // The generated grid is regular between pT=250-800 GeV, mass=50-250 GeV and |eta|=0-2.5 in the sense it can be defined with 
  // a pT and mass vectors 
  json jsonGridContiguousRegularBinning = json::parse(
    "{ \"listBins\": [" + 
    makeRegular3DBinsGridpTMassAbseta(
      {250, 270, 290, 310, 330, 350, 370, 400, 500, 600, 700, 800},
      {50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250},
      {0., 0.2, 0.4, 0.6, 0.8, 1., 1.25, 1.5, 2.}
    ) + "]}");

  // The grid below is an irregular grid it covers the full phase space 
  // for pT=250-800 GeV, mass=50-250 GeV and |eta|=0-2.5 ranges
  // i.e. it does not have any hole and no overlapping bins 
  json jsonGridContiguousIrregularBinning = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 270], "mass": [50,  75 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 270], "mass": [75,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 280], "mass": [100, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 500], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [150, 250] , "abseta": [0  , 0.5]} },

      { "binVariables": { "pT": [250, 270], "mass": [50,  120] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [250, 270], "mass": [120, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [110, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [110, 250] , "abseta": [0.5, 1.0]} },

      { "binVariables": { "pT": [250, 295], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [295, 400], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  250] , "abseta": [1.0, 1.5]} },

      { "binVariables": { "pT": [250, 280], "mass": [50,  85 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 350], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [350, 500], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 280], "mass": [85,  175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 500], "mass": [130, 150] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [130, 250] , "abseta": [1.5, 2]} }
    ]})###"
  );


  // Same config as previously but the grid has an hole for
  // pT=270-300 GeV, mass=110-130 GeV  and abseta=0.5-1.0
  // this is a configuration that is fine the code will not raise an issue 
  // it's up to when the JSON file is created to be careful of such issue 
  // But below we check that jets falling in the hole are not associated with 
  // an handler (since they should not) 
  const double pTMinHole = 270.;
  const double pTMaxHole = 300.;
  const double massMinHole = 110.;
  const double massMaxHole = 130.;
  const double absetaMinHole = 0.5;
  const double absetaMaxHole = 1.0;
  // The grid below is an irregular grid it covers the full phase space 
  // for pT=250-800 GeV, mass=50-250 GeV and |eta|=0-2.5 ranges
  // i.e. it does not have any hole and no overlapping bins 
  json jsonGridHole = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 270], "mass": [50,  75 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 270], "mass": [75,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 280], "mass": [100, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 500], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [150, 250] , "abseta": [0  , 0.5]} },

      { "binVariables": { "pT": [250, 270], "mass": [50,  120] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [250, 270], "mass": [120, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [130, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [110, 250] , "abseta": [0.5, 1.0]} },

      { "binVariables": { "pT": [250, 295], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [295, 400], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [50,  250] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  250] , "abseta": [1.0, 1.5]} },

      { "binVariables": { "pT": [250, 280], "mass": [50,  85 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 350], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [350, 500], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 280], "mass": [85,  175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 500], "mass": [130, 150] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [130, 250] , "abseta": [1.5, 2]} }
    ]})###"
  );

  // The grid has an overlap 
  // The bins (pT: [250, 270], mass: [50, 250], abseta: [0.5, 1]) and (pT: [250, 270], mass: [50, 120], abseta: [0.5, 1])
  // are overlapping 
  json jsonGridOverlap = json::parse(R"###({
    "listBins": [
      { "binVariables": { "pT": [250, 270], "mass": [50,  75 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [50,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 270], "mass": [75,  100] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 280], "mass": [100, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 500], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 150] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [0  , 0.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [150, 250] , "abseta": [0  , 0.5]} },

      { "binVariables": { "pT": [250, 270], "mass": [50,  250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [50,  250] , "abseta": [0.5, 1.0]} },

      { "binVariables": { "pT": [250, 270], "mass": [50,  120] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [50,  110] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [250, 270], "mass": [120, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [270, 300], "mass": [110, 250] , "abseta": [0.5, 1.0]} },
      { "binVariables": { "pT": [300, 800], "mass": [110, 250] , "abseta": [0.5, 1.0]} },

      { "binVariables": { "pT": [250, 295], "mass": [50,  250 ] , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [295, 400], "mass": [50,  250]  , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [400, 500], "mass": [50,  250]  , "abseta": [1.0, 1.5]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  250]  , "abseta": [1.0, 1.5]} },

      { "binVariables": { "pT": [250, 280], "mass": [50,  85 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 350], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [350, 500], "mass": [50,  130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [50,  80 ] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 750], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [750, 800], "mass": [80,  100] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 280], "mass": [85,  175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 500], "mass": [130, 150] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [100, 130] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [280, 400], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [400, 500], "mass": [150, 175] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 500], "mass": [175, 200] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [250, 300], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [300, 500], "mass": [200, 250] , "abseta": [1.5, 2]} },
      { "binVariables": { "pT": [500, 800], "mass": [130, 250] , "abseta": [1.5, 2]} }
    ]})###"
  );


  // Define an alias 
  using GridBoundsHandlerUniquePtr = std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>>;

  // Normal retrieval no error expected 
  GridBoundsHandlerUniquePtr gridHandlerContiguousRegularBinning = getGriBoundsHandler( jsonGridContiguousRegularBinning, funcGetJetQuantity.get() );
  GridBoundsHandlerUniquePtr gridHandlerContiguousIrregularBinning = getGriBoundsHandler( jsonGridContiguousIrregularBinning, funcGetJetQuantity.get() );
  GridBoundsHandlerUniquePtr gridHandlerdHole = getGriBoundsHandler( jsonGridHole, funcGetJetQuantity.get() );

  // Issue expected 
  const std::string errorMsgExpectedGridOverlap = "Handler overlap issue found, Overlapping bins for GridBoundsHandler initialization: BTaggingBoundsHandler(pT: [250, 270], mass: [50, 250], abseta: [0.5, 1]) and BTaggingBoundsHandler(pT: [250, 270], mass: [50, 120], abseta: [0.5, 1])";
  GridBoundsHandlerUniquePtr gridHandlerOverlap = getGriBoundsHandler( jsonGridOverlap, funcGetJetQuantity.get(), true, errorMsgExpectedGridOverlap);


  // Make sure the valid pointers are not null pointers 
  if (!gridHandlerContiguousRegularBinning){
    throw std::runtime_error("gridHandlerContiguousRegularBinning should never be a null pointer as the grid is invalid");
  }
  if (!gridHandlerContiguousIrregularBinning){
    throw std::runtime_error("gridHandlerContiguousIrregularBinning should never be a null pointer as the grid is invalid");
  }
  // if (!gridHandlerdHole){
  //   throw std::runtime_error("gridHandlerdHole should never be a null pointer as the grid is invalid");
  // }

  // Make sure the problematic pointers should be null pointers 
  if (gridHandlerOverlap){ 
    throw std::runtime_error("gridHandlerOverlap should be a null pointer as the grid is invalid");
  }

  // Now only use the valid handler and check that the jet indeed falls into the the handlers returned by the grid handlers 
  // In this way it ensures that the implementation of the BTaggingGridBoundsHandlerTester is valid 

  // The pT, mass and eta ranges are larger than the bins of the grid on purpose 
  // Reduced a bit those ranges and step with respect to the 1D and 2D tests
  // as here looping over 3 variables
  const double pTMin = 240.; 
  const double pTMax = 810.; 
  const double pTStep = 2.5;
  const double massMin = 40.;
  const double massMax = 260.;
  const double massStep = 2.5;
  const double etaMin = 0.;
  const double etaMax = 2.2;
  const double etaStep = 0.05;

  const int nPT   = static_cast<int>(std::lround((pTMax   - pTMin)   / pTStep));
  const int nMass = static_cast<int>(std::lround((massMax - massMin) / massStep));
  const int nEta = static_cast<int>(std::lround((etaMax - etaMin) / etaStep));

  // Min and Max values where the grid should be defined 
  // Outside of the range no handler should be found 
  const double pTMinGridValid = 250.;
  const double pTMaxGridValid = 800.;
  const double massMinGridValid = 50.;
  const double massMaxGridValid = 250.;
  // Here absolute value of eta hence min is 0. 
  const double absetaMinGridValid = 0.;
  const double absetaMaxGridValid = 2.0;

  // Fix phi of the jet 
  const double phiJet = 0.;

  xAOD::Jet jet;
  jet.makePrivateStore();

  // Loop over pT and eta and create a new jet then check if the handler retrieved is the one we expect 
  for (int iPT = 0; iPT < nPT; ++iPT) {
    // Use 0.5 to be at the center 
    const double pTJet = pTMin + (iPT + 0.5) * pTStep; 
    for (int iMass = 0; iMass < nMass; ++iMass) {
      const double massJet = massMin + (iMass + 0.5) * massStep;
      for (int iEta = 0; iEta < nEta; ++iEta) {
        const double etaJet = etaMin + (iEta + 0.5) * etaStep; 

        // Change the jet 4-momentum
        // Reminder units should be MeV hence multiplying by 1000. the mass and the pT
        jet.setJetP4(xAOD::JetFourMom_t(pTJet*1000., etaJet, phiJet, massJet*1000.));
        
        // Print jet inforamtion 
        ANA_MSG_INFO("test3DGrid checking bounds handlers for jet=" << getJetInfo(jet));

        // Retrieve the corresponding handlers
        const BTaggingBoundsHandler* handlerContiguousRegularBinning = gridHandlerContiguousRegularBinning->getHandlerPtr(jet);
        const BTaggingBoundsHandler* handlerContiguousIrregularBinning = gridHandlerContiguousIrregularBinning->getHandlerPtr(jet);
        const BTaggingBoundsHandler* handlerHole = gridHandlerdHole->getHandlerPtr(jet);

        // The jet is within the valid grid i.e. if that boolean is true the handlers should NOT be null pointers
        // except for handlerHole is the jet falls in the hole of the gridHandlerdHole
        bool jetIsWithinValidGrid = ((jet.pt()/1000. >= pTMinGridValid) && (jet.pt()/1000 < pTMaxGridValid) && (jet.m()/1000 >= massMinGridValid) && (jet.m()/1000 < massMaxGridValid) && std::abs(jet.eta())>= absetaMinGridValid && jet.eta() < absetaMaxGridValid);
        // The jet falls in the hole of the gridHandlerdHole i.e. if that boolean is true then the handlerHole is expected to be a null pointer 
        bool jetIsWithinHoleOfGridHandlerHole =((jet.pt()/1000. >= pTMinHole) && (jet.pt()/1000 < pTMaxHole) && (jet.m()/1000 >= massMinHole) && (jet.m()/1000 < massMaxHole) && (std::abs(jet.eta()) >= absetaMinHole) && (std::abs(jet.eta()) < absetaMaxHole));

        if (!handlerContiguousRegularBinning && jetIsWithinValidGrid){
          // The handler is a null pointer
          // If the handler is a null pointer while the jet is within the valid grid 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerContiguousRegularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
        }
        if (!handlerContiguousIrregularBinning && jetIsWithinValidGrid){
          // The handler is a null pointer
          // If the handler is a null pointer while the jet is within the valid grid 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerContiguousIrregularBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
        }
        if (!handlerHole && jetIsWithinValidGrid && !jetIsWithinHoleOfGridHandlerHole){
          // The handler is a null pointer
          // Here the jet is not in the hole of the grid so the handlerHole should not be a null pointer 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerHole should never be a null pointer for a jet within the valid grid except if the jet falls in the hole of that grid: jet=" + getJetInfo(jet));
        }

        if (handlerContiguousRegularBinning && !jetIsWithinValidGrid){
          // The handler is a null pointer
          // If the handler is a null pointer while the jet is within the valid grid 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerContiguousRegularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string());
        }
        if (handlerContiguousIrregularBinning && !jetIsWithinValidGrid){
          // The handler is a null pointer
          // If the handler is a null pointer while the jet is within the valid grid 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerContiguousIrregularBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousIrregularBinning=" + handlerContiguousIrregularBinning->to_string());
        }
        if (handlerHole && !jetIsWithinValidGrid){
          // The handler is a not null pointer while jet is outside of the valid grid 
          // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
          throw std::runtime_error("handlerHole should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerHole=" + handlerHole->to_string());
        }

        if (handlerHole && jetIsWithinHoleOfGridHandlerHole){
          // The jet falls in the hole of the grid so the handler should be a null pointer 
          // but if entering in this if it means the handlerHole is NOT a null pointer... 
          // implying there is a logical issue in the implementation of the BTaggingGridBoundsHandler class 
          throw std::runtime_error("handlerHole expected to be a null pointer but it is not a null pointer for jet=" + getJetInfo(jet) + " as the jet is falling in the hole of the gridHandlerdHole: handlerHole=" + handlerHole->to_string() );
        }

        // Check that the jet is indeed falling in the handler that was returned by the grid 
        // If that's not the case there is a logical in the implementation of the BTaggingGridBoundsHandler class 
        if (handlerContiguousRegularBinning && !handlerContiguousRegularBinning->isJetWithinBounds(jet)){
          throw std::runtime_error( "jet and handlerContiguousRegularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousRegularBinning=" + handlerContiguousRegularBinning->to_string() );
        }
        if (handlerContiguousIrregularBinning && !handlerContiguousIrregularBinning->isJetWithinBounds(jet)){
          throw std::runtime_error( "jet and handlerContiguousIrregularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousIrregularBinning=" + handlerContiguousIrregularBinning->to_string() );
        }
        if (handlerHole && !handlerHole->isJetWithinBounds(jet)){
          throw std::runtime_error( "jet and handlerContiguousIrregularBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerHole=" + handlerHole->to_string() );
        }
      }
    }
  }

  return 0;
}


int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[])
{
  int intTest1DGrid = 1;
  int intTest2DGrid = 1;
  int intTest3DGrid = 1;

  try {
    intTest1DGrid = test1DGrid(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "Failed test1DGrid test, exception: " << e.what() << "\n";
    return 1;
  }

  try {
    intTest2DGrid = test2DGrid(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "Failed test2DGrid test, exception: " << e.what() << "\n";
    return 1;
  }

  try {
    intTest3DGrid = test3DGrid(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "Failed test3DGrid test, exception: " << e.what() << "\n";
    return 1;
  }

  // If an integer for a test is not 0 it means there was an issue 
  if ((intTest1DGrid != 0) || (intTest2DGrid != 0) || (intTest3DGrid != 0)) { 
    std::cerr << "test1DGrid or test2DGrid or intTest3DGrid failed, returned values are " 
    << "intTest1DGrid=" << intTest1DGrid 
    << ", intTest2DGrid=" << intTest2DGrid 
    << ", and intTest3DGrid=" << intTest3DGrid 
    << "\n";

    return 1; 
  }

  // Otherwise all good return 0
  return 0;
}