/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGEPPERF_WTACONEJETMAKER_H
#define TRIGGEPPERF_WTACONEJETMAKER_H

#include "IJetMaker.h"
#include "Jet.h"
#include "Cluster.h"

#include "TrigGepPerf/WTAConeMaker.h" // WTAConeMaker is the core header
#include "TrigGepPerf/WTACone2PassMaker.h" // WTACone2PassMaker is the 2-Pass header
#include "TrigGepPerf/WTAConeParallelHelper.h"

#include <string>
#include <vector>
#include <memory>

 enum WTAConeMakerEnum{ // use WTAConeMakerEnum for algorithm variants
  Baseline = 0,
  TwoPass = 1
};

 
 namespace Gep
 {
   class WTAConeJetMaker : public IJetMaker
   {
   public:
 
   WTAConeJetMaker(unsigned int block_n = 4, unsigned int rolloff_buffersize = 155) :
           m_GEPWTAParameters(),
           m_BlockN(block_n), m_SeedCleaningAlgo(0), m_RollOffBufferSize(rolloff_buffersize)
           {};
 
     
     virtual std::string toString() const override{ return "WTAConeJet"; }
     virtual std::vector<Gep::Jet> makeJets(const std::vector<Gep::Cluster>& TopoTowers) const override; // To makeJet, m_seeds and m_consts must not be empty vectors

     std::unique_ptr<WTAConeMaker> CreateWTAConeMaker(enum WTAConeMakerEnum seed_cleaning_algo) const
     { // Allow user to choose the seed cleaning algorithm
        switch(seed_cleaning_algo)
        {
            case Baseline:
                return std::make_unique<WTAConeMaker>();
            case TwoPass:
                return std::make_unique<WTACone2PassMaker>();
        }
        return nullptr;
    }

     void SetBlockN(unsigned int block_n){m_BlockN = block_n;};
     void SetSeedCleaningAlgo(unsigned int algo){m_SeedCleaningAlgo = algo;};
     void SetRollOffBufferSize(int rolloff_buffersize){m_RollOffBufferSize = rolloff_buffersize;}
     int GetRollOffBufferSize(){return m_RollOffBufferSize;}

     #ifdef FLOATING_POINT_SIMULATION
      // Left empty on purpose
     #else
      // Convertors between the floating point and integer
      WTATrigObj fTower_to_iTower(const Gep::Cluster& topotower, int idx = -99) const { // Need to convert to iTower
            // Convert floating-point physical values to fixed-point integers
            
            // Step 1: Scale float values to integers
            const double pt_scaled  = topotower.vec.Pt()  / LSB;
            const double m_scaled   = topotower.vec.M()   / LSB;
            const double eta_scaled = (topotower.vec.Eta() + fl_ETA_MAX) / ETA_WIDTH;
            const double phi_scaled = (topotower.vec.Phi() + fl_PHI_MAX) / PHI_WIDTH;

            // Step 2: Explicitly cast to int, then construct FixedInt types
            pt_t  i_pt (static_cast<int>(pt_scaled));
            m_t   i_m  (static_cast<int>(m_scaled));
            eta_t i_eta(static_cast<int>(eta_scaled));
            phi_t i_phi(static_cast<int>(phi_scaled));

            // Step 3: Return constructed WTATrigObj
            return WTATrigObj(i_pt, i_eta, i_phi, i_m, idx);
      }
      Gep::Jet iJet_to_fJet(const WTAJet &iJet) const { // Need to convert to fJet

            // Step 1: Extract raw integer values from FixedInt
            #ifdef BITWISE_SIMULATION
              const double f_pt  = static_cast<double>(iJet.pt().raw())  * LSB;
              const double f_m   = static_cast<double>(iJet.m().raw())   * LSB;
              const double f_eta = static_cast<double>(iJet.eta().raw()) * ETA_WIDTH - fl_ETA_MAX;
              const double f_phi = static_cast<double>(iJet.phi().raw()) * PHI_WIDTH - fl_PHI_MAX;
            #elif defined(INTEGER_SIMULATION)
              const double f_pt  = static_cast<double>(iJet.pt())  * LSB;
              const double f_m   = static_cast<double>(iJet.m())   * LSB;
              const double f_eta = static_cast<double>(iJet.eta()) * ETA_WIDTH - fl_ETA_MAX;
              const double f_phi = static_cast<double>(iJet.phi()) * PHI_WIDTH - fl_PHI_MAX;
            #endif

            // Step 2: Construct and return floating-point Gep::Jet
            Gep::Jet fJet;
            fJet.vec.SetPtEtaPhiM(f_pt, f_eta, f_phi, f_m);

            // Step 3: Include the ERing information and return floating-point Gep::Jet
            WTA4JetERingInfo ering_info = iJet.GetERingInfo();
            #ifdef BITWISE_SIMULATION
              fJet.ring0_Et = static_cast<double>(ering_info.ring0_Et.raw()) * LSB;
              fJet.ring1_Et = static_cast<double>(ering_info.ring1_Et.raw()) * LSB;
              fJet.ring2_Et = static_cast<double>(ering_info.ring2_Et.raw()) * LSB;
              fJet.ring3_Et = static_cast<double>(ering_info.ring3_Et.raw()) * LSB;
              fJet.ring4_Et = static_cast<double>(ering_info.ring4_Et.raw()) * LSB;
            #elif defined(INTEGER_SIMULATION)
              fJet.ring0_Et = static_cast<double>(ering_info.ring0_Et) * LSB;
              fJet.ring1_Et = static_cast<double>(ering_info.ring1_Et) * LSB;
              fJet.ring2_Et = static_cast<double>(ering_info.ring2_Et) * LSB;
              fJet.ring3_Et = static_cast<double>(ering_info.ring3_Et) * LSB;
              fJet.ring4_Et = static_cast<double>(ering_info.ring4_Et) * LSB;
            #endif
            fJet.total_TobN = ering_info.total_TobN;
            fJet.ring0_TobN = ering_info.ring0_TobN;
            fJet.ring1_TobN = ering_info.ring1_TobN;
            fJet.ring2_TobN = ering_info.ring2_TobN;
            fJet.ring3_TobN = ering_info.ring3_TobN;
            fJet.ring4_TobN = ering_info.ring4_TobN;
            return fJet;
      }
      #endif
 
     WTAParameters m_GEPWTAParameters;
 
   private:
    //  WTAConeParallelHelper m_WTAParallelHelper;
     unsigned int m_BlockN{};
     unsigned int m_SeedCleaningAlgo{};
     unsigned int m_RollOffBufferSize{}; // Only for TwoPass
 
   };
 
 }
 
 #endif //TRIGGEPPERF_EXCONEJETMAKER_H
 