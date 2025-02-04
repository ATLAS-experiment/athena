/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MSVERTEXUTILS_MSVERTEX_H
#define MSVERTEXUTILS_MSVERTEX_H

#include <vector>

#include "GaudiKernel/MsgStream.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "xAODTracking/TrackParticle.h"

class MSVertex {
public:
    MSVertex() = default;
    MSVertex(const MSVertex&) = default;
    MSVertex(int, const Amg::Vector3D&, double, double, int, int, int);
    MSVertex(int, const Amg::Vector3D&, const std::vector<const xAOD::TrackParticle*>&, double, double, int, int, int);
    MSVertex& operator=(const MSVertex& msvx) = default;

    virtual ~MSVertex();


    void setPosition(const Amg::Vector3D&);

    const Amg::Vector3D& getPosition() const;

    const std::vector<const xAOD::TrackParticle*>* getTracks() const;

    void setAuthor(const int);

    int getAuthor() const;

    double getChi2Probability() const;
    double getChi2() const;

    int getNTracks() const;

    void setNMDT(const int, const int, const int, const int, const int, const int);
    void setNRPC(const int, const int, const int, const int, const int, const int);
    void setNTGC(const int, const int, const int, const int, const int, const int);

    int getNMDT() const;
    int getNRPC() const;
    int getNTGC() const;
    const std::vector<int> getNMDT_all() const;
    const std::vector<int> getNRPC_all() const;
    const std::vector<int> getNTGC_all() const;

private:
    unsigned int m_author{0};

    Amg::Vector3D m_position{Amg::Vector3D::Zero()};

    std::vector<const xAOD::TrackParticle*> m_tracks{};

    double m_chi2prob{-1.f};
    double m_chi2{-1.f};

    int m_nMDT{0}, m_nMDT_inwards{0}, m_nMDT_I{0}, m_nMDT_E{0}, m_nMDT_M{0}, m_nMDT_O{0};
    int m_nRPC{0}, m_nRPC_inwards{0}, m_nRPC_I{0}, m_nRPC_E{0}, m_nRPC_M{0}, m_nRPC_O{0};
    int m_nTGC{0}, m_nTGC_inwards{0}, m_nTGC_I{0}, m_nTGC_E{0}, m_nTGC_M{0}, m_nTGC_O{0};
};

std::string str(const MSVertex& a);

MsgStream& operator<<(MsgStream& m, const MSVertex& a);

bool operator==(const MSVertex& a, const MSVertex& b);

inline bool operator!=(const MSVertex& a, const MSVertex& b) { return !(a == b); }

#endif
