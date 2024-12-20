/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Utils.h"


namespace MuonVertexValidationMacroUtils {


// Detector region: Separation into barrel and endcaps 
bool inBarrel(double eta){
    return std::abs(eta) <= fidVol_barrel_etaCut;
}


bool inBarrel(const Amg::Vector3D &vtx){
    return inBarrel(vtx.eta());
}


bool inEndcaps(double eta){
    return (std::abs(eta) > fidVol_endcaps_etaCut_low && std::abs(eta) < fidVol_endcaps_etaCut_up); 
}


bool inEndcaps(const Amg::Vector3D &vtx){
    return inEndcaps(vtx.eta()); 
}


bool inDetectorRegion(const Amg::Vector3D &vtx){
    return inBarrel(vtx) || inEndcaps(vtx);
}


int getNvtxBarrel(const std::vector<Amg::Vector3D> &vertices){
    // find the number of vertices in the barrel region
    return std::accumulate(vertices.begin(), vertices.end(), 0, [](unsigned int N, const Amg::Vector3D& vtx){return N+inBarrel(vtx);});
}


int getNvtxEndcaps(const std::vector<Amg::Vector3D> &vertices){
    // find the number of vertices in the endcap region
    return std::accumulate(vertices.begin(), vertices.end(), 0, [](unsigned int N, const Amg::Vector3D& vtx){return N+inEndcaps(vtx);});
}


int getNvtxDetectorRegion(const std::vector<Amg::Vector3D> &vertices){
    // find the number of vertices in the either the barrel or endcaps
    return getNvtxBarrel(vertices) + getNvtxEndcaps(vertices);
}
// --- //


// Muon spectrometer displaced vertex search fiducial region //
bool inFiducialVolBarrel(const Amg::Vector3D &vtx){
    bool eta = inBarrel(vtx);
    bool Lxy = vtx.perp() > fidVol_Lxy_low && vtx.perp() < fidVol_Lxy_up; 
    return eta && Lxy;
}


bool inFiducialVolEndcaps(const Amg::Vector3D &vtx){
    bool eta = inEndcaps(vtx);
    bool z = std::abs(vtx.z()) > fidVol_z_low && std::abs(vtx.z()) < fidVol_z_up;
    return eta && z;
}


bool inFiducialVol(const Amg::Vector3D &vtx){
    return inFiducialVolBarrel(vtx) || inFiducialVolEndcaps(vtx);
}


int NvtxFiducialVol(const std::vector<Amg::Vector3D> &vertices){
    // find the number ofvertices in the fiducial volume
    unsigned int N = 0;
    for (const Amg::Vector3D &vtx : vertices) if (inFiducialVol(vtx)) ++N;
    return N;
}


bool isGoodVtx(const Amg::Vector3D &vtx){
    // add further conditions for a vertex to be considered in efficiency calculations here
    bool is_good = true;
    if (!inFiducialVol(vtx)) is_good = false;
    return is_good;
}
// --- //


// matching of vertices
double getMatchMetric(const Amg::Vector3D &vtx1, const Amg::Vector3D &vtx2){
    // Computes the metric on which matches are made between two vertices
    return xAOD::P4Helpers::deltaR(vtx1.eta(), vtx1.phi(), vtx2.eta(), vtx2.phi());
}


Amg::Vector3D findBestMatch(const Amg::Vector3D &vtx, const std::vector<Amg::Vector3D> &candidates){
    // finds the best match for vtx among candidates
    // The match metric is assumed to decreases for better matches. If no match could be found, a zero vector is returned
    Amg::Vector3D best{Amg::Vector3D::Zero()}; // initialised with zeros
    double aux(0), min(999);

    for (unsigned int i=0; i<candidates.size(); ++i){
        aux = getMatchMetric(vtx, candidates[i]);
        if (aux > match_max) continue;
        if (aux < min){
            min = aux;
            best = candidates[i];
        }
    }

    return best;
}


bool isValidMatch(const Amg::Vector3D &match_candidate){
    return match_candidate.mag() > 0.001;
} 


bool hasMatch(const Amg::Vector3D &vtx1, const std::vector<Amg::Vector3D> &vtx2_vec){
    // Checks if vtx1 has a match in vtx2_vec
    Amg::Vector3D candidate = findBestMatch(vtx1, vtx2_vec);
    return isValidMatch(candidate);
}


std::vector<Amg::Vector3D> getVertexPos(const std::vector<double> &vtx_x, const std::vector<double> &vtx_y, const std::vector<double> &vtx_z){
    std::vector<Amg::Vector3D> vertices;
    for (unsigned int i=0; i<vtx_x.size(); ++i){
        Amg::Vector3D vtx_pos(vtx_x[i], vtx_y[i], vtx_z[i]);
        vertices.push_back(vtx_pos);
    }

    return vertices;
}


std::vector<std::vector<Amg::Vector3D>> getConstituentPos(int Nvtx, const std::vector<int> &obj_vtx_link,
                                                          const std::vector<double> &obj_x, const std::vector<double> &obj_y, const std::vector<double> &obj_z){
    // returns the constituent position vectors for each reconstructed vertex  
    std::vector<std::vector<Amg::Vector3D>> consti_vec;
    for (int j=0; j<Nvtx; ++j){
        std::vector<Amg::Vector3D> consti;
        for (unsigned int k=0; k<obj_x.size(); ++k){
            if (obj_vtx_link[k] == j){consti.push_back(Amg::Vector3D(obj_x[k], obj_y[k], obj_z[k]));}
        }
        consti_vec.push_back(consti);
    }

    return consti_vec;
}

} // namespace MuonVertexValidationMacroUtils
