/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * InvariantMassInclusive1Disambiguation2.cpp
 * Created by Gabriel Oliveira Correa on 10/2025.
 * 
 * @brief Algorithm selects events based on invariant masses of TOB pairs from one list. Additionally there shall be a disambiguation with TOBs from a second list from which there must be at least one valid object that does not overlap (within a configurable distance DeltaR) with the TOBs forming the TOBs from the first list that form a given invM pair.
 *
 * @param NumberLeading
**********************************/

#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include "L1TopoAlgorithms/InvariantMassInclusive1Disambiguation2.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"
#include "L1TopoSimulationUtils/Helpers.h"

REGISTER_ALG_TCS(InvariantMassInclusive1Disambiguation2)


// not the best solution but we will move to athena where this comes for free
// #define LOG cout << "TCS::InvariantMassInclusive1Disambiguation2:     "

TCS::InvariantMassInclusive1Disambiguation2::InvariantMassInclusive1Disambiguation2(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth1", 6);
   defineParameter("InputWidth2", 6);
   defineParameter("MaxTob1", 0); 
   defineParameter("MaxTob2", 0); 
   defineParameter("NumResultBits", 6);
   setNumberOutputBits(6);

   for (unsigned int i=0;i<numberOutputBits();i++){
      // Algo parameters
      defineParameter("MinMSqr", 0, i);
      defineParameter("MaxMSqr", 999, i);
      defineParameter("MinET1a", 0, i);
      defineParameter("MinET1b", 0, i);
      defineParameter("MinET2", 0, i);
      defineParameter("DisambDRSqrMin", 0, i);
  }

}

TCS::InvariantMassInclusive1Disambiguation2::~InvariantMassInclusive1Disambiguation2(){}


TCS::StatusCode
TCS::InvariantMassInclusive1Disambiguation2::initialize() {

   p_NumberLeading1a = parameter("InputWidth1").value();
   p_NumberLeading1b = parameter("InputWidth1").value();
   p_NumberLeading2 = parameter("InputWidth2").value();
   if(parameter("MaxTob1").value() > 0) {
      p_NumberLeading1a = parameter("MaxTob1").value();
      p_NumberLeading1b = parameter("MaxTob1").value();
   }
   if(parameter("MaxTob2").value() > 0) {
      p_NumberLeading2 = parameter("MaxTob2").value();
   }

   TRG_MSG_INFO("NumberLeading1a : " << p_NumberLeading1a);
   TRG_MSG_INFO("NumberLeading1b : " << p_NumberLeading1b);
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_InvMassMin[i] = parameter("MinMSqr", i).value();
      p_InvMassMax[i] = parameter("MaxMSqr", i).value();
   
      p_MinET1a[i] = parameter("MinET1a",i).value();
      p_MinET1b[i] = parameter("MinET1b",i).value();
      p_MinET2[i] = parameter("MinET2",i).value();

      p_DisambDR[i] = parameter("DisambDRSqrMin", i).value();
   }

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      TRG_MSG_INFO("InvMassMin   " << i << " : " << p_InvMassMin[i]);
      TRG_MSG_INFO("InvMassMax   " << i << " : " << p_InvMassMax[i]);
   
      TRG_MSG_INFO("MinET1a      " << i << " : " << p_MinET1a[i]);
      TRG_MSG_INFO("MinET1b      " << i << " : " << p_MinET1b[i]);
      TRG_MSG_INFO("MinET2       " << i << " : " << p_MinET2[i]);

      TRG_MSG_INFO("DisambDR     " << i << " : " << p_DisambDR[i]);
   }
   TRG_MSG_INFO("number output : " << numberOutputBits());

   // book histograms
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      std::string hname_accept = "hInvariantMassInclusive1Disambiguation2_accept_bit"+std::to_string((int)i);
      std::string hname_reject = "hInvariantMassInclusive1Disambiguation2_reject_bit"+std::to_string((int)i);
      // mass
      bookHist(m_histAccept, hname_accept, "INVM", 100, sqrt(p_InvMassMin[i]), sqrt(p_InvMassMax[i]));
      bookHist(m_histReject, hname_reject, "INVM", 100, sqrt(p_InvMassMin[i]), sqrt(p_InvMassMax[i]));
   }
   
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::InvariantMassInclusive1Disambiguation2::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

   if(input.size() == 2) {     
     
      for( TOBArray::const_iterator tob1a = input[0]->begin(); 
           tob1a != input[0]->end() && distance( input[0]->begin(), tob1a) < p_NumberLeading1a;
           ++tob1a) {
            
         TCS::TOBArray::const_iterator tob1b = tob1a; ++tob1b;      
         for( ;
              tob1b != input[0]->end() && distance( input[0]->begin(), tob1b) < p_NumberLeading1b;
              ++tob1b) {
               
            // Inv Mass calculation
            unsigned int invmass2 = calcInvMassBW( *tob1a, *tob1b );
            for(unsigned int i=0; i<numberOutputBits(); ++i) {

               bool accept = false;
               if( parType_t((*tob1a)->Et()) <= std::min(p_MinET1a[i],p_MinET1b[i])) continue; // ET cut
               if( parType_t((*tob1b)->Et()) <= std::min(p_MinET1a[i],p_MinET1b[i])) continue; // ET cut
               if( (parType_t((*tob1a)->Et()) <= std::max(p_MinET1a[i],p_MinET1b[i])) && (parType_t((*tob1b)->Et()) <= std::max(p_MinET1a[i],p_MinET1b[i]))) continue;

               bool passInvM = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i];

               for( TOBArray::const_iterator tob2 = input[1]->begin(); 
                    tob2 != input[1]->end() && distance( input[1]->begin(), tob2) < p_NumberLeading2;
                    ++tob2) {

                  if( parType_t((*tob2)->Et()) <= p_MinET2[i]) continue; // ET cut

                  unsigned int deltaR2a = calcDeltaR2BW( *tob1a, *tob2 );
                  unsigned int deltaR2b = calcDeltaR2BW( *tob1b, *tob2 );
                  
                  accept = passInvM && deltaR2a > p_DisambDR[i] && deltaR2b > p_DisambDR[i];
                  const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                  const bool fillReject = fillHistos() and not fillAccept;
                  const bool alreadyFilled = decision.bit(i);
                  if( accept ) {
                     decision.setBit(i, true);
                     output[i]->push_back( TCS::CompositeTOB(*tob1a, *tob1b) );
                  }
                  if(fillAccept and not alreadyFilled) {
                     fillHist1D(m_histAccept[i],sqrt((float)invmass2));
                  } else if(fillReject) {
                     fillHist1D(m_histReject[i],sqrt((float)invmass2));
                  }
                  TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2);
               }
            }
         }
      }
      for (unsigned int i=0; i < numberOutputBits(); ++i) {
         bool hasAmbiguousInputs =  TSU::isAmbiguousTruncation(input[0], p_NumberLeading1a, p_MinET1a[i])
                                 || TSU::isAmbiguousTruncation(input[0], p_NumberLeading1b, p_MinET1b[i])
                                 || TSU::isAmbiguousTruncation(input[1], p_NumberLeading2, p_MinET2[i]);
         output[i]->setAmbiguityFlag(hasAmbiguousInputs);
      }
   } else {

      TCS_EXCEPTION("InvariantMassInclusive1Disambiguation2 alg must have 2 inputs, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::InvariantMassInclusive1Disambiguation2::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{

   if(input.size() == 2) {     
     
      for( TOBArray::const_iterator tob1a = input[0]->begin(); 
           tob1a != input[0]->end() && distance( input[0]->begin(), tob1a) < p_NumberLeading1a;
           ++tob1a) {
            
         TCS::TOBArray::const_iterator tob1b = tob1a; ++tob1b;      
         for( ;
              tob1b != input[0]->end() && distance( input[0]->begin(), tob1b) < p_NumberLeading1b;
              ++tob1b) {
               
            // Inv Mass calculation
            unsigned int invmass2 = calcInvMass( *tob1a, *tob1b );

            for(unsigned int i=0; i<numberOutputBits(); ++i) {

               bool accept = false;

               if( parType_t((*tob1a)->Et()) <= std::min(p_MinET1a[i],p_MinET1b[i])) continue; // ET cut
               if( parType_t((*tob1b)->Et()) <= std::min(p_MinET1a[i],p_MinET1b[i])) continue; // ET cut
               if( (parType_t((*tob1a)->Et()) <= std::max(p_MinET1a[i],p_MinET1b[i])) && (parType_t((*tob1b)->Et()) <= std::max(p_MinET1a[i],p_MinET1b[i]))) continue;

               bool passInvM = invmass2 >= p_InvMassMin[i] && invmass2 <= p_InvMassMax[i];

               for( TOBArray::const_iterator tob2 = input[1]->begin(); 
                    tob2 != input[1]->end() && distance( input[1]->begin(), tob2) < p_NumberLeading2;
                    ++tob2) {

                  if( parType_t((*tob2)->Et()) <= p_MinET2[i]) continue; // ET cut

                  unsigned int deltaR2a = calcDeltaR2( *tob1a, *tob2 );
                  unsigned int deltaR2b = calcDeltaR2( *tob1b, *tob2 );

                  accept = passInvM && deltaR2a > p_DisambDR[i] && deltaR2b > p_DisambDR[i];
                  const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
                  const bool fillReject = fillHistos() and not fillAccept;
                  const bool alreadyFilled = decision.bit(i);
                  if( accept ) {
                     decision.setBit(i, true);
                     output[i]->push_back( TCS::CompositeTOB(*tob1a, *tob1b) );
                  }
                  if(fillAccept and not alreadyFilled) {
                     fillHist1D(m_histAccept[i],sqrt((float)invmass2));
                  } else if(fillReject) {
                     fillHist1D(m_histReject[i],sqrt((float)invmass2));
                  }
                  TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " invmass2 = " << invmass2);
               }
            }
         }
      }
   } else {

      TCS_EXCEPTION("InvariantMassInclusive1Disambiguation2 alg must have either 2 inputs, but got " << input.size());

   }

   return TCS::StatusCode::SUCCESS;
}
