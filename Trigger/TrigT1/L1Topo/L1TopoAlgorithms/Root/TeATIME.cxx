/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * TeATIME.cpp
 * Created by Jack Harrison on 23/05/25.
 *
 * @brief Total Energy Anti-Trigger for Imminent Massive Events / Total energy Ahead-of-Time Indicator for Massive Events: 
 * Dummy algorithm in simulation to allow athena to run properly, allowing FW algorithm comparing current (BC+0) with next (BC+1) TE values.
 * Algorithm never fires, purely dummy in simulation, as we don't simulate BC+1 values.
 *
 * @param 
**********************************/


#include <cmath>

#include "L1TopoAlgorithms/TeATIME.h"
#include "L1TopoCommon/Exception.h"
#include "L1TopoInterfaces/Decision.h"

REGISTER_ALG_TCS(TeATIME)


TCS::TeATIME::TeATIME(const std::string & name) : DecisionAlg(name)
{

  defineParameter("InputWidth", 1);
  defineParameter("MaxTob", 0);
  defineParameter("NumResultBits", 4);
  setNumberOutputBits(4);
  for (unsigned int i=0;i<numberOutputBits();i++){
    // Algo parameters
    defineParameter("algoLogic", 0, i);
    defineParameter("nextBcOffset", 0, i);
    defineParameter("nextBcFactor", 0, i);
  }
  
}

TCS::StatusCode
TCS::TeATIME::initialize() {

   // book histograms
  for(unsigned int i=0; i<numberOutputBits(); ++i) {
    std::string hname_accept = "hTeATIME_accept_bit"+std::to_string((int)i);
    std::string hname_reject = "hTeATIME_reject_bit"+std::to_string((int)i);
    bookHist(m_histAccept, hname_accept, "TeTIME accept", 1, 0, 2);
    bookHist(m_histReject, hname_reject, "TeTIME accept", 1, 0, 2);
  }

   return StatusCode::SUCCESS;
}


TCS::StatusCode
TCS::TeATIME::processBitCorrect( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{
  if(input.size() == 1) {

    for( TOBArray::const_iterator jte = input[0]->begin(); 
           jte != input[0]->end();
           ++jte)
         {

      for(unsigned int i=0; i<numberOutputBits(); ++i) {

        decision.setBit(i, false);
        fillHist1D(m_histReject[i], 1);
        output[i]->push_back( TCS::CompositeTOB(*jte) );
      }

     }

      return TCS::StatusCode::SUCCESS;
    } else {
  TCS_EXCEPTION("TeATIME alg must have 1 input, but got " << input.size());
  }
}

TCS::StatusCode
TCS::TeATIME::process( const std::vector<TCS::TOBArray const *> & input,
                             const std::vector<TCS::TOBArray *> & output,
                             Decision & decision )
{
  return processBitCorrect(input, output, decision);
}
