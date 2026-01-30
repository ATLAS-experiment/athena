/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUDQA_EVENTPLOTS_H
#define TAUDQA_EVENTPLOTS_H

#include "TrkValHistUtils/PlotBase.h"
#include "xAODEventInfo/EventInfo.h"

class TH1;

namespace Tau{

class EventPlots: public PlotBase {
   public:
      EventPlots(PlotBase *pParent, const std::string& sDir);
      virtual ~EventPlots();
      
      void fill(float avg_mu, float weight);

      TH1* m_avgmu{};

   private:
      void initializePlots();
};

}

#endif
