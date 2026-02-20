/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamplesHistory
   @brief Class to fit shapes
 */


#ifndef LArSamples_ShapeDrawer_H
#define LArSamples_ShapeDrawer_H

#include <vector>
#include <memory>
#include "LArCafJobs/AbsShape.h"
#include "CxxUtils/checker_macros.h"

class TGraphErrors;
class TLegend;

namespace LArSamples {
    
  enum DrawParams { DataFirst = 1, SamplingTimeUnits = 2, Legend = 4, AtlasStyle = 8 };
  
  class ATLAS_NOT_THREAD_SAFE ShapeDrawer  {
  
    public:
   
      /** @brief Constructor (takes ownership of LArCellInfo object) */
      ShapeDrawer(int pars) : m_pars(pars) { }
      
      TGraphErrors* graph(const AbsShape& shape, const char* title = "") const;
      
      TGraphErrors* draw(const AbsShape* shape, const char* title = "", bool drawAxes = true, const char* gopt = "P") const;
      
      bool draw(const char* title, const std::vector<const AbsShape*>& shapes,
                const AbsShape* reference = 0, const AbsShape* refSamples = 0) const;
      bool draw(const char* title, const AbsShape* shape, const AbsShape* reference = 0, const AbsShape* refSamples = 0) const;
      
      bool drawAndDelete(const char* title,
                         std::vector<std::unique_ptr<const AbsShape> >&& shapes,
                         std::unique_ptr<const AbsShape> reference = 0,
                         std::unique_ptr<const AbsShape> refSamples = 0) const;
      bool drawAndDelete(const char* title,
                         std::unique_ptr<const AbsShape> shape,
                         std::unique_ptr<const AbsShape> reference = nullptr,
                         std::unique_ptr<const AbsShape> refSamples = nullptr) const;
      
      bool colorize(TGraphErrors* data, unsigned int index) const;
      
      bool yAxisLimits(const std::vector<const AbsShape*>& shapes, double& yMin, double& yMax) const;
      
    private:
      
      bool drawRef(const char* title, const AbsShape* reference, const AbsShape* refSamples, bool first = true, TLegend* legend = 0) const;
      bool drawData(const char* title, const std::vector<const AbsShape*>& shapes, const AbsShape* reference, bool first = true, TLegend* legend = 0) const;

      int m_pars;
  };
}
#endif
