// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATRACKSIMOBJECTS_FPGATRACKSIMGNNEDGE_H
#define TRIGFPGATRACKSIMOBJECTS_FPGATRACKSIMGNNEDGE_H

/**
 * @file FPGATrackSimGNNEdge.h
 * @author Jared Burleson - jared.dynes.burleson@cern.ch
 * @date November 27th, 2024
 * @brief FPGATrackSim-specific class to represent an edge as a connection between two hits in the detector used for GNN pattern recognition. 
 */

class FPGATrackSimGNNEdge
{
public:

    ///////////////////////////////////////////////////////////////////////
    // Constructors

    FPGATrackSimGNNEdge() = default;
    virtual ~FPGATrackSimGNNEdge() = default;

    ///////////////////////////////////////////////////////////////////////
    // Getters/Setters

    // --- Edge Features ---
    void setEdgeIndex1(int v) { m_edge_index_1 = v; }
    void setEdgeIndex2(int v) { m_edge_index_2 = v; }
    void setEdgeDR(float v) { m_edge_dr = v; }
    void setEdgeDPhi(float v) { m_edge_dphi = v; }
    void setEdgeDZ(float v) { m_edge_dz = v; }
    void setEdgeDEta(float v) { m_edge_deta = v; }
    void setEdgePhiSlope(float v) { m_edge_phislope = v; }
    void setEdgeRPhiSlope(float v) { m_edge_rphislope = v; }
    void setEdgeScore(float v) { m_edge_score = v; }

    int getEdgeIndex1() const { return m_edge_index_1; }
    int getEdgeIndex2() const { return m_edge_index_2; }
    float getEdgeDR() const { return m_edge_dr; }
    float getEdgeDPhi() const { return m_edge_dphi; }
    float getEdgeDZ() const { return m_edge_dz; }
    float getEdgeDEta() const { return m_edge_deta; }
    float getEdgePhiSlope() const { return m_edge_phislope; }
    float getEdgeRPhiSlope() const { return m_edge_rphislope; }
    float getEdgeScore() const { return m_edge_score; }

protected:
    int m_edge_index_1 = -1;
    int m_edge_index_2 = -1;
    float m_edge_dr = 0.0;
    float m_edge_dphi = 0.0;
    float m_edge_dz = 0.0;
    float m_edge_deta = 0.0;
    float m_edge_phislope = 0.0;
    float m_edge_rphislope = 0.0;
    float m_edge_score = -1.0;

};

#endif // FPGATRACKSIMHIT_H