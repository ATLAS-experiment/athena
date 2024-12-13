/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

class TH2;

namespace InDet {

/** Common base class for the specializations of the DefectsEmulatorAlg template.
  * Provides access to debug histograms to optionally monitor the dropped RDOs.
  */
class DefectsEmulatorBase : public AthReentrantAlgorithm {
public:
  DefectsEmulatorBase(const std::string &name,ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

protected:
   ServiceHandle<ITHistSvc> m_histSvc
      {this,"HistSvc","THistSvc"};
   Gaudi::Property<std::string> m_histogramGroupName
      {this,"HistogramGroupName","", "Histogram group name or empty to disable histogramming"};

   // calls during execute must be protected by m_histMutex
   TH2 *findHist(unsigned int n_rows, unsigned int n_cols) const;

   mutable std::mutex m_histMutex ;
   // access to the following must be protected by m_histMutex during execute
   mutable std::vector<unsigned int>  m_dimPerHist ATLAS_THREAD_SAFE;
   mutable TH2 *                      m_moduleHist ATLAS_THREAD_SAFE = nullptr;

   mutable std::vector< TH2 *>        m_hist ATLAS_THREAD_SAFE;

   // Simple counters for dropped and copied RDOs
   mutable std::atomic<std::size_t>   m_rejectedRDOs {};
   mutable std::atomic<std::size_t>   m_totalRDOs {};

   bool m_histogrammingEnabled = false;

};

}

#endif
