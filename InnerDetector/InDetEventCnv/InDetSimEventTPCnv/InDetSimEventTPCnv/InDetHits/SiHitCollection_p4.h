/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETSIMEVENTTPCNV_SIHITCOLLECTION_P4_H
#define INDETSIMEVENTTPCNV_SIHITCOLLECTION_P4_H

#include <vector>
#include <string>

class SiHitCollection_p4
{
 public:
/// Default constructor
  SiHitCollection_p4 ();
  //private:

  std::vector<float>          m_hit1_meanTime;   //  1 element per string
  std::vector<float>          m_hit1_x0;         //
  std::vector<float>          m_hit1_y0;         //
  std::vector<float>          m_hit1_z0;         //
  std::vector<float>          m_hit1_theta;      //
  std::vector<float>          m_hit1_phi;        //
  std::vector<unsigned short> m_nHits;           //

  std::vector<unsigned short> m_hitEne_2b;       //  1 element per hit
  std::vector<unsigned short> m_hitLength_2b;    //

  std::vector<unsigned short> m_dTheta;          //  1 element per hit except for first hit in string
  std::vector<unsigned short> m_dPhi;            //

  std::vector<float>          m_hitEne_4b;       //  1 element per hit with  m_hitEne_2b[i] == 2**16

  std::vector<float>          m_hitLength_4b;    //  1 element per hit with  m_hitLength_2b[i] == 2**16

  std::vector<unsigned long>  m_truthID;
  std::vector<unsigned short> m_mcEvtIndex;
  std::vector<unsigned short> m_nTruthID;

  std::vector<unsigned long>  m_id;
  std::vector<unsigned short> m_nId;
};


// inlines

inline
SiHitCollection_p4::SiHitCollection_p4 () {}

#endif // INDETSIMEVENTTPCNV_SIHITCOLLECTION_P4_H
