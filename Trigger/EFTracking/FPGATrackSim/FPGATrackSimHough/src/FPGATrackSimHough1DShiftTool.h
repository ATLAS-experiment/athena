// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMHOUGH1DSHIFTTOOL_H
#define FPGATRACKSIMHOUGH1DSHIFTTOOL_H

/**
 * @file FPGATrackSimHough1DShiftTool.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date November 6th, 2020
 * @brief Implements road finding using Elliot's simplified Hough transform.
 *
 * Declarations in this file:
 *      class FPGATrackSimHough1DShiftTool : public AthAlgTool, virtual public FPGATrackSimRoadFinderToolI
 *
 * Using the Lorentz force equation, one can relate the phi of a track and the
 * coordinate of a single hit:
 *
 *      q / pT = sin(phi_track - phi_hit) / (A * r)
 *
 *      phi_track = phi_hit + asin(A * r * q / pT)
 *
 * where
 *      A   : 10^-4 GeV / (c*mm*e)
 *      q   : charge of the particle
 *      pT  : transverse momentum
 *      r   : cylindrical radius of the hit from the beamline
 *      phi : in radians
 *
 * Here, q/pT and phi_track are unknown. This equation forms a line in q/pT vs
 * phi_track space. However, note that hits belonging to the same track will have
 * lines that intersect at the track's q/pT and phi. In this manner, we can conduct
 * road-finding by looking for intersections of these pT-phi lines.
 *
 * Note that in the barrel, r is approximately constant in each layer. Then scanning
 * q/pT is the same as shifting phi in each layer. If you find a q/pT such that, after
 * the shifts, each layer has a hit at the same phi, that indicates a track with the given
 * q/pT and phi.
 */

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimVectors.h"
#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimFunctions.h"
#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimHough/IFPGATrackSimRoadFinderTool.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"

#include "TFile.h"

#include <string>
#include <vector>
#include <map>
#include <boost/dynamic_bitset_fwd.hpp>



/*
 * The hits of an event are stored in a bit vector for each layer, with the number
 * of bits being the number of phi bins. A hit sets a bit (or a range of bits to
 * account for resolution effects) in the respective layer's bit vector.
 *
 * Shifting phi is now simply a bit shift of the bit vectors. Roads are created
 * whenever a threshold number of layers have the same phi bin set after the shift.
 *
 * Shifts are precalculated in the initialize routine. Instead of iterating over
 * q/pt, we can iterate over n-bin shift increments in the outer layer. Each shift
 * in the outer layer uniquely determines a q/pt and the corresponding shifts in
 * the other layers. This is better than a q/pt scan by taking into account the
 * quantization in the binning. Actually, we can optimize even better by iterating
 * over n-bin shifts in the difference between the inner and outer layer, to help
 * account also for the hit extension.
 *
 * I use the following units for relevant variables:
 *      x,y,z,r : mm
 *      q       : e
 *      pT      : GeV / c
 */
class FPGATrackSimHough1DShiftTool : public extends <AthAlgTool, IFPGATrackSimRoadFinderTool>
{
    public:
        /// Constructor
        using base_class::base_class;

        virtual StatusCode initialize() override;
        virtual StatusCode finalize() override;

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimRoadFinderToolI

        virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<std::shared_ptr<const FPGATrackSimRoad>> & roads) override;
        virtual int getSubRegion() const override{return m_subRegion;}
    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles
        ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel {this, "FPGATrackSimEventSelectionSvc", "FPGATrackSimEventSelectionSvc"};
        ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc {this, "FPGATrackSimBankSvc", "FPGATrackSimBankSvc"};
	ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};

        ///////////////////////////////////////////////////////////////////////
        // Properties

        Gaudi::Property<bool> m_traceHits { this, "traceHits", true, "Trace each hit that goes in a bin. Disabling this will save memory/time since each bin doesn't have to store all its hits but the roads created won't have hits from convolution, etc."};

        Gaudi::Property<bool> m_doEtaPatternConsts { this, "doEtaPatternConsts", false, "Whether to use the eta pattern tool for constant generation"};
        Gaudi::Property<bool> m_useSpacePoints { this, "useSpacePoints", false, "Whether we are using spacepoints." };
        Gaudi::Property<bool> m_useSectors { this, "useSectors", false, "Will reverse calculate the sector for track-fitting purposes" };
        Gaudi::Property<bool> m_idealGeoRoads { this, "IdealGeoRoads", false, "Set sectors to use ideal geometry fit constants" };
        Gaudi::Property<bool> m_doRegionalMapping { this, "RegionalMapping", false, "Use the sub-region maps to define the sector" };
        Gaudi::Property<bool> m_drawHitMasks { this, "drawHitMasks", false, "Draws hit masks if true"};

        Gaudi::Property<int> m_subRegion { this, "subRegion", 0, "Sub region of this transform, or -1 for full region" };
        Gaudi::Property<float> m_phiMin { this, "phiMin", 0, "Minimum phi of transform" };
        Gaudi::Property<float> m_phiMax { this, "phiMax", 0, "Maximum phi of transform" };
        Gaudi::Property<float> m_qptMin { this, "qptMin", 0, "Minimum q/pT of transform" };
        Gaudi::Property<float> m_qptMax { this, "qptMax", 0, "Maximum q/pT of transform" };

        Gaudi::Property<unsigned> m_phiBins { this, "nBins", 0, "Number of phi bins used by transform." };
        Gaudi::Property<unsigned> m_threshold { this, "threshold", 0, "Minimum number of layers hit to accept as a road (inclusive)" };
        Gaudi::Property<unsigned> m_iterStep { this, "iterStep", 0, "Instead of iterating over steps in pT, we iterate over iterStep-bin shifts in iterLayer" };
        Gaudi::Property<unsigned> m_iterLayer { this, "iterLayer", 0, "Instead of iterating over steps in pT, we iterate over iterStep-bin shifts in iterLayer" };

        Gaudi::Property<bool> m_useDiff { this, "useDiff", false, "Use the diff of inner and outer layer" };
        Gaudi::Property<bool> m_variableExtend { this, "variableExtend", false, "Do variable extension based off of hit delta R" };
        Gaudi::Property<bool> m_phiRangeCut {this, "phiRangeCut", false, "Require tracks to be in phi range to avoid counting minbias roads for larger region" };

        Gaudi::Property<float> m_d0spread {this, "d0spread", -1.0, "Make patterns with a d0spread as given, negative value turns it off" };
        Gaudi::Property<std::vector<float>> m_hitExtendProperty {this, "hitExtend", {}, "Number of adjacent bins that a hit triggers" };
        Gaudi::Property<std::string> m_bitShift_path { this, "bitShifts", "", "Instead of calculating bit shifts, input a list of shifts via a text file" };
        Gaudi::Property<bool> m_applyDropable { this, "applyDropable", false, "Enable logic that prevents redundant patterns with dropped hits" };
        Gaudi::Property<int> m_neighborWindow { this, "neighborWindow", 0, "Supress if neighbors have higher number of hit layers" };
        Gaudi::Property<unsigned> m_historyWindow {this, "historyWindow", 0, "Suppress if previous N bit shifts have neighbors with higher nubmer of hit layers" };
        Gaudi::Property<bool> m_fieldCorrection {this, "fieldCorrection", true, "Apply corrections to hough equation due to field nonuniformity" };
        Gaudi::Property<float> m_enhanceHighPt {this, "enhanceHighPt", -1.0, "if positive, double number of patterns for region with qpT below value" };


        std::vector<float> m_hitExtend; // need second copy because property is "const" and can't be changed to default
        std::vector<float> m_r;  // will be filled from m_radii_file (now loaded through region map class).

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        unsigned m_nLayers = 0; // alias to m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();

        float m_phiStep = 0; // width of one phi bin
        std::vector<double> m_bins; // size == m_phiBins + 1.
            // Bin boundaries, where m_bins[i] is the lower bound of bin i.
            // These are calculated from m_phiMin/Max.

        FPGATrackSimTrackPars m_regionMin; // alias to m_EvtSel->getRegions()->getMin(m_EvtSel->getReginID())
        FPGATrackSimTrackPars m_regionMax; // alias to m_EvtSel->getRegions()->getMax(m_EvtSel->getReginID())

        std::vector<std::vector<int>> m_shifts; // size (nShifts, nLayers)
        std::vector<boost::dynamic_bitset<>> m_dropable; // size (nShifts, nLayers)
            // for shift patterns, we can cut duplication by only allowing some layers to be missed when doing 7/8

        std::vector<float> m_qpt; // size (nShifts)
        std::vector<std::vector<float>> m_phivals; // size (nShifts, nLayers)
        std::vector<std::vector<int>> m_d0shifts; // size (nShifts, nLayers), optional subshifts by d0
        std::vector<unsigned> m_currentcounts; // just here so we don't reallocate every call
        std::deque<std::vector<unsigned>> m_vetolist; // last N events, bit string of hit layers per bin

  
        ///////////////////////////////////////////////////////////////////////
        // Event Storage

        std::vector<FPGATrackSimRoad> m_roads;

        ///////////////////////////////////////////////////////////////////////
        // Metadata and Monitoring

        unsigned m_event = 0;
        std::string m_name; // Gets the instance name from the full gaudi name
        TFile m_monitorFile;

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        void calculateShifts();
        float getPtFromShiftDiff(int shift) const;
        void readShifts(std::string const & filepath);
        std::vector<boost::dynamic_bitset<>> makeHitMasks(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits);

        FPGATrackSimRoad makeRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>>& hits, int bin_track, size_t iShift);
        bool passThreshold(std::vector<boost::dynamic_bitset<>>& binHits, int bin ) const;
        void printHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks) const;

        void drawHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks, std::string const & name);
        void drawHitMasks(std::vector<boost::dynamic_bitset<>> const & hitMasks, std::string const & name, std::vector<int> const & shifts);
        void printShifts() const;

        std::pair<int, int> getBins(const std::shared_ptr<const FPGATrackSimHit>& hit) const;
        float phitrkDiff(float r1, float phi1,  float r2,  float phi2) const;
        std::pair<float, bool> phitrk(int bin, std::vector<int> const &  shifts ) const; // returns phi of track and a bool if the
                                                                                       // value is invalid because at the edge
        float qPt(float r, float deltaPhi) const;
        float deltaPhi(float r, float qPt) const;

        void calculated0Shifts();
        std::vector<int> applyVariation(const std::vector<int>& base, const std::vector<int>& var, int sign) const;
        void calculateDropable();
        std::vector<int> shiftWithDrop(std::vector<int>& shift,unsigned droplayer) const;


};


#endif // FPGATRACKSIMHOUGH1DSHIFTTOOL_H
