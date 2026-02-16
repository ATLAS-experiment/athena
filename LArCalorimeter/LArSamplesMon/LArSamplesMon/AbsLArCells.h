/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   @class LArSamples::AbsLArCells
   @brief A base class for accessing ntuple data
*/

#ifndef LArSamples_AbsLArCells_H
#define LArSamples_AbsLArCells_H

#include "LArCafJobs/Definitions.h"
#include "CxxUtils/checker_macros.h"

#include <vector>
#include <memory>

namespace LArSamples {
  
  class CellInfo;
  class History;
  class FilterParams;
  
  class ATLAS_NOT_THREAD_SAFE AbsLArCells  {
  
    public:
      
      AbsLArCells();
      virtual ~AbsLArCells();
      
      virtual std::unique_ptr<const History> newCellHistory(unsigned int i) const;
      virtual const History* cellHistory(unsigned int i) const;
      virtual std::unique_ptr<const CellInfo> cellInfo(unsigned int i) const;
      virtual unsigned int nChannels() const { return Definitions::nChannels; }
      virtual unsigned int nChannelsSC() const { return Definitions::nChannelsSC; }

      virtual void resetCache() const;

      const CellInfo* cellInfoCache(unsigned int i) const;
      const History* pass(unsigned int i, const FilterParams& f) const;

      virtual std::unique_ptr<const History> getCellHistory(unsigned int i) const = 0;
      virtual std::unique_ptr<const CellInfo> getCellInfo(unsigned int i) const;

      virtual std::unique_ptr<const History> getSCHistory(unsigned int i) const = 0;

      const History* cellCache() const { return m_cellCache.get(); }
      unsigned int cachePos() const { return m_pos; }


    private:

      mutable unsigned int m_pos;
      mutable std::unique_ptr<const History> m_cellCache;
      mutable std::vector<std::unique_ptr<CellInfo> > m_cellInfoCache;

  };
}
  
#endif
