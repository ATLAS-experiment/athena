/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <dqm_algorithms/EfficiencyRefComp.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_algorithms/tools/SimpleAlgorithmConfig.h>
#include <TH1.h>
#include <TClass.h>
#include <TObjArray.h>
#include <dqm_core/AlgorithmManager.h>

#include <iostream>

namespace{
static dqm_algorithms::EfficiencyRefComp erc_instance;
}


dqm_algorithms::EfficiencyRefComp::EfficiencyRefComp(){
  dqm_core::AlgorithmManager::instance().registerAlgorithm("EfficiencyRefComp",this);
}

dqm_algorithms::EfficiencyRefComp* dqm_algorithms::EfficiencyRefComp::clone(){
  return new EfficiencyRefComp();
}

dqm_core::Result* dqm_algorithms::EfficiencyRefComp::execute(const std::string& name, const TObject& object, const dqm_core::AlgorithmConfig& config){
  const TH1 * histogram;

  if( object.IsA()->InheritsFrom( "TH1" ) ) {
    histogram = static_cast<const TH1*>(&object);
  } else {
    throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH1" );
  }

  const double minstat = dqm_algorithms::tools::GetFirstFromMap( "MinStat", config.getParameters(), -1);
  const bool publish = (bool) dqm_algorithms::tools::GetFirstFromMap( "PublishBins", config.getParameters(), 0); 
  const int maxpublish = (int) dqm_algorithms::tools::GetFirstFromMap( "MaxPublish", config.getParameters(), 20); 
  

  if (histogram->GetEntries() < minstat ) {
    dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    result->tags_["InsufficientEffectiveEntries"] = histogram->GetEffectiveEntries();
    delete histogram;
    return result;
  }

  TObject* ro = config.getReference();
  const TObject* firstReference=0;
  TObject* secondReference=0;
  try { 
    dqm_algorithms::tools::handleReference( *ro , firstReference , secondReference );
  }catch ( dqm_core::Exception & ex ) {
    throw dqm_core::BadRefHist(ERS_HERE,name," Could not retreive reference");
  }

  //Check the reference
  const TH1* refhist = dynamic_cast<const TH1*>(firstReference);
  if ( refhist==0 ){
    throw dqm_core::BadRefHist( ERS_HERE, "Dimension", name );
  }

  if ((histogram->GetDimension() != refhist->GetDimension()) || (histogram->GetNbinsX() != refhist->GetNbinsX()) || (histogram->GetNbinsY() != refhist->GetNbinsY()) || refhist->GetNbinsZ() != histogram->GetNbinsZ() ) {
    throw dqm_core::BadRefHist( ERS_HERE, "number of bins", name );
  }

  //get range of input histograms
  std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters()); 

  //get config
  bool check_bin_err = (dqm_algorithms::tools::GetFirstFromMap("CheckBinErr", config.getParameters(), -1) > 0); //if the bin diff < err on bin ignore it
  double posMaxDiff = dqm_algorithms::tools::GetFirstFromMap("PosMaxDiff", config.getParameters(), 1.0);
  double negMaxDiff = dqm_algorithms::tools::GetFirstFromMap("NegMaxDiff", config.getParameters(), 1.0);

  int count = 0;

  dqm_core::Result* result = new dqm_core::Result();
  TH1* resulthisto;
  resulthisto=(TH1*)(histogram->Clone());
  resulthisto->Reset();

  //loop over each bin and calulcate the difference from the reference
  for(int i=range[0]; i< range[1]; i++){
    double binContent = histogram->GetBinContent(i);
    double refContent = refhist->GetBinContent(i);
    double binErr = histogram->GetBinError(i);

    if(std::abs(binContent - refContent) < binErr && check_bin_err){
      //difference between ref and bin content is less than 1 sigma
      continue;
    }

    double diff = binContent - refContent;
    double adiff = std::abs(diff);
    double threshold = 1000;

    if(diff < 0){
      threshold = negMaxDiff;
    }else{
      threshold = posMaxDiff;
    }

    if(adiff > threshold){
      //bin differs from reference by more than threshold
      count++;
      resulthisto->SetBinContent(i, binContent);
      if (publish && count< maxpublish){
        dqm_algorithms::tools::PublishBin(histogram,i,0,binContent,result);
      }
    }
  }

  result->tags_["NBins"] = count;
  result->object_ =  (boost::shared_ptr<TObject>)(TObject*)(resulthisto);

  double rthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getRedThresholds() );
  double gthreshold = dqm_algorithms::tools::GetFromMap( "NBins", config.getGreenThresholds() );

  if(count >= rthreshold){
    result->status_ = dqm_core::Result::Red;
  }else if(count <= gthreshold){
    result->status_ = dqm_core::Result::Green;
  }else{
    result->status_ = dqm_core::Result::Yellow;
  }

  return result;
}



void dqm_algorithms::EfficiencyRefComp::printDescription(std::ostream& out) {
  out<<"EfficiencyRefComp_: Compares each bin to the reference bin and calculates the difference. The difference for each bin is then compared to a threshold value. Different thresholds are supported for bins > ref and < ref."<<std::endl;
}
