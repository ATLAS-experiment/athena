/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// AnomalyDetectionBDT.h
// Created by Santiago Cané on 4/15/2025
*/

#ifndef __TopoCore__AnomalyDetectionBDT__
#define __TopoCore__AnomalyDetectionBDT__

#include <iostream>
#include <vector>
#include "L1TopoInterfaces/DecisionAlg.h"
#include "nlohmann/json.hpp"

namespace TCS {
   class Bin {
   public:
      // constructor
      Bin(const nlohmann::json& obj, int nVars);
         
      // check if input event is inside the bin
      bool isInside(const std::vector<int64_t>& inputEvent) const;
      
      // output streaming
      friend std::ostream& operator<<(std::ostream& os, const Bin& bin);

      int64_t score; // score of the bin

   private:
      int nVar; // dimension 
      std::vector<int64_t> minVals; // lower bin limit for each dimension
      std::vector<int64_t> maxVals; // upper bin limit for each dimension
   };
   
   class Tree {
   public:
      // constructor
      Tree(const nlohmann::json& obj, int nVars);
      
      // score for input event (score of bin which contains the input event)
      int64_t getTreeScore(const std::vector<int64_t>& inputEvent) const;

   private:
      std::vector<Bin> bins; // all bins of the tree
   };


   class AnomalyDetectionBDT : public DecisionAlg {
   public:
      AnomalyDetectionBDT(const std::string & name);
      virtual ~AnomalyDetectionBDT();

      virtual StatusCode initialize();

      virtual StatusCode processBitCorrect(const std::vector<TCS::TOBArray const *> & input,
                                           const std::vector<TCS::TOBArray *> & output,
                                           Decision & decision);

      virtual StatusCode process(const std::vector<TCS::TOBArray const *> & input,
                                 const std::vector<TCS::TOBArray *> & output,
                                 Decision & decision);

   private:
      std::vector<Tree> m_trees;  // list of all trees
      int m_nVar{0};              // dimension of the BDT
      int64_t m_totalScore{0};    // score of the input event
      float m_mu1_ptmin, m_mu1_ptmax, m_mu1_etamin, m_mu1_etamax, m_mu1_phimin, m_mu1_phimax; // range of the first muon
      float m_mu2_ptmin, m_mu2_ptmax, m_mu2_etamin, m_mu2_etamax, m_mu2_phimin, m_mu2_phimax; // range of the second muon
      parType_t p_ScoreThreshold[2] = { 0, 0 };
   };
}

#endif 

