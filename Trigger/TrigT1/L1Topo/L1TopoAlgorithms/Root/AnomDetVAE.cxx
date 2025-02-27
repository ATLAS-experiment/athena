/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * AnomDetVAE.cxx
 * Created by Sagar Addepalli on 25/11/2024.
 * 
 * @brief algorithm uses a variational auto-encoder based anomaly detection network
 * Uses 6 jets, 4 muons, 4 taus, and MET to calculate the anomaly score
 * based on the KL-divergence in a lower dimension latent space
 *
 * @param AnomalyScoreThresh
**********************************/

#include <cmath>

#include "L1TopoAlgorithms/AnomDetVAE.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"
#include <VAENetwork.h>

REGISTER_ALG_TCS(ADVAE_2A)


TCS::ADVAE_2A::ADVAE_2A(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 6);
   defineParameter("InputWidth2", 6);
   defineParameter("InputWidth3", 6);
   defineParameter("InputWidth4", 1);
   defineParameter("MaxTob1", 6);
   defineParameter("MaxTob2", 4);
   defineParameter("MaxTob3", 4);
   defineParameter("MaxTob4", 1);
   defineParameter("NumResultBits", 2);

   // Version parameter, used for L1TopoFW book-keeping, no practical application
   defineParameter("ADVAEVersion", 1);
   //minEt cuts, one per input list (TOBs failing these are set to ET = eta = phi = 0)
   defineParameter("MinET1",0);
   defineParameter("MinET2",0);
   defineParameter("MinET3",0);
   defineParameter("MinET4",0);
   //The value used is sum of squares of the NN result vector elements, bit-shifted by 8 to get all decimal bits
   defineParameter("AnomalyScoreThresh", 1000000, 0);
   defineParameter("AnomalyScoreThresh", 1000000, 1);

   setNumberOutputBits(2);
}

TCS::ADVAE_2A::~ADVAE_2A(){}


TCS::StatusCode
TCS::ADVAE_2A::initialize() {
   p_NumberLeading1 = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   p_NumberLeading3 = parameter("InputWidth3").value();
   p_NumberLeading4 = parameter("InputWidth4").value();

   if(parameter("MaxTob1").value() > 0) p_NumberLeading1 = parameter("MaxTob1").value();
   if(parameter("MaxTob2").value() > 0) p_NumberLeading2 = parameter("MaxTob2").value();
   if(parameter("MaxTob3").value() > 0) p_NumberLeading3 = parameter("MaxTob3").value();
   if(parameter("MaxTob4").value() > 0) p_NumberLeading4 = parameter("MaxTob4").value();

   p_minEt1 = parameter("MinET1").value();
   p_minEt2 = parameter("MinET2").value();
   p_minEt3 = parameter("MinET3").value();
   p_minEt4 = parameter("MinET4").value();

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_AnomalyScoreThresh[i] = parameter("AnomalyScoreThresh", i).value();
   }

   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      std::string hname_accept = "hAnomalyScore_accept_bit"+std::to_string((int)i);
      std::string hname_reject = "hAnomalyScore_reject_bit"+std::to_string((int)i);
      // score
      bookHist(m_histAccept, hname_accept, "ADScore", 2000, 0, 2000000);
      bookHist(m_histReject, hname_reject, "ADScore", 2000, 0, 2000000);
   }

   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::ADVAE_2A::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                                  const std::vector<TCS::TOBArray *> & output,
                                  Decision & decision )
{


   if( input.size() == 4) {

      TCS::TOBArray const* jets = input[0];
      TCS::TOBArray const* taus = input[1];
      TCS::TOBArray const* mus  = input[2];
      TCS::TOBArray const* met  = input[3];
      TRG_MSG_DEBUG("Number of jets are " << (*jets).size());
      TRG_MSG_DEBUG("Number of taus are " << (*taus).size());
      TRG_MSG_DEBUG("Number of mus are " << (*mus).size());
      TRG_MSG_DEBUG("Number of met are " << (*met).size());

      std::vector<u_int> jet_pt(6,0), tau_pt(4,0), mu_pt(4,0), met_pt(1,0);
      std::vector<int>   jet_eta(6,0), tau_eta(4,0), mu_eta(4,0); //no met_eta
      std::vector<int>   jet_phi(6,0), tau_phi(4,0), mu_phi(4,0), met_phi(1,0);

      for (u_int i = 0; i<(*jets).size() && i<6; ++i) {
         if ( parType_t( (*jets)[i].Et() ) <= p_minEt1 ) continue; //ET cut, leave NN inputs at default values (0)
         jet_pt[i] = (*jets)[i].Et();
         jet_eta[i] = (*jets)[i].eta();
         jet_phi[i] = (*jets)[i].phi();
      }
      for (u_int i = 0; i < (*taus).size() && i<4; ++i) {
         if ( parType_t( (*taus)[i].Et() ) <= p_minEt2 ) continue; //ET cut, leave NN inputs at default values (0)
         tau_pt[i] = (*taus)[i].Et();
         tau_eta[i] = (*taus)[i].eta();
         tau_phi[i] = (*taus)[i].phi();
      }
      for (u_int i = 0; i < (*mus).size() && i<4; ++i) {
         if ( parType_t( (*mus)[i].Et() ) <= p_minEt3 ) continue; //ET cut, leave NN inputs at default values (0)
         mu_pt[i] = (*mus)[i].Et();
         mu_eta[i] = (*mus)[i].eta();
         mu_phi[i] = (*mus)[i].phi();
      }
      for (u_int i = 0; i < (*met).size() && i<1; ++i) {
         if ( parType_t( (*met)[i].Et() ) <= p_minEt4 ) continue; //ET cut, leave NN inputs at default values (0)
         met_pt[i] = (*met)[i].Et();
         met_phi[i] = (*met)[i].phi();
      }

      ADVAE2A::VAENetwork AD_Network( jet_pt[0], jet_eta[0], jet_phi[0],
                              jet_pt[1], jet_eta[1], jet_phi[1],
                              jet_pt[2], jet_eta[2], jet_phi[2],
                              jet_pt[3], jet_eta[3], jet_phi[3],
                              jet_pt[4], jet_eta[4], jet_phi[4],
                              jet_pt[5], jet_eta[5], jet_phi[5],
                              tau_pt[0], tau_eta[0], tau_phi[0],
                              tau_pt[1], tau_eta[1], tau_phi[1],
                              tau_pt[2], tau_eta[2], tau_phi[2],
                              tau_pt[3], tau_eta[3], tau_phi[3],
                              mu_pt [0], mu_eta [0], mu_phi [0],
                              mu_pt [1], mu_eta [1], mu_phi [1],
                              mu_pt [2], mu_eta [2], mu_phi [2],
                              mu_pt [3], mu_eta [3], mu_phi [3],
                              met_pt[0], met_phi[0] );
      int64_t anomScoreInt64 = AD_Network.getAnomalyScoreInt64();

      for(u_int i=0; i<numberOutputBits(); ++i) {
         bool accept = false;
         int32_t threshold = int32_t ( p_AnomalyScoreThresh[i] );
         if ( anomScoreInt64 > threshold ) {
            accept = true;
            decision.setBit(i, true);
            for ( u_int j = 0; j<6 && j<(*jets).size(); ++j ) output[i]->push_back((*jets)[j]);
            for ( u_int j = 0; j<4 && j<(*taus).size(); ++j ) output[i]->push_back((*taus)[j]);
            for ( u_int j = 0; j<4 && j<(*mus).size() ; ++j ) output[i]->push_back((*mus) [j]);
            output[i]->push_back((*met)[0]);
         }

         if(fillHistos() and accept) {
            fillHist1D(m_histAccept[i],anomScoreInt64);
         } else if(fillHistos() && !accept) {
            fillHist1D(m_histReject[i],anomScoreInt64);
         }

         TRG_MSG_DEBUG("Decision for bit" << i << ": " << (accept?"pass":"fail") << " anomaly score = " << anomScoreInt64 << std::endl);
      }
   } else {
      TCS_EXCEPTION("ADVAE_2A alg must have 4 inputs, but got " << input.size());
   }

   return TCS::StatusCode::SUCCESS;
}

TCS::StatusCode
TCS::ADVAE_2A::process( const std::vector<TCS::TOBArray const *> & input,
                        const std::vector<TCS::TOBArray *> & output,
                        Decision & decision )
{

      
   if( input.size() == 4) {

      TCS::TOBArray const* jets = input[0];
      TCS::TOBArray const* taus = input[1];
      TCS::TOBArray const* mus  = input[2];
      TCS::TOBArray const* met  = input[3];
      TRG_MSG_DEBUG("Number of jets are " << (*jets).size());
      TRG_MSG_DEBUG("Number of taus are " << (*taus).size());
      TRG_MSG_DEBUG("Number of mus are " << (*mus).size());
      TRG_MSG_DEBUG("Number of met are " << (*met).size());

      std::vector<u_int> jet_pt(6,0), tau_pt(4,0), mu_pt(4,0), met_pt(1,0);
      std::vector<int>   jet_eta(6,0), tau_eta(4,0), mu_eta(4,0); //no met_eta
      std::vector<int>   jet_phi(6,0), tau_phi(4,0), mu_phi(4,0), met_phi(1,0);

      for (u_int i = 0; i<(*jets).size() && i<6; ++i) {
         if ( parType_t( (*jets)[i].Et() ) <= p_minEt1 ) continue; //ET cut, leave NN inputs at default values (0)
         jet_pt[i] = (*jets)[i].Et();
         jet_eta[i] = (*jets)[i].eta();
         jet_phi[i] = (*jets)[i].phi();
      }
      for (u_int i = 0; i < (*taus).size() && i<4; ++i) {
         if ( parType_t( (*taus)[i].Et() ) <= p_minEt2 ) continue; //ET cut, leave NN inputs at default values (0)
         tau_pt[i] = (*taus)[i].Et();
         tau_eta[i] = (*taus)[i].eta();
         tau_phi[i] = (*taus)[i].phi();
      }
      for (u_int i = 0; i < (*mus).size() && i<4; ++i) {
         if ( parType_t( (*mus)[i].Et() ) <= p_minEt3 ) continue; //ET cut, leave NN inputs at default values (0)
         mu_pt[i] = (*mus)[i].Et();
         mu_eta[i] = (*mus)[i].eta();
         mu_phi[i] = (*mus)[i].phi();
      }
      for (u_int i = 0; i < (*met).size() && i<1; ++i) {
         if ( parType_t( (*met)[i].Et() ) <= p_minEt4 ) continue; //ET cut, leave NN inputs at default values (0)
         met_pt[i] = (*met)[i].Et();
         met_phi[i] = (*met)[i].phi();
      }

      ADVAE2A::VAENetwork AD_Network( jet_pt[0], jet_eta[0], jet_phi[0],
                              jet_pt[1], jet_eta[1], jet_phi[1],
                              jet_pt[2], jet_eta[2], jet_phi[2],
                              jet_pt[3], jet_eta[3], jet_phi[3],
                              jet_pt[4], jet_eta[4], jet_phi[4],
                              jet_pt[5], jet_eta[5], jet_phi[5],
                              tau_pt[0], tau_eta[0], tau_phi[0],
                              tau_pt[1], tau_eta[1], tau_phi[1],
                              tau_pt[2], tau_eta[2], tau_phi[2],
                              tau_pt[3], tau_eta[3], tau_phi[3],
                              mu_pt [0], mu_eta [0], mu_phi [0],
                              mu_pt [1], mu_eta [1], mu_phi [1],
                              mu_pt [2], mu_eta [2], mu_phi [2],
                              mu_pt [3], mu_eta [3], mu_phi [3],
                              met_pt[0], met_phi[0] );
      int64_t anomScoreInt64 = AD_Network.getAnomalyScoreInt64();

      for(u_int i=0; i<numberOutputBits(); ++i) {
         bool accept = false;
         int32_t threshold = int32_t ( p_AnomalyScoreThresh[i] );
         if ( anomScoreInt64 > threshold ) {
            accept = true;
            decision.setBit(i, true);
            for ( u_int j = 0; j<6 && j<(*jets).size(); ++j ) output[i]->push_back((*jets)[j]);
            for ( u_int j = 0; j<4 && j<(*taus).size(); ++j ) output[i]->push_back((*taus)[j]);
            for ( u_int j = 0; j<4 && j<(*mus).size() ; ++j ) output[i]->push_back((*mus) [j]);
            output[i]->push_back((*met)[0]);
         }

         if(fillHistos() and accept) {
            fillHist1D(m_histAccept[i],anomScoreInt64);
         } else if(fillHistos() && !accept) {
            fillHist1D(m_histReject[i],anomScoreInt64);
         }

         TRG_MSG_DEBUG("Decision for bit" << i << ": " << (accept?"pass":"fail") << " anomaly score = " << anomScoreInt64 << std::endl);
      }
   } else {
      TCS_EXCEPTION("ADVAE_2A alg must have 4 inputs, but got " << input.size());
   }

   return TCS::StatusCode::SUCCESS;
}
