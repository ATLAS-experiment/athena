/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamplesDataTweaker
   @brief storage of a collection of digits for one cell
*/

#ifndef LArSamples_DataTweaker_H
#define LArSamples_DataTweaker_H


#include "LArSamplesMon/Chi2Calc.h"
#include "CxxUtils/checker_macros.h"
#include <map>

namespace LArSamples {
  
  class Data;
  class EventData;
  class ScaledErrorData;
  
  class ATLAS_NOT_THREAD_SAFE DataTweaker  {
  
   public:
   
    DataTweaker() = default;

    bool set(const TString& tweaks);
    bool findOption(TString& tweaks, const TString& option) const;

    Data* tweak(const Data& data, int evtIndex = -1) const;
    EventData* tweak(const EventData& eventData, int runIndex = -1) const;

    void setNSamplesTrunctation(unsigned int n) { m_nSamplesTruncation = n; }
    void addSample(unsigned int n) { m_samples[n] = true; }
    void setRemoveNoiseCorrelations(bool remove = true) { m_removeNoiseCorrelations = remove; }
    void setRefit(bool refit = true) { m_refit = refit; }
    void setFitParams(Chi2Params params) { m_fitParams = params; }
    void setAdjust(bool adjust = true) { m_adjust = adjust; }
    void setRemoveRoIs(bool removeRoIs = true) { m_removeRoIs = removeRoIs; }

  private:
   
    unsigned int m_nSamplesTruncation{0};
    bool m_removeNoiseCorrelations{false}, m_refit{false}, m_adjust{false}, m_removeRoIs{false};
    std::map<unsigned int, bool> m_samples; //automatically ordered... 
    Chi2Params m_fitParams{DefaultChi2};
  };
}
#endif
