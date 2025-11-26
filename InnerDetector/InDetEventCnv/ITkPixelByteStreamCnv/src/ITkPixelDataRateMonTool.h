/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Paul Chabrillat, starting from Ondra Kovanda and Noemi Calace work
* Date: 12/11/2024
* Description: Athena tool wrapper around the ITkPix encoder to study data rate as a function of chip's position in detector
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELDATARATEMONTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELDATARATEMONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "InDetIdentifier/PixelID.h"
#include "Identifier/Identifier.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ITkPixLayout.h"

#include "ReadoutGeometryBase/DetectorDesign.h"
#include "ITkPixelCabling/ITkPixelCablingData.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"

class PixelID;

class ITkPixelDataRateMonTool: public AthAlgTool {

    public:

        ITkPixelDataRateMonTool(const std::string& type,const std::string& name,const IInterface* parent);
        virtual ~ITkPixelDataRateMonTool()=default;

        virtual StatusCode initialize() override;

        void fill(const uint32_t offlineID,
                  const std::vector<unsigned int>& encodedstream,
                  const ITkPixLayout<uint16_t>& hitMap) const;

    private:
        // Function to book histograms
        StatusCode bookHistograms(const std::vector<std::vector<float >>& barrel_z,
                                  const std::vector<std::vector<float >>& endcap_z);

        void fillExpertPlots(const uint32_t offlineID,
                             const ITkPixLayout<uint16_t>& hitMap) const;

        // Path to histograms
        Gaudi::Property< std::string > m_path { this, "MonitoringPath", "/DataRateMon/", "Name of directory for plots" };

        // boolean histogram to save expert plots
        Gaudi::Property< bool > m_doExpertPlots { this, "DoExpertPlots", false, "Enable expert plots" };

        // Histogram service
        ServiceHandle<ITHistSvc> m_thistSvc{this, "HistSvc", "THistSvc", "The histogram service"};

        // Readout service manager
        ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout {this, "PixelReadoutManager", "ITkPixelReadoutManager", "Pixel readout manager" };
        
        // Detector manager with same method as in hitSortigTool
        const InDetDD::PixelDetectorManager* m_detManager = nullptr;

        // Pixel id helper tool
        const PixelID* m_pixIdHelper = nullptr;

        // Parameters of the analysis

        // Number of bits contained in a pack from the encoder
        const static int s_bitsPerPack = 32;

        // Readout frequency for a chip
        const static int s_chipReadoutFrequency = 1000000;

        // Number of columns in one chip
        const static int s_col_chip = 400;

        // Number of rows in one chip
        const static int s_row_chip = 384;

        // --------------For all detector--------------

        // Number of layers
        const static int N_LAYERS=9;

        // Regions of the detector
        enum Region {
            INVALID_REGION=-1, REGION_BARREL, REGION_ENDCAP, N_REGIONS
        };

        // Labels of regions
        std::map<int, std::string > m_regionLabels {
            {INVALID_REGION, "invalid"}, {REGION_BARREL, "barrel"}, {REGION_ENDCAP, "endcap"}
        };

        // Side of the detector
        enum Side {
            INVALID_SIDE=-1, SIDE_POSITIVE, SIDE_NEGATIVE, N_SIDES
        };

        // Labels of side if the sides are in different histograms
        std::map<int, std::string > m_sideLabels {
            {INVALID_SIDE, "invalid"}, {SIDE_POSITIVE, "pos"}, {SIDE_NEGATIVE, "neg"}
        };


        // Histogram of stream length distribution in bits for the whole detector and all the chips
        mutable TH1* m_encoded_streamLength ATLAS_THREAD_SAFE = nullptr;

        // --------------Each region and no z dependance--------------

        // Chip hit map
        mutable TH2* m_h2_chip_hitmap[N_LAYERS][N_REGIONS] ATLAS_THREAD_SAFE = {};

        // Chip ToT map
        mutable TH2* m_h2_chip_totmap[N_LAYERS][N_REGIONS] ATLAS_THREAD_SAFE = {};

        // --------------Each region and separated positive and negative z--------------

        // T profile of the stream length for different layers and regions
        mutable TProfile* m_p_streamLength[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};

        // Two dimensional histogram of the stream length for different layers and regions
        mutable TH2* m_h2_streamLength[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};

        // T profile of the data rate from chips for different layers and regions
        mutable TProfile* m_p_dataRate[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};

        // Two dimensional histogem of the data rate from chips for different layers and regions
        mutable TH2* m_h2_dataRate[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};

        // T profile of the hits from chips for different layers and regions
        mutable TProfile* m_p_hits[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};

        // Two dimensional histogram of the hits from chips for different layers and regions
        mutable TH2* m_h2_hits[N_LAYERS][N_REGIONS][N_SIDES] ATLAS_THREAD_SAFE = {};


};


#endif // ITKPIXELBYTESTREAMCNV_ITKPIXELDATARATEMONTOOL_H
