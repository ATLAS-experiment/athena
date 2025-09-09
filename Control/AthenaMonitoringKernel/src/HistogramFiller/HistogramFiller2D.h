/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AthenaMonitoringKernel_HistogramFiller_HistogramFiller2D_h
#define AthenaMonitoringKernel_HistogramFiller_HistogramFiller2D_h

#include "TH2.h"

#include "HistogramFiller.h"
#include "HistogramFillerUtils.h"
#include "CxxUtils/AthUnlikelyMacros.h"
#include "GaudiKernel/MsgStream.h"

namespace Monitored {

  /**
   * @brief Generic filler for 2D histogram
   *
   * @tparam H  Type of 2D histogram (TH2, TProfile)
   */
  template<typename H>
  class HistogramFiller2DGeneric : public HistogramFiller {
  public:
    HistogramFiller2DGeneric(const HistogramDef& definition, std::shared_ptr<IHistogramProvider> provider)
      : HistogramFiller(definition, std::move(provider)) {}

    virtual unsigned fill( const HistogramFiller::VariablesPack& vars) const override {
      if (ATH_UNLIKELY(vars.var[0] == nullptr or vars.var[1] == nullptr )) return 0;

      const size_t size0 = vars.var[0]->size();
      const size_t size1 = vars.var[1]->size();

      if (ATH_UNLIKELY(size0 == 0 || size1 == 0)) {
        // nothing to do
        return 0;
      }

      if (ATH_UNLIKELY(size0 > 1 && size1 > 1 && size0 != size1)) {
        MsgStream log(Athena::getMessageSvc(), "HistogramFiller2D");
        log << MSG::ERROR << "Mismatch of provided vector sizes "
            << size0 << "," << size1 << " for " << m_histDef->alias << endmsg;
        return 0;
      }

      if (vars.cut) {
        const size_t maskSize = vars.cut->size();
        // Abort if no cut entries or first (and only) entry is false
        if (maskSize == 0 || (maskSize == 1 && !vars.cut->get(0))) { return 0; }
        if (ATH_UNLIKELY(size0 > 1 && size1 > 1 &&
                         maskSize > 1 && size0 != maskSize)) {
          MsgStream log(Athena::getMessageSvc(), "HistogramFiller2D");
          log << MSG::ERROR << "CutMask does not match the size of plotted variable: "
              << maskSize << " " << size0 << endmsg;
          return 0;
        }
      }

      // Accessor for cut mask in case one is defined
      auto cutMaskAccessor = [&](size_t i) { return static_cast<bool>(vars.cut->get(i)); };

      if (vars.weight) {
        auto weightAccessor = [&](size_t i){ return vars.weight->get(i); };
        if (ATH_UNLIKELY(size0 > 1 && size1 > 1 &&
                         vars.weight->size() > 1 && size0 != vars.weight->size())) {
          MsgStream log(Athena::getMessageSvc(), "HistogramFiller2D");
          log << MSG::ERROR << "Weight does not match the size of plotted variable: "
              << vars.weight->size() << " " << size0 << endmsg;
          return 0;
        }
        // Need to fill here while weightVector is still in scope
        if (not vars.cut) return HistogramFiller::fill<H>(weightAccessor, detail::noCut, *vars.var[0], *vars.var[1]);
        else                  return HistogramFiller::fill<H>(weightAccessor, cutMaskAccessor, *vars.var[0], *vars.var[1]);
      }

      if (not vars.cut) return HistogramFiller::fill<H>(detail::noWeight, detail::noCut, *vars.var[0], *vars.var[1]);
      else                  return HistogramFiller::fill<H>(detail::noWeight, cutMaskAccessor, *vars.var[0], *vars.var[1]);
    }
  };

  typedef HistogramFiller2DGeneric<TH2> HistogramFiller2D;
}

#endif /* AthenaMonitoringKernel_HistogramFiller_HistogramFiller2D_h */

