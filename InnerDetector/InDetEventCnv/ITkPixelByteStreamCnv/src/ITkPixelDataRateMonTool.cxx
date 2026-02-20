/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
* Author: Paul Chabrillat, starting from Ondra Kovanda and Noemi Calace work
* Date: 12/11/2024
* Description: Athena tool wrapper around the ITkPix encoder to study data rate as a function of chip's position in detector
*/

#include "ITkPixelDataRateMonTool.h"
#include <stdexcept>

ITkPixelDataRateMonTool::ITkPixelDataRateMonTool(const std::string& type, const std::string& name, const IInterface* parent)
  : AthAlgTool(type, name, parent)
{
}


StatusCode ITkPixelDataRateMonTool::initialize() {

    // ----------- Check services presence -----------

    // Checks that the histograming service is up and running
    ATH_CHECK(m_thistSvc.retrieve());

    // Checks that the pixel readout service is up and running
    ATH_CHECK(m_pixelReadout.retrieve());

    // Checks that the pixel id help service is up and running
    ATH_CHECK(detStore()->retrieve(m_pixIdHelper, "PixelID"));

    // Checks that the pixel manager service is up and running
    ATH_CHECK(detStore()->retrieve(m_detManager, "ITkPixel"));

    // ----------- Finds the different regions and layers to create histograms for them -----------
    std::vector<std::vector<float>> barrel_module_z(10); // List of z coordinates for modules in barrel
    std::vector<std::vector<float>> endcap_module_z(10); //List of z coordinates for modules in endcap

    for (InDetDD::SiDetectorElementCollection::const_iterator iter = m_detManager->getDetectorElementBegin();
         iter != m_detManager->getDetectorElementEnd(); ++iter) {
        const InDetDD::SiDetectorElement* element = *iter;

        if (!element) {
            ATH_MSG_ERROR("Problems with pointer to Detector Element !!!");
            return StatusCode::FAILURE;
        }

        // Get the element indices
        const Identifier identifier = element->identify();
        const int pixelPhiModule = m_pixIdHelper->phi_module(identifier);
        const int pixelBarrelEndcap = m_pixIdHelper->barrel_ec(identifier);
        const int pixelLayerDisk = m_pixIdHelper->layer_disk(identifier);
        const int pixelEtaModule = m_pixIdHelper->eta_module(identifier);

        // Use one module to save the z location, using phi_module == 0 is an arbitrary choice
        // also we can skip all the negative z modules since the geometry is symmetric
        if (pixelBarrelEndcap<0 or pixelPhiModule > 0 or (pixelBarrelEndcap == 0 and pixelEtaModule<0))
            continue;

        if (dynamic_cast <const InDetDD::PixelModuleDesign*>(&(element->design())) == nullptr){
            throw std::runtime_error("Pointer to PixelModuleDesign object is null");
        }
        const InDetDD::PixelModuleDesign *moduleDesign = static_cast<const InDetDD::PixelModuleDesign*>(&(element->design()));

        // Get the number of chips per module
        int chips = moduleDesign->numberOfCircuits();
        
        float module_z = element->center().z();
        ATH_MSG_DEBUG("Modules BarrelEndcap LayerDisk Phi Eta z : " << pixelBarrelEndcap << " " << pixelLayerDisk << " " << pixelPhiModule << " " << pixelEtaModule << " " << module_z << "  | nb chips : " << chips << " rows-cols: " << moduleDesign->rowsPerCircuit() << "-" << moduleDesign->columnsPerCircuit() << " thickness:" << moduleDesign->thickness());

        element->isBarrel() ? barrel_module_z.at(pixelLayerDisk).push_back(module_z) : endcap_module_z.at(pixelLayerDisk).push_back(module_z);

    }

    // When booking the histograms, you pass as well the module position
    // it is then stored in the tools and is used to bin the histograms accordingly
    ATH_CHECK(ITkPixelDataRateMonTool::bookHistograms(barrel_module_z, endcap_module_z));

    return StatusCode::SUCCESS;
}


void ITkPixelDataRateMonTool::fill(const uint32_t offlineID,
                                   const std::vector<unsigned int>& encodedstream,
                                   const ITkPixLayout<uint16_t>& hitMap) const {
    // -----------Receives the encoded stream and distributes it to the differents histograms-----------

    // Find the stream length as the number of bits in the stream
    const float streamLength = encodedstream.size() * s_bitsPerPack;
    // Find the data rate as the number of bits in the stream multiplied by the chip readout frequency to get it in b/s
    const float data_rate = streamLength * s_chipReadoutFrequency;

    // -----------Stream length distribution in bits for the whole detector and all the chips-----------
    m_encoded_streamLength->Fill(streamLength);

    // Finds module properties
    const Identifier waferID = Identifier(offlineID);
    const int pixelBarrelEndcap = m_pixIdHelper->barrel_ec(waferID);
    const int pixelLayerDisk = m_pixIdHelper->layer_disk(waferID);
    const int pixelEtaModule = m_pixIdHelper->eta_module(waferID);
    
    const Region region = (pixelBarrelEndcap==0) ? REGION_BARREL : REGION_ENDCAP;
    const Side side = (pixelEtaModule < 0 || pixelBarrelEndcap < 0) ? SIDE_NEGATIVE : SIDE_POSITIVE;
    const float z = std::abs(m_detManager->getDetectorElement(waferID)->center().z());

    // -----------Stream length distribution in bits-----------
    m_p_streamLength[pixelLayerDisk][region][side]->Fill(z, streamLength);
    m_h2_streamLength[pixelLayerDisk][region][side]->Fill(z, streamLength);

    // -----------Data rate distribution in bits per second-----------
    m_p_dataRate[pixelLayerDisk][region][side]->Fill(z, data_rate);
    m_h2_dataRate[pixelLayerDisk][region][side]->Fill(z, data_rate);

    // -----------Hits per chip-----------
    m_p_hits[pixelLayerDisk][region][side]->Fill(z, hitMap.nHits());
    m_h2_hits[pixelLayerDisk][region][side]->Fill(z, hitMap.nHits());

    if (m_doExpertPlots)
        fillExpertPlots(offlineID, hitMap);
}


void ITkPixelDataRateMonTool::fillExpertPlots(const uint32_t offlineID, const ITkPixLayout<uint16_t>& hitMap) const {

    // -----------Receives the hits and distributes it to the differents histograms-----------

    // Finds module properties
    const Identifier waferID = Identifier(offlineID);
    const int pixelBarrelEndcap = m_pixIdHelper->barrel_ec(waferID);
    const int pixelLayerDisk = m_pixIdHelper->layer_disk(waferID);

    const Region region = (pixelBarrelEndcap==0) ? REGION_BARREL : REGION_ENDCAP;

    // ---Chip hit map---

    for (uint16_t col = 0; col < s_col_chip-1; col++){
        for (uint16_t row = 0; row < s_row_chip-1; row++){

            if (hitMap(col, row) > 0){
                m_h2_chip_hitmap[pixelLayerDisk][region]->Fill(row, col, 1);
                m_h2_chip_totmap[pixelLayerDisk][region]->Fill(row, col, hitMap(col, row));
            }
        }
    }

}


StatusCode ITkPixelDataRateMonTool::bookHistograms(const std::vector<std::vector<float >>& barrel_z,
                                                   const std::vector<std::vector<float >>& endcap_z) {

    // -----------Stream length distribution in bits for the whole detector and all the chips-----------
    m_encoded_streamLength = new TH1F("m_encoded_streamLength", "Encoded Stream Length in bits", 2000, 0, 2000);
    if ((m_thistSvc->regHist(m_path + m_encoded_streamLength->GetName(), m_encoded_streamLength)).isFailure())
     return StatusCode::FAILURE;

    ATH_MSG_DEBUG("Histogram " << m_encoded_streamLength->GetName() << " successfully registered.");

    // -----------Stream length distribution in bits for different regions and layers-----------

    std::array<std::vector<std::vector<double>>, N_REGIONS> bins;
    for (auto& r : bins) r.resize(N_LAYERS);

    // New identifier scheme
    for (unsigned int layer = 0; layer<N_LAYERS ; layer++) {
        if (not barrel_z[layer].empty()) {
            for (unsigned int z_bin = 0; z_bin<(barrel_z[layer].size()-1); z_bin++) {
                // evaluate middle point between consecutive modules
                float interModulePoint = 0.5 * ( barrel_z[layer].at(z_bin) + barrel_z[layer].at(z_bin+1) );
                if (z_bin==0)
                    bins[REGION_BARREL][layer].push_back(0.);
                bins[REGION_BARREL][layer].push_back( interModulePoint );
            }
            //Creates the end of region covered by the module
            double last_value = 2.*barrel_z[layer].back() - barrel_z[layer].at( barrel_z[layer].size() - 2 );
            bins[REGION_BARREL][layer].push_back(last_value);
        }

        if (not endcap_z[layer].empty()) {
            for (unsigned int z_bin = 0; z_bin<(endcap_z[layer].size()-1); z_bin++) {
                // evaluate middle point between consecutive modules
                float interModulePoint = 0.5 * ( endcap_z[layer].at(z_bin) + endcap_z[layer].at(z_bin+1) );
                if (z_bin==0) {
                    float initialValue = 2.*endcap_z[layer].at(z_bin)-interModulePoint;
                    bins[REGION_ENDCAP][layer].push_back((initialValue));
                }
                bins[REGION_ENDCAP][layer].push_back( interModulePoint );
            }
            //Creates the end of region covered by the module
            double last_value = 2.*endcap_z[layer].back() - endcap_z[layer].at( endcap_z[layer].size() - 2);
            bins[REGION_ENDCAP][layer].push_back(last_value);
        }
    }

    for (unsigned int region=0; region<N_REGIONS; region++) {
        for (unsigned int layer=0; layer<N_LAYERS; layer++) {


            if (bins[region][layer].empty())
                continue;

            if (m_doExpertPlots) {
                m_h2_chip_hitmap[layer][region] = new TH2F(("m_h2_chip_hitmap_" + m_regionLabels[region] + "_" + std::to_string(layer)).c_str(),
                                                           (m_regionLabels[region] + " - Layer " + std::to_string(layer) + " Chip hit map; x [px]; y [px]; #epsilon").c_str(),
                                                           s_row_chip, -0.5, s_row_chip-0.5, s_col_chip, -0.5, s_col_chip-0.5);
                if ((m_thistSvc->regHist(m_path + m_h2_chip_hitmap[layer][region]->GetName(), m_h2_chip_hitmap[layer][region])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_h2_chip_hitmap[layer][region]->GetName() << " successfully registered.");

                m_h2_chip_totmap[layer][region] = new TH2F(("m_h2_chip_totmap_" + m_regionLabels[region] + "_" + std::to_string(layer)).c_str(),
                                                           (m_regionLabels[region] + " - Layer " + std::to_string(layer) + " Chip ToT map; x [px]; y [px]; #epsilon").c_str(),
                                                           s_row_chip, -0.5, s_row_chip-0.5, s_col_chip, -0.5, s_col_chip-0.5);
                if ((m_thistSvc->regHist(m_path + m_h2_chip_totmap[layer][region]->GetName(), m_h2_chip_totmap[layer][region])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_h2_chip_totmap[layer][region]->GetName() << " successfully registered.");
            }

            for (int side=0; side<N_SIDES; side++) {
                // Create names and title of histograms for full z coverage
                const std::string name_base = m_regionLabels[region] + "_" + std::to_string(layer) + "_" + m_sideLabels[side];
                const std::string title_base = m_regionLabels[region] + " - Layer " + std::to_string(layer) + " - Side " + m_sideLabels[side];

                // -----------Stream length distribution in bits for different regions and layers-----------
                m_p_streamLength[layer][region][side] = new TProfile(("m_p_streamLength_" + name_base).c_str(),
                                                                     (title_base + " Stream Length; z[mm]; <stream length> [bits]").c_str(),
                                                                     int(bins[region][layer].size()-1), &bins[region][layer][0]);
                if ((m_thistSvc->regHist(m_path + m_p_streamLength[layer][region][side]->GetName(), m_p_streamLength[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_p_streamLength[layer][region][side]->GetName() << " successfully registered.");

                m_h2_streamLength[layer][region][side] = new TH2F(("m_h2_streamLength_" + name_base).c_str(),
                                                                 (title_base + " Stream Length; z[mm]; <stream length> [bits]").c_str(),
                                                                 int(bins[region][layer].size()-1), &bins[region][layer][0],
                                                                 1000, 0., 10000.);
                if ((m_thistSvc->regHist(m_path + m_h2_streamLength[layer][region][side]->GetName(), m_h2_streamLength[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_h2_streamLength[layer][region][side]->GetName() << " successfully registered.");


                // -----------Data rate in b/s for different regions and layers-----------

                m_p_dataRate[layer][region][side] = new TProfile(("m_p_dataRate_" + name_base).c_str(),
                                                                 (title_base + " Data rate per chip; z[mm]; <data rate per chip> [b/s]").c_str(),
                                                                 int(bins[region][layer].size()-1), &bins[region][layer][0]);
                if ((m_thistSvc->regHist(m_path + m_p_dataRate[layer][region][side]->GetName(), m_p_dataRate[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_p_dataRate[layer][region][side]->GetName() << " successfully registered.");

                m_h2_dataRate[layer][region][side] = new TH2F(("m_h2_dataRate_" + name_base).c_str(),
                                                              (title_base + " Data rate per chip; z[mm]; <data rate per chip> [b/s]").c_str(),
                                                              int(bins[region][layer].size()-1), &bins[region][layer][0],
                                                              1000, 0., 10000.);
                if ((m_thistSvc->regHist(m_path + m_h2_dataRate[layer][region][side]->GetName(), m_h2_dataRate[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_h2_dataRate[layer][region][side]->GetName() << " successfully registered.");

                // -----------Hits per chip for different regions and layers-----------
                m_p_hits[layer][region][side] = new TProfile(("m_p_hits_" + name_base).c_str(),
                                                             (title_base + " Hits per chip; z[mm]; <hits/chip>").c_str(),
                                                             int(bins[region][layer].size()-1), &bins[region][layer][0]);
                if ((m_thistSvc->regHist(m_path + m_p_hits[layer][region][side]->GetName(), m_p_hits[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_p_hits[layer][region][side]->GetName() << " successfully registered.");

                m_h2_hits[layer][region][side] = new TH2F(("m_h2_hits_" + name_base).c_str(),
                                                                  (title_base + " Hits per chip; z[mm]; <hits/chip>").c_str(),
                                                                  int(bins[region][layer].size()-1), &bins[region][layer][0],
                                                                  1000, 0., 10000.);
                if ((m_thistSvc->regHist(m_path + m_h2_hits[layer][region][side]->GetName(), m_h2_hits[layer][region][side])).isFailure())
                    return StatusCode::FAILURE;
                ATH_MSG_DEBUG("Histogram " << m_h2_hits[layer][region][side]->GetName() << " successfully registered.");
            }
        }
    }

    return StatusCode::SUCCESS;

}
