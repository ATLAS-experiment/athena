/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * KalmanMETCorrection.cpp
 * Created by Joerg Stelzer on 11/16/12.
 * Re-written by Ralf Gugel on 04/19/24.
 *
 * @brief algorithm calculates the KF correction per jet , get new XE and apply cut 
 *
 * @param NumberLeading
**********************************/

#include <cmath>
#include <string>

#include "L1TopoAlgorithms/KalmanMETCorrection.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"
#include "L1TopoSimulationUtils/KFLUT.h"
#include "L1TopoSimulationUtils/Kinematics.h"
// Bitwise implementation utils
#include "L1TopoSimulationUtils/L1TopoDataTypes.h"
#include "L1TopoSimulationUtils/Trigo.h"
#include "L1TopoSimulationUtils/Conversions.h"
//

REGISTER_ALG_TCS(KalmanMETCorrection)

TCS::KalmanMETCorrection::KalmanMETCorrection(const std::string & name) : DecisionAlg(name)
{
   defineParameter("InputWidth", 9);
   defineParameter("NumResultBits", 6);
   for (size_t weightIndex = 0; weightIndex < TCS::KFMET::nWeightWords; weightIndex++) {
      defineParameter("weights"+std::to_string(weightIndex), 0);   
   }
   defineParameter("MinET", 0);
   defineParameter("KFXE",0,0);
   defineParameter("KFXE",0,1);
   defineParameter("KFXE",0,2);
   defineParameter("KFXE",0,3);
   defineParameter("KFXE",0,4);
   defineParameter("KFXE",0,5);
   setNumberOutputBits(6);
}

TCS::KalmanMETCorrection::~KalmanMETCorrection(){}


TCS::StatusCode
TCS::KalmanMETCorrection::initialize() {
   
   p_NumberLeading2 = parameter("InputWidth").value();

   p_MinEt = parameter("MinET").value();

   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      p_XE[i] = parameter("KFXE",i).value();

   }
   TRG_MSG_INFO("NumberLeading2 : " << p_NumberLeading2);  
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
    TRG_MSG_INFO("KFXE   : " << p_XE[i]);
   }
   TRG_MSG_INFO("MinET          : " << p_MinEt);

   TRG_MSG_INFO("number output : " << numberOutputBits());
   
   //retrieve all weight words (in compacted representation)
   std::vector<unsigned> weightWords(TCS::KFMET::nWeightWords);
   for (size_t weightIndex = 0; weightIndex < TCS::KFMET::nWeightWords; weightIndex++) {
      weightWords[weightIndex] = parameter("weights"+std::to_string(weightIndex)).value();
   }
   //unpack and convert individual weights
   for (size_t iET=0; iET < TCS::KFMET::nLogEtBins; ++iET) {
      for (size_t jEta=0; jEta < TCS::KFMET::nEtaBins; ++jEta) {
          if (jEta == TCS::KFMET::nEtaBins-1) {
            p_correctionLut[jEta][iET] = 0; //eta fallback bin
            continue;
          }
          //assuming here that weights are max. 32 bits each to simplify unpacking logic
          size_t startBit = (iET + jEta * TCS::KFMET::nLogEtBins) * TCS::KFMET::correctionBitWidth;
          constexpr unsigned weightMask = (1 << TCS::KFMET::correctionBitWidth)-1;
          unsigned rawValue = ( weightWords[startBit/32] >> (startBit%32) ) & weightMask;
          int nOverflowBits = (startBit%32) + TCS::KFMET::correctionBitWidth - 32;
          if (nOverflowBits > 0) {
            //overflow into next word
            //          ( next word & bit mask for parts of 2nd word ) << bits already taken from previus word
            rawValue |= (weightWords[startBit/32 + 1] & ( (1 << nOverflowBits) - 1 ) ) <<  (TCS::KFMET::correctionBitWidth - nOverflowBits);
          }
          //convert raw value to suitable signed integer (assuming 2's complement)
          p_correctionLut[jEta][iET] = TSU::toSigned(rawValue, TCS::KFMET::correctionBitWidth );
          /*
          if (rawValue >> (TCS::KFMET::correctionBitWidth-1) == 0) { //sign bit is not set
            p_correctionLut[jEta][iET] = rawValue;
          } else {
            constexpr int twosComplementOffset = 1 << TCS::KFMET::correctionBitWidth; // 2^bitwidth
            p_correctionLut[jEta][iET] = rawValue - twosComplementOffset;
          }
          */
      }
   }
   
   return StatusCode::SUCCESS;
}



TCS::StatusCode
TCS::KalmanMETCorrection::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                      const std::vector<TCS::TOBArray *> & /*output*/,
                      Decision & decision )

{
   
   if (isLegacyTopo()){
      //KFMET was never fully commissioned on legacy L1Topo, hence, simply ignore
      return TCS::StatusCode::SUCCESS;
   }
   
   if(input.size()!=2) {
      TCS_EXCEPTION("KalmanMETCorrection alg must have exactly two input list (jets and MET list), but got " << input.size());
   }
   
   const TCS::GenericTOB & met = (*input[0])[0];
   int64_t metXY[2] {met.Ex(), met.Ey()};
   int64_t jetSumXY[2] {0, 0};
   for( TOBArray::const_iterator tob = input[1]->begin(); 
           tob != input[1]->end() && distance( input[1]->begin(), tob) < p_NumberLeading2;
           ++tob) {
      if( (*tob)->Et() <= p_MinEt ) continue; // E_T cut
      unsigned tobEta = abs((*tob)->eta());
      size_t etaBin = TCS::KFMET::lookupEtaBin.count(tobEta) ?  TCS::KFMET::lookupEtaBin.at(tobEta) : TCS::KFMET::lookupEtaBinFallback;
      
      //ignore given number of least significant bits right away
      unsigned tobET  = (*tob)->Et() >> TCS::KFMET::jetEtBinOffset;
      unsigned etBin = 0;
      //KFMET LUT is binned in log2(ET) with ET in Topo's internal granularity
      //-> determining the bin index reduces to determining the position of the higest non-zero bit
      //   highest log2(ET) bin also acts as overflow bin
      while (tobET > 1 && etBin < TCS::KFMET::nLogEtBins-1) {
          etBin++;
          tobET >>= 1;
      }
      int scaledEt = (*tob)->Et() * p_correctionLut[etaBin][etBin];
      unsigned tobPhi = (*tob)->phi();
      jetSumXY[0] += scaledEt * TSU::Trigo::CosInt.at(tobPhi);
      jetSumXY[1] += scaledEt * TSU::Trigo::SinInt.at(tobPhi);
      
   }
   
   //compute "corrected" MET values
   int64_t kfmetXY[2] {
      metXY[0] + ( jetSumXY[0] >> (TCS::KFMET::correctionDecimalBitWidth + 10 /*cos/sin decimal bits*/) ),
      metXY[1] + ( jetSumXY[1] >> (TCS::KFMET::correctionDecimalBitWidth + 10 /*cos/sin decimal bits*/) )
   };
   
   uint64_t kfmetSq = kfmetXY[0] * kfmetXY[0] + kfmetXY[1] * kfmetXY[1];
   
   for(unsigned int i=0; i<numberOutputBits(); ++i) {
      decision.setBit( i, kfmetSq > p_XE[i]*p_XE[i] );
    }    
    
    return TCS::StatusCode::SUCCESS;

}

TCS::StatusCode
TCS::KalmanMETCorrection::process( const std::vector<TCS::TOBArray const *> & input,
                      const std::vector<TCS::TOBArray *> & output,
                      Decision & decision )

{
  //we have a bitwise correct implementation, so use it
  return this->processBitCorrect(input, output, decision);
}
