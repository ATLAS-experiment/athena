// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimROAD_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimROAD_H

/**
 * @file FPGATrackSimRoad.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date Janurary 13th, 2020
 * @brief Defines a class for roads.
 *
 * Roads are triggered bins in Hough. They collect
 * the hits that fired the bin with some auxillary information.
 */


#include <vector>
#include <unordered_set>
#include <ostream>
#include <memory>
#include <bit>

#include "TObject.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimMultiTruth.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"

class FPGATrackSimRoad
{
public:

    ///////////////////////////////////////////////////////////////////////
    // Constructors

    FPGATrackSimRoad() = default;

    FPGATrackSimRoad(unsigned nLayers) : m_hits_trans(nLayers) { }

    FPGATrackSimRoad(int roadID, pid_t pid, sector_t sector, layer_bitmask_t hit_layers,
        layer_bitmask_t wildcard_layers, std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> && hits)
        : m_roadID(roadID), m_pid(pid), m_sector(sector), m_hit_layers(hit_layers), m_wildcard_layers(wildcard_layers)/* , m_hits_trans(hits) */
    {
        setHits(std::move(hits));
    }
    //Move operators
    FPGATrackSimRoad(FPGATrackSimRoad&&) noexcept = default;
    FPGATrackSimRoad& operator=(FPGATrackSimRoad&&) noexcept = default;
    //Copy operators
    FPGATrackSimRoad(const FPGATrackSimRoad&) = default;
    FPGATrackSimRoad& operator=(const FPGATrackSimRoad&) = default;
    virtual ~FPGATrackSimRoad() = default;

    ///////////////////////////////////////////////////////////////////////
    // Setters

    void setRoadID(int roadID) { m_roadID = roadID; }
    void setPID(pid_t pid) { m_pid = pid; }
    void setSector(sector_t sector) { m_sector = sector; }
    void setSectorBin(int sectorbin) { m_sectorbin = sectorbin; }

    void setHitLayers(layer_bitmask_t hit_layers) { m_hit_layers = hit_layers; }
    void setWCLayers(layer_bitmask_t wc_layers) { m_wildcard_layers = wc_layers; }

    void setNLayers(unsigned layers) { m_hits_trans.resize(layers); m_hits.resize(layers); }
    void setHits(std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> &&hits);
    void setHits(unsigned layer, std::vector<std::shared_ptr<const FPGATrackSimHit>> && hits);

    void repopulateTransHits();

    void setEtaPatternID(int patternID) { m_etaPatternID = patternID; }

    void setSubRegion(int v) { m_subRegion = v; }
    void setXBin(unsigned v) { m_xBin = v; }
    void setYBin(unsigned v) { m_yBin = v; }
    void setX(float v) { m_x = v; }
    void setY(float v) { m_y = v; }

    void setFitParams(const FPGATrackSimTrackPars& v) { m_fitTrackPars = v; }
    void setFitChi2(double v) { m_fitChi2 = v; }
    void setFitChi2_2d(double chi2_phi, double chi2_eta) { m_fitChi2_phi = chi2_phi; m_fitChi2_eta = chi2_eta; }

    ///////////////////////////////////////////////////////////////////////
    // Getters

    int getSubRegion() const { return m_subRegion; }
    unsigned getXBin() const { return m_xBin; }
    unsigned getYBin() const { return m_yBin; }
    float getX() const { return m_x; }
    float getY() const { return m_y; }

    int getRoadID() const { return m_roadID; }
    pid_t getPID() const { return m_pid; }
    sector_t getSector() const { return m_sector; }
    int getSectorBin() const { return m_sectorbin; }

    layer_bitmask_t getHitLayers() const { return m_hit_layers; }
    layer_bitmask_t getWCLayers() const { return m_wildcard_layers; }

    int getEtaPatternID() const { return m_etaPatternID; }

    const std::vector<std::shared_ptr<const FPGATrackSimHit>> &getHits(size_t layer) const { return m_hits_trans.at(layer); }
    const std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> &getAllHits() const { return m_hits_trans; }
    std::unordered_set<std::shared_ptr<const FPGATrackSimHit>> getHits_flat() const;

    const FPGATrackSimTrackPars& getFitParams() const { return m_fitTrackPars; }
    double getFitChi2() const { return m_fitChi2; }
    double getFitChi2Phi() const { return m_fitChi2_phi; }
    double getFitChi2Eta() const { return m_fitChi2_eta; }

    ///////////////////////////////////////////////////////////////////////
    // Utility

    size_t getNLayers() const { return m_hits_trans.size(); }
    size_t getNHitLayers() const { return std::popcount(m_hit_layers); }
    size_t getNWCLayers() const { return std::popcount(m_wildcard_layers); }

    size_t getNHits() const;
    std::vector<size_t> getNHits_layer() const;
    size_t getNHitCombos() const;

    // Bin ID, if using FPGATrackSim binning.
    void setBinIdx(std::vector<unsigned> x) { m_binIdx = std::move(x); }
    const std::vector<unsigned>& getBinIdx() const { return m_binIdx; }

    // Weight of each barcode is the fraction of layers with corresponding hits
    // where pixels are weighted twice as much
    FPGATrackSimMultiTruth getTruth() const;

private:

    int m_roadID = 0;       // Currently just a count set by RoadFinder.
    pid_t m_pid = 0;        // the pattern ID that fired this road
    sector_t m_sector = 0;  // Sector this road belongs to / should be fitted with
    int m_sectorbin = -1;   // The bin/ID of the sector that this road belongs to.

    layer_bitmask_t m_hit_layers = 0;       // Layers that had hits match the pattern, not including wildcards
    layer_bitmask_t m_wildcard_layers = 0;  // Layers that matched because of a wildcard in the pattern

    // Eta pattern associated with the road.
    int m_etaPatternID = -1;

    int m_subRegion = 0; // slice that the road came from
    unsigned m_xBin = 0;
    unsigned m_yBin = 0;
    float m_x = 0; // x value of Hough bin
    float m_y = 0; // y value of Hough bin

    FPGATrackSimTrackPars m_fitTrackPars;
    double m_fitChi2 = 0;
    double m_fitChi2_phi = 0;
    double m_fitChi2_eta = 0;

    std::vector<FPGATrackSimHitCollection> m_hits; // [layer, hit#] (used for ROOT storing)
    std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> m_hits_trans; //! (transient) [layer, hit#]
    // A list of hits in the road for each layer.
    // These pointers are not owned by the road.

    // bin ID. Just store this as a vector<unsigned>.
    std::vector<unsigned> m_binIdx;

    ///////////////////////////////////////////////////////////////////////
    // Misc
    friend std::ostream& operator<<(std::ostream& os, const FPGATrackSimRoad& road);
    ClassDefNV(FPGATrackSimRoad, 9);
};

#endif // FPGATrackSimROAD_H

