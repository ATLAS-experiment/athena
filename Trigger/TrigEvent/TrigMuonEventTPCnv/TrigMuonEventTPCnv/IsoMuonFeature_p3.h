/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: Trigger
 * @Package: TrigMuonEventTPCnv
 * @class  : IsoMuonFeature_p3
 *
 * @brief persistent partner for IsoMuonFeature
 *
 * @author Stefano Giagu <Stefano.Giagu@cern.ch>  - U. of Rome
 * @modified by Fabrizio Salvatore <p.salvatore@cern.ch> - U of Sussex
 **********************************************************************************/
#ifndef TRIGMUONEVENTTPCNV_ISOMUONFEATURE_P3_H
#define TRIGMUONEVENTTPCNV_ISOMUONFEATURE_P3_H

class IsoMuonFeature_p3 {
  friend class IsoMuonFeatureCnv_p3;

 public:

  IsoMuonFeature_p3() = default;
  ~IsoMuonFeature_p3() = default;

  //private:

  float m_allTheFloats[14]{};
  int   m_flag{};
  int   m_RoiIdMu{};

};

#endif

