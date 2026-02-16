/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "LArSamplesMon/AbsLArCells.h"

#include "LArSamplesMon/History.h"
#include "LArCafJobs/CellInfo.h"
#include "LArSamplesMon/FilterParams.h"


#include <iostream>
using std::cout;
using std::endl;

using namespace LArSamples;


//std::vector<CellInfo*> AbsLArCells::m_cellInfoCache(Definitions::nChannels,nullptr);

AbsLArCells::AbsLArCells():
  m_pos(nChannels() + 1), 
  m_cellInfoCache(nChannels())
{ }

AbsLArCells::~AbsLArCells()
{
  AbsLArCells::resetCache();
}


void AbsLArCells::resetCache() const
{
  m_cellCache.reset();
  m_pos = nChannels() + 1;
}


std::unique_ptr<const History> AbsLArCells::newCellHistory(unsigned int i) const
{ 
  std::unique_ptr<const History> history = getCellHistory(i);
  if (!history) return nullptr;
  if (!m_cellInfoCache[i]) {
    const CellInfo* ci=history->cellInfo();
    if (ci) {
      m_cellInfoCache[i]=std::make_unique<CellInfo>(*ci,false);
    }
  }
  //  m_cellInfoCache[i] = (history->cellInfo() ? new CellInfo(*history->cellInfo(), false) : new CellInfo());
  return history;
}


const History* AbsLArCells::cellHistory(unsigned int i) const 
{ 
  if (m_pos == i) return m_cellCache.get();
  resetCache();
  std::unique_ptr<const History> history = newCellHistory(i);
  if (!history) return nullptr;
  m_cellCache = std::move(history);
  m_pos = i;
  return m_cellCache.get();
}


std::unique_ptr<const CellInfo> AbsLArCells::cellInfo(unsigned int i) const
{
  const CellInfo* info = cellInfoCache(i);
  if (info) {
    if (info->isValid()) {
      return std::make_unique<CellInfo> (*info);
    }
    return nullptr;
  }
  std::unique_ptr<const CellInfo> infop = getCellInfo(i);
  if (infop)  m_cellInfoCache[i] =  std::make_unique<CellInfo>(*infop, false);
  return infop;
}


const CellInfo* AbsLArCells::cellInfoCache(unsigned int i) const
{
  return m_cellInfoCache[i].get();
}


std::unique_ptr<const CellInfo> AbsLArCells::getCellInfo(unsigned int i) const
{
  std::unique_ptr<const History> history = this->getCellHistory(i);
  if (!history) return nullptr;
  if (!history->cellInfo()) return nullptr;
  return std::make_unique<CellInfo>(*history->cellInfo());
}


const History* AbsLArCells::pass(unsigned int i, const FilterParams& f) const 
{ 
  //std::cout << "Called AbsLArCells with hash " << i  << std::endl;
  if (!f.passHash(i)) return nullptr;
  std::unique_ptr<const CellInfo> info = cellInfo(i);
  if (!info) {
    return nullptr;
  }
  //std::cout << "Called AbsLArCells::pass on a cell belonging to " << Id::str(info->calo()) << std::endl;
  bool result = f.passCell(*info);
  return result ? cellHistory(i) : nullptr;
}


