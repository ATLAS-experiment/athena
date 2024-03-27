/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4SIMTPCNV_TRACKRECORD_P2_H
#define G4SIMTPCNV_TRACKRECORD_P2_H
#include <string>

class TrackRecord_p2 {
public:
  TrackRecord_p2() {};
  int PDG_code() const {return m_PDG_code;}
  int status() const {return m_status;}
  float energy() const {return m_energy;}
  float momentumX() const {return m_momentumX;}
  float momentumY() const {return m_momentumY;}
  float momentumZ() const {return m_momentumZ;}
  float positionX() const {return m_positionX;}
  float positionY() const {return m_positionY;}
  float positionZ() const {return m_positionZ;}
  float time() const {return m_time;}
  int uniqueID() const {return m_uniqueID;}
  std::string volName() const {return m_volName;}
  friend class TrackRecordCnv_p2;

private:
  int m_PDG_code{0};
  int m_status{0};
  float m_energy{0};
  float m_momentumX{0}, m_momentumY{0}, m_momentumZ{0};
  float m_positionX{0}, m_positionY{0}, m_positionZ{0};
  float m_time{0};
  int m_uniqueID{0};
  std::string m_volName{""};
};

#endif // G4SIMTPCNV_TRACKRECORD_P2_H
