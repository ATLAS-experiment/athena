/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVTXVALIDATIONMACROUTILS_H
#define MSVTXVALIDATIONMACROUTILS_H

#include <vector>
#include <string>

#include "GaudiKernel/SystemOfUnits.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "GeoPrimitives/GeoPrimitives.h"


namespace MuonVertexValidationMacroUtils {
    // fiducial volume definition
    constexpr double fidVol_barrel_etaCut = 0.7;
    constexpr double fidVol_Lxy_low = 3*Gaudi::Units::m;
    constexpr double fidVol_Lxy_up = 8*Gaudi::Units::m;

    constexpr double fidVol_endcaps_etaCut_low = 1.3;
    constexpr double fidVol_endcaps_etaCut_up = 2.5;
    constexpr double fidVol_endcaps_Lxy_up = 10*Gaudi::Units::m;
    constexpr double fidVol_z_low = 5*Gaudi::Units::m;
    constexpr double fidVol_z_up = 15*Gaudi::Units::m;

    constexpr double match_max = 0.4;

    // Detector region separation into barrel and endcaps
    bool inBarrel(double eta);
    bool inBarrel(const Amg::Vector3D &vtx);
    bool inEndcaps(double eta);
    bool inEndcaps(const Amg::Vector3D &vtx);
    bool inDetectorRegion(const Amg::Vector3D &vtx);
    int getNvtxBarrel(const std::vector<Amg::Vector3D> &vertices);
    int getNvtxEndcaps(const std::vector<Amg::Vector3D> &vertices);
    int getNvtxDetectorRegion(const std::vector<Amg::Vector3D> &vertices);

    // MSVtx search fiducial volume
    bool inFiducialVolBarrel(const Amg::Vector3D &vtx);
    bool inFiducialVolEndcaps(const Amg::Vector3D &vtx);
    bool inFiducialVol(const Amg::Vector3D &vtx);
    int NvtxFiducialVol(const std::vector<Amg::Vector3D> &vertices);
    bool isGoodVtx(const Amg::Vector3D &vtx);

    // Matching of vertices 
    double getMatchMetric(const Amg::Vector3D &vtx1, const Amg::Vector3D &vtx2);
    Amg::Vector3D findBestMatch(const Amg::Vector3D &vtx1, const std::vector<Amg::Vector3D> &vtx2_vec);
    bool isValidMatch(const Amg::Vector3D &match_candidate);
    bool hasMatch(const Amg::Vector3D &vtx1, const std::vector<Amg::Vector3D> &vtx2_vec);

    // position vectors
    std::vector<Amg::Vector3D> getVertexPos(const std::vector<double> &vtx_x, const std::vector<double> &vtx_y, const std::vector<double> &vtx_z);
    std::vector<std::vector<Amg::Vector3D>> getConstituentPos(int Nvtx, const std::vector<int> &obj_vtx_link,
                                                              const std::vector<double> &obj_x, const std::vector<double> &obj_y, const std::vector<double> &obj_z);
}

#endif // MSVTXVALIDATIONMACROUTILS_H
