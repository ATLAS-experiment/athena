/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILEWIENERFILTERWEIGHTS_H
#define TILECONDITIONS_TILEWIENERFILTERWEIGHTS_H

#include "GaudiKernel/MsgStream.h"
#include <memory>

struct TileWienerFilterWeightsStruct
{
  int luminosity = 0;
  double generalWeights[4][48][8] = {{{0}}}; // one set of weights for each TileCall cell
  double optimalWeights[7][8] = {{0}}; // weights for E3 and E4 only
};

class  TileWienerFilterWeights {

  friend class TileInfoLoader;

 public:

  TileWienerFilterWeights();
  ~TileWienerFilterWeights();

  void loadWeights(MsgStream &log);
  const TileWienerFilterWeightsStruct * TileWFstruct() const { return m_weights.get(); }
  bool loaded()    { return m_loaded; }

 private:

  //variables
  int  m_Luminosity;
  int  m_NSamples_Phys = 0;
  bool m_loaded;
  std::unique_ptr<TileWienerFilterWeightsStruct> m_weights;

};

#endif // TILECONDITIONS_TILEWIENERFILTERWEIGHTS_H
