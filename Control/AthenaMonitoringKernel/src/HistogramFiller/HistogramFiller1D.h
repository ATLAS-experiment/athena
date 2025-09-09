/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaMonitoringKernel_HistogramFiller_HistogramFiller1D_h
#define AthenaMonitoringKernel_HistogramFiller_HistogramFiller1D_h

#include "TH1.h"

#include "AthenaMonitoringKernel/HistogramFiller.h"
#include "HistogramFillerUtils.h"

#include "AthenaKernel/getMessageSvc.h"
#include "CxxUtils/AthUnlikelyMacros.h"
#include "GaudiKernel/MsgStream.h"

namespace Monitored {

  /**
   * @brief Filler for plain 1D histograms
   */
  class HistogramFiller1D : public HistogramFiller {
  public:
    HistogramFiller1D(const HistogramDef& definition, std::shared_ptr<IHistogramProvider> provider)
      : HistogramFiller(definition, std::move(provider)) {
    }

    virtual unsigned fill( const HistogramFiller::VariablesPack& vars ) const override {

      if ( vars.cut ) {
        const size_t maskSize = vars.cut->size();
        // Abort if no cut entries or first (and only) entry is false
        if (maskSize == 0 || (maskSize == 1 && !vars.cut->get(0))) { return 0; }
        if (ATH_UNLIKELY(maskSize > 1 && maskSize != vars.var[0]->size())) {
          MsgStream log(Athena::getMessageSvc(), "HistogramFiller1D");
          log << MSG::ERROR << "CutMask does not match the size of plotted variable: "
              << maskSize << " " << vars.var[0]->size() << endmsg;
        }
      }

      // Accessor for cut mask in case one is defined
      auto cutMaskAccessor = [&](size_t i) { return static_cast<bool>(vars.cut->get(i)); };

      if (vars.weight) {
        auto weightAccessor = [&](size_t i){ return vars.weight->get(i); };

        if (ATH_UNLIKELY(vars.weight->size() != vars.var[0]->size())) {
          MsgStream log(Athena::getMessageSvc(), "HistogramFiller1D");
          log << MSG::ERROR << "Weight does not match the size of plotted variable: "
              << vars.weight->size() << " " << vars.var[0]->size() << endmsg;
        }
        // Need to fill here while weightVector is still in scope
        if (not vars.cut) return HistogramFiller::fill<TH1>(weightAccessor, detail::noCut, *vars.var[0]);
        else              return HistogramFiller::fill<TH1>(weightAccessor, cutMaskAccessor, *vars.var[0]);
      }

      if (not vars.cut) return HistogramFiller::fill<TH1>(detail::noWeight, detail::noCut, *vars.var[0]);
      else              return HistogramFiller::fill<TH1>(detail::noWeight, cutMaskAccessor, *vars.var[0]);
    }
  };
}

#endif /* AthenaMonitoringKernel_HistogramFiller_HistogramFiller1D_h */
