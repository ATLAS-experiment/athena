/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "EventPlots.h"
#include "AthenaKernel/Units.h"

namespace Tau{

EventPlots::EventPlots(PlotBase* pParent, const std::string& sDir):
   PlotBase(pParent, sDir)
{	
}

void EventPlots::initializePlots(){

   m_avgmu = Book1D("AverageMu","Average Interaction per bunch crossing; <mu>; # Events", 16, 0.0, 80.0);  
   m_highavgmu = Book1D("HighAverageMu","Average Interaction per bunch crossing; <mu> (high <mu> regime); # Events", 16, 140.0, 220.0); 
}
  
void EventPlots::fill(float avg_mu, float weight) {

   m_avgmu->Fill(avg_mu,weight);  
   m_highavgmu->Fill(avg_mu,weight); 

}


}
