/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This Algorithm simulates the energy encoding of all LAr cells for Global and simulated the truncation of cells
  from overflowing FEB2s. The hardware-accurate cells are then stored in a GlobalLArCellContainer object within 
  StoreGate
*/

#include "LArCellPreparationAlg.h"

#include "PathResolver/PathResolver.h"
#include "CaloEvent/CaloCell.h"
#include "xAODEventInfo/EventInfo.h"

#include "TMath.h"
#include <fstream>
#include <vector>
#include <cmath>

namespace GlobalSim {

  // Initialize function which defines the parameters of the multilinear energy encoding, calculates the ranges,
  // determines the maximum number of cells per FEB2 that can be sent to Global and reads the LAr cell map to get
  // the full list of associated boards and how they are wired to the Global MUXs
  StatusCode LArCellPreparationAlg::initialize() {
    ATH_MSG_INFO ("Initializing " << name());
    ATH_MSG_INFO ("Target name of GlobalLArCellContainer is " << m_LArCellContainerKey);

    CHECK(m_eventInfo.initialize());
    CHECK(m_caloCellsKey.initialize());
    CHECK(m_LArCellContainerKey.initialize());

    ATH_CHECK(m_totalNoiseKey.initialize());

    ATH_MSG_INFO("Active energy encoding scheme for LAr cells is " << m_numberOfEnergyBits.value() << " energy bits with " << m_valueLSB.value() << " MeV for the least significant bit and a gain factor of " << m_valueGainFactor.value());

    m_stepsPerRange = std::pow(2,m_numberOfEnergyBits.value()-2);

    m_readoutRanges[0] = 0;
    m_readoutRanges[1] = m_stepsPerRange*m_valueLSB.value();
    m_readoutRanges[2] = ((m_valueGainFactor.value()*m_stepsPerRange)+m_stepsPerRange)*m_valueLSB.value();
    m_readoutRanges[3] = (m_stepsPerRange+(m_stepsPerRange*m_valueGainFactor.value())+(m_stepsPerRange*m_valueGainFactor.value()*m_valueGainFactor.value()))*m_valueLSB.value();
    m_readoutRanges[4] = (m_stepsPerRange+(m_stepsPerRange*m_valueGainFactor.value())+(m_stepsPerRange*m_valueGainFactor.value()*m_valueGainFactor.value()) +
                         (m_stepsPerRange*m_valueGainFactor.value()*m_valueGainFactor.value()*m_valueGainFactor.value()))*m_valueLSB.value();

    ATH_MSG_DEBUG("Readout scheme with " << m_numberOfEnergyBits.value() << "-bits provides the following four energy thresholds (with " << m_stepsPerRange << " discrete steps on each threshold)");
    ATH_MSG_DEBUG("GEP cell energy range 0: min = " << m_readoutRanges[0] << " MeV -> max = " << m_readoutRanges[1] << " MeV");
    ATH_MSG_DEBUG("GEP cell energy range 1: min = " << m_readoutRanges[1] + m_valueLSB.value() << " MeV -> max = " << m_readoutRanges[2] << " MeV");
    ATH_MSG_DEBUG("GEP cell energy range 2: min = " << m_readoutRanges[2]+(m_valueGainFactor.value()*m_valueLSB.value()) << " MeV -> max = " << m_readoutRanges[3] << " MeV");
    ATH_MSG_DEBUG("GEP cell energy range 3: min = " << m_readoutRanges[3]+(m_valueGainFactor.value()*m_valueGainFactor.value()*m_valueLSB.value()) << " MeV -> max = " << m_readoutRanges[4] << " MeV");

    // At the moment only have a detailed scheme for 6-10 bit readouts, thus rejecting any other value
    switch(m_numberOfEnergyBits.value()) {
        case 6: m_maxCellsPerFEB = 62; break;
        case 7: m_maxCellsPerFEB = 54; break;
        case 8: m_maxCellsPerFEB = 48; break;
        case 9: m_maxCellsPerFEB = 43; break;
        case 10: m_maxCellsPerFEB = 39; break;
        default: ATH_MSG_FATAL("A LAr cell energy encoding scheme with " << m_numberOfEnergyBits.value() << " energy bits is currently not defined");
        return StatusCode::FAILURE;
    }

    ATH_MSG_INFO("Loading cell map associating LAr cells to FEB2s");

    std::string cellMapPath = PathResolverFindCalibFile(m_LArCellMap);
    if(cellMapPath.empty()) ATH_MSG_ERROR("Could not find file with cell map data: " << m_LArCellMap.value());

    std::ifstream file(cellMapPath.c_str());

    unsigned n_cells = 0;
    m_gblLArCellMap.clear();

    std::map<std::string,Feb2MuxInfo> feb2MuxAssoc;

    // Read input file
    if (file.is_open()) {

        int online_id, offline_id, channel, con_num, fbr;
        std::string assocFEB2, con_type, muxname, muxrack, cnnctr, laspname, lasprack;

        // Skipping header of file
        std::getline(file, assocFEB2);

        // start reading data
        while (true) {

            file >> offline_id >> online_id >> assocFEB2 >> channel >> con_type >> con_num >> fbr >> muxname >> muxrack >> cnnctr >> laspname >> lasprack;

            if (file.eof()) break;

            GlobalSim::GlobalLArCell gblLArCell(offline_id, assocFEB2, channel);
            gblLArCell.setBoardConnector(cnnctr, con_type, con_num, fbr);
            gblLArCell.setMUX(muxname);
            gblLArCell.setLASP(laspname);

            m_gblLArCellMap.insert(std::pair<int, GlobalSim::GlobalLArCell>(offline_id, gblLArCell));

            int indexOnMux = fbr;
            if (cnnctr == "B") indexOnMux += 24;
            if (cnnctr == "C") indexOnMux += 32;

            // Add FEB2 to MUX association map
            auto itr = feb2MuxAssoc.find(assocFEB2);
            if (itr == feb2MuxAssoc.end()) {
                feb2MuxAssoc.insert(std::pair<std::string,Feb2MuxInfo>(assocFEB2, {muxname, indexOnMux}));
            }

            ++n_cells;
        }
    }
    else {
            ATH_MSG_ERROR("Could not open file containing the cell to FEB2 association");
            return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Loaded FEB2 information for " << n_cells << " LAr cells");

    // Constructing the GlobalLArCellContainer
    m_gblLArCellContainerTemplate = std::make_unique<GlobalSim::GlobalLArCellContainer>(feb2MuxAssoc);
    m_gblLArCellContainerTemplate->setMaxCellsPerFeb2(m_maxCellsPerFEB);
  
    return StatusCode::SUCCESS;
  }


  // Read in a CaloCell container, encode the cell energy according to the active encoding scheme, perform
  // the FEB2 truncation and then store all cells which would be sent to Global in a GlobalLArCellContainer
  StatusCode LArCellPreparationAlg::execute(const EventContext& ctx) const {

    ATH_MSG_DEBUG ("Executing LArCellPreparationAlg algorithm");
    
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo, ctx);
    CHECK(eventInfo.isValid());

    // Read in container containing calorimeter cells
    auto h_caloCells = SG::makeHandle(m_caloCellsKey, ctx);
    CHECK(h_caloCells.isValid());
    const auto & cells = *h_caloCells;

    ATH_MSG_DEBUG("Reading " << std::to_string(h_caloCells->size()) << " cells in input cell container");

    SG::ReadCondHandle<CaloNoise> totalNoiseHdl{m_totalNoiseKey, ctx};
    if (!totalNoiseHdl.isValid()) {return StatusCode::FAILURE;}
    const CaloNoise* totalNoiseCDO = *totalNoiseHdl;

    std::map<std::string,std::vector<GlobalSim::GlobalLArCell>> gblLArCellsPerFEB2;

    for(const auto *cell: cells){

        int cell_id = (cell->ID().get_identifier32()).get_compact();

        auto gblLArCell_itr = m_gblLArCellMap.find(cell_id);
        if (gblLArCell_itr == m_gblLArCellMap.end()) continue;

        GlobalSim::GlobalLArCell gblLArCell = gblLArCell_itr->second;

        float totalNoise = totalNoiseCDO->getNoise(cell->ID(), cell->gain());
        float sigma = cell->energy() / totalNoise;

        // Only send positive-energy 2sigma cells to the GEP
        if (sigma < 2.0) continue;

        if (cell->badcell()) continue;

        std::pair<float, boost::dynamic_bitset<>> gep_energy = encodeEnergy(cell->energy() / TMath::CosH(cell->eta()));

        gblLArCell.setEnergy(gep_energy.first, std::move(gep_energy.second));
        gblLArCell.setSigma(sigma);
        gblLArCell.setPosition(cell->eta(), cell->phi());
        gblLArCell.setSampling(cell->caloDDE()->getSampling());
        gblLArCell.setLayer(cell->caloDDE()->getLayer());

        // Fill cells into map according to FEB  
        auto feb2_itr = gblLArCellsPerFEB2.find(gblLArCell.getFEB2());
        if (feb2_itr != gblLArCellsPerFEB2.end()) feb2_itr->second.push_back(gblLArCell);
        else {
            std::vector<GlobalSim::GlobalLArCell> cellsThisFEB;
            cellsThisFEB.push_back(gblLArCell);
            gblLArCellsPerFEB2.insert(std::pair<std::string,std::vector<GlobalSim::GlobalLArCell>>(gblLArCell.getFEB2(),cellsThisFEB));
        }
    }

    // Set up a GlobalLArCellContainer from template
    const GlobalSim::GlobalLArCellContainer& templateRef = *m_gblLArCellContainerTemplate;
    auto gblLArCellContainer = std::make_unique<GlobalSim::GlobalLArCellContainer>(templateRef);

    // do truncation
    for (auto& [feb2Name, cells] : gblLArCellsPerFEB2) {

        // Overflow and error flags
        bool inOverflow = false;
        bool inError = false;

        // LAr FEBs might overflow, so they will get truncated
        if (cells.size() > m_maxCellsPerFEB) {
            ATH_MSG_INFO("FEB " << feb2Name << " is sending " << cells.size() << " cells, which is more cells than GEP can receive. Removing all but the possible " << m_maxCellsPerFEB << " cells.");
            CHECK(removeCellsFromOverloadedFEB(cells));
            inOverflow = true;
        }

        for (auto& gblLArCell : cells)
            gblLArCellContainer->push_back(std::move(gblLArCell));

        gblLArCellContainer->setFeb2Flags(feb2Name, inOverflow, inError);
    }
    ATH_MSG_DEBUG("Global is receiving a total of " << gblLArCellContainer->size() << " LAr cells in this event");

    SG::WriteHandle<GlobalSim::GlobalLArCellContainer> h_gblLArCellContainer = SG::makeHandle(m_LArCellContainerKey, ctx);
    ATH_CHECK( h_gblLArCellContainer.record( std::move(gblLArCellContainer) ) );

    return StatusCode::SUCCESS;
  }


  // Function to emulate the hardware realistic energy of the cell by applying the
  // multilinear energy encoding scheme defined in the initialize function and at
  // the same time building the bitstring encoding the energy
  std::pair<float,boost::dynamic_bitset<>> LArCellPreparationAlg::encodeEnergy(float energy) const {

    // Negative energy cell
    if (energy < 0) return std::pair<float,boost::dynamic_bitset<>>(0.0,boost::dynamic_bitset<>(m_numberOfEnergyBits.value(),0));
  
    // Saturated cell
    if (energy > m_readoutRanges[4]) {
          int max_value = ( m_stepsPerRange + m_stepsPerRange*m_valueGainFactor.value() +
                            m_stepsPerRange*m_valueGainFactor.value()*m_valueGainFactor.value() +
                            (m_stepsPerRange-1)*m_valueGainFactor.value()*m_valueGainFactor.value()*m_valueGainFactor.value() ) * m_valueLSB.value();
          return std::pair<float,boost::dynamic_bitset<>>(max_value,boost::dynamic_bitset<>(m_numberOfEnergyBits.value(),std::pow(2,m_numberOfEnergyBits.value())-1));
    }
  
    int range = 0;
    for (int i = 1; i <= 3; ++i) {
          if (energy > m_readoutRanges[i]) range = i;
    }
  
    float step = ((float) m_readoutRanges[range+1] - (float) m_readoutRanges[range]) / m_stepsPerRange;
  
    float encoded_energy = -1;
    int used_steps = 0;
    for (int i = 0; i < m_stepsPerRange; ++i) {
          encoded_energy = m_readoutRanges[range]+(step*i);
          used_steps = i;
          if (energy < (m_readoutRanges[range]+(step*(i+1)))) break;
    }
  
    std::size_t n_bitsE = m_numberOfEnergyBits.value() - 2;
    boost::dynamic_bitset<> energy_bits(m_numberOfEnergyBits.value(), used_steps);
    energy_bits |= boost::dynamic_bitset<>(m_numberOfEnergyBits.value(), static_cast<unsigned long>(range) << n_bitsE);

    return std::pair<float,boost::dynamic_bitset<>>(encoded_energy,energy_bits);
  }

  
  // Function to find FEB2s which have more 2sigma cells in this event than the available
  // latency allows to send and truncates overflowing cells
  StatusCode LArCellPreparationAlg::removeCellsFromOverloadedFEB(std::vector<GlobalSim::GlobalLArCell> &cells) const {

    // Sort cells by channel
    std::sort(cells.begin(), cells.end(), [](const auto& a, const auto& b) {
        return a.getChannel() < b.getChannel(); });

    // Remove overflowing cells from vector
    if (cells.size() > m_maxCellsPerFEB) 
        cells.erase(std::next(cells.begin(), m_maxCellsPerFEB), cells.end());

    return StatusCode::SUCCESS;
  }

} // namespace GlobalSim

