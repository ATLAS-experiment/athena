/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimOFFLINEHIT_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimOFFLINEHIT_H


#include <TObject.h>
#include "FPGATrackSimObjects/FPGATrackSimTypes.h"

class FPGATrackSimOfflineHit {
public:
  FPGATrackSimOfflineHit() {m_locX=0; m_locY=0; m_isPixel = false; m_isBarrel = false; m_layer = -1; m_clustID = -1; m_trackNumber = -1; m_hitType = OfflineHitType::Other;}
  virtual ~FPGATrackSimOfflineHit() = default;

  void setLocX(float locx) { m_locX = locx; }
  void setLocY(float locy) { m_locY = locy; }
  void setIsPixel(bool is) { m_isPixel = is; }
  void setIsBarrel(bool is) { m_isBarrel = is; }
  void setLayer(int lay) { m_layer = lay; }
  void setClusterID(int clus) { m_clustID = clus; }
  void setTrackNumber(int track) { m_trackNumber = track; }
  void setHitType(OfflineHitType type) { m_hitType = type; }

  float getLocX() const { return m_locX; }
  float getLocY() const { return m_locY; }
  bool isPixel()  const { return m_isPixel; }
  bool isBarrel() const { return m_isBarrel; }
  int getLayer()  const { return m_layer; }
  int getClusterID() const { return m_clustID; }
  int getTrackNumber() const { return m_trackNumber; }
  OfflineHitType getHitType() const { return m_hitType; }

private:
  float   m_locX;
  float   m_locY;
  bool    m_isPixel;
  bool    m_isBarrel;
  int     m_layer;
  int     m_clustID;
  int     m_trackNumber;
  OfflineHitType   m_hitType;

  ClassDefNV(FPGATrackSimOfflineHit, 3)
};

std::ostream& operator<<(std::ostream&, const FPGATrackSimOfflineHit&);
#endif // TRIGFPGATrackSimOBJECTS_FPGATrackSimOFFLINETRACK_H

