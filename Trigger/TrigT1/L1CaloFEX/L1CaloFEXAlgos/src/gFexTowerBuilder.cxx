/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexTowerBuilder  -  description
//                              -------------------
//         Builds a gFexTowerContainer from CaloCellContainer (for supercells)
//                  TriggerTowerContainer (for ppm tile towers)
//         Information about SCellContainer objects are in:
//         - https://gitlab.cern.ch/atlas/athena/-/blob/22.0/Calorimeter/CaloEvent/CaloEvent/CaloCell.h
//
//     begin                : 22 04 2025
//     email                : jared.little@cern.ch
//***************************************************************************/

#include "gFexTowerBuilder.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "L1CaloFEXSim/gFEXCompression.h"

namespace LVL1 {

    gFexTowerBuilder::gFexTowerBuilder(const std::string& name, ISvcLocator* svc)
            : AthReentrantAlgorithm(name, svc) {}

    StatusCode gFexTowerBuilder::initialize() {

        ATH_MSG_INFO(
                "Initializing L1CaloFEXAlgos/gFexEmulatedTowers algorithm with name: "
                        << name());
        ATH_MSG_INFO("Writing into SG key: " << m_gTowersWriteKey);
        ATH_MSG_INFO("SCell masking: " << m_apply_masking);

        ATH_CHECK(m_SCellKey.initialize());
        ATH_CHECK(m_triggerTowerKey.initialize());
        ATH_CHECK(m_gTowersWriteKey.initialize());

        // Reading from CVMFS Fiber mapping
        ATH_CHECK(ReadFibersfromFile(PathResolver::find_calib_file(m_FiberMapping)));

        // Reading from CVMFS Trigger Tower and their corresponding SCell ID
        ATH_CHECK(ReadSCfromFile(PathResolver::find_calib_file(m_gFEX2Scellmapping)));
        ATH_CHECK(ReadTilefromFile(PathResolver::find_calib_file(m_gFEX2Tilemapping)));

        return StatusCode::SUCCESS;
    }

    StatusCode gFexTowerBuilder::execute(const EventContext& ctx) const {

        // Reading the Scell container
        SG::ReadHandle<CaloCellContainer> ScellContainer(m_SCellKey, ctx);
        if (!ScellContainer.isValid()) {
            ATH_MSG_ERROR("Could not retrieve collection " << ScellContainer.key());
            return StatusCode::FAILURE;
        }

        // Reading the TriggerTower container
        SG::ReadHandle<xAOD::TriggerTowerContainer> triggerTowerContainer(
                m_triggerTowerKey, ctx);
        if (!triggerTowerContainer.isValid()) {
            ATH_MSG_ERROR("Could not retrieve collection "
                                  << triggerTowerContainer.key());
            return StatusCode::FAILURE;
        }

        // WriteHandle for gFEX EDMs
        SG::WriteHandle<xAOD::gFexTowerContainer> gTowersContainer(m_gTowersWriteKey, ctx);
        ATH_CHECK( gTowersContainer.record(std::make_unique<xAOD::gFexTowerContainer>(),
                                        std::make_unique<xAOD::gFexTowerAuxContainer>()) );
        ATH_MSG_DEBUG("Recorded gFexEmulatedTower container with key " << gTowersContainer.key());

        if (ScellContainer->empty() || triggerTowerContainer->empty()) {
            ATH_MSG_WARNING(
                    "Cannot fill gTowers here, at least one container is empty. "
                    "ScellContainer.size="
                            << ScellContainer->size()
                            << " or triggerTowerContainer.size=" << triggerTowerContainer->size());
            return StatusCode::SUCCESS;
        }

        // building Scell ID pointers
        std::unordered_map<uint64_t, const CaloCell*> map_ScellID2ptr;
	map_ScellID2ptr.reserve(ScellContainer->size());

        for (const CaloCell* scell : *ScellContainer) {
            const uint64_t ID = scell->ID().get_compact();
            map_ScellID2ptr[ID] = scell;
        }

        // building Tile ID pointers
        std::unordered_map<uint32_t, const xAOD::TriggerTower*> map_TileID2ptr;
	map_TileID2ptr.reserve(triggerTowerContainer->size());

        for (const xAOD::TriggerTower* tower : *triggerTowerContainer) {
            map_TileID2ptr[tower->coolId()] = tower;
        }
        for (const auto& [key, element] : m_Firm2Tower_map) {
            unsigned int towerID = key;
            const auto [fpga, eta, phi, source] = element;

            // the summed encoded Et from LAr or Tile
            uint16_t total_et_encoded = 0;
            char gTower_sat = 0;
            // Note input fpga distinguishes between LAr (0,1,2), Tile (3) and duplicated channels (4)
            if (source == 0) {

                // check if the towerID exists in the LAr map
                auto it_TTower2SCells = m_map_TTower2SCells.find(towerID);
                if (it_TTower2SCells == m_map_TTower2SCells.end()) {
                    ATH_MSG_ERROR("gFEX ID: " << towerID
                                              << " not found on map m_map_TTower2SCells");
                    return StatusCode::FAILURE;
                }


                bool invalid = m_apply_masking && m_isDATA; // the isDATA is because there is no concept of invalid supercell in MC (the provenance bit is actually used for BCID in MC), so can never have an invalid jTower
                bool masked = m_apply_masking;

                // loop over the SCell IDs and calculate encoded Et
                int total_Et = 0;
                bool isConnected = true;
                for (const auto& scellID : it_TTower2SCells->second) {
                    // check if the SCell ID exists in the map
                    auto it_ScellID2ptr = map_ScellID2ptr.find(scellID);

                    // unconnected Towers have a single SCell with special id, set energy to zero
                    if (scellID == 0xffffffffffffffff) {
                        isConnected = false;
                        continue;
                    }

                    // check if other SCells are in the map
                    std::string str_hex = std::format("{:x}", scellID);

                    if (it_ScellID2ptr == map_ScellID2ptr.end()) {
                        if (m_isDATA)
                            ATH_MSG_WARNING("SCell ID: "
                                                    << scellID
                                                    << " not found in the CaloCellContainer, skipping");
                        continue;
                    }

                    const CaloCell* scell = it_ScellID2ptr->second;
                    int val = std::round(
                            scell->energy() /
                            (12.5 * std::cosh(scell->eta())));  // 12.5 is b.c. energy is in
                    // units of 12.5 MeV per count

                    bool isMasked =
                            m_apply_masking ? ((scell)->provenance() & 0x80) : false;
                    bool isInvalid =
                            (m_apply_masking&&m_isDATA) ? ((scell)->provenance()&0x40) : false;
                    bool isSaturated = (m_isDATA) ? scell->quality() : false; // saturation algorithm not implemented in MC yet

                    invalid &= isInvalid;
                    masked &= isMasked;
		    if (!isMasked) {
		      gTower_sat |= isSaturated;
		    }

                    if (isMasked) {
                        val = 0;
                    } else if( isInvalid&&m_isDATA) {
                        val = 0;
                    }

                    if (val != 0)
                        total_Et += val;

                }  // end of SCell loop

                // now must convert Total_Et int value into fex value: multi-level encoding
                if(!isConnected) {
                    total_et_encoded = 0; // no data
                } else if(masked) {
                    total_et_encoded = 0; // no data	
                } else if(invalid) {
                    total_et_encoded = 4095; // invalid
                } else {
                    total_et_encoded = gFEXCompression::compress(12.5 * total_Et);
                }


            } else if (source == 1) {

                // Tile
                // check that the gFEX Tower ID exists in the Tile map
                auto it_TTower2Tile = m_map_TTower2Tile.find(towerID);

                int Tile_Et = 0;
                for (auto const& TileTowerID : it_TTower2Tile->second) {
                    auto it_TileID2ptr = map_TileID2ptr.find(TileTowerID);
                    if (it_TileID2ptr == map_TileID2ptr.end()) {
                        if(m_isDATA) {
                            ATH_MSG_WARNING("Tile cool ID: " << TileTowerID
                                                             << " not found in the xAOD::TriggerTower (map_TileID2ptr)");
                        }
                        continue; // in MC the xAODTriggerTowers have variable size due to noise cuts, continue on to the next tower
                    } else {
                        const xAOD::TriggerTower* tileTower = it_TileID2ptr->second;
                        unsigned int jepEt = tileTower->jepET();
                        Tile_Et += jepEt;  // add the encoded energies
                    }
                }
                total_et_encoded = Tile_Et;

            } else if (source == 2) {
                // duplicated Towers, LATOME sends 0
                total_et_encoded = 0;
            }

            // the EDM requires a float
            float total_et_encoded_flt = total_et_encoded;

            unsigned int fpga_out = (towerID < 10000) ? 0 : (towerID < 20000) ? 1 : 2;
            unsigned int iEta = 0;
            unsigned int iPhi = 0;

            gTowersContainer->push_back(std::make_unique<xAOD::gFexTower>());
            gTowersContainer->back()->initialize(iEta, iPhi, eta, phi,
                                                 total_et_encoded_flt, fpga_out,
                                                 gTower_sat, towerID);
        }

        return StatusCode::SUCCESS;
    }

    StatusCode gFexTowerBuilder::ReadFibersfromFile(const std::string& fileName) {
        // opening file with ifstream
        std::ifstream file(fileName);

        if (!file.is_open()) {
            ATH_MSG_ERROR("Could not open file:" << fileName);
            return StatusCode::FAILURE;
        }
        std::string line;
        // loading the mapping information
        while (std::getline(file, line)) {
            // removing the header of the file (it is just information!)
            if (line[0] == '#') continue;

            // Splitting line in different substrings
            std::stringstream oneLine(line);
            // reading elements
            std::vector<float> elements;
	    elements.reserve(5);
            std::string element;
            while (std::getline(oneLine, element, ' ')) {
                elements.push_back(std::stof(element));
            }

            // It should have 5 elements
            // ordered as: towerID fpga source eta phi

            if (elements.size() != 5) {
                ATH_MSG_ERROR(
                        "Unexpected number of elements (5 expected) in file: " << fileName);
                return StatusCode::FAILURE;
            }

            // building array of  <fpga, eta, phi, source>
            std::array<float, 4> aux_arr{{elements.at(1), elements.at(3),
                                          elements.at(4), elements.at(2)}};

            // filling the map with the hash given by mapIndex()
            m_Firm2Tower_map[elements.at(0)] = aux_arr;
        }

        file.close();

        return StatusCode::SUCCESS;
    }

    StatusCode gFexTowerBuilder::ReadSCfromFile(const std::string& fileName) {

        // opening file with ifstream
        std::ifstream file(fileName);

        if (!file.is_open()) {
            ATH_MSG_ERROR("Could not open file:" << fileName);
            return StatusCode::FAILURE;
        }

        std::string line;
        // loading the mapping information into an unordered_map <Fex Tower ID, vector
        // of SCell IDs>
        while (std::getline(file, line)) {
            // removing the header of the file (it is just information!)
            if (line[0] == '#')
                continue;

	    std::vector<uint64_t> SCellvector;

            // Splitting line in different substrings
            std::stringstream oneSCellID(line);

            // reading elements
            std::string substr = "";
            int TTID = 0;
            int elem = 0;

            while (std::getline(oneSCellID, substr, ' ')) {
                ++elem;
                if (elem == 1) {
                    TTID = std::stoi(substr);
                } else {
                    // Check if it looks like a SCell Identifier
                    if (isBadSCellID(substr)) {
                        return StatusCode::FAILURE;
                    }

                    // converts hex number to unsigned long long int
                    // unconnnected slots are filled with 0xffffffffffffffff
                    // otherwise SCell map used shorter form, add extra zeroes back
                    std::string scell_full =
                            (substr == "0xffffffffffffffff") ? substr : substr + "00000000";
                    uint64_t scid_uint64 = std::strtoull(scell_full.c_str(), nullptr, 0);
                    SCellvector.push_back(scid_uint64);
                }
            }

            m_map_TTower2SCells[TTID] = std::move(SCellvector);
        }
        file.close();

        return StatusCode::SUCCESS;
    }

    bool gFexTowerBuilder::isBadSCellID(const std::string& ID) const {

        // does it start with "0x"?, if so then is a GOOD SCell ID!
        if (ID.find("0x") == std::string::npos) {
            ATH_MSG_ERROR("Invalid SuperCell ID "
                                  << ID
                                  << ". Expecting hexadecimal number on the mapping file");
            return true;
        }
        return false;
    }

    StatusCode gFexTowerBuilder::ReadTilefromFile(const std::string& fileName) {

        std::string myline;

        // openning file with ifstream
        std::ifstream myfile(fileName);

        if (!myfile.is_open()) {
            ATH_MSG_FATAL("Could not open file:" << fileName);
            return StatusCode::FAILURE;
        }

        // loading the mapping information into an unordered_map <Fex Tower ID, vector
        // of Tile IDs>
        while (std::getline(myfile, myline)) {
            // removing the header of the file
            myline.erase(myline.begin(),
                         std::find_if(myline.begin(), myline.end(),
                                      [](int ch) { return !std::isspace(ch); }));
            if (myline[0] == '#')
                continue;

	    std::vector<uint32_t> Tilevector;

            // Splitting myline in different substrings
            std::stringstream oneTileID(myline);

            // reading elements
            std::string substr = "";
            int gTowerID = 0;
            int elem = 0;

            while (std::getline(oneTileID, substr, ' ')) {
                ++elem;
                if (elem == 1) {
                    gTowerID = std::stoi(substr);
                } else {
                    uint32_t tileid_uint32 = std::strtoul(substr.c_str(), nullptr, 0);
                    Tilevector.push_back(tileid_uint32);
                }
            }
            m_map_TTower2Tile[gTowerID] = std::move(Tilevector);
        }
        myfile.close();

        return StatusCode::SUCCESS;
    }


}  // namespace LVL1
