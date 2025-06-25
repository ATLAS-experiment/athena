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
   size_t scale(int value, float minVal, float maxVal, int binMinRange=0, int binMaxRange=127) {
      return 1+(value-minVal)/(maxVal-minVal)*(binMaxRange-binMinRange);
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
   // throw std::runtime_error("AnomalyDetection: the code has reached an unreachable state");
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
   setNumberOutputBits(2); // 2 decision bit for 2 different score thresholds
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
      Tree t = Tree(tree, m_nVar);
      m_trees.push_back(t);
   }

   TRG_MSG_DEBUG("In initialize. There are " << m_trees.size() << " trees for AnomalyDetectionBDT.");
   
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

   if (input.size() != 1){
      TCS_EXCEPTION("ADBDT algorithm expects only one TOBArray input (muons), but got " << input.size());
   }

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
      
      eventValues.push_back(scale(mu1.Et(), m_mu1_ptmin, m_mu1_ptmax)); // muon pT (100MeV)
      eventValues.push_back(scale(mu1.eta()/40, m_mu1_etamin, m_mu1_etamax)); // muon eta (25mrad)
      eventValues.push_back(scale(mu1.phi()/20, m_mu1_phimin, m_mu1_phimax)); // muon phi (50mrad)
    
      eventValues.push_back(scale(mu2.Et(), m_mu2_ptmin, m_mu2_ptmax)); // muon pT (100MeV)
      eventValues.push_back(scale(mu2.eta()/40, m_mu2_etamin, m_mu2_etamax)); // muon eta (25mrad)
      eventValues.push_back(scale(mu2.phi()/20, m_mu2_phimin, m_mu2_phimax)); // muon phi (50mrad)
   
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
