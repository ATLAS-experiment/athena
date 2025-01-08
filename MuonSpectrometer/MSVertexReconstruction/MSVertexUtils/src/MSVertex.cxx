/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MSVertexUtils/MSVertex.h"

#include <sstream>


MSVertex::MSVertex(int author, const Amg::Vector3D& position, double chi2prob, double chi2, int nMDT, int nRPC, int nTGC) :
    m_author(author), m_position(position), m_tracks(0), m_chi2prob(chi2prob), m_chi2(chi2), m_nMDT(nMDT), m_nRPC(nRPC), m_nTGC(nTGC) {}

MSVertex::MSVertex(int author, const Amg::Vector3D& position, const std::vector<const xAOD::TrackParticle*>& tracks, double chi2prob, double chi2,
                   int nMDT, int nRPC, int nTGC) :
    m_author(author),
    m_position(position),
    m_tracks(tracks),
    m_chi2prob(chi2prob),
    m_chi2(chi2),
    m_nMDT(nMDT),
    m_nRPC(nRPC),
    m_nTGC(nTGC) {}

MSVertex::~MSVertex() = default;

void MSVertex::setPosition(const Amg::Vector3D& position) { m_position = position; }

const Amg::Vector3D& MSVertex::getPosition() const { return m_position; }

const std::vector<const xAOD::TrackParticle*>* MSVertex::getTracks(void) const { return &m_tracks; }

void MSVertex::setAuthor(const int author) { m_author = author; }

int MSVertex::getAuthor() const { return m_author; }
double MSVertex::getChi2Probability() const { return m_chi2prob; }
double MSVertex::getChi2() const { return m_chi2; }

int MSVertex::getNTracks() const {
    if (getTracks())
        return getTracks()->size();
    else
        return 0;
}

void MSVertex::setNMDT(const int nMDT, const int nMDT_inwards, const int nMDT_I, const int nMDT_E, const int nMDT_M, const int nMDT_O) { 
    m_nMDT = nMDT;                  // total number of MDT hits around the vertex
    m_nMDT_inwards = nMDT_inwards;  // number of MDT hits around the vertex inwards of the vertex position
    m_nMDT_I = nMDT_I;              // number of MDT hits around the vertex in the inner layer
    m_nMDT_E = nMDT_E;              // number of MDT hits around the vertex in the extended layer
    m_nMDT_M = nMDT_M;              // number of MDT hits around the vertex in the middle layer
    m_nMDT_O = nMDT_O;              // number of MDT hits around the vertex in the outer layer
}

void MSVertex::setNRPC(const int nRPC, const int nRPC_inwards, const int nRPC_I, const int nRPC_E, const int nRPC_M, const int nRPC_O) { 
    m_nRPC = nRPC; 
    m_nRPC_inwards = nRPC_inwards;
    m_nRPC_I = nRPC_I;
    m_nRPC_E = nRPC_E;
    m_nRPC_M = nRPC_M;
    m_nRPC_O = nRPC_O;
}

void MSVertex::setNTGC(const int nTGC, const int nTGC_inwards, const int nTGC_I, const int nTGC_E, const int nTGC_M, const int nTGC_O) { 
    m_nTGC = nTGC; 
    m_nTGC_inwards = nTGC_inwards;
    m_nTGC_I = nTGC_I;
    m_nTGC_E = nTGC_E;
    m_nTGC_M = nTGC_M;
    m_nTGC_O = nTGC_O;
}


int MSVertex::getNMDT() const { return m_nMDT; }
int MSVertex::getNRPC() const { return m_nRPC; }
int MSVertex::getNTGC() const { return m_nTGC; }

std::vector<int> MSVertex::getNMDT_all() const { return std::vector<int> {m_nMDT, m_nMDT_inwards, m_nMDT_I, m_nMDT_E, m_nMDT_M, m_nMDT_O}; }
std::vector<int> MSVertex::getNRPC_all() const { return std::vector<int> {m_nRPC, m_nRPC_inwards, m_nRPC_I, m_nRPC_E, m_nRPC_M, m_nRPC_O}; }
std::vector<int> MSVertex::getNTGC_all() const { return std::vector<int> {m_nTGC, m_nTGC_inwards, m_nTGC_I, m_nTGC_E, m_nTGC_M, m_nTGC_O}; }


std::string str(const MSVertex& a) {
    std::stringstream ss;

    ss << "author = " << a.getAuthor() << "; x = " << a.getPosition().x() << "; y = " << a.getPosition().y()
       << "; z = " << a.getPosition().z() << "; phi = " << a.getPosition().phi() << "; eta = " << a.getPosition().eta()
       << "; chi2 prob. = " << a.getChi2Probability() << "; # tracks = " << a.getNTracks() << "; # MDT hits = " << a.getNMDT()
       << "; # RPC hits = " << a.getNRPC() << "; # TGC hits = " << a.getNTGC();

    return ss.str();
}

MsgStream& operator<<(MsgStream& m, const MSVertex& a) { return (m << str(a)); }

bool operator==(const MSVertex& a, const MSVertex& b) {
    //* distance used to compare floats *//
    const double DELTA = 1e-3;

    if (std::abs(a.getPosition().x() - b.getPosition().x()) > DELTA) return false;
    if (std::abs(a.getPosition().y() - b.getPosition().y()) > DELTA) return false;
    if (std::abs(a.getPosition().z() - b.getPosition().z()) > DELTA) return false;
    if (std::abs(a.getPosition().eta() - b.getPosition().eta()) > DELTA) return false;
    if (std::abs(a.getPosition().phi() - b.getPosition().phi()) > DELTA) return false;
    if (std::abs(a.getChi2Probability() - b.getChi2Probability()) > DELTA) return false;

    if (a.getAuthor() - b.getAuthor() != 0) return false;
    if (a.getNTracks() - b.getNTracks() != 0) return false;
    if (a.getNMDT() - b.getNMDT() != 0) return false;
    if (a.getNRPC() - b.getNRPC() != 0) return false;
    if (a.getNTGC() - b.getNTGC() != 0) return false;

    return true;
}
