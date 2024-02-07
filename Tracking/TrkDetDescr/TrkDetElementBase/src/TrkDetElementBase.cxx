/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkDetElementBase/TrkDetElementBase.h"

Trk::TrkDetElementBase::TrkDetElementBase(const GeoVFullPhysVol* fullPhysVol)
    : GeoVDetectorElement(fullPhysVol) {}

std::string Trk::TrkDetElementBase::detectorTypeString() const {
  auto type = detectorType();
  if (type == Trk::DetectorElemType::SolidState)
    return "SolidState";
  else if (type == Trk::DetectorElemType::Silicon)
    return "Silicon";
  else if (type == Trk::DetectorElemType::TRT)
    return "TRT";
  else if (type == Trk::DetectorElemType::Csc)
    return "Csc";
  else if (type == Trk::DetectorElemType::Mdt)
    return "Mdt";
  else if (type == Trk::DetectorElemType::Rpc)
    return "Rpc";
  else if (type == Trk::DetectorElemType::Tgc)
    return "Tgc";
  else if (type == Trk::DetectorElemType::sTgc)
    return "sTgc";
  else if (type == Trk::DetectorElemType::MM)
    return "Mm";
  return "Unknown";
}
