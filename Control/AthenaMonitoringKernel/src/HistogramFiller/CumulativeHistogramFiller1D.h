/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/  
  
#ifndef AthenaMonitoringKernel_HistogramFiller_CumulativeHistogramFiller1D_h
#define AthenaMonitoringKernel_HistogramFiller_CumulativeHistogramFiller1D_h

#include "HistogramFiller1D.h"

namespace Monitored {
  /**
   * @brief Filler for 1D histograms filled in cummulative mode
   */
  class CumulativeHistogramFiller1D : public HistogramFiller1D {
  public:
    CumulativeHistogramFiller1D(const HistogramDef& definition, std::shared_ptr<IHistogramProvider> provider)
      : HistogramFiller1D(definition, std::move(provider)) {}


    
    virtual unsigned fill( const HistogramFiller::VariablesPack& vars ) const override {
      if ( vars.size() != 1) {
        return 0;
      }

      const size_t varVecSize = vars[0]->size();

      if (vars.cut) {
        const size_t maskSize = vars.cut->size();
        // Abort if no cut entries or first (and only) entry is false
        if (maskSize == 0 || (maskSize == 1 && !vars.cut->get(0))) { return 0; }
        if (ATH_UNLIKELY(maskSize > 1 && maskSize != varVecSize)) {
          MsgStream log(Athena::getMessageSvc(), "CumulativeHistogramFiller1D");
          log << MSG::ERROR << "CutMask does not match the size of plotted variable: "
              << maskSize << " " << varVecSize << endmsg;
        }
      }

      unsigned i{0};
      auto histogram = this->histogram<TH1>();
      for (; i < varVecSize; i++) {
        if (vars.cut && !vars.cut->get(i)) { continue; }
        const unsigned bin = histogram->FindBin(vars[0]->get(i));

        for (unsigned j = bin; j > 0; --j) {
          histogram->AddBinContent(j);
        }
      }

      return i;  
    }
  };
}

#endif /* AthenaMonitoringKernel_HistogramFiller_CumulativeHistogramFiller1D_h */
