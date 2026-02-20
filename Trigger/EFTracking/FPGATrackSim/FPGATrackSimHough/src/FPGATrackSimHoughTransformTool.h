// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimHOUGHTRANSFORMTOOL_H
#define FPGATrackSimHOUGHTRANSFORMTOOL_H

/**
 * @file FPGATrackSimHoughTransformTool.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date October 31st, 2020
 * @brief Implements road finding using a Hough transform.
 *
 * Declarations in this file:
 *      class FPGATrackSimHoughTransformTool : public AthAlgTool, virtual public IFPGATrackSimRoadFinderTool
 *
 * Using the Lorentz force equation, one can relate the phi of a track and the
 * coordinate of a single hit:
 *
 *      A * q / pT = sin(phi_track - phi_hit) / r
 *
 * where
 *      A   : 3 * 10^-4 GeV / (c*mm*e)
 *      q   : charge of the particle
 *      pT  : transverse momentum
 *      r   : cylindrical radius of the hit from the beamline
 *      phi : in radians
 *
 * Here, q/pT and phi_track are unknown. This equation forms a line in q/pT vs
 * phi_track space. Each hit will have its own line based on its phi and r.
 * However, note that hits belonging to the same track will have lines that
 * intersect at the track's q/pT and phi. In this manner, we can conduct pattern
 * -matching by looking for intersections of these pT-phi lines.
 *
 * To easily find intersections, we first pixelate (equivalently, we make a 2d
 * histogram from) the graph of all the hit's lines in q/pT vs phi_track space.
 * We then apply a convolution (i.e. a scanning window) to pick out points with
 * multiple lines going through them. These points become our roads.
 *
 * In principle the Hough transform can be used for an entire region (i.e. .2 phi x .2 eta) or larger.
 * However this can lead to an excessive number of hits/lines in the transform image, leading
 * to spurious intersections. Instead, we can use multiple transforms that each cover a slice
 * in z0, and simply combine all the roads found.
 *
 * References:
 *      Martensson Thesis: http://uu.diva-portal.org/smash/get/diva2:1341509/FULLTEXT01.pdf
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
#include "FPGATrackSimHough/IFPGATrackSimRoadFilterTool.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"

#include "TFile.h"

#include <string>
#include <vector>
#include <utility>
#include <unordered_set>


/* I adopt the following nomenclature within this class:
 *      image: The 'graph' in q/pT vs phi_track space, filled with a line calculated as above for each hit.
 *      point: A specific q/pT and phi_track bin in the above image; i.e. what is normally called a pixel
 *             but I don't want to confuse this with the detector type. A point's value is the number of
 *             lines that go through it.
 *
 * For the first iteration, x refers to phi_track, and y refers to q/pT, although
 * this should remain flexible. These are set via the variables m_par_x and m_par_y.
 *
 * NOTE: We store the image in graph sense and not computer-science sense. That is,
 * the row-index is y. The y-axis still points downwards, so that y=0 represents the
 * lowest bin.
 *      image[y=0][x=0]      : lowest q/pT and lowest phi_track bin
 *      image[y=size-1][x=0] : highest q/pT and lowest phi_track bin
 *
 * I use the following units for relevant variables:
 *      x,y,z,r : mm
 *      q       : e
 *      pT      : GeV / c
 */
class FPGATrackSimHoughTransformTool : public extends <AthAlgTool, IFPGATrackSimRoadFinderTool>
{
    public:
        /// Constructor
        using base_class::base_class;

        virtual StatusCode initialize() override;

        ///////////////////////////////////////////////////////////////////////
        // IFPGATrackSimRoadFinderTool

        virtual StatusCode getRoads(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, std::vector<FPGATrackSimRoad> & roads) override; 

        ///////////////////////////////////////////////////////////////////////
        // FPGATrackSimHoughTransformTool

        double getMinX() const { return m_parMin[m_par_x]; }
        double getMaxX() const { return m_parMax[m_par_x]; }
        double getMinY() const { return m_parMin[m_par_y]; }
        double getMaxY() const { return m_parMax[m_par_y]; }
        unsigned getThreshold() const { return m_threshold[m_threshold.size() / 2]; }
        virtual int getSubRegion() const override{return m_subRegion;}

        double yToX(double y, const std::shared_ptr<const FPGATrackSimHit> &hit) const;


        typedef vector2D<std::pair<int, std::unordered_set<std::shared_ptr<const FPGATrackSimHit>>>> Image;
            // An image is a 2d array of points, where each point has a value.
            // The value starts as the number of hit layers, but can change with effects
            // like a convolution. Also stored are all hits that contributed to each bin.
            // Size m_imageSize_y * m_imageSize_x. (NOTE y is row coordinate)
        Image const & getImage() const { return m_image; } // Returns the image generated from the last call of getRoads

        //used for FPGA emulation
        struct pos
        {
          int x;
          int y;
          int layer;
        };
 
        struct LUT
        {
          int input_begin;
          int input_end;
          int layer;
          std::vector<pos> output;
        };
        int m_bitlength = 16;

    private:

        ///////////////////////////////////////////////////////////////////////
        // Handles

        ServiceHandle<IFPGATrackSimEventSelectionSvc> m_EvtSel {this, "FPGATrackSimEventSelectionSvc", ""};
        ServiceHandle<IFPGATrackSimBankSvc> m_FPGATrackSimBankSvc {this, "FPGATrackSimBankSvc", "FPGATrackSimBankSvc"};
	ServiceHandle<IFPGATrackSimMappingSvc> m_FPGATrackSimMapping {this, "FPGATrackSimMappingSvc", "FPGATrackSimMappingSvc"};
        ///////////////////////////////////////////////////////////////////////
        // Properties


        // === Image ===
        FPGATrackSimTrackPars m_parMin; // These are the bounds of the image, i.e. the region of interest
        FPGATrackSimTrackPars m_parMax; // Only the two parameters chosen above are used for the image, however others may be used for slicing

        FPGATrackSimTrackPars::pars_index m_par_x = FPGATrackSimTrackPars::IPHI; // sets phi as the x variable
        FPGATrackSimTrackPars::pars_index m_par_y = FPGATrackSimTrackPars::IHIP; // sets q/pT as the y variable

        Gaudi::Property <int> m_subRegion { this, "subRegion", 0," -1 for entire region (no slicing)"};
	Gaudi::Property <float> m_tempMin_phi { this, "phi_min", 0, "min phi"};
	Gaudi::Property <float> m_tempMax_phi { this, "phi_max", 0, "max phi"};
	Gaudi::Property <float> m_tempMin_qOverPt { this, "qpT_min", 0, "min q/pt"};
	Gaudi::Property <float> m_tempMax_qOverPt { this, "qpT_max", 0, "max q/pt"};
	Gaudi::Property <float> m_tempMin_d0 { this, "d0_min", 0, "min q/pt"};
	Gaudi::Property <float> m_tempMax_d0 { this, "d0_max", 0, "max q/pt"};
	Gaudi::Property<std::vector<int> > m_threshold  { this, "threshold", {},"Minimum number of hit layers to fire a road"};
	Gaudi::Property <unsigned> m_imageSize_x { this, "nBins_x", 0, ""};
	Gaudi::Property <unsigned> m_imageSize_y { this, "nBins_y", 0, ""};
	Gaudi::Property<std::vector<int> > m_conv  { this, "convolution", {}, "Convolution filter, with size m_convSize_y * m_convSize_x"};
	Gaudi::Property<std::vector<unsigned> > m_combineLayers  { this, "combine_layers", {}, ""};
	Gaudi::Property<std::vector<unsigned> > m_binScale  { this, "scale", {}, "Vector containing the scales for each layers"};
	Gaudi::Property <unsigned> m_convSize_x { this, "convSize_x", 0, ""};
	Gaudi::Property <unsigned> m_convSize_y { this, "convSize_y", 0, ""};
	Gaudi::Property<std::vector<unsigned> > m_hitExtend_x  { this, "hitExtend_x", {}, "Hit lines will fill extra bins in x by this amount on each side, size == nLayers"};
	Gaudi::Property <bool> m_traceHits { this, "traceHits", true, "Trace each hit that goes in a bin. Disabling this will save memory/time since each bin doesn't have to store all its hits but the roads created won't have hits from convolution, etc."};
	Gaudi::Property <bool> m_localMaxWindowSize { this, "localMaxWindowSize", 0, "Only create roads that are a local maximum within this window size. Set this to 0 to turn off local max filtering"};
	Gaudi::Property <bool> m_fieldCorrection { this, "fieldCorrection", true, "Apply corrections to hough equation due to field nonuniformity"};
	Gaudi::Property <bool> m_useSectors { this, "useSectors", false, "Will reverse calculate the sector for track-fitting purposes"};
	Gaudi::Property <bool> m_idealGeoRoads { this, "IdealGeoRoads", true, "Set sectors to use ideal geometry fit constants"};
	Gaudi::Property <bool> m_doRegionalMapping { this, "RegionalMapping", false,  "Use the sub-region maps to define the sector"};
	Gaudi::Property <bool> m_doEtaPatternConsts { this, "doEtaPatternConsts", false, "Whether to use the eta pattern tool for constant generation"};
	Gaudi::Property <bool> m_useSpacePoints { this, "useSpacePoints", false, "Whether we are using spacepoints."};
	Gaudi::Property <std::string> m_houghType { this, "houghType", "Original", "Switch Hough strategy. Original: close to mathematical HT, LowResource: emulate pre calculated LUTs base FPGA performance, Flexible: emulate culculation on FPGA performance"};
	Gaudi::Property <bool> m_roadMerge { this, "roadMerge", false};
	Gaudi::Property <std::string> m_requirements { this, "requirements", "", "path of the requirements file"};

        //Parameters generally used by Hough Transform 2D Flexible
        Gaudi::Property <int> m_r_max { this, "r_max", 2047," - maximum -"};   
        Gaudi::Property <int> m_phi_coord_max { this, "phi_coord_max", 65535," - maximum - phi"};
        Gaudi::Property <double> m_phi_range { this, "phi_range", 6.399609375,"range of phi in rad, customized ofr the algo"}; 
        Gaudi::Property <double> m_r_max_mm { this, "r_max_mm", 1137.5,"r value maximum used in the mm-to-bits conversion"};        
        Gaudi::Property <int> m_bitwise_qApt_conv { this, "bitwise_qApt_conv", 16384, "exponential of 2 to multiply with qA/pt in case the latter in < 1"};
        Gaudi::Property <int> m_bitwise_phi0_conv { this, "bitwise_phi0_conv", 1, "exponential of 2 to multiply with phi0 in case the latter in < 1"};    
        Gaudi::Property <int> m_phi0_sectors { this, "phi0_sectors", 1,"firmware method to draw monotonic lines. separation alongside phi0"};
        Gaudi::Property <int> m_qApt_sectors { this, "qApt_sectors", 7,"firmware method to draw monotonic lines. separation alongside qA/pt"};
        Gaudi::Property <int> m_pipes_qApt { this, "pipes_qApt", 8,"!!!!!!MAX 32 !!!!!!clock domains separation alongside qA/pt bins"};
        Gaudi::Property <int> m_pipes_phi0 { this, "pipes_phi0", 1," !!!!!!MAX 32 !!!!!!clock domains separation alongside phi0  bins"};    

        std::vector<std::vector<unsigned>> m_combineLayer2D; // 2d array of combined layers i.e. [[1,2,3],[0,4,5],[6,7]] will combine (L1, L2, L3), (L0, L4, L5), (L6, L7)

        ///////////////////////////////////////////////////////////////////////
        // Convenience

        unsigned m_nLayers = 0U; // alias to m_FPGATrackSimMapping->PlaneMap1stStage()->getNLogiLayers();
        unsigned m_nCombineLayers = 0U; // number of layers after combined

        double m_step_x = 0; // step size of the bin boundaries in x
        double m_step_y = 0; // step size of the bin boundaries in y
        std::vector<double> m_bins_x; // size == m_imageSize_x + 1.
        std::vector<double> m_bins_y; // size == m_imageSize_y + 1
            // Bin boundaries, where m_bins_x[i] is the lower bound of bin i.
            // These are calculated from m_parMin/Max.

        ///////////////////////////////////////////////////////////////////////
        // Conistants for HT2D Flex
	int m_HT_sel = 0;
	long int m_one_r_const_twoexp = 0;
	double m_phi0_min_bit = 0;
	double m_qApt_min_bit = 0;
	long int m_DBinQApt_bit_int = 0;
	long int m_DBinPhi0_bit_int = 0;
	long int m_qAptBins_bit = 0;
	long int m_phi0Bins_bit = 0;
	long int m_phi0_bins_first_sector = 0;
	long int m_qApt_bins_first_sector = 0;
	long int m_tot_bitwise_conv = 0;
	long int m_one_r_const_twoexp_post_conv = 0;
	long int m_qApt_min_post_conv = 0;

        ///////////////////////////////////////////////////////////////////////
        // Event Storage

        Image m_image;
        std::vector<FPGATrackSimRoad> m_roads;


        ///////////////////////////////////////////////////////////////////////
        // Core

        // std::vector<FPGATrackSimHit const *> filterHits(std::vector<FPGATrackSimHit const *> const & hits) const;
        Image createLayerImage(std::vector<unsigned> const & combine_layers, const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned const scale) const;
        Image createImage(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits) const;
        std::vector<std::vector<int>> lineGenLay(const std::shared_ptr<const FPGATrackSimHit> &hit) const;
        Image convolute(Image const & image) const;

        ///////////////////////////////////////////////////////////////////////
        // Helpers

        std::pair<unsigned, unsigned> yToXBins(size_t yBin_min, size_t yBin_max, const std::shared_ptr<const FPGATrackSimHit> &hit) const;
        unsigned getExtension(unsigned y, unsigned layer) const;
        bool passThreshold(Image const & image, unsigned x, unsigned y) const;
        void addRoad(const std::vector<std::vector<std::shared_ptr<const FPGATrackSimHit>>> & hits, layer_bitmask_t hitLayers, unsigned x, unsigned y);
        void addRoad(const std::unordered_set<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned x, unsigned y);
        void addRoad(const std::vector<std::shared_ptr<const FPGATrackSimHit>> & hits, unsigned x, unsigned y);
        int conv(unsigned y, unsigned x) { return m_conv[y * m_convSize_x + x]; } // NOTE: y index is first
        void drawImage(Image const & image, std::string const & name);
	///////////////////////////////////////////////////////////////////////
        // FPGA emulation functions
        std::vector<std::vector<std::vector<LUT>>> m_LUT;
        std::vector<TH1D*> m_h_rfix;
	void makeLUT(std::vector<std::vector<std::vector<LUT>>> &v_LUT,std::vector<TH1D*> &v_h, const std::string& tag="");
        ///////////////////////////////////////////////////////////////////////
};



#endif // FPGATrackSimHOUGHTRANSFORMTOOL_H
