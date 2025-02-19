/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_DEFECTSEMULATORBASE_H
#define INDET_DEFECTSEMULATORBASE_H

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <string_view>
#include <array>

class TH2;

namespace InDet {

/** Common base class for the specializations of the DefectsEmulatorAlg template.
  * Provides access to debug histograms to optionally monitor the dropped RDOs.
  */
class DefectsEmulatorBase : public AthReentrantAlgorithm {
public:
  DefectsEmulatorBase(const std::string &name,ISvcLocator *pSvcLocator);

  StatusCode initializeBase(unsigned int wafer_hash_max);
  virtual StatusCode finalize() override;

protected:
   ServiceHandle<ITHistSvc> m_histSvc
      {this,"HistSvc","THistSvc"};
   Gaudi::Property<std::string> m_histogramGroupName
      {this,"HistogramGroupName","", "Histogram group name or empty to disable histogramming"};

   // Get rejected hits histogram for a certain module design
   // @param n_rows the number of rows of the module design
   // @param n_cols the number of columns of the module design
   // @return rejected_hits per cell histogram
   // calls during execute must be protected by m_histMutex
   TH2 *findHist(unsigned int n_rows, unsigned int n_cols) const;

   enum EHistType {
      kRejectedHits,
      kNHistTypes
   };
   static const std::array<std::string_view,kNHistTypes> s_histNames;
   static const std::array<std::string_view,kNHistTypes> s_histTitles;

   mutable std::mutex m_histMutex ;
   // access to the following must be protected by m_histMutex during execute
   mutable std::vector<unsigned int>  m_dimPerHist ATLAS_THREAD_SAFE;
   mutable std::array<TH2 *,kNHistTypes> m_moduleHist ATLAS_THREAD_SAFE {};
   mutable std::array<std::vector< TH2 *>,kNHistTypes> m_hist ATLAS_THREAD_SAFE;


   // Simple counters for dropped and copied RDOs
   mutable std::atomic<std::size_t>   m_rejectedRDOs {};
   mutable std::atomic<std::size_t>   m_totalRDOs {};
   mutable std::atomic<std::size_t>   m_splitRDOs {};

   bool m_histogrammingEnabled = false;

};

}

#endif
