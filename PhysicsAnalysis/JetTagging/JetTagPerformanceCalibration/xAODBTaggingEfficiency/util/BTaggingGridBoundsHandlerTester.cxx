#include "AsgTools/AsgTool.h"
#include <AsgTools/PropertyWrapper.h>
#include "AsgTools/ToolHandle.h"
#include "xAODBTaggingEfficiency/BTaggingToolUtil.h"
#include "xAODBTaggingEfficiency/BTaggingBoundsHandler.h"
#include "xAODBTaggingEfficiency/BTaggingGridBoundsHandler.h"
#include <nlohmann/json.hpp>
#include <map>
#include <vector>
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

std::unique_ptr<BTaggingGridBoundsHandler<BTaggingBoundsHandler>> getGriBoundsHandler(const json &jsonConfig, const bool errorExpected = false, const std::string &errorMsgExpected =""){
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
      vecBoundsHandlers.push_back(BTaggingBoundsHandler(binConfig, &getJetQuantity));
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
    gridBoundHandlerPtr = std::make_unique<BTaggingGridBoundsHandler<BTaggingBoundsHandler>>(context, vecBoundsHandlers);
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

int test1 ATLAS_NOT_THREAD_SAFE (int /* argc */ , char* /* argv */ [] ) {
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

  // To visualize the grids below use an AI agent to schematize it 
  // or refer to the MR: https://gitlab.cern.ch/atlas/athena/-/merge_requests/92159
  // NB: the grid are not at all representative of 
  // any binning used within the FTAG group 
  // it is ONLY for testing purposes 

  // The grid below is an irregular grid it covers the full phase space 
  // covering the pT = 250-1000 GeV and mass = 50-250 GeV ranges
  // i.e. it does not have any hole and no overlapping bins 
  json jsonGridContiguousBinning = json::parse(R"###({
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
  GridBoundsHandlerUniquePtr gridHandlerContiguousBinning = getGriBoundsHandler( jsonGridContiguousBinning );
  GridBoundsHandlerUniquePtr gridHandlerdHole = getGriBoundsHandler( jsonGridHole );

  // Issue expected 
  const std::string errorMsgExpectedGridOverlap = "Handler overlap issue found, Overlapping bins for GridBoundsHandler initialization: BTaggingBoundsHandler(pT: [250, 285], mass: [100, 175]) and BTaggingBoundsHandler(pT: [275, 400], mass: [150, 175])";
  GridBoundsHandlerUniquePtr gridHandlerOverlap = getGriBoundsHandler( jsonGridOverlap, true, errorMsgExpectedGridOverlap);

  const std::string errorMsgExpectedGridAdditionalVarForOneBin = "Handler overlap issue found, Cannot check overlap of handlers with a different number of variables: BTaggingBoundsHandler(pT: [250, 270], mass: [50, 75]) versus BTaggingBoundsHandler(pT: [750, 1000], mass: [80, 100], abseta: [0, 2.5])";
  GridBoundsHandlerUniquePtr gridAdditionalVarForOneBin = getGriBoundsHandler( jsonGridAdditionalVarForOneBin, true, errorMsgExpectedGridAdditionalVarForOneBin);

  // Make sure the valid pointers are not null pointers 
  if (!gridHandlerContiguousBinning){
     throw std::runtime_error("gridHandlerContiguousBinning should never be a null pointer as the grid is invalid");
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

  // The pT and range is larger than the bins of the grid on purpose 
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
      ANA_MSG_INFO("Checking bounds handlers for jet=" << getJetInfo(jet));

      // Retrieve the corresponding handlers  
      const BTaggingBoundsHandler* handlerContiguousBinning = gridHandlerContiguousBinning->getHandlerPtr(jet);
      const BTaggingBoundsHandler* handlerHole = gridHandlerdHole->getHandlerPtr(jet);

      // The jet is within the valid grid i.e. if that boolean is true the handlers should NOT be null pointers
      // except for handlerHole is the jet falls in the hole of the gridHandlerdHole
      bool jetIsWithinValidGrid = ((jet.pt()/1000. >= pTMinGridValid) && (jet.pt()/1000 < pTMaxGridValid) && (jet.m()/1000 >= massMinGridValid) && (jet.m()/1000 < massMaxGridValid)); 
      // The jet falls in the hole of the gridHandlerdHole i.e. if that boolean is true then the handlerHole is expected to be a null pointer 
      bool jetIsWithinHoleOfGridHandlerHole = (jet.pt()/1000. >= pTMinHole) && (jet.pt()/1000 < pTMaxHole) && (jet.m()/1000 >= massMinHole) && (jet.m()/1000 < massMaxHole);

      if (!handlerContiguousBinning && jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousBinning should never be a null pointer for a jet within the valid grid: jet=" + getJetInfo(jet));
      }
      if (!handlerHole && jetIsWithinValidGrid && !jetIsWithinHoleOfGridHandlerHole){
        // The handler is a null pointer
        // Here the jet is not in the hole of the grid so the handlerHole should not be a null pointer 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerHole should never be a null pointer for a jet within the valid grid except if the jet falls in the hole of that grid: jet=" + getJetInfo(jet));
      }

      if (handlerContiguousBinning && !jetIsWithinValidGrid){
        // The handler is a null pointer
        // If the handler is a null pointer while the jet is within the valid grid 
        // it means there is a logical issue in the implementation of the BTaggingGridBoundsHandler class
        throw std::runtime_error("handlerContiguousBinning should be a null pointer for a jet outside of the valid grid: jet=" + getJetInfo(jet) + " and handlerContiguousBinning=" + handlerContiguousBinning->to_string());
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
      if (handlerContiguousBinning && !handlerContiguousBinning->isJetWithinBounds(jet)){
        throw std::runtime_error( "jet and handlerContiguousBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerContiguousBinning=" + handlerContiguousBinning->to_string() );
      }
      if (handlerHole && !handlerHole->isJetWithinBounds(jet)){
        throw std::runtime_error( "jet and handlerContiguousBinning are not matching: jet=" +  getJetInfo(jet) + " and handlerHole=" + handlerHole->to_string() );
      }
    }
  }

  return 0;
}

int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[])
{
  try {
    return test1(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}