/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// $Id: CheckBinSpike.cxx,v 1.0 2024-06-12 19:40:53 jlieberm $
// **********************************************************************

#include "dqm_algorithms/CheckBinSpike.h"

#include <cmath>
#include <iostream>


#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/Result.h"
#include <dqm_algorithms/tools/AlgorithmHelper.h>

namespace{
  dqm_algorithms::CheckBinSpike CheckBinSpike_1D("1D");
}

namespace dqm_algorithms {

// *********************************************************************
// Public Methods
// *********************************************************************

CheckBinSpike::CheckBinSpike( const std::string & name )
  : m_name ( name )
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm("CheckBinSpike_" + name, this );
}

dqm_algorithms::CheckBinSpike * CheckBinSpike::clone(){
  return new CheckBinSpike(m_name);
}


dqm_core::Result* CheckBinSpike::execute( const std::string& name, const TObject& data, const dqm_core::AlgorithmConfig & config ){
  // Cast to the type of TObject to assess
  const TH1* h = dynamic_cast<const TH1*>( &data );
  if( h == 0 ) {
    throw dqm_core::BadConfig( ERS_HERE, name, "Cannot cast data to type TH1" );
  }
  const TH1* ref = dynamic_cast<const TH1*>( config.getReference() );
  if( ref == 0 ) {
    throw dqm_core::BadConfig( ERS_HERE, name, "Cannot obtain reference of type TH1" );
  }  
  double greenThr, redThr;
  
  std::string thrName="MaxDist";
 
  try {
    greenThr = dqm_algorithms::tools::GetFromMap( thrName, config.getGreenThresholds() );
    redThr = dqm_algorithms::tools::GetFromMap( thrName, config.getRedThresholds() );
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );

  }
  float bin = 0;
	float maxValue = 0;
  checkBinByBin(h, maxValue, bin);
  std::unique_ptr<dqm_core::Result> result (new dqm_core::Result());
  result->tags_["MaximumValue"] = maxValue;
  result->tags_["SpottedBin"] = bin;
  
  if( maxValue < greenThr ) {
    result->status_ = dqm_core::Result::Green;
  }
  else if( maxValue > redThr ) {
    result->status_ = dqm_core::Result::Red;
  }
  else {
    result->status_ = dqm_core::Result::Yellow;
  }
  return result.release();
}

void CheckBinSpike::checkBinByBin(const TH1* hist, float &result,  float &detectedBin) {
  std::vector<float> values;
  for ( int ibin=1; ibin<hist->GetNbinsX()+1; ibin++ ){
    float ref = (hist->GetBinContent(ibin-1) + hist->GetBinContent(ibin+1))*0.5;
    if ( ibin==1 ) {
      ref = (hist->GetBinContent(hist->GetNbinsX()) + hist->GetBinContent(ibin+1))*0.5;
    }
    if ( ibin==hist->GetNbinsX() ){
      ref = (hist->GetBinContent(1) + hist->GetBinContent(ibin-1))*0.5;
    }
    float test = hist->GetBinContent(ibin);
    values.push_back(test/ref);
  }
  result = *std::max_element(values.begin(),values.end());
  int bin = std::max_element(values.begin(),values.end()) - values.begin();
  detectedBin = hist->GetXaxis()->GetBinCenter(bin);
}

void CheckBinSpike::printDescription(std::ostream& out){
  std::string message;
  message += "\n";
  message += "Algorithm: \"" + m_name + "\"\n";
  message += "Description: Checks if the maximum ratio between bins in a TH1F\n";
  message += "             is above a given threshold. Useful for peak detection\n";
  message += "Parameters: none\n";
  message += "\n";
  out << message;
}

} // namespace dqm_algorithms

