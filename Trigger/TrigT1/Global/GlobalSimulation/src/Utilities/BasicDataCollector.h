/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_BASICDATACOLLECTOR_H
#define GLOBALSIM_BASICDATACOLLECTOR_H

#include "IDataCollector.h"
#include "GaudiKernel/IAlgTool.h"
#include <vector>
#include <chrono>


namespace GlobalSim {
  class BasicDataCollector: public IDataCollector {
  public:
    BasicDataCollector();
    virtual ~BasicDataCollector() = default;
    virtual void collect(const IAlgTool&, const std::string& msg);
    virtual std::string to_string() const;

  private:
    std::vector<std::pair<std::string, std::vector<std::string>>> m_msgs{};
    std::chrono::high_resolution_clock::time_point m_t0;
  };

  std::ostream& operator << (std::ostream& os, const BasicDataCollector& bdc);
}
#endif
