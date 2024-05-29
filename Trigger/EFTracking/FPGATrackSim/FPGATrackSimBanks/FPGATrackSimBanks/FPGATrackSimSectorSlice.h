// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimSectorSlice_h
#define FPGATrackSimSectorSlice_h

/**
 * @file FPGATrackSimSectorSlice.h
 * @author major update Riley Xu - rixu@cern.ch
 * @date March 20, 2020
 * @brief Stores the range of eta/phi/etc. of each sector.
 *
 * Sector slices store the range of eta/phi/etc. of each sector. This is done for
 * all 5 track parameters by binning each parameter, with one bitvector per bin.
 * The size of the bitvectors is the number of sectors, so that bv[sector] represents
 * whether that sector goes through that parameter value.
 *
 * Sector slices are created by ConstGenAlgo and stored in a ROOT file.
 * Sector slices are used by track inversion to select the sectors that it uses to do
 * the inverse fit.
 */

#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimObjects/FPGATrackSimTypes.h" //typedef for sector_t
#include <vector>
#include <utility> //for std::pair
class TClonesArray;




class FPGATrackSimSectorSlice
{
    public:

        // Create blank bitvectors for writing new slice file
        FPGATrackSimSectorSlice(size_t nSectors,
                FPGATrackSimTrackParsI const & nBins, FPGATrackSimTrackPars const & min, FPGATrackSimTrackPars const & max);
        // Read from file
        FPGATrackSimSectorSlice(const std::string & filepath);

        FPGATrackSimSectorSlice(const FPGATrackSimSectorSlice&) = delete;
        FPGATrackSimSectorSlice& operator=(const FPGATrackSimSectorSlice&) = delete;


        // Slice writing functions
        void addSectorToSlice(sector_t sector, FPGATrackSimTrackParsI const & bins);
        void saveSlices(const std::string & filepath);


        // Finds the parameter boundaries for where bitmasks are non-empty.
        // Returns (min, max).
        std::pair<FPGATrackSimTrackPars, FPGATrackSimTrackPars> getBoundaries() const;

        // Returns a list of sectors that contain pars.
        std::vector<sector_t> searchSectors(FPGATrackSimTrackPars const & pars) const;

    private:

        // TBits bitmask arrays. The use of TClonesArray is probably overkill but whatever, kept from FTK.
        // I wish ROOT would update these classes to use templating X(
        TClonesArray *m_bits_phi    = nullptr;
        TClonesArray *m_bits_c      = nullptr;
        TClonesArray *m_bits_d0     = nullptr;
        TClonesArray *m_bits_z0     = nullptr;
        TClonesArray *m_bits_eta = nullptr;

        // Number of bins for each parameter, size of arrays above
        FPGATrackSimTrackParsI m_nBins;

        // Min and max parameters found out in the slice file
        FPGATrackSimTrackPars m_min;
        FPGATrackSimTrackPars m_max;

        // Step sizes, (max - min) / nbins
        FPGATrackSimTrackPars m_step;

        // Helper functions
        void calcDependentVals();
        void getBoundary(const TClonesArray *bitmasks, double x_min, double x_max,
                double &autoMin, double &autoMax, bool wraps, const char *debug) const;
        bool checkTrackPars(FPGATrackSimTrackPars const & pars) const;
};

#endif // FPGATrackSimSectorSlice_h

