/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/*! \file BinThreshold.cxx checks bins wrt to a threshold value and returns dqm_core::Result
 * \author Haleh Hadavand
 */

#include <dqm_core/AlgorithmConfig.h>
#include <dqm_algorithms/BinThreshold.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <TH1.h>
#include <TF1.h>
#include <TClass.h>
#include <TRegexp.h>
#include <ers/ers.h>

#include <iostream>
#include <cmath>
#include <dqm_core/AlgorithmManager.h>

namespace
{
  dqm_algorithms::BinThreshold GreaterThan( "GreaterThan" );
  dqm_algorithms::BinThreshold GreaterThanAbs( "GreaterThanAbs" );
  dqm_algorithms::BinThreshold GreaterThanNonZeroMedian( "GreaterThanNonZeroMedian" );
  dqm_algorithms::BinThreshold GreaterThanEqual( "GreaterThanEqual" );
  dqm_algorithms::BinThreshold LessThan( "LessThan" );
  dqm_algorithms::BinThreshold LessThanAbs( "LessThanAbs" );
  dqm_algorithms::BinThreshold LessThanNonZeroMedian( "LessThanNonZeroMedian" );
  dqm_algorithms::BinThreshold LessThanEqual( "LessThanEqual" );
  dqm_algorithms::BinThreshold Equal( "Equal" );
  dqm_algorithms::BinThreshold NotEqual( "NotEqual" );
}


dqm_algorithms::BinThreshold::BinThreshold( const std::string & name )
  : m_name( name )
{
  dqm_core::AlgorithmManager::instance().registerAlgorithm("Bins_"+name+"_Threshold", this);
}

dqm_algorithms::BinThreshold * 
dqm_algorithms::BinThreshold::clone()
{
  
  return new BinThreshold( m_name );
}


dqm_core::Result *
dqm_algorithms::BinThreshold::execute(	const std::string &  name, 
						const TObject & object, 
                                                const dqm_core::AlgorithmConfig & config )
{  
  const TH1 * histogram;
  
  if( object.IsA()->InheritsFrom( "TH1" ) ) {
    histogram = static_cast<const TH1*>(&object);
    if (histogram->GetDimension() > 2 ){ 
      throw dqm_core::BadConfig( ERS_HERE, name, "dimension > 2 " );
    }
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }
  
  const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  const bool publish = (bool) dqm_algorithms::tools::GetFirstFromMap( "PublishBins", config.getParameters(), 0); 
  const int maxpublish = (int) dqm_algorithms::tools::GetFirstFromMap( "MaxPublish", config.getParameters(), 20); 
  const int xmax = (int) dqm_algorithms::tools::GetFirstFromMap( "xMax", config.getParameters(), -1); 
  const int xmin = (int) dqm_algorithms::tools::GetFirstFromMap( "xMin", config.getParameters(), -1); 
  const int ymax = (int) dqm_algorithms::tools::GetFirstFromMap( "yMax", config.getParameters(), -1); 
  const int ymin = (int) dqm_algorithms::tools::GetFirstFromMap( "yMin", config.getParameters(), -1); 

  std::string ignoreBins = dqm_algorithms::tools::GetFirstFromMap("IgnoreBins", config.getGenericParameters(), "-1");  

  std::vector<std::string> hotRows;
  std::vector<std::string> hotCols;
  std::vector<std::pair<std::string,std::string>> hotBins;

  parseIgnoreList(ignoreBins, hotRows, hotCols, hotBins);

  if (histogram->GetEntries() < minstat ) {
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEntries"] = histogram->GetEntries();
    return result;
  }
  
  double bin_threshold;
  double gthreshold;
  double rthreshold;
  try {
    bin_threshold = dqm_algorithms::tools::GetFirstFromMap( "BinThreshold", config.getParameters() );
    rthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getRedThresholds() );
    gthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getGreenThresholds() );
  }
  catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadConfig( ERS_HERE, name, ex.what(), ex );
  }
  
  
  int count = 0; 
  std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters()); 
  dqm_core::Result* result = new dqm_core::Result();
  TH1* resulthisto;
  if (histogram->InheritsFrom("TH2")) {
	resulthisto=(TH1*)(histogram->Clone());
  } else if (histogram->InheritsFrom("TH1")) {
        resulthisto=(TH1*)(histogram->Clone());
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }
 
  resulthisto->Reset();
  if(m_name.find("Median")!=std::string::npos){
    std::vector<double> bin_vals;
    for ( int i = range[0]; i <= range[1]; ++i ) {
      for ( int j = range[2]; j <= range[3]; ++j ) {
	if(histogram -> GetBinContent(i,j) > 0) bin_vals . push_back( histogram -> GetBinContent(i,j) );
      }
    }
    std::sort(bin_vals.begin(), bin_vals.end());
    unsigned bin_vals_size = bin_vals.size();
    double median = 0;
    if(bin_vals_size%2==0 && bin_vals_size>1) median = (bin_vals.at(bin_vals_size/2-1)+bin_vals.at(bin_vals_size/2))/2;
    else if(bin_vals_size%2==1 && bin_vals_size>1) median = bin_vals.at(bin_vals_size/2-1);
    else if(bin_vals_size==1) median = bin_vals.at(0);

    double min_bin_threshold = dqm_algorithms::tools::GetFirstFromMap( "MinBinThreshold", config.getParameters() , -99999);
    double max_bin_threshold = dqm_algorithms::tools::GetFirstFromMap( "MaxBinTrheshold", config.getParameters() , -99999);
    bin_threshold=median*bin_threshold;

    bin_threshold = std::max(bin_threshold, min_bin_threshold);
    if(max_bin_threshold > -1 ) bin_threshold = std::min(bin_threshold, max_bin_threshold);

    result->tags_["Effective_BinThreshold"] = bin_threshold;
  }

  //bools for skiping threshold comparison of ignored rows, columns and bins
  bool skipCol;
  bool skipRow;
  std::pair<bool,bool> skipBin;
  std::pair<const char*, const char*> binLabel;

  for ( int i = range[0]; i <= range[1]; ++i ) {
    skipCol = false;
    binLabel.first  = nullptr;
    binLabel.second = nullptr;

    if ( !hotCols.empty() ) {
      for (const auto& col : hotCols){

        // Check if the given string matches the bin index or the bin label
        binLabel.first = histogram->GetXaxis()->GetBinLabel(i);
        if ( std::to_string(i)==col || TString(binLabel.first).Contains(TRegexp((col).c_str(),true)) ) skipCol = true;
      }
    }
    //Skip column if ignored
    if (skipCol) continue;

    //Skip bin threshold comparison if xmax!=-1 and i>xmax
    if (xmax!=-1 && i>xmax) continue;
    //Skip bin threshold comparison if xmin!=-1 and i<xmin
    if (xmin!=-1 && i<xmin) continue;

    for ( int j = range[2]; j <= range[3]; ++j ) {
      skipRow = false;
      binLabel.first  = nullptr; 
      binLabel.second = nullptr; 

      if ( !hotRows.empty() ) {
        for (const auto& row : hotRows){

          // Check if the given string matches the bin index or the bin label
          binLabel.second = histogram->GetYaxis()->GetBinLabel(j);
          if ( std::to_string(j)==row || TString(binLabel.second).Contains(TRegexp((row).c_str(),true)) ) skipRow = true;
        }
      }
      //Skip row if ignored
      if (skipRow) continue;

      skipBin.first  = false;
      skipBin.second = false;
      binLabel.first  = nullptr; 
      binLabel.second = nullptr;
 
      if ( !hotBins.empty() ) {
        for (const auto& bin : hotBins){

          // Check col (bin.first)
          binLabel.first = histogram->GetXaxis()->GetBinLabel(i);
          if ( std::to_string(i)==bin.first || TString(binLabel.first).Contains(TRegexp((bin.first).c_str(),true)) ) skipBin.first = true;
 
          // Check row (bin.second)
          binLabel.second = histogram->GetYaxis()->GetBinLabel(j);
          if ( std::to_string(j)==bin.second || TString(binLabel.second).Contains(TRegexp((bin.second).c_str(),true)) ) skipBin.second = true;
        }
      }
      //Skip bin if ignored
      if (skipBin.first && skipBin.second) continue;

      //Skip bin threshold comparison if ymax!=-1 and j>ymax
      if (ymax!=-1 && j>ymax) continue;
      //Skip bin threshold comparison if ymin!=-1 and j<ymin
      if (ymin!=-1 && j<ymin) continue;

      double content= histogram -> GetBinContent(i,j);
      if ( CompareBinThreshold(m_name, content, bin_threshold )) {
	++count;
          resulthisto->SetBinContent(i,j,content);
	  if (publish && count< maxpublish){
           dqm_algorithms::tools::PublishBin(histogram,i,j,content,result);
	  }
      }
    }
  }

  ERS_DEBUG(1,"Number of bins " << m_name << " treshold of " << bin_threshold << " is " << count );
  ERS_DEBUG(1,"Green threshold: "<< gthreshold << " bin(s);   Red threshold : " << rthreshold << " bin(s) ");    
  
  int TotalBins = (int) dqm_algorithms::tools::GetFirstFromMap( "TotalBins", config.getParameters() , -99999);
  if(TotalBins > -10){
    if(TotalBins < 0) TotalBins = histogram->GetNbinsX()*histogram->GetNbinsY()*histogram->GetNbinsZ();
    int effectiveCount = count - (histogram->GetNbinsX()*histogram->GetNbinsY()*histogram->GetNbinsZ() - TotalBins);
    if(m_name.find("LessThan")!=std::string::npos) result->tags_["NBins_%"] = 100.*effectiveCount/TotalBins;
    else result->tags_["NBins_%"] = 100.*count/TotalBins;
  }

  result->tags_["NBins"] = count;
  result->object_ =  boost::shared_ptr<TObject>(resulthisto);
  if (gthreshold > rthreshold) {
     if ( count >= gthreshold ) {
        result->status_ = dqm_core::Result::Green;
     } else if ( count > rthreshold ) {
        result->status_ = dqm_core::Result::Yellow;
     } else {
        result->status_ = dqm_core::Result::Red;
     }
  } else {
     if ( count <= gthreshold ) {
        result->status_ = dqm_core::Result::Green;
     } else if ( count < rthreshold ) {
        result->status_ = dqm_core::Result::Yellow;
     } else {
        result->status_ = dqm_core::Result::Red;
     }
  }
  
  return result;
  
}

bool
dqm_algorithms::BinThreshold::CompareBinThreshold(const std::string & type, double bincontent, double threshold) {
  
  if (type == "GreaterThan") return (bincontent > threshold);
  if (type == "GreaterThanAbs") return (std::abs(bincontent) > threshold);
  if (type == "GreaterThanNonZeroMedian") return (bincontent > threshold);
  if (type == "LessThan") return (bincontent < threshold);
  if (type == "LessThanAbs") return (std::abs(bincontent) < threshold);
  if (type == "LessThanNonZeroMedian") return (bincontent < threshold);
  if (type == "LessThanEqual") return (bincontent <= threshold);
  if (type == "GreaterThanEqual") return (bincontent >= threshold);
  if (type == "Equal") return (bincontent == threshold);
  if (type == "NotEqual") return (bincontent != threshold);

  return 0;
}

void dqm_algorithms::BinThreshold::parseIgnoreList(const std::string& inputBins, 
                                                   std::vector<std::string>& ignoredRows,
                                                   std::vector<std::string>& ignoredCols,
                                                   std::vector<std::pair<std::string,std::string>>& ignoredBins) {
  // Remove "" from input string
  std::string inputBins_new = "";
  for (char c : inputBins) if (c != '"') inputBins_new+=c;

  if (!inputBins_new.empty()) {
    std::stringstream ss(inputBins_new);
    std::string token;
  
    while (std::getline(ss, token, ',')) {
      std::stringstream pairStream(token);
      std::string first, second;
  
      if (std::getline(pairStream, first, ':') && std::getline(pairStream, second, ':')) {
        if (first == "*") {
          ignoredRows.push_back(second);
        } else if (second == "*") {
          ignoredCols.push_back(first);
        } else {
          ignoredBins.push_back(std::make_pair(first,second));
        }
      }
    }
  }
}

void
dqm_algorithms::BinThreshold::printDescription(std::ostream& out)
{
  
  out<<"Bins_"+m_name+"_Threshold: Checks for number of bins "+m_name+" threshold value\n"<<std::endl;
  
  out<<"Mandatory Parameter: BinThreshold: Look for bins "+m_name+" BinTreshold; Count number of bins satifying requirement \n"<<std::endl;
  
  out<<"Mandatory Green/Red Threshold: NBins: Number of bins satifying "+m_name+" BinThreshold constraint to give Green/Red result\n"<<std::endl;
  
  out<<"Optional Parameter: PublishBins: Save bins which are different from average in Result (set to 1)\n"<<std::endl; 
  out<<"Optional Parameter: MaxPublish: Max number of bins to save (default 20)"<<std::endl; 
  out<<"Optional Parameter: MinStat: Minimum histogram statistics needed to perform Algorithm"<<std::endl;
  out<<"Optional Parameter: xmin: minimum x range"<<std::endl;
  out<<"Optional Parameter: xmax: maximum x range"<<std::endl;
  out<<"Optional Parameter: ymin: minimum y range"<<std::endl;
  out<<"Optional Parameter: ymax: maximum y range\n"<<std::endl;
  out<<"Optional Parameter: IgnoreBins: List of bins to be plotted in the DQ histograms but ignored in the DQ mismatch check>\n"<<std::endl;
  
}

