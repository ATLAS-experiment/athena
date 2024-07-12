/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include<map>
#include<vector>
#include<string>

dqm_core::Result * TestTrigger()
{   
    dqm_algorithms::TileTriggerMonitor * algorithm = new dqm_algorithms::TileTriggerMonitor();

    TH1F *histogram=new TH1F("histogram","histogram",3,0,3);
    histogram->SetBinContent(2,0);
    histogram->SetBinContent(3,1);

   dqm_core::test::DummyAlgorithmConfig *aconfig = new dqm_core::test::DummyAlgorithmConfig(histogram);
    aconfig->addParameter("XBin", 2);
    aconfig->addGenericParameter("IgnoredBins","2,")

    
    dqm_core::Result * result = algorithm->execute( "test", *histogram, *aconfig);
    std::cout << "Result " << result->status_<< std::endl;    
    return result;

}
