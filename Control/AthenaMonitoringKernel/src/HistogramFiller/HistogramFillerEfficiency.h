/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/  
  
#ifndef AthenaMonitoringKernel_HistogramFiller_HistogramFillerEfficiency_h
#define AthenaMonitoringKernel_HistogramFiller_HistogramFillerEfficiency_h

#include "TEfficiency.h"

#include "AthenaMonitoringKernel/HistogramFiller.h"
#include "HistogramFillerUtils.h"

namespace Monitored {
  /**
   * @brief Filler for TEfficiency graphs
   */
  class HistogramFillerEfficiency : public HistogramFiller {
  public:
    HistogramFillerEfficiency(const HistogramDef& definition, std::shared_ptr<IHistogramProvider> provider)
      : HistogramFiller(definition, std::move(provider)) {}

    virtual unsigned fill( const HistogramFiller::VariablesPack& vars ) const override {

      if ( ATH_UNLIKELY( vars.var[0] == nullptr or vars.var[1] == nullptr ) ) return 0;

      if (vars.cut) {
        const size_t maskSize = vars.cut->size();
        // Abort if no cut entries or first (and only) entry is false
        if (maskSize == 0 || (maskSize == 1 && !vars.cut->get(0))) { return 0; }
        if (ATH_UNLIKELY(maskSize > 1 && maskSize != vars.var[0]->size())) {
          MsgStream log(Athena::getMessageSvc(), "HistogramFillerEfficiency");
          log << MSG::ERROR << "CutMask does not match the size of plotted variable: "
              << maskSize << " " << vars.var[0]->size() << endmsg;
        }
      }

      TEfficiency* efficiency = this->histogram<TEfficiency>();
      const TH1* efftot = efficiency->GetTotalHistogram();

      const unsigned nMonVar = vars.size();
      const size_t var0Size = vars.var[0]->size();
      if ( nMonVar==2 ) { // Single observable (1D TEfficiency)
        for (unsigned i = 0; i < var0Size; ++i) {
          if (vars.cut==nullptr || vars.cut->get(i)) {
            efficiency->Fill(vars.var[0]->get(i),
                             detail::getFillValue<Axis::X>(efftot, vars.var[1], i));
          }
        }
        return var0Size;
      } else if ( nMonVar==3 ) { // Two observables (2D TEfficiency)
        for (unsigned i = 0; i < var0Size; ++i) {
          if (vars.cut==nullptr || vars.cut->get(i)) {
            efficiency->Fill(vars.var[0]->get(i),
                             detail::getFillValue<Axis::X>(efftot, vars.var[1], i),
                             detail::getFillValue<Axis::Y>(efftot, vars.var[2], i));
          }
        }
        return var0Size;
      } else if ( nMonVar==4 ) { // Three observables (3D Efficiency)
        for (unsigned i = 0; i < var0Size; ++i) {
          if (vars.cut==nullptr || vars.cut->get(i)) {
            efficiency->Fill(vars.var[0]->get(i),
                             detail::getFillValue<Axis::X>(efftot, vars.var[1], i),
                             detail::getFillValue<Axis::Y>(efftot, vars.var[2], i),
                             detail::getFillValue<Axis::Z>(efftot, vars.var[3], i));
          }
        }
        return var0Size;
      } else {
        return 0;
      }
      return var0Size;
    }
  };
}

#endif /* AthenaMonitoringKernel_HistogramFiller_HistogramFillerEfficiency_h */
