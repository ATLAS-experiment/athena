/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * TeAsymmetry.cpp
 * Created by Jack Harrison on 12/05/25.
 *
 * @brief Based on the JIRA ticket: https://its.cern.ch/jira/browse/ATR-31097
 *        Three conditions are implemented:
 *        abs(jTE_A - jTE_C) > deltaAbsMin
 *        abs(jTE_A - jTE_C) > asymFactor * (jTE + asymOffset) 
 *        jTE_A * jTE_C < maxTeProduct
 *
 * @param deltaAbsMin, asymFactor, asymOffset, maxTeProduct
**********************************/


#include <cmath>

#include "L1TopoAlgorithms/TeAsymmetry.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"
#include "L1TopoEvent/jTETOBArray.h"

REGISTER_ALG_TCS(TeAsymmetry)


TCS::TeAsymmetry::TeAsymmetry(const std::string & name) : DecisionAlg(name)
{

  defineParameter("InputWidth", 1);
  defineParameter("MaxTob", 0);
  defineParameter("NumResultBits", 4);
  defineParameter("Delay", 1);
  setNumberOutputBits(4);

  for (unsigned int i=0;i<numberOutputBits();i++){
    // Algo parameters
    defineParameter("deltaAbsMin", 0, i);
    defineParameter("asymFactor", 0, i);
    defineParameter("asymOffset", 0, i);
    defineParameter("maxTeProduct", 0, i);
  }

  // Histo monitoring parameters
  defineParameter("MinSidejTE",0);
  defineParameter("MaxSidejTE",999);

}

TCS::TeAsymmetry::~TeAsymmetry(){}


TCS::StatusCode
TCS::TeAsymmetry::initialize() {

  //Algo parameters
  for (unsigned int i=0;i<numberOutputBits();i++){
    p_deltaAbsMin[i] = parameter("deltaAbsMin", i).value();
    p_asymFactor[i] = parameter("asymFactor", i).value();
    p_asymOffset[i] = parameter("asymOffset", i).value();
    p_maxTeProduct[i] =  parameter("maxTeProduct", i).value();
  }
  
  // Histo monitoring
  p_MinSidejTE = parameter("MinSidejTE").value();
  p_MaxSidejTE = parameter("MaxSidejTE").value();

  TRG_MSG_INFO("number output : " << numberOutputBits());
    
   // book histograms
  for(unsigned int i=0; i<numberOutputBits(); ++i) {
    std::string hname_accept = "hTeAsymmetry_accept_bit"+std::to_string((int)i);
    std::string hname_reject = "hTeAsymmetry_reject_bit"+std::to_string((int)i);
    bookHist(m_histAccept, hname_accept, "jTE SideA vs jTE SideC", 100, p_MinSidejTE, p_MaxSidejTE, 100, p_MinSidejTE, p_MaxSidejTE);
    bookHist(m_histReject, hname_reject, "jTE SideA vs jTE SideC", 100, p_MinSidejTE, p_MaxSidejTE, 100, p_MinSidejTE, p_MaxSidejTE);
  }
   
   return StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::TeAsymmetry::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{
  if(input.size() == 1) {

    if (input[0]->size()!=1) {
      TCS_EXCEPTION("TeAsymmetry alg needs input list with a single jTE TOB, got " << input[0]->size());
    }

    for( TOBArray::const_iterator jte = input[0]->begin(); 
           jte != input[0]->end();
           ++jte)
         {

      for(unsigned int i=0; i<numberOutputBits(); ++i) {

        bool accept = false;

        // Cast to long to avoid overflow
        long long jteSideA = (*jte)->sumEtSideA();
        long long jteSideC = (*jte)->sumEtSideC();

        //std::cout << "jTE SideA: " << jteSideA << ", jTE SideC: " << jteSideC << " , " << p_deltaAbsMin[i] << " , " << abs(jteSideA - jteSideC)  << std::endl;

        bool condition_1 = std::abs(jteSideA - jteSideC) > p_deltaAbsMin[i];
        
        //std::cout << "Condition 1: " << condition_1 << std::endl;
        //attention: some of these values are to be interpreted as signed, 
        //but also need to explicitly cast unsigned quantities to have correct results
        static constexpr unsigned c2fractionalBits = 8; // 8 fractional bits in p_asymFactor
        int offsetSumEt = static_cast<int>((*jte)->sumEt()) + static_cast<int>(p_asymOffset[i]);
        bool condition_2  = std::abs(jteSideA - jteSideC) >  (( static_cast<long long>(p_asymFactor[i]) * offsetSumEt ) >> c2fractionalBits); 
        //std::cout << "Condition 2: " << condition_2 << std::endl;
        bool condition_3  = jteSideA * jteSideC < p_maxTeProduct[i];
        //std::cout << "Condition 3: " << condition_3 << std::endl;

        //Write out to the output bit
        accept = condition_1 && condition_2 && condition_3;
        //std::cout << "Accept: " << accept << std::endl;
        
        
        const bool fillAccept = fillHistos() and (fillHistosBasedOnHardware() ? getDecisionHardwareBit(i) : accept);
        const bool fillReject = fillHistos() and not fillAccept;
        const bool alreadyFilled = decision.bit(i);
        
        TRG_MSG_DEBUG("Decision " << i << ": " << (accept?"pass":"fail") << " jTE_A = " << (*jte)->sumEtSideA() << " , jTE_C = " << (*jte)->sumEtSideC());
        
        if( accept ) {
          decision.setBit(i, true);  
          output[i]->push_back( TCS::CompositeTOB(*jte) );
        }
        if(fillAccept and not alreadyFilled) {
          fillHist2D(m_histAccept[i], jteSideA, jteSideC);
        } else if(fillReject) {
          fillHist2D(m_histReject[i], jteSideA, jteSideC);
        }
      }
    } 
  return TCS::StatusCode::SUCCESS;
  } else {
  TCS_EXCEPTION("TeAsymmetry alg must have 1 input, but got " << input.size());
  }
}

TCS::StatusCode
TCS::TeAsymmetry::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{
  return processBitCorrect(input, output, decision);
}
