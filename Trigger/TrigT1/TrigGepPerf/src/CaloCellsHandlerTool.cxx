/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <vector>
#include <fstream>
#include "TMath.h"
#include "PathResolver/PathResolver.h"
#include "./CaloCellsHandlerTool.h"

CaloCellsHandlerTool::CaloCellsHandlerTool(const std::string& type,
					   const std::string& name,
					   const IInterface* parent): AthAlgTool(type, name, parent){
}

CaloCellsHandlerTool::~CaloCellsHandlerTool() {}


StatusCode CaloCellsHandlerTool::initialize() {

  ATH_CHECK(m_electronicNoiseKey.initialize());
  ATH_CHECK(m_totalNoiseKey.initialize());
  
  // CaloIndentifier
  CHECK( detStore()->retrieve (m_CaloCell_ID, "CaloCell_ID") );

  ATH_MSG_INFO("Hardware-style energy encoding for GEP cells has been set to " << m_doGepHardwareStyleEnergyEncoding.value());
  if (!m_doGepHardwareStyleEnergyEncoding) ATH_MSG_WARNING("Hardware-style energy encoding for GEP cells has been disabled. Cell energies will have larger precision than realistically possible");

  ATH_MSG_INFO("Truncation of Gep cells from FEBs which are overflowing has been set to " << m_doTruncationOfOverflowingFEBs.value());
  if (!m_doTruncationOfOverflowingFEBs) ATH_MSG_WARNING("Truncation of GEP cells from overflowing FEBs has been disabled. More GEP cells will be send to algorithms than realistically possible");

  // Setting up GEP energy encoding scheme
  if (m_doGepHardwareStyleEnergyEncoding) {

	// Read values of Gep readout scheme from argument
	int GepScheme[3];
	for (int i = 0; i < 3; ++i) {
		GepScheme[i] = std::stoi(m_GepEnergyEncodingScheme.value().substr(0,m_GepEnergyEncodingScheme.value().find("-")));
		m_GepEnergyEncodingScheme.value().erase(0, m_GepEnergyEncodingScheme.value().find("-")+1);
	}

	CHECK(setNumberOfEnergyBits(GepScheme[0]));
	CHECK(setLeastSignificantBit(GepScheme[1]));
	CHECK(setG(GepScheme[2]));

	m_stepsPerRange = std::pow(2,m_nEnergyBits-2);

	m_readoutRanges[0] = 0;
	m_readoutRanges[1] = (m_stepsPerRange-1)*m_valLeastSigBit;
	m_readoutRanges[2] = ((m_valG*(m_stepsPerRange-1))+m_stepsPerRange)*m_valLeastSigBit;
	m_readoutRanges[3] = (m_stepsPerRange+(m_stepsPerRange*m_valG)+((m_stepsPerRange-1)*m_valG*m_valG))*m_valLeastSigBit;
	m_readoutRanges[4] = (m_stepsPerRange+(m_stepsPerRange*m_valG)+(m_stepsPerRange*m_valG*m_valG)+((m_stepsPerRange-1)*m_valG*m_valG*m_valG))*m_valLeastSigBit;

	ATH_MSG_INFO("Readout scheme with " << m_nEnergyBits << "-bits provides the following four energy thresholds (with " << m_stepsPerRange << " discrete steps on each threshold)");
	ATH_MSG_INFO("GEP cell energy range 0: min = " << m_readoutRanges[0] << " MeV -> max = " << m_readoutRanges[1] << " MeV");
	ATH_MSG_INFO("GEP cell energy range 1: min = " << m_readoutRanges[1] + m_valLeastSigBit << " MeV -> max = " << m_readoutRanges[2] << " MeV");
	ATH_MSG_INFO("GEP cell energy range 2: min = " << m_readoutRanges[2]+(m_valG*m_valLeastSigBit) << " MeV -> max = " << m_readoutRanges[3] << " MeV");
	ATH_MSG_INFO("GEP cell energy range 3: min = " << m_readoutRanges[3]+(m_valG*m_valG*m_valLeastSigBit) << " MeV -> max = " << m_readoutRanges[4] << " MeV");
  }

  if (m_doTruncationOfOverflowingFEBs) {
	// Loading the invariant cell data and storing in m_gepCellsBase for later use on event-by-event basis
	m_gepCellsBase.clear();

	ATH_MSG_INFO("Loading cell map associating cells to FEBs");

	std::string cellMapPath = PathResolverFindCalibFile(m_LArCellMap);
	if(cellMapPath.empty()) ATH_MSG_ERROR("Could not find file with cell map data: " << m_LArCellMap.value());

	std::ifstream file(cellMapPath.c_str());

	unsigned n_cells = 0;

	// Read input file
	if (file.is_open()) {

        	int online_id, offline_id, ch, con_num, fbr;
	        std::string feb, con_type;

        	// Skipping header of file
	        std::getline(file, feb);

        	// start reading data
	        while (true) {

        	        file >> offline_id >> online_id >> feb >> ch >> con_type >> con_num >> fbr;

                	if (file.eof()) break;

			Gep::GepCaloCell caloCell;
			caloCell.id = offline_id;
			caloCell.FEB = feb;
			caloCell.channel = ch;
			caloCell.fiber = fbr;
			caloCell.connection_type = con_type;
			caloCell.connection_number = con_num;

    			m_gepCellsBase.insert(std::pair<unsigned int, Gep::GepCaloCell>(caloCell.id, caloCell));

                	++n_cells;
	        }
	}
	else {
        	ATH_MSG_ERROR("Could not open file containing the cell to FEB association");
	        return StatusCode::FAILURE;
	}

	ATH_MSG_INFO("Loaded FEB information for " << n_cells << " cells");
  }

  return StatusCode::SUCCESS;
}

// Get calo cells map

StatusCode CaloCellsHandlerTool::getGepCellMap(const CaloCellContainer& cells,
					       pGepCellMap& gepCellsMap, 
					       const EventContext& ctx) const {

  // PS this function creates and returns a map which has Gep::GepCaloCells as its values
  // The cells are made up of data which is invariant for all events, and dynamic
  // data which varies with event.
  // The invariant data should be setup in initialize(), and the dynamic
  // data should be updated here in a way which is compatible with the const-ness of this
  // function.
  // This will be attended to in the future.

  SG::ReadCondHandle<CaloNoise> electronicNoiseHdl{m_electronicNoiseKey,  ctx};
  if (!electronicNoiseHdl.isValid()) {return StatusCode::FAILURE;}
  const CaloNoise* electronicNoiseCDO = *electronicNoiseHdl;

  SG::ReadCondHandle<CaloNoise> totalNoiseHdl{m_totalNoiseKey, ctx};
  if (!totalNoiseHdl.isValid()) {return StatusCode::FAILURE;}
  const CaloNoise* totalNoiseCDO = *totalNoiseHdl;

  int idx = -1;
  std::map<std::string,std::vector<Gep::GepCaloCell>> gepCellsPerFEB;

  for(const auto *cell: cells){

    Gep::GepCaloCell caloCell; 
    ++idx;

    caloCell.id = (cell->ID().get_identifier32()).get_compact();
    auto base_cell_itr = m_gepCellsBase.find(caloCell.id);
    if (base_cell_itr != m_gepCellsBase.end()) caloCell = base_cell_itr->second;
    else {
	// Tile cells are not included in the cell base map
	// In the future this might change, for now just setting FEB value to a dummy
	caloCell.FEB = "Tile";
    }

    float electronicNoise = electronicNoiseCDO->getNoise(cell->ID(), cell->gain());
    float totalNoise = totalNoiseCDO->getNoise(cell->ID(), cell->gain());
   
    // Only send positive-energy 2sigma cells to the GEP
    if ((cell->energy() / totalNoise) < 2.0) continue;

    // GEP will only have ET available for LAr cells, so convert to energy from ET
    if (m_doGepHardwareStyleEnergyEncoding && !m_CaloCell_ID->is_tile(cell->ID())) {
	caloCell.et	= getGepEnergy(cell->energy() / TMath::CosH(cell->eta()));
	caloCell.e	= caloCell.et * TMath::CosH(cell->eta());
    }
    else {
	caloCell.e	= cell->energy();
	caloCell.et	= caloCell.e / TMath::CosH(cell->eta());
    }
    caloCell.time       = cell->time();
    caloCell.quality    = cell->quality();
    caloCell.provenance = cell->provenance();    
          
    caloCell.totalNoise = totalNoise;
    caloCell.electronicNoise = electronicNoise;
    caloCell.sigma = cell->energy() / totalNoise;

    caloCell.isBad = cell->badcell();    
    caloCell.eta    = cell->eta();
    caloCell.phi    = cell->phi();
    caloCell.sinTh  = cell->sinTh();
    caloCell.cosTh  = cell->cosTh();
    caloCell.sinPhi = cell->sinPhi();
    caloCell.cosPhi = cell->cosPhi();
    caloCell.cotTh  = cell->cotTh();
    caloCell.x = cell->x();
    caloCell.y = cell->y();
    caloCell.z = cell->z();
    
    unsigned int samplingEnum = m_CaloCell_ID->calo_sample(cell->ID());
    
    bool IsEM = m_CaloCell_ID->is_em(cell->ID());
    bool IsEM_Barrel=false;
    bool IsEM_EndCap=false;
    bool IsEM_BarrelPos=false;
    bool IsEM_BarrelNeg=false;
    if(IsEM){
      IsEM_Barrel=m_CaloCell_ID->is_em_barrel(cell->ID());
      if(IsEM_Barrel){
	if(m_CaloCell_ID->pos_neg(cell->ID())>0) IsEM_BarrelPos=true;
      }
      IsEM_EndCap=m_CaloCell_ID->is_em_endcap(cell->ID());
    }
    
    caloCell.isEM           = IsEM;
    caloCell.isEM_barrel    = IsEM_Barrel;
    caloCell.isEM_endCap    = IsEM_EndCap;
    caloCell.isEM_barrelPos = IsEM_BarrelPos;
    caloCell.isEM_barrelNeg = IsEM_BarrelNeg;  //always false?
    caloCell.isFCAL = m_CaloCell_ID->is_fcal(cell->ID());
    caloCell.isHEC  = m_CaloCell_ID->is_hec(cell->ID());
    caloCell.isTile = m_CaloCell_ID->is_tile(cell->ID());
    
    caloCell.sampling = samplingEnum;
    caloCell.detName = CaloSampling::getSamplingName(samplingEnum);
    
    caloCell.neighbours = getNeighbours(cells, cell, ctx);
    caloCell.id = (cell->ID().get_identifier32()).get_compact(); 
    
    const CaloDetDescriptor *elt = cell->caloDDE()->descriptor();
    caloCell.layer = cell->caloDDE()->getLayer();
    
    float deta = elt->deta();
    float dphi = elt->dphi();
    
    float etamin = caloCell.eta - (0.5*deta);
    float etamax = caloCell.eta + (0.5*deta);
    
    float phimin = caloCell.phi - (0.5*dphi);
    float phimax = caloCell.phi + (0.5*dphi);
    
    caloCell.etaMin = etamin;
    caloCell.etaMax = etamax;
    caloCell.phiMin = phimin;
    caloCell.phiMax = phimax;
    caloCell.etaGranularity = deta;
    caloCell.phiGranularity = dphi;

    caloCell.index = idx;
  
    // Fill cells into map according to FEB  
    auto feb_itr = gepCellsPerFEB.find(caloCell.FEB);
    if (feb_itr != gepCellsPerFEB.end()) feb_itr->second.push_back(caloCell);
    else {
	std::vector<Gep::GepCaloCell> cellsThisFEB;
	cellsThisFEB.push_back(caloCell);
	gepCellsPerFEB.insert(std::pair<std::string,std::vector<Gep::GepCaloCell>>(caloCell.FEB,cellsThisFEB));
    }
  }

  // store cells map
  auto itr = gepCellsPerFEB.begin();
  for ( ;itr != gepCellsPerFEB.end(); ++itr) {

	// LAr FEBs might overflow, so they will get truncated
	if (m_doTruncationOfOverflowingFEBs && itr->second.size() > m_maxCellsPerFEB && itr->first != "Tile") {
		ATH_MSG_DEBUG("FEB " << itr->first << " is sending " << itr->second.size() << " cells, which is more cells than GEP can receive. Removing all but the possible " << m_maxCellsPerFEB << " cells.");
		CHECK(removeCellsFromOverloadedFEB(itr->second));
	}
  	for (const Gep::GepCaloCell& cell : itr->second)
		gepCellsMap->insert(std::pair<unsigned int, Gep::GepCaloCell>(cell.id, cell)); 
  }
  ATH_MSG_DEBUG("GEP is receiving a total of " << gepCellsMap->size() << " cells in this event");

  return StatusCode::SUCCESS;
}



int CaloCellsHandlerTool::getGepEnergy(float offline_et) const {

  // If cell saturates readout range, largest possible value is send
  if (offline_et > m_readoutRanges[4]) return m_readoutRanges[4];

  int readoutRange = 0;
  for (int i = 1; i <= 3; ++i) {
        if (offline_et > m_readoutRanges[i]) readoutRange = i;
  }

  float step = ((float) m_readoutRanges[readoutRange+1] - (float) m_readoutRanges[readoutRange]) / (m_stepsPerRange-1);
  int gep_energy = -1;
  for (int i = 0; i < m_stepsPerRange; ++i) {
        if (offline_et < (m_readoutRanges[readoutRange]+(step*i))) break;
        gep_energy = m_readoutRanges[readoutRange]+(step*i);
  }

  return gep_energy;
}



// Get neighbours of a given calo cell
std::vector<unsigned int>
CaloCellsHandlerTool::getNeighbours(const CaloCellContainer& allcells,
				    const CaloCell* acell,
				    const EventContext&) const {

  // get all neighboring cells
  std::vector<IdentifierHash> cellNeighbours;

  IdentifierHash cellHashID = m_CaloCell_ID->calo_cell_hash(acell->ID());
  m_CaloCell_ID->get_neighbours(cellHashID,LArNeighbours::super3D,cellNeighbours);

  std::vector<unsigned int> neighbour_ids;
  for (unsigned int iNeighbour = 0;
       iNeighbour < cellNeighbours.size();
       ++iNeighbour) {
    
    const CaloCell* neighbour = allcells.findCell(cellNeighbours[iNeighbour]);
    if (neighbour) {
      neighbour_ids.push_back((neighbour->ID().get_identifier32()).get_compact());
    } else {
      ATH_MSG_ERROR("Couldn't access neighbour #" << iNeighbour
		    << " for cell ID "
		    << (acell->ID().get_identifier32()).get_compact());
    }
  }
  return neighbour_ids;
}



StatusCode CaloCellsHandlerTool::removeCellsFromOverloadedFEB(std::vector<Gep::GepCaloCell> &cells) const {

  std::map<int,Gep::GepCaloCell> orderedCells;
  for (const Gep::GepCaloCell& cell : cells) 
	orderedCells.insert(std::pair<int,Gep::GepCaloCell>(cell.channel,cell));

  cells.clear();

  std::map<int,Gep::GepCaloCell>::iterator cell_itr = orderedCells.begin();
  for ( ;cell_itr != orderedCells.end(); ++cell_itr) {
	cells.push_back(cell_itr->second);
	if (cells.size() == m_maxCellsPerFEB) break;
  }

  return StatusCode::SUCCESS;
}




