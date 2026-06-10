/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
// Filename : TileMuId2DBAlg.h
// Author   : Aranzazu Ruiz
// Created  : May 2009
//
// DESCRIPTION
//    Create ASCII file with TileMuId thresholds to be stored in COOL DB.
//
// BUGS:
//  
// History:
//  
//****************************************************************************

#ifndef TileCalibAlgs_TileMuId2DBAlg_h
#define TileCalibAlgs_TileMuId2DBAlg_h

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/ReadCondHandleKey.h"
class CaloCell_ID;

class TileMuId2DBAlg: public AthAlgorithm {

 public:

  TileMuId2DBAlg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~TileMuId2DBAlg() = default;
  virtual StatusCode initialize() override;  
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

 private:

  const CaloCell_ID* m_calo_id{};

  SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey
    { this, "TotalNoiseKey", "totalNoise", "SG key for total noise" };
};

#endif // TileCalibAlgs_TileMuId2DBAlg_h
