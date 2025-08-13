/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*! \file BinsDiffFromPreviousLBs.cxx evaluates the number of consecutive bins with bin value different from set threshold. Also usable for single bins. 
 * \author Giulia Ucchielli
 */

#include <dqm_core/AlgorithmConfig.h>
#include <dqm_algorithms/BinsDiffFromPreviousLBs.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmManager.h>

#include <TH1.h>
#include <TF1.h>
#include <TClass.h>
#include <cmath>
#include <algorithm>

#include <iostream>


static dqm_algorithms::BinsDiffFromPreviousLBs myInstance;

dqm_algorithms::BinsDiffFromPreviousLBs::BinsDiffFromPreviousLBs( )
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm("BinsDiffFromPreviousLBs", this);
}

dqm_algorithms::BinsDiffFromPreviousLBs::~BinsDiffFromPreviousLBs()
{
}

dqm_algorithms::BinsDiffFromPreviousLBs * 
dqm_algorithms::BinsDiffFromPreviousLBs::clone()
{
  
  return new BinsDiffFromPreviousLBs();
}

bool dqm_algorithms::BinsDiffFromPreviousLBs::areConsecutive(const std::vector<int>& lbs) 
{ 
  if (lbs.empty()) return false;
  if (lbs.size() == 1) return true;
  const bool monotonic = std::is_sorted(lbs.begin(), lbs.end());
  const bool withinRange = (lbs.back() - lbs.front() + 1) == std::ssize(lbs);
  return monotonic and withinRange;
} 


dqm_core::Result *
dqm_algorithms::BinsDiffFromPreviousLBs::execute(const std::string &  name, 
					   const TObject& object, 
					   const dqm_core::AlgorithmConfig& config )
{ 
  const TH1* histogram = nullptr;

  if( object.IsA()->InheritsFrom( "TH1" ) ) {
    histogram = static_cast<const TH1*>(&object);
    if (histogram->GetDimension() > 2 ){ 
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }
  
  const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  const double ignoreval = dqm_algorithms::tools::GetFirstFromMap( "ignoreval", config.getParameters(), -99999);
  bool greaterthan = (bool) dqm_algorithms::tools::GetFirstFromMap( "GreaterThan", config.getParameters(), 0);
  bool lessthan = (bool) dqm_algorithms::tools::GetFirstFromMap( "LessThan", config.getParameters(), 0);
  const bool publish = (bool) dqm_algorithms::tools::GetFirstFromMap( "PublishBins", config.getParameters(), 0); 
  const int maxpublish = (int) dqm_algorithms::tools::GetFirstFromMap( "MaxPublish", config.getParameters(), 20); 
  const double maxdiffabs = dqm_algorithms::tools::GetFirstFromMap( "MaxDiffAbs", config.getParameters(), -1); 
  
  if (greaterthan && lessthan) {
    ERS_INFO("Both GreaterThan and LessThan parameters set: Will check for for both");
    greaterthan = false;
    lessthan = false;
  }
  
  if ( histogram->GetEntries() < minstat ) {
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEntries"] = histogram->GetEntries();
    return result;
  }
  
  double bin_threshold;
  double nsigmas;
  double gthreshold;
  double rthreshold;
  try {
    nsigmas = dqm_algorithms::tools::GetFirstFromMap( "NSigma", config.getParameters() );
    bin_threshold = dqm_algorithms::tools::GetFirstFromMap( "BinThreshold", config.getParameters() );
    rthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getRedThresholds() );
    gthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getGreenThresholds() );
  }
  catch( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }
  
  TH1* resulthisto = nullptr;
  if (histogram->InheritsFrom("TH2")) {
	resulthisto=(TH1*)(histogram->Clone());
  } else if (histogram->InheritsFrom("TH1")) {
        resulthisto=(TH1*)(histogram->Clone());
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }

  resulthisto->Reset();

  std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters()); 

  std::vector<int> LBs;
  dqm_core::Result* result = new dqm_core::Result();

  for ( int k = range[0]; k <= range[1]; ++k ) {
    for ( int l = range[2]; l <= range[3]; ++l ) {
      double inputcont = histogram->GetBinContent(k,l);
      double inputerr = histogram->GetBinError(k,l);
      double diff=inputcont - bin_threshold;
      if (inputcont == ignoreval) continue;
      if (inputerr != 0){
	double sigma=diff/inputerr;	
	if (greaterthan && diff < 0. ) continue;  
	if (lessthan && diff > 0. ) continue;  
	if ( (std::abs(sigma) > nsigmas) && (std::abs(diff) > maxdiffabs) ) {
          resulthisto->SetBinContent(k,l,inputcont);
	  LBs.push_back(k); 
	  if (publish && (int)LBs.size() < maxpublish){
            dqm_algorithms::tools::PublishBin(histogram,k,l,inputcont,result);
	  }
	}
      }
      
    }    
  }

  int count =  LBs.size();

  result->tags_["NBins"] = count;
  result->object_ =  (boost::shared_ptr<TObject>)static_cast<TObject*>(resulthisto);

  ERS_DEBUG(1,"Number of bins " << nsigmas << " Sigma away from average of "<< bin_threshold << " is " << count);
  ERS_DEBUG(1,"Green threshold: "<< gthreshold << " bin(s);   Red threshold : " << rthreshold << " bin(s) ");
  
  if ( count <= gthreshold ) {
    result->status_ = dqm_core::Result::Green;
  } else if ( count < rthreshold ) {
    result->status_ = dqm_core::Result::Yellow;
  } else {
    result->status_ = dqm_core::Result::Red;
  }
  return result;
  
}
void
dqm_algorithms::BinsDiffFromPreviousLBs::printDescription(std::ostream& out)
{
  
  out<<"BinsDiffFromPreviousLBs: Checks for number of bins (subsequent LBs) NSigma within threshold \n"<<std::endl;

  out<<"Mandatory Parameter: BinThreshold: Look for bins NSigna away from threshold \n"<<std::endl;  
  out<<"Mandatory Parameter: NSigma: Number of sigma each bin must be within average bin value\n"<<std::endl;
  out<<"Mandatory Green/Red Threshold: NBins: number of bins N sigma away from average to give Green/Red result\n"<<std::endl;
  
  out<<"Optional Parameter: MinStat: Minimum histogram statistics needed to perform Algorithm"<<std::endl;
  out<<"Optional Parameter: doCheckOnSingleLB: check done not necessarly on subsequent bins (set to 0)"<<std::endl; 
  out<<"Optional Parameter: ignoreval: valued to be ignored for calculating average"<<std::endl; 
  out<<"Optional Parameter: GreaterThan: check only for bins which are GreaterThan average (set to 1)"<<std::endl; 
  out<<"Optional Parameter: LessThan: check only for bins which are LessThan average (set to 1)"<<std::endl; 
  out<<"Optional Parameter: PublishBins: Save bins which are different from average in Result (set to 1)"<<std::endl; 
  out<<"Optional Parameter: MaxPublish: Max number of bins to save (default 20)"<<std::endl; 
  out<<"Optional Parameter: MaxDiffAbs: test fails if NBins more than NSigma away and NBins more than MaxDiffAbs (absolut difference) away from average"<<std::endl; 
  
}

