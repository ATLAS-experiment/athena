/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * InvariantMassDeltaPhiSumEtInclusive1.cxx
 * Based on implementation of InvariantMassDeltaPhiInclusive2. For questions contact atlas-trig-l1topo-algcom@cern.ch.
 *
 * @brief algorithm calculates the sqr of the INVMASS between all element of a list and applies invmass criteria. 
 * For pairs passing the INVMASS cut a further requirement based on DeltaPhi is applied, addressing ATR-19377
 *
**********************************/
//  TO DO size of the input list to be possbly refined 

#include "L1TopoAlgorithms/InvariantMassDeltaPhiSumEtInclusive1.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

#include <cmath>

REGISTER_ALG_TCS(InvariantMassDeltaPhiSumEtInclusive1)


TCS::InvariantMassDeltaPhiSumEtInclusive1::InvariantMassDeltaPhiSumEtInclusive1(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth", 9);
   defineParameter("MaxTob", 0);
   defineParameter("NumResultBits", 6);
   defineParameter("MinMSqr",   0, 0);
   defineParameter("MaxMSqr", 999, 0);
   defineParameter("MinMSqr",   0, 1);
   defineParameter("MaxMSqr", 999, 1);
   defineParameter("MinMSqr",   0, 2);
   defineParameter("MaxMSqr", 999, 2);
   defineParameter("MinMSqr",   0, 3);
   defineParameter("MaxMSqr", 999, 3);
   defineParameter("MinMSqr",   0, 4);
   defineParameter("MaxMSqr", 999, 4);
   defineParameter("MinMSqr",   0, 5);
   defineParameter("MaxMSqr", 999, 5);
   defineParameter("MinSumEt",   0, 0);
   defineParameter("MaxSumEt", 999, 0);
   defineParameter("MinSumEt",   0, 1);
   defineParameter("MaxSumEt", 999, 1);
   defineParameter("MinSumEt",   0, 2);
   defineParameter("MaxSumEt", 999, 2);
   defineParameter("MinSumEt",   0, 3);
   defineParameter("MaxSumEt", 999, 3);
   defineParameter("MinSumEt",   0, 4);
   defineParameter("MaxSumEt", 999, 4);
   defineParameter("MinSumEt",   0, 5);
   defineParameter("MaxSumEt", 999, 5);
   defineParameter("MinET1",0,0);
   defineParameter("MinET1",0,1);
   defineParameter("MinET1",0,2);
   defineParameter("MinET1",0,3);
   defineParameter("MinET1",0,4);
   defineParameter("MinET1",0,5);
   defineParameter("MinET2",0,0);
   defineParameter("MinET2",0,1);
   defineParameter("MinET2",0,2);
   defineParameter("MinET2",0,3);
   defineParameter("MinET2",0,4);
   defineParameter("MinET2",0,5);
   defineParameter("ApplyEtaCut", 0);
   defineParameter("MinEta1",  0);
   defineParameter("MaxEta1", 31);
   defineParameter("MinEta2",  0);
   defineParameter("MaxEta2", 31);
   defineParameter("MinDeltaPhi",  0, 0);
   defineParameter("MaxDeltaPhi", 31, 0);
   defineParameter("MinDeltaPhi",  0, 1);
   defineParameter("MaxDeltaPhi", 31, 1);
   defineParameter("MinDeltaPhi",  0, 2);
   defineParameter("MaxDeltaPhi", 31, 2);
   defineParameter("MinDeltaPhi",  0, 3);
   defineParameter("MaxDeltaPhi", 31, 3);
   defineParameter("MinDeltaPhi",  0, 4);
   defineParameter("MaxDeltaPhi", 31, 4);
   defineParameter("MinDeltaPhi",  0, 5);
   defineParameter("MaxDeltaPhi", 31, 5);
   //does this need to change? 
   setNumberOutputBits(6);
}

TCS::InvariantMassDeltaPhiSumEtInclusive1::~InvariantMassDeltaPhiSumEtInclusive1(){}


TCS::StatusCode
TCS::InvariantMassDeltaPhiSumEtInclusive1::initialize() {
   p_NumberLeading = parameter("InputWidth").value();
   if(parameter("MaxTob").value() > 0) p_NumberLeading = parameter("MaxTob").value();

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_InvMassMin[i] = parameter("MinMSqr", i).value();
      p_InvMassMax[i] = parameter("MaxMSqr", i).value();
      p_SumEtMin[i] = parameter("MinSumEt", i).value();
      p_SumEtMax[i] = parameter("MaxSumEt", i).value();
      p_DeltaPhiMin[i] = parameter("MinDeltaPhi", i).value();
      p_DeltaPhiMax[i] = parameter("MaxDeltaPhi", i).value();
      p_MinET1[i] = parameter("MinET1",i).value();
      p_MinET2[i] = parameter("MinET2",i).value();
   }

   TRG_MSG_INFO("NumberLeading : " << p_NumberLeading);
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
    TRG_MSG_INFO("InvMassMin   : " << p_InvMassMin[i]);
    TRG_MSG_INFO("InvMassMax   : " << p_InvMassMax[i]);    
    TRG_MSG_INFO("SumEtMin   : " << p_SumEtMin[i]);
    TRG_MSG_INFO("SumEtMax   : " << p_SumEtMax[i]);    
    TRG_MSG_INFO("DeltaPhiMin   : " << p_DeltaPhiMin[i]);
    TRG_MSG_INFO("DeltaPhiMax   : " << p_DeltaPhiMax[i]);
    TRG_MSG_INFO("MinET1          : " << p_MinET1[i]);
    TRG_MSG_INFO("MinET2          : " << p_MinET2[i]);
   }

   p_ApplyEtaCut = parameter("ApplyEtaCut").value();
   p_MinEta1     = parameter("MinEta1"    ).value();
   p_MaxEta1     = parameter("MaxEta1"    ).value();
   p_MinEta2     = parameter("MinEta2"    ).value();
   p_MaxEta2     = parameter("MaxEta2"    ).value();
   TRG_MSG_INFO("ApplyEtaCut : "<<p_ApplyEtaCut );
   TRG_MSG_INFO("MinEta1     : "<<p_MinEta1     );
   TRG_MSG_INFO("MaxEta1     : "<<p_MaxEta1     );
   TRG_MSG_INFO("MinEta2     : "<<p_MinEta2     );
   TRG_MSG_INFO("MaxEta2     : "<<p_MaxEta2    );

   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
       std::string hname_accept = "hInvariantMassDeltaPhiSumEtInclusive1_accept_bit"+std::to_string((int)i);
       std::string hname_reject = "hInvariantMassDeltaPhiSumEtInclusive1_reject_bit"+std::to_string((int)i);
       // mass
       bookHist(m_histAcceptM, hname_accept, "INVM vs DPHI", 100, sqrt(p_InvMassMin[i]), sqrt(p_InvMassMax[i]), 100, p_DeltaPhiMin[i], p_DeltaPhiMax[i]);
       bookHist(m_histRejectM, hname_reject, "INVM vs DPHI", 100, sqrt(p_InvMassMin[i]), sqrt(p_InvMassMax[i]), 100, p_DeltaPhiMin[i], p_DeltaPhiMax[i]);
       // eta2 vs. eta1
       bookHist(m_histAcceptEta1Eta2, hname_accept, "ETA vs ETA", 100, p_MinEta1, p_MaxEta1, 100, p_MinEta2, p_MaxEta2);
       bookHist(m_histRejectEta1Eta2, hname_reject, "ETA vs ETA", 100, p_MinEta1, p_MaxEta1, 100, p_MinEta2, p_MaxEta2);
   }
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::InvariantMassDeltaPhiSumEtInclusive1::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{
  int k=0; int j;
   if( input.size() == 1) {
      for( TOBArray::const_iterator tob1 = input[0]->begin();
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading;
           ++tob1)
         {
	    k++; j=0;
            if (p_NumberLeading < input[0]->size()) { 
               TCS::TOBArray::const_iterator tob1_plus1 = tob1; ++tob1_plus1;
               if ((*tob1)->Et() == (*tob1_plus1)->Et() && distance(input[0]->begin(), tob1) == p_NumberLeading - 1) { 
                  for(unsigned int i=0; i<numberOutputBits(); ++i) {
                     output[i]->setAmbiguityFlag(true); 
                  }
               }
            }
            for( TCS::TOBArray::const_iterator tob2 = input[0]->begin();
                 tob2 != input[0]->end() && distance(input[0]->begin(), tob2) < p_NumberLeading;
                 ++tob2) {
	        j++; if(j==k) continue; //Avoid diagonal cases. Both above and below diagonal cases are calculated due to asymmetric ET and eta cuts

	        // Inv Mass calculation
                unsigned int invmass2 = calcInvMassBW( *tob1, *tob2 );
		// test DeltaPhiMin, DeltaPhiMax                                       
		unsigned int deltaPhi = calcDeltaPhiBW( *tob1, *tob2 );
                const int eta1 = (*tob1)->eta();
                const int eta2 = (*tob2)->eta();
                const unsigned int aeta1 = std::abs(eta1);
                const unsigned int aeta2 = std::abs(eta2);
		for(unsigned int i=0; i<numberOutputBits(); ++i) {
		   bool accept = false;
		   if( parType_t((*tob1)->Et()) + parType_t((*tob2)->Et()) <= p_SumEtMin[i] ) continue; // MinSumEt cut
		   if( parType_t((*tob1)->Et()) + parType_t((*tob2)->Et()) >= p_SumEtMax[i] ) continue; // MaxSumEt cut
                   if( parType_t((*tob1)->Et()) <= p_MinET1[i]) continue; // ET cut
                   if( parType_t((*tob2)->Et()) <= p_MinET2[i]) continue; // ET cut
                   if(p_ApplyEtaCut &&
                      ((aeta1 < p_MinEta1 || aeta1 > p_MaxEta1 ) ||
                       (aeta2 < p_MinEta2 || aeta2 > p_MaxEta2 ) ))  continue;
                   accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && deltaPhi >= p_DeltaPhiMin[i] && deltaPhi <= p_DeltaPhiMax[i];
                   const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                   const bool fillReject = fillHistos() and not fillAccept;
                   const bool alreadyFilled = decision.bit(i);
                   if( accept ) {
                       decision.setBit(i, true);
                       output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                   }
                   if(fillAccept and not alreadyFilled) {
		       fillHist2D(m_histAcceptM[i],sqrt((float)invmass2),(float)deltaPhi);
		       fillHist2D(m_histAcceptEta1Eta2[i],eta1, eta2);
                   } else if(fillReject) {
		       fillHist2D(m_histRejectM[i],sqrt((float)invmass2),(float)deltaPhi);
		       fillHist2D(m_histRejectEta1Eta2[i],eta1, eta2);
                   }
                   TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2);
               }
            }
         }
   } else {

      TCS_EXCEPTION("InvariantMassDeltaPhiSumEtInclusive1 alg must have 1 inputs, but got " << input.size());

   }
   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::InvariantMassDeltaPhiSumEtInclusive1::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{


   if( input.size() == 1) {
      int k=0; int j;
      for( TOBArray::const_iterator tob1 = input[0]->begin();
           tob1 != input[0]->end() && distance(input[0]->begin(), tob1) < p_NumberLeading;
           ++tob1)
         {
    	    k++; j=0;
            if (p_NumberLeading < input[0]->size()) { 
               TCS::TOBArray::const_iterator tob1_plus1 = tob1; ++tob1_plus1;
               if ((*tob1)->Et() == (*tob1_plus1)->Et() && distance(input[0]->begin(), tob1) == p_NumberLeading - 1) { 
                  for(unsigned int i=0; i<numberOutputBits(); ++i) {
                     output[i]->setAmbiguityFlag(true); 
                  }
               }
            }
            for( TCS::TOBArray::const_iterator tob2 = input[0]->begin();
                 tob2 != input[0]->end() && distance(input[0]->begin(), tob2) < p_NumberLeading;
                 ++tob2) {
	        j++; if (j==k) continue; //Avoid diagonal cases. Both above and below diagonal cases are calculated due to asymmetric ET and eta cuts 
                // Inv Mass calculation
                unsigned int invmass2 = calcInvMass( *tob1, *tob2 );
		// test DeltaPhiMin, DeltaPhiMax                                     
		unsigned int deltaPhi = calcDeltaPhi( *tob1, *tob2 );
                const int eta1 = (*tob1)->eta();
                const int eta2 = (*tob2)->eta();
                const unsigned int aeta1 = std::abs(eta1);
                const unsigned int aeta2 = std::abs(eta2);
                for(unsigned int i=0; i<numberOutputBits(); ++i) {
		   bool accept = false; 
		   if( parType_t((*tob1)->Et()) + parType_t((*tob2)->Et()) <= p_SumEtMin[i] ) continue; // MinSumEt cut
		   if( parType_t((*tob1)->Et()) + parType_t((*tob2)->Et()) >= p_SumEtMax[i] ) continue; // MaxSumEt cut
                   if( parType_t((*tob1)->Et()) <= p_MinET1[i]) continue; // ET cut
                   if( parType_t((*tob2)->Et()) <= p_MinET2[i]) continue; // ET cut
                   if(p_ApplyEtaCut &&
                      ((aeta1 < p_MinEta1 || aeta1 > p_MaxEta1 ) ||
                       (aeta2 < p_MinEta2 || aeta2 > p_MaxEta2 ) )) continue;
                   accept = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i] && deltaPhi >= p_DeltaPhiMin[i] && deltaPhi <= p_DeltaPhiMax[i];
                   const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                   const bool fillReject = fillHistos() and not fillAccept;
                   const bool alreadyFilled = decision.bit(i);
                   if( accept ) {
                       decision.setBit(i, true);
                       output[i]->push_back( TCS::CompositeTOB(*tob1, *tob2) );
                   }
                   if(fillAccept and not alreadyFilled) {
		       fillHist2D(m_histAcceptM[i],sqrt((float)invmass2),(float)deltaPhi);
		       fillHist2D(m_histAcceptEta1Eta2[i],eta1, eta2);
                   } else if(fillReject) {
		       fillHist2D(m_histRejectM[i],sqrt((float)invmass2),(float)deltaPhi);
		       fillHist2D(m_histRejectEta1Eta2[i],eta1, eta2);
                   }
                  TRG_MSG_DEBUG("Decision " << i << ": " << (accept ?"pass":"fail") << " invmass2 = " << invmass2);
               }
            }
         }
   } else {
      TCS_EXCEPTION("InvariantMassDeltaPhiSumEtInclusive1 alg must have 1 inputs, but got " << input.size());
   }
   return TCS::StatusCode::SUCCESS;
}
