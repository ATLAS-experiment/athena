/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPCELLMAP_H
#define TRIGGEPPERF_GEPCELLMAP_H

#include "./GepCaloCell.h"
#include <map>
#include <memory>
#include "AthenaKernel/CLASS_DEF.h"

typedef std::unique_ptr<std::map<unsigned int,Gep::GepCaloCell>> pGepCellMap;

namespace Gep{

  class GepCellMap {

  public:
    GepCellMap() {}
    ~GepCellMap() {}

  void insert(unsigned int id, const Gep::GepCaloCell & cell) {
	  m_cellMap.emplace(id, cell);
  }

  unsigned int size() {
	  return m_cellMap.size();
  }

  pGepCellMap getCellMap() {
	return std::make_unique<std::map<unsigned int,Gep::GepCaloCell>>(m_cellMap);
  }
  
  private:  

    std::map<unsigned int,Gep::GepCaloCell> m_cellMap;

  };
}

CLASS_DEF(Gep::GepCellMap, 252505461, 1 )

#endif //TRIGGEPPERF_GEPCELLMAP_H
