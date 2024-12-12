/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigMuonEventTPCnv
 * @class  : IsoMuonFeature_p1
 *
 * @brief persistent partner for IsoMuonFeature
 *
 * @author Stefano Giagu <Stefano.Giagu@cern.ch>  - U. of Rome
 * @modified by Fabrizio Salvatore <p.salvatore@cern.ch> - U of Sussex
 **********************************************************************************/
#ifndef TRIGMUONEVENTTPCNV_ISOMUONFEATURE_P2_H
#define TRIGMUONEVENTTPCNV_ISOMUONFEATURE_P2_H

//#include "AthenaPoolUtilities/TPObjRef.h"

class IsoMuonFeature_p2 {
  friend class IsoMuonFeatureCnv_p2;

 public:

  IsoMuonFeature_p2() = default;
  ~IsoMuonFeature_p2() = default;

  float m_allTheFloats[11]{};
  //  float m_EtInnerConeEC;
  //  float m_EtOuterConeEC;
  //  float m_EtInnerConeHC;
  //  float m_EtOuterConeHC;
  int   m_NTracksCone{};
  //  float m_SumPtTracksCone;
  //  float m_PtMuTracksCone;
  //  float m_LAr_w;
  //  float m_Tile_w;
  int   m_RoiIdMu{};
  // float m_PtMu;
  // float m_EtaMu;
  // float m_PhiMu;

};

#endif

