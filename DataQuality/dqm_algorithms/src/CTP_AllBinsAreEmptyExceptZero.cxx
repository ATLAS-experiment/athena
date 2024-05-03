/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/CTP_AllBinsAreEmptyExceptZero.h"

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


static dqm_algorithms::CTP_AllBinsAreEmptyExceptZero staticInstance;


namespace dqm_algorithms {

// *********************************************************************
// Public Methods
// *********************************************************************

CTP_AllBinsAreEmptyExceptZero::CTP_AllBinsAreEmptyExceptZero()
  : m_name("CTP_AllBinsAreEmptyExceptZero")
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm( m_name, this );
}

dqm_core::Algorithm*
CTP_AllBinsAreEmptyExceptZero::clone()
{
  return new CTP_AllBinsAreEmptyExceptZero(*this);
}


dqm_core::Result*
CTP_AllBinsAreEmptyExceptZero::execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config)
{
  using namespace std;

  const TH1 *hist;

  if(object.IsA()->InheritsFrom( "TH1" )) {
    hist = static_cast<const TH1*>( &object );
    if (hist->GetDimension() > 1 ){ 
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 1 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }
  
  //Get Parameters and Thresholds
  double threshold = 0;

  try {
      threshold = dqm_algorithms::tools::GetFirstFromMap("threshold", config.getParameters(), 0);
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }
  
  //Algo
  dqm_core::Result* result = new dqm_core::Result();
  std::map<std::string,double> tags; //defined in https://gitlab.cern.ch/atlas-tdaq-software/dqm_core/-/blob/master/dqm_core/Result.h

  //assume all good, until find a bad one
  result->status_ = dqm_core::Result::Green;
  int nbinsx = hist->GetXaxis()->GetNbins();
  int badBins = 0;
  
  for(int iBin = 1; iBin <= nbinsx-1; ++iBin) //foreach bin of the projection (0=underflow)
  {
	// If bin content is nonzero, and 0 does not lie in between bin edges 
	if( hist->GetBinContent(iBin) > threshold && hist->GetBinLowEdge(iBin) * hist->GetBinLowEdge(iBin+1) > 0 )
	{
	  result->status_ = dqm_core::Result::Red;
	  ++ badBins;
	}
  }
  tags["# Bad bins"] = badBins;

  //set the result tags
  result->tags_ = tags;

  // Return the result
  return result;
}


void
CTP_AllBinsAreEmptyExceptZero::printDescription(std::ostream& out){
  std::string message;
  message += "\n";
  message += "Algorithm: \"" + m_name + "\"\n";
  message += "Description: Check that the only non-empty bin is the one centered in zero.\n";
  out << message;
}

} // namespace dqm_algorithms
