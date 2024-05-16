/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*! \file TileTriggerMonitor.cxx checks an individual bin of a 1D histogram wrt to a threshold value and returns dqm_core::Result
 * \author Kyle Lleras
 */

#include <dqm_algorithms/TileTriggerMonitor.h>
#include <dqm_algorithms/tools/AlgorithmHelper.h>
#include <dqm_core/AlgorithmConfig.h>
#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TClass.h>
#include <ers/ers.h>


#include <dqm_core/AlgorithmManager.h>

static dqm_algorithms::TileTriggerMonitor myInstance;

dqm_algorithms::TileTriggerMonitor::TileTriggerMonitor()

{
  dqm_core::AlgorithmManager::instance().registerAlgorithm("TileTriggerMonitor", this);
}

dqm_algorithms::TileTriggerMonitor * 
dqm_algorithms::TileTriggerMonitor::clone()
{
  
  return new TileTriggerMonitor();
}


dqm_core::Result *
dqm_algorithms::TileTriggerMonitor::execute(const std::string & name,
					const TObject & object, 
					const dqm_core::AlgorithmConfig & config)
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

  dqm_core::Result* result = new dqm_core::Result();
  //Count each category of the bins, if you find a red bin show red, if you find a yellow bin show yellow
  std::vector<int> range=dqm_algorithms::tools::GetBinRange(histogram, config.getParameters());
  int worstStatus = -3;
  for ( int towerNum = range.at(0); towerNum <= range.at(1); ++towerNum ) {
    double inputCont = histogram->GetBinContent(towerNum);
    if (inputCont>worstStatus) worstStatus=inputCont; //if you want to have a test for every bin remove the for loop and replace execute with 1D black bin algorithm
    if (inputCont>0) dqm_algorithms::tools::PublishBin(histogram,towerNum,0,inputCont,result);
    }

  result->tags_["BinContent"] = worstStatus;
  if(worstStatus==2) result->status_ = dqm_core::Result::Red;
  else if(worstStatus==1) result->status_ = dqm_core::Result::Yellow;
  else if(worstStatus==0) result->status_ = dqm_core::Result::Green;
  else {
    result->status_ = dqm_core::Result::Disabled;
  }
  
  return result;


}


void
dqm_algorithms::TileTriggerMonitor::printDescription(std::ostream& out)
{
  //put optional description for developers
  out<<"Red bin: The tower has experienced a large spike in rate. The large spike threshold algorithm is written in TilePPMContainerSpike in TriggerMonitor in L1Calo's git repository. \n" << std::endl;

  out<<"Yellow bin: The tower has experienced a spike in rate. The spike threshold algorithm is written in TilePPMContainerSpike in TriggerMonitor in L1Calo's git repository. \n" << std::endl;

  out<<"Black bin: This tower is disabled\n" << std::endl;

  out<<"Optional Parameter, Publish Bin: Publishes the content of the bins that are different than result." << std::endl;

  
}

