/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/***************************
 
 * AnomalyDetectionBDT.cxx
 * Created by Santiago Cane 4/15/2025
 *
 * @brief Algorithm is a Boosted Decision Tree for regression. 
 * It uses the Pt, Eta and Phi of 3  muons to regress an anomaly score of a 
 * previously trained Variational Autoencoder and compares it to a threshold value. 
 * 
 * 
 *@param ScoreThreshold
 *
 ***************************/

#include "L1TopoAlgorithms/AnomalyDetectionBDT.h"
#include "L1TopoInterfaces/Decision.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoSimulationUtils/Helpers.h"
#ifndef TRIGCONF_STANDALONE
#include <PathResolver/PathResolver.h>
#endif
#include <fstream>

REGISTER_ALG_TCS(AnomalyDetectionBDT)

template <typename T>
std::string
vectorToString(const std::vector<T>& vec) {
   std::ostringstream oss;
   oss << "[";
   bool first = true;
      for (const auto& val : vec) {
      if (!first) {
         oss << ", ";
      }
      oss << val;
      first = false;
   }
   oss << "]";
   return oss.str();
}

namespace TCS {
   std::ostream&
   operator<<(std::ostream& os, const TCS::Bin& bin) {
      os << "Bin(score=" << bin.score << ", nVar=" << bin.nVar
         << ", minVals=" << vectorToString(bin.minVals)
         << ", maxVals=" << vectorToString(bin.maxVals) << ")";
      return os;
   }
}

namespace {
   
   int scale(float val,float min, float max, int bits=7) {
      float arrRange = max - min;
      int nPoints = (1 << bits) - 1;
      float resolution = arrRange / nPoints;
      int out = static_cast<int>((val - min) / resolution);

      // Staying within a valid range
      if (out < 0) out = 0;
      if (out > nPoints) out = nPoints;

      return out;
   }

}

namespace {
   int mapval(int value, const std::string & vartype){

      static const std::map<int,int> eta_LUT = {
      ///////////////////////
      //Add eta LUT here
      {-100,4},
      {-99,5},
      {-98,5},
      {-97,6},
      {-96,7},
      {-95,7},
      {-94,8},
      {-93,8},
      {-92,9},
      {-91,9},
      {-90,10},
      {-89,11},
      {-88,11},
      {-87,12},
      {-86,12},
      {-85,13},
      {-84,14},
      {-83,14},
      {-82,15},
      {-81,15},
      {-80,16},
      {-79,17},
      {-78,17},
      {-77,18},
      {-76,18},
      {-75,19},
      {-74,19},
      {-73,20},
      {-72,21},
      {-71,21},
      {-70,22},
      {-69,22},
      {-68,23},
      {-67,24},
      {-66,24},
      {-65,25},
      {-64,25},
      {-63,26},
      {-62,27},
      {-61,27},
      {-60,28},
      {-59,28},
      {-58,29},
      {-57,29},
      {-56,30},
      {-55,31},
      {-54,31},
      {-53,32},
      {-52,32},
      {-51,33},
      {-50,34},
      {-49,34},
      {-48,35},
      {-47,35},
      {-46,36},
      {-45,37},
      {-44,37},
      {-43,38},
      {-42,38},
      {-41,39},
      {-40,39},
      {-39,40},
      {-38,41},
      {-37,41},
      {-36,42},
      {-35,42},
      {-34,43},
      {-33,44},
      {-32,44},
      {-31,45},
      {-30,45},
      {-29,46},
      {-28,47},
      {-27,47},
      {-26,48},
      {-25,48},
      {-24,49},
      {-23,49},
      {-22,50},
      {-21,51},
      {-20,51},
      {-19,52},
      {-18,52},
      {-17,53},
      {-16,54},
      {-15,54},
      {-14,55},
      {-13,55},
      {-12,56},
      {-11,57},
      {-10,57},
      {-9,58},
      {-8,58},
      {-7,59},
      {-6,59},
      {-5,60},
      {-4,61},
      {-3,61},
      {-2,62},
      {-1,62},
      {0,63},
      {1,64},
      {2,64},
      {3,65},
      {4,65},
      {5,66},
      {6,67},
      {7,67},
      {8,68},
      {9,68},
      {10,69},
      {11,69},
      {12,70},
      {13,71},
      {14,71},
      {15,72},
      {16,72},
      {17,73},
      {18,74},
      {19,74},
      {20,75},
      {21,75},
      {22,76},
      {23,77},
      {24,77},
      {25,78},
      {26,78},
      {27,79},
      {28,79},
      {29,80},
      {30,81},
      {31,81},
      {32,82},
      {33,82},
      {34,83},
      {35,84},
      {36,84},
      {37,85},
      {38,85},
      {39,86},
      {40,87},
      {41,87},
      {42,88},
      {43,88},
      {44,89},
      {45,89},
      {46,90},
      {47,91},
      {48,91},
      {49,92},
      {50,92},
      {51,93},
      {52,94},
      {53,94},
      {54,95},
      {55,95},
      {56,96},
      {57,97},
      {58,97},
      {59,98},
      {60,98},
      {61,99},
      {62,99},
      {63,100},
      {64,101},
      {65,101},
      {66,102},
      {67,102},
      {68,103},
      {69,104},
      {70,104},
      {71,105},
      {72,105},
      {73,106},
      {74,107},
      {75,107},
      {76,108},
      {77,108},
      {78,109},
      {79,109},
      {80,110},
      {81,111},
      {82,111},
      {83,112},
      {84,112},
      {85,113},
      {86,114},
      {87,114},
      {88,115},
      {89,115},
      {90,116},
      {91,117},
      {92,117},
      {93,118},
      {94,118},
      {95,119},
      {96,119},
      {97,120},
      {98,121},
      {99,121},
      {100,122} 
      ///////////////////////
      };

      static const std::map<int,int> phi_LUT = []{
         std::map<int,int> m;
	 for (int i = 0; i <= 125; ++i) m.emplace(i,i);
	 m.emplace(126,127);
	 m.emplace(127,127);
	 return m;
      }();
  	      
      static const std::map<int,int> pt_LUT = {
      ///////////////////////
      //Add pt LUT here 
      {30,0},
      {40,8},
      {50,15},
      {60,22},
      {70,30},
      {80,37},
      {90,45},
      {100,52},
      {110,59},
      {120,67},
      {130,74},
      {140,81},
      {150,89},
      {160,96},
      {170,104},
      {180,111},
      {190,118},
      {200,126} 
      ///////////////////////
      };

      //If a transformation is not found in the provided LUT, the function will return the original input
      
      if(vartype=="eta"){
         auto it_eta = eta_LUT.find(value);
	 return it_eta != eta_LUT.end() ? it_eta->second : value;
      }
      
      else if (vartype=="phi"){
         auto it_phi = phi_LUT.find(value);
	 return it_phi != phi_LUT.end() ? it_phi->second : value;
      }
      
      else if(vartype =="pt"){
         auto it_pt = pt_LUT.find(value);
	 return it_pt != pt_LUT.end() ? it_pt->second : value;
      }
      
      else{
         return value;
      }
   }
}

TCS::Bin::Bin(const nlohmann::json& obj, int nVars)
: score(obj["score"]), nVar(nVars) {
   for (auto &[name, val] : obj["ranges"]["min"].items()) {
      // this works for vectors [a,b,c] and dictionary values {"0": a, "1": b, "2": c}
      minVals.push_back(val);
   }
   for (auto &[name, val] : obj["ranges"]["max"].items()) {
      maxVals.push_back(val);
   }
}

bool
TCS::Bin::isInside(const std::vector<int64_t>& inputEvent) const {
   for (size_t i = 0; i < inputEvent.size(); ++i) {
      if (inputEvent[i] <= minVals[i]) {
         return false;
      }
      else if (inputEvent[i] > maxVals[i]) {
         return false;
      }
   }
   return true;
}


TCS::Tree::Tree(const nlohmann::json& obj, int nVars) {
   for (auto &[binName, bin] : obj["bins"].items()) {
      bins.push_back(Bin(bin, nVars));
   } 
}

int64_t
TCS::Tree::getTreeScore(const std::vector<int64_t>& inputEvent) const {
   for(const Bin& bin : bins) {
      if(bin.isInside(inputEvent)) {
         return bin.score;
      }
   }
   return 0;
   //throw std::runtime_error("AnomalyDetection: the code has reached an unreachable state");
}


// constructor
TCS::AnomalyDetectionBDT::AnomalyDetectionBDT(const std::string& name)
: DecisionAlg(name) {
   defineParameter("MinET1",0);
   defineParameter("MinET2",0); 
   defineParameter("ScoreThreshold", 1, 0);
   defineParameter("ScoreThreshold", 1, 1);
   defineParameter("MaxTob",3);
   defineParameter("NumResultBits",2);
   setNumberOutputBits(2); // 2 decision bits for 2 different score thresholds
                           // (BDT Regression score is calculated internally)  
}

// destructor
TCS::AnomalyDetectionBDT::~AnomalyDetectionBDT()
{}

TCS::StatusCode
TCS::AnomalyDetectionBDT::initialize() {
   
   p_MaxTob = parameter("MaxTob").value(); 
   p_minEt1 = parameter("MinET1").value();
   p_minEt2 = parameter("MinET2").value();
   for (size_t i=0; i <numberOutputBits(); ++i) {
      p_ScoreThreshold[i] = parameter("ScoreThreshold", i).value();
   }
   TRG_MSG_INFO("ADBDT: Threshold set to " << p_ScoreThreshold);
   const std::string bdtfn = "TrigAnomalyDetectionBDT/2025-06-18/fwX-config_nomAD_Jun3_2pi200t20d.json";
   
   std::string fileLocation;
   
   #ifndef TRIGCONF_STANDALONE
   fileLocation = PathResolver::find_calib_file(bdtfn);
   #else
   return StatusCode::SUCCESS;
   #endif 
   // Reading in the config
   std::ifstream configFile(fileLocation); 

   if (!configFile.is_open()) {
      TRG_MSG_ERROR("Unable to open BDT configuration file." << fileLocation);
      return StatusCode::FAILURE;
   }

   nlohmann::json bdt_config;    // BDT configuration 
   configFile >> bdt_config;
   m_nVar = bdt_config["nDim"];
   m_mu1_ptmin = bdt_config["variable_float_ranges"]["flat_dimu_mu1_pt_100MeV"]["min"];
   m_mu1_ptmax = bdt_config["variable_float_ranges"]["flat_dimu_mu1_pt_100MeV"]["max"];
   m_mu1_etamin = bdt_config["variable_float_ranges"]["flat_dimu_mu1_eta"]["min"];
   m_mu1_etamax = bdt_config["variable_float_ranges"]["flat_dimu_mu1_eta"]["max"];
   m_mu1_phimin = bdt_config["variable_float_ranges"]["flat_dimu_mu1_phi_2pi"]["min"];
   m_mu1_phimax = bdt_config["variable_float_ranges"]["flat_dimu_mu1_phi_2pi"]["max"];
   m_mu2_ptmin = bdt_config["variable_float_ranges"]["flat_dimu_mu2_pt_100MeV"]["min"];
   m_mu2_ptmax = bdt_config["variable_float_ranges"]["flat_dimu_mu2_pt_100MeV"]["max"];
   m_mu2_etamin = bdt_config["variable_float_ranges"]["flat_dimu_mu2_eta"]["min"];
   m_mu2_etamax = bdt_config["variable_float_ranges"]["flat_dimu_mu2_eta"]["max"];
   m_mu2_phimin = bdt_config["variable_float_ranges"]["flat_dimu_mu2_phi_2pi"]["min"];
   m_mu2_phimax = bdt_config["variable_float_ranges"]["flat_dimu_mu2_phi_2pi"]["max"];

   for (auto& [treeName, tree] : bdt_config["trees"].items()) {
      m_trees.emplace_back(tree, m_nVar);
   }

   TRG_MSG_DEBUG("In initialize. There are " << m_trees.size() << " trees for AnomalyDetectionBDT.");
 
   //Setting simulation mode.
   //This can be set to "lut" or to "math" 
   //"lut" mimics exactly what is done in firmware
   m_sim_mode = "lut"; 
   TRG_MSG_INFO("The sim_mode is " << m_sim_mode);

   //Booking histograms
   for (size_t i=0; i<numberOutputBits(); ++i) {
      std::string hname_accept = "hAnomalyDetecionBDT_accept_bit"+std::to_string((int)i);
      std::string hname_reject = "hAnomalyDetecionBDT_reject_bit"+std::to_string((int)i);
   
      bookHist(m_histAccept, hname_accept, "AnomalyScore", 100, 0, 128);
      bookHist(m_histReject, hname_reject, "AnomalyScore", 100, 0, 128); 
   }
   return StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::AnomalyDetectionBDT::processBitCorrect(const std::vector<TCS::TOBArray const *> & input,
                                            const std::vector<TCS::TOBArray *> & output,
                                            Decision & decision) {
   
   if (input.size() != 1){
      TCS_EXCEPTION("ADBDT algorithm expects only one TOBArray input (muons), but got " << input.size());
   }

   const TCS::TOBArray* muons = input[0];
   
   //Reading in number of muons:
   TRG_MSG_DEBUG("There are " << muons->size() << " muons."); 
   
   if (muons->size() < 2) {
      TRG_MSG_DEBUG("There are less than 2 muons. Skipping event.");
      m_totalScore = 0;
      return StatusCode::SUCCESS;
   }
   
   bool hasAmbiguousInputs = TSU::isAmbiguousAnywhere(muons, p_MaxTob, std::min(p_minEt1, p_minEt2));
   
   int64_t maxScore = 0;
   
   size_t nMuons = p_MaxTob > 0 ? std::min(muons->size(), p_MaxTob) : muons->size();
   //We define muon pairs based on the 2-3 leading muons
   std::vector<std::pair<int, int>> muonPairs;
   for (size_t i = 0; i < nMuons; ++i) {
     for (size_t j = i+1; j < nMuons; ++j) {
       muonPairs.emplace_back(i, j);
     }
   }
   
   for (const auto& [i,j] : muonPairs) {

      std::vector<int64_t> eventValues;
    
      const auto& mu1 = (*muons)[i];
      const auto& mu2 = (*muons)[j];
      
      //ignore combinations failing minET cuts
      if ( parType_t( mu1.Et() ) <= p_minEt1 ) continue; 
      if ( parType_t( mu2.Et() ) <= p_minEt2 ) continue; 
      
      if (scale(mu1.Et(),m_mu1_ptmin, m_mu1_ptmax) != mapval(mu1.Et(), "pt") 
              || scale(mu1.eta()/40.0, m_mu1_etamin, m_mu1_etamax) != mapval(mu1.eta(), "eta")
	      || scale(mu1.phi()/20.0, m_mu1_phimin, m_mu1_phimax) != mapval(mu1.phi(), "phi")
              || scale(mu2.Et(),m_mu2_ptmin, m_mu2_ptmax) != mapval(mu2.Et(), "pt")
	      || scale(mu2.eta()/40.0, m_mu2_etamin, m_mu2_etamax) != mapval(mu2.eta(), "eta")
              || scale(mu2.phi()/20.0, m_mu2_phimin, m_mu2_phimax) != mapval(mu2.phi(), "phi")){

         TRG_MSG_WARNING("The math function for the mapping of Muon pt,eta,or phi does not match the LUT mapping. Check LUT, 'scale' function, or if illegal vartype was given to 'mapval' function");
      
      }	 
     
      if (mapval(mu1.Et(), "pt") == int(mu1.Et()) || mapval(mu2.Et(), "pt") == int(mu2.Et())) {
     
         TRG_MSG_WARNING("No input transformation done by LUT for muon ET");
     
      }
      
      if (mapval(mu1.eta(), "eta") == mu1.eta() || mapval(mu2.eta(), "eta") == mu2.eta()) {
     
         TRG_MSG_WARNING("No input transformation done by LUT for muon Eta");
     
      }

      if (mapval(mu1.phi(), "phi") == mu1.phi() || mapval(mu2.phi(), "phi") == mu2.phi()) {
     
         
	 TRG_MSG_DEBUG("No input transformation done by LUT for muon Phi"); //Not a warning, since current transformation for phi
	 						  		    //does not change the value except for one input number
      }

      if (m_sim_mode == "math"){
         eventValues.push_back(scale(mu1.Et(), m_mu1_ptmin, m_mu1_ptmax)); // muon pT (100MeV)
         eventValues.push_back(scale(mu1.eta()/40.0, m_mu1_etamin, m_mu1_etamax)); // muon eta (25mrad)
         eventValues.push_back(scale(mu1.phi()/20.0, m_mu1_phimin, m_mu1_phimax)); // muon phi (50mrad)
         
         eventValues.push_back(scale(mu2.Et(), m_mu2_ptmin, m_mu2_ptmax)); // muon pT (100MeV)
         eventValues.push_back(scale(mu2.eta()/40.0, m_mu2_etamin, m_mu2_etamax)); // muon eta (25mrad)
         eventValues.push_back(scale(mu2.phi()/20.0, m_mu2_phimin, m_mu2_phimax)); // muon phi (50mrad)
      }
    
      else if(m_sim_mode == "lut"){
         eventValues.push_back(mapval(mu1.Et(), "pt")); 
         eventValues.push_back(mapval(mu1.eta(), "eta")); 
         eventValues.push_back(mapval(mu1.phi(), "phi")); 
         
         eventValues.push_back(mapval(mu2.Et(), "pt")); 
         eventValues.push_back(mapval(mu2.eta(), "eta")); 
         eventValues.push_back(mapval(mu2.phi(), "phi")); 
      }
      
      else{
         eventValues.push_back(scale(mu1.Et(), m_mu1_ptmin, m_mu1_ptmax)); // muon pT (100MeV)
         eventValues.push_back(scale(mu1.eta()/40.0, m_mu1_etamin, m_mu1_etamax)); // muon eta (25mrad)
         eventValues.push_back(scale(mu1.phi()/20.0, m_mu1_phimin, m_mu1_phimax)); // muon phi (50mrad)
         
         eventValues.push_back(scale(mu2.Et(), m_mu2_ptmin, m_mu2_ptmax)); // muon pT (100MeV)
         eventValues.push_back(scale(mu2.eta()/40.0, m_mu2_etamin, m_mu2_etamax)); // muon eta (25mrad)
         eventValues.push_back(scale(mu2.phi()/20.0, m_mu2_phimin, m_mu2_phimax)); // muon phi (50mrad)
      }
   
      // calculate the score as sum of score from all trees
      int64_t score = 0;
      for (Tree tree : m_trees) {
         score += tree.getTreeScore(eventValues);
      }
    
      // from all muon combinations keep the highest score 
      if (score > maxScore) {
         maxScore = score;
      }
   } 
    
   m_totalScore = maxScore; 
   TRG_MSG_DEBUG("Anomaly Score = " << m_totalScore);

   //Now, we compare and set the decision bit:
   for (size_t i=0; i<numberOutputBits(); ++i){
      bool accept = false;
      // const bool fillAccept = fillHistos() && (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
      // const bool fillReject = fillHistos() && !fillAccept;
      // const bool alreadyFilled = decision.bit(i);
      
      if (m_totalScore > p_ScoreThreshold[i]) {
         accept = true;
         decision.setBit(i,true);
         for (size_t k = 0; k < 3 && k < muons->size(); ++k){
            output[i]->push_back((*muons)[k]);
         }
      }
      
      output[i]->setAmbiguityFlag(hasAmbiguousInputs);
      
      if(fillHistos()) {
         const bool fillAccept = fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept;
         if (fillAccept) {
            fillHist1D(m_histAccept[i], m_totalScore);
         } else {
            fillHist1D(m_histReject[i], m_totalScore);
         }
      }

      TRG_MSG_DEBUG("Decision for bit " << i << ": " <<(accept?"pass":"fail") << "with an anomaly score = " << m_totalScore << std::endl);    
   }
   return StatusCode::SUCCESS;
}

TCS::StatusCode TCS::AnomalyDetectionBDT::process(const std::vector<TCS::TOBArray const *> & input,
                                               const std::vector<TCS::TOBArray *> & output,
                                               Decision & decision) {
#ifdef TRIGCONF_STANDALONE
  return StatusCode::SUCCESS;
#endif
   TCS::StatusCode status = processBitCorrect(input,output,decision);
   if (!status.isSuccess()){
      return status;
   }	
    
   // Check if totalScore exceeds the bits that we expect
   if (m_totalScore >= (1L << 8)) {
      TRG_MSG_WARNING("ADBDT:The calculated anomaly score exceeds the allowed number of bits.");
   }

	return TCS::StatusCode::SUCCESS;
}
