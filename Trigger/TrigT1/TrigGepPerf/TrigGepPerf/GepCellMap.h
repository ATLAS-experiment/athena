/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPCELLMAP_H
#define TRIGGEPPERF_GEPCELLMAP_H

#include "src/GepCaloCell.h"
#include <map>
#include <memory>
#include "AthenaKernel/CLASS_DEF.h"

typedef std::unique_ptr<std::map<unsigned int,Gep::GepCaloCell>> pGepCellMap;

namespace Gep{

  class GepCellMap {

  public:

  void insert(unsigned int id, const Gep::GepCaloCell & cell) {
	  m_cellMap.emplace(id, cell);
  }

  unsigned int size() {
	  return m_cellMap.size();
  }

  pGepCellMap getCellMap() {
	return std::make_unique<std::map<unsigned int,Gep::GepCaloCell>>(m_cellMap);
  }

  void setNumberOfOverflowingFEB2s(int n) { m_nFeb2sInOverflow = n; }

  int getNumberOfOverflowingFEB2s() { return m_nFeb2sInOverflow; }
  
  private:  

    std::map<unsigned int,Gep::GepCaloCell> m_cellMap;
    int m_nFeb2sInOverflow = -1;

  };
}

CLASS_DEF(Gep::GepCellMap, 252505461, 1 )

#endif //TRIGGEPPERF_GEPCELLMAP_H
