/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "dqm_algorithms/MUCTPISLAnyHit.h"

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


static dqm_algorithms::MUCTPISLAnyHit staticInstance;


namespace dqm_algorithms {

// *********************************************************************
// Public Methods
// *********************************************************************

MUCTPISLAnyHit::MUCTPISLAnyHit()
  : m_name("MUCTPISLAnyHit")
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm( m_name, this );
}

dqm_core::Algorithm*
MUCTPISLAnyHit::clone()
{
  return new MUCTPISLAnyHit(*this);
}


dqm_core::Result*
MUCTPISLAnyHit::execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config)
{
  using namespace std;

  const TH2 *hist{};
  
  //ensure that input histo is 2D
  if( object.IsA()->InheritsFrom( "TH2" ) ) {
    hist = static_cast<const TH2 *>(&object);
    if (hist->GetDimension() != 2 ){
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension != 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
  }   

  //Get Parameters and Thresholds
  double thresh=0.1;

  try {
      thresh     = dqm_algorithms::tools::GetFirstFromMap("thresh", config.getParameters(), 0.1);
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }

 //Algo

  //project our 2D histogram to 1D (on the y-axis), thus summing up along x
  TH1* projection = hist->ProjectionY();

  dqm_core::Result* result = new dqm_core::Result();
  std::map<std::string,double> tags; //defined in https://gitlab.cern.ch/atlas-tdaq-software/dqm_core/-/blob/master/dqm_core/Result.h

  //assume all good, until find a bad one
  result->status_ = dqm_core::Result::Green;
  uint howmanybad=0;

  for(int iBin=1;iBin<=projection->GetNbinsX();iBin++)//foreach bin of the projection (0=underflow)
  {
      if( projection->GetBinContent(iBin) < thresh  ) //if bin content less than algorithm threshold => bad
      {
          result->status_ = dqm_core::Result::Yellow;
          howmanybad++;
      }
  }
  tags["howmanybad"] = howmanybad;

  //set the result tags
  result->tags_ = std::move(tags);

  // Return the result
  return result;
}


void
MUCTPISLAnyHit::printDescription(std::ostream& out){
  std::string message;
  message += "\n";
  message += "Algorithm: \"" + m_name + "\"\n";
  message += "Description: Makes the ProjectionY of the 2D input histo. If any bin content is below the given threshold => warning.\n";
  message += "Optional Parameters: thresh = if any bin content in the ProjectionY is below this value => warning\n";
  out << message;
}

} // namespace dqm_algorithms
