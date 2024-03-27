/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/  
  
#ifndef AthenaMonitoringKernel_HistogramFiller_HistogramFiller2DProfile_h
#define AthenaMonitoringKernel_HistogramFiller_HistogramFiller2DProfile_h

#include "TProfile2D.h"
#include "boost/range/combine.hpp"

#include "AthenaMonitoringKernel/HistogramFiller.h"

namespace Monitored {
  /**
   * @brief Filler for profile 2D histogram
   */
  class HistogramFiller2DProfile : public HistogramFiller {
    public:
      HistogramFiller2DProfile(const HistogramDef& definition, std::shared_ptr<IHistogramProvider> provider)
        : HistogramFiller(definition, provider) {}


      virtual unsigned fill( const HistogramFiller::VariablesPack& vars ) const override {
      if ( vars.size() != 3) {
        return 0;
      }

      // handling of the cutmask
      auto cutMaskValuePair = getCutMaskFunc(vars.cut);
      if (cutMaskValuePair.first == 0) { return 0; }
      auto cutMaskAccessor = cutMaskValuePair.second;

      if (vars.weight) {
        // Weighted fill
        auto weightAccessor = [&](size_t i){ return vars.weight->get(i); };
        const size_t size0 = vars.var[0]->size();
        const size_t size1 = vars.var[1]->size();
        const size_t size2 = vars.var[2]->size();
        const size_t sizeWeight = vars.weight->size();
        if (ATH_UNLIKELY(size0 > 1 && size1 > 1 && size2 > 1 &&
                         sizeWeight > 1 && size0 != sizeWeight)) {
            MsgStream log(Athena::getMessageSvc(), "HistogramFiller2DProfile");
            log << MSG::ERROR << "Weight does not match the size of plotted variable: "
                << vars.weight->size() << " " << size0 << endmsg;
            return 0;
        }
        // Need to fill here while weightVector is still in scope
        if (not vars.cut) return HistogramFiller::fill<TProfile2D>(weightAccessor, detail::noCut, *vars.var[0], *vars.var[1], *vars.var[2]);
        else              return HistogramFiller::fill<TProfile2D>(weightAccessor, cutMaskAccessor, *vars.var[0], *vars.var[1], *vars.var[2]);
      } 
      // Unweighted fill
      if (not vars.cut) return HistogramFiller::fill<TProfile2D>(detail::noWeight, detail::noCut, *vars.var[0], *vars.var[1], *vars.var[2]);
      else              return HistogramFiller::fill<TProfile2D>(detail::noWeight, cutMaskAccessor, *vars.var[0], *vars.var[1], *vars.var[2]);
    }
  };

    /// TProfile2D filler with rebinable x-axis
    typedef HistogramFillerRebinableAxis<HistogramFiller2DProfile, Axis::X> HistogramFiller2DProfileRebinable;

}

#endif /* AthenaMonitoringKernel_HistogramFiller_HistogramFiller2DProfile_h */
