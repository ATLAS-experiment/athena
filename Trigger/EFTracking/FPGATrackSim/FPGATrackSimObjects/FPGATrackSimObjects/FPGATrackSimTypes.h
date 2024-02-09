/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimTYPES_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimTYPES_H

#include <iostream>
#include <cmath>
#include <array>
#include <cstdint>

#define MODULE_BADMODULE -1
#define SECTOR_NOTFOUND -1
#define FITTRACKS_OK 0
#define FITTRACKS_BAD -1


typedef int32_t pid_t;      // pattern id
typedef int32_t module_t;   // module id
typedef int32_t sector_t;   // sector number
typedef uint32_t layer_bitmask_t; // bitmask that stores at least as many bits as layers

//To label the origin of FPGATrackSimHits
enum class SiliconTech { strip, pixel, nTechs, undefined };

//The detector zone that the FPGATrackSimHit was in
enum class DetectorZone { barrel, posEndcap, negEndcap, nZones, undefined };

// Enum to hold the different types of hits in FPGATrackSimOfflineHit
enum class OfflineHitType { Measurement, InertMaterial, BremPoint, Scatterer, Perigee, Outlier, Hole, Other };

// An Enum to hold the stage of the track
enum class TrackStage { FIRST, SECOND };

// Enum to hold the type of correction for hits on track
enum class TrackCorrType { None = 0, First = 1, Second = 2 };

// Enum to hold type of sample (for truth cuts)
enum class SampleType { skipTruth, singleElectrons=11, singleMuons=13, singlePions=211, LLPs=1000 };

std::ostream& operator<<(std::ostream& os, SiliconTech t);
std::ostream& operator<<(std::ostream& os, DetectorZone t);

// Calculate euclidean norm
inline double eucnorm(double x, double y, double z)
{
  return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

// Calculate opening angle between two vectors in euclidean space
inline double eucangle(double x, double y, double z, double u, double v, double w)
{
  return acos((x * u + y * v + z * w) / (eucnorm(x, y, z) * eucnorm(u, v, w)));
}


constexpr std::array< double, 8 > TARGET_R_1STAGE = { 291.3, 396.9, 559.5, 565.5, 759.3, 765.3, 996.9, 1003};
constexpr std::array< double, 13 > TARGET_R_2STAGE = { 33.9, 99.7, 160.4, 228.5, 291.3, 396.9, 403.2, 559.5, 565.5, 759.3, 765.3, 996.9, 1003};

#endif
