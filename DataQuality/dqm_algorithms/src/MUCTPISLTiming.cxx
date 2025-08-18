/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/MUCTPISLTiming.h"

#include <cmath>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include <TClass.h>
#include <TH1.h>
#include <TH2.h>

#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/Result.h"
#include "dqm_algorithms/tools/AlgorithmHelper.h"
#include "ers/ers.h"


static dqm_algorithms::MUCTPISLTiming staticInstance;


namespace dqm_algorithms {

// *********************************************************************
// Public Methods
// *********************************************************************

MUCTPISLTiming::MUCTPISLTiming()
  : m_name("MUCTPISLTiming")
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm( m_name, this );
}

dqm_core::Algorithm*
MUCTPISLTiming::clone()
{
  return new MUCTPISLTiming(*this);
}


dqm_core::Result*
MUCTPISLTiming::execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config)
{
  using namespace std;

  const TH2 * hist;
  //TH2 * ref; //not using reference in this case
  
  if( object.IsA()->InheritsFrom( "TH2" ) ) {
    hist = static_cast<const TH2*>(&object);
    if (hist->GetDimension() != 2 ){
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension != 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
  }   

  //Get Parameters and Thresholds
  double thresh=0.5;
  double minstat=-1;
  int centralBin=4; // the middle of 7 bins;

  try {
      thresh  = dqm_algorithms::tools::GetFirstFromMap("thresh", config.getParameters(), 0.5);
      minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }

 //Algo

  //project our 2D histogram to 1D (x-axis), thus summing up along y
  //will then compare every bin of the projection with the GetBinContent of the central bin in the 2D histo
  TH1* projection = hist->ProjectionX();

  dqm_core::Result* result = new dqm_core::Result();
  std::map<std::string,double> tags; //defined in https://gitlab.cern.ch/atlas-tdaq-software/dqm_core/-/blob/master/dqm_core/Result.h

  //assume all good, until find a bad one
  result->status_ = dqm_core::Result::Green;
  uint howmanybad=0;

  for(int iBin=1;iBin<=projection->GetNbinsX();iBin++)//0=underflow bin
  {
      if( projection->GetBinContent(iBin)*thresh > hist->GetBinContent(iBin,centralBin)  ) //if central slice is less than half of SL sum => bad
      {
          result->status_ = dqm_core::Result::Yellow;
          howmanybad++;
          //add tag to print which bins are actually the "problematic" ones
          std::string slNum = std::to_string(iBin-1);
          if(iBin < 10) slNum = "0" + slNum; // leading zero for conformity

          tags[slNum] = hist->GetBinContent(iBin,centralBin)*1.0 / projection->GetBinContent(iBin) ;
      }
  }
  tags["howmanybad"] = howmanybad;

  //set the result tags
  result->tags_ = std::move(tags);

  //If more than 2SL have bad timing, make it red
  if(howmanybad>2)
      result->status_ = dqm_core::Result::Red;

  Double_t stats[7];
  hist->GetStats(stats);
  //if not enough stats, reset the result (but keep this here, so can still see the tag for bad SL)
  if(stats[0]<minstat)
      result->status_ = dqm_core::Result::Undefined;

  // Return the result
  return result;
}


void
MUCTPISLTiming::printDescription(std::ostream& out){
  std::string message;
  message += "\n";
  message += "Algorithm: \"" + m_name + "\"\n";
  message += "Description: for each SL x-slice (set of y-bins for the same x value) check that the central y bin has at least 'thresh' percent(ratio) of the sum of values in the SL \n";
  out << message;
}

} // namespace dqm_algorithms
