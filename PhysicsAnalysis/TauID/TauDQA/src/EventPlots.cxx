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

EventPlots::~EventPlots()
{
}

void EventPlots::initializePlots(){

   m_avgmu = Book1D("AverageMu","Average Interaction per bunch crossing; <mu>; # Events", 16, 0.0, 80.0);  
}
  
void EventPlots::fill(float avg_mu, float weight) {

   m_avgmu->Fill(avg_mu,weight);  

}


}
