/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// **********************************************************************
// $Id: ZDCPercentageThreshTests.cxx,v 1.0 2009/06/10 Valerio Consorti
// **********************************************************************


#include "dqm_algorithms/ZDCPercentageThreshTests.h"

#include <cmath>
#include <iostream>
#include <map>

#include <TClass.h>
#include <TF1.h>

#include "dqm_core/exceptions.h"
#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/AlgorithmManager.h"
#include "dqm_core/Result.h"
#include "dqm_algorithms/tools/AlgorithmHelper.h"
#include "ers/ers.h"


namespace
{
  dqm_algorithms::ZDCPercentageThreshTests UnderThreshold ( "UnderThreshold" );
  dqm_algorithms::ZDCPercentageThreshTests AboveThreshold ( "AboveThreshold" );
  dqm_algorithms::ZDCPercentageThreshTests XthBin ( "XthBin" );
  dqm_algorithms::ZDCPercentageThreshTests XthBin_NormalizeToFirstBin ( "XthBin_NormalizeToFirstBin" );
}


namespace dqm_algorithms {

// *********************************************************************
// Public Methods
// *********************************************************************

ZDCPercentageThreshTests::ZDCPercentageThreshTests( const std::string & name )
  : m_name(name)
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm( "ZDCPercentEvents_" + m_name, this );
}


ZDCPercentageThreshTests::~ZDCPercentageThreshTests()
{
}


dqm_core::Algorithm* ZDCPercentageThreshTests::clone()
{
  return new ZDCPercentageThreshTests(*this);
}


dqm_core::Result* ZDCPercentageThreshTests::execute( const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config)
{
  const TH1 * hist;
  
  // check histogram is 1D
  if( object.IsA()->InheritsFrom( "TH1" ) ) {
    hist = static_cast<const TH1*>(&object);
    if (hist->GetDimension() >= 2 ){
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension >= 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }   
  
  //Get Parameters and Thresholds
  try {
    m_thresh        = dqm_algorithms::tools::GetFirstFromMap( "Thresh", config.getParameters(),-1000.);
    m_binNum        = dqm_algorithms::tools::GetFirstFromMap( "BinNum", config.getParameters(),-1000);
    m_minstat       = dqm_algorithms::tools::GetFirstFromMap("MinStat", config.getParameters(), 0);
    m_redTh         = dqm_algorithms::tools::GetFromMap( "PercentBar", config.getRedThresholds());
    m_greenTh       = dqm_algorithms::tools::GetFromMap( "PercentBar", config.getGreenThresholds() );
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }

  // case-by-case check for mandatory parameters
  if ((m_name == "XthBin" || m_name == "XthBin_NormalizeToFirstBin") && m_binNum == -1000){
    throw dqm_core::BadConfig( ERS_HERE, name, "required parameter BinNum is not provided!" );
  }else if ((m_name == "UnderThreshold" || m_name == "AboveThreshold") && m_thresh == -1000.){
    throw dqm_core::BadConfig( ERS_HERE, name, "required parameter Thresh is not provided!" );
  }

  // check that red and green thresholds are in range
  if (m_redTh > 1 || m_redTh < 0 || m_greenTh > 1 || m_greenTh < 0){
    throw dqm_core::BadConfig( ERS_HERE, name, "thresholds m_redTh and m_greenTh must be in the range [0,1]! Threshold(s) out of range." );
  }

  // check that red threshold is greater than or equal to green
  if (m_redTh < m_greenTh){
    throw dqm_core::BadConfig( ERS_HERE, name, "m_redTh must be above or equal to m_greenTh!" );
  }
    
 //Check of statistics - only call algorithm if we meet minimum statistics requirement
  if (hist->GetEntries() < m_minstat ) {
    ERS_INFO("Histogram does not satisfy MinStat requirement " <<hist->GetName());
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEntries"] = hist->GetEntries();
    return result;
  }
  ERS_DEBUG(1,"Statistics: "<< hist->GetEntries()<< " entries ");

 // Calling algorithm - return value in range [0,1]
  dqm_core::Result* result = new dqm_core::Result(); 
  double percent = calculatePercentage(hist, result);
  if (percent == -1.){ // percentage invalid; result with tag for reason of failing is updated by helper function
    return result;
  }

 // Result
  result->tags_["Entries_under_selection(%)"] = percent;

  if( percent <= m_greenTh ) { // percentage of "bad" events below green threshold
    result->status_ = dqm_core::Result::Green;
  }
  else if( percent > m_greenTh && percent <= m_redTh ) { // percentage of "bad" events between green and red threshold
    result->status_ = dqm_core::Result::Yellow;
  }
  else { // percentage of "bad" events above red threshold
    result->status_ = dqm_core::Result::Red;
  }
  
  // Return the result
  return result;
}


double ZDCPercentageThreshTests::calculatePercentage(const TH1 * hist, dqm_core::Result * result){
  // function that calculates percentage of "bad" events
  // Parameters: hist - pointer to 1D histogram
  //             result - pointer to a dqm_core::Result instance: in case of failure, for recording status and reason of failure
  // if percentage calculation is invalid, update result with tag for reason of failure
  // and set result status 
  // return value in [0,1]

  // fetch the denominator: total number of entries (or content of first bin if normalization-to-1st-bin is requested)
  Double_t N = (m_name == "XthBin_NormalizeToFirstBin")? hist->GetBinContent(1) : hist->GetEntries();

  if(N == 0) {
      ERS_INFO("Histogram " << hist->GetName() << " has zero entries. Percentage calculation fails.");
      result->tags_["Empty_Histogram_found_N_Entries"] = N;
      result->status_ = dqm_core::Result::Undefined;
      return -1.;
  };

  // fetch number of "bad" events
  Double_t hit_under_selection = 0;

  if (m_name == "XthBin" || m_name == "XthBin_NormalizeToFirstBin"){ // if "bad" events are in a user-specified bin
    hit_under_selection = hist->GetBinContent(m_binNum);
  }
  else if (m_name == "UnderThreshold"){ // if "bad" events are those with value under a user-specified threshold
    hit_under_selection += hist->GetBinContent(1); // add at least bin content of the first bin to avoid meaningless check (always green)
    int i=2;

    while(hist->GetBinLowEdge(i+1) <= m_thresh){
      if(i>hist->GetNbinsX()){
        ERS_INFO("For histogram " << hist->GetName() << ", the user input threshold " << m_thresh << " is out of range (larger than max value). Percentage calculation fails.");
        result->tags_["Config_error_maximum_bin_exceed_looking_for_bin_number"] = i;
        result->status_ = dqm_core::Result::Undefined;
        return -1.;
      };
      hit_under_selection += hist->GetBinContent(i);
      i++;
    };
  }
  else if (m_name == "AboveThreshold"){ // if "bad" events are those with value above a user-specified threshold
    hit_under_selection += hist->GetBinContent(hist->GetNbinsX()+1); // include number of overflow events in the sum
    int i = hist->GetNbinsX()+1;
    do{
      i--;
      if(i <= 0){
        ERS_INFO("For histogram " << hist->GetName() << ", the user input threshold " << m_thresh << " is out of range (smaller than min value). Percentage calculation fails.");
        result->tags_["Config_error_minimum_bin_below_one"] = i;
        result->status_ = dqm_core::Result::Undefined;
        return -1.;
      };
      hit_under_selection += hist->GetBinContent(i);
    } while(hist->GetBinLowEdge(i) > m_thresh);
  }
  
  // calculate and return percentage
  double percent = ((double) hit_under_selection)/((double) N);
  return percent;
}

void ZDCPercentageThreshTests::printDescription(std::ostream& out){
  std::string message;
  message += "\n";
  message += "Algorithm: ZDCPercentEvents_" + m_name + "\n";
  if (m_name == "XthBin"){
    message += "Description: Retrieve the percent of entries in the x-th bin and compare it with user defined green and red Threshold\n";
  }else if (m_name == "XthBin_NormalizeToFirstBin"){
    message += "Description: Retrieve the percent of entries in the x-th bin, normalized to #entries in the first bin, and compare it with user defined green and red Threshold\n";
  }else if (m_name == "AboveThreshold"){
    message += "Description: Compute the percent of entries above a threshold and compare it with user defined green and red Threshold\n";
  }else if (m_name == "UnderThreshold"){
    message += "Description: Compute the percent of entries under a threshold and compare it with user defined green and red Threshold\n";
  }
  message += "             Green if below green Threshold; red if above red Threshold\n";
  
  message += "Mandatory Green/Red Threshold: PercentBar: Percent bar imposed upon #entries passing selection"; 
  if (m_name == "XthBin" || m_name == "XthBin_NormalizeToFirstBin"){
    message += "Mandatory Parameter: BinNum = number of the bin we impose check on\n";
  }else if (m_name == "AboveThreshold" || m_name == "UnderThreshold"){
    message += "Mandatory Parameter: Thresh = threshold\n";
  }
  message += "Optional Parameters: MinStat = Minimum histogram statistics needed to perform Algorithm\n";
  message += "\n";
  
  out << message;
}

} // namespace dqm_algorithms
