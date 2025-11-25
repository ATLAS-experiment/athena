/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm outputs Calorimeter strip data in the region of
  eFex RoIs.
*/

#include "Egamma1_LArStrip_Fex_RowAware.h"
#include "../IO/LArStripNeighborhoodDumper.h"
#include "../IO/eEmNbhoodTOB.h"

#include "CaloEvent/CaloCell.h"

#include "xAODEventInfo/EventInfo.h"

#include <fstream>
#include <vector>
#include <algorithm>
#include <optional>

namespace GlobalSim {

  std::optional<std::vector<std::size_t>> wrap5(std::size_t center) {
    if (center > 63) {
      return std::optional<std::vector<std::size_t>>{};
    }

    if (center == 63) {
      return std::make_optional(std::vector<std::size_t>({61ul, 62ul, 63ul, 0ul, 1ul}));
    }
    
    if (center == 0) {
      return std::make_optional(std::vector<std::size_t>({62ul, 63ul, 0ul, 1ul, 2ul}));
    }
    
    return std::make_optional(std::vector<std::size_t>(
						       {center-2,
							center-1,
							center,
							center+1,
							center+2}));
  }

  Egamma1_LArStrip_Fex_RowAware::Egamma1_LArStrip_Fex_RowAware(const std::string& name, ISvcLocator* pSvcLocator ) : 
    AthReentrantAlgorithm(name, pSvcLocator){
  }


  StatusCode Egamma1_LArStrip_Fex_RowAware::initialize() {
    ATH_MSG_INFO ("Initializing " << name());

    CHECK(m_cellProducer.retrieve());
    CHECK(m_roiAlgTool.retrieve());
    CHECK(m_neighKey.initialize());
    CHECK(m_phimaxKey.initialize());
    CHECK(m_eventInfoKey.initialize());
  
    return StatusCode::SUCCESS;
  }


  StatusCode Egamma1_LArStrip_Fex_RowAware::execute(const EventContext& ctx) const {
    // Read in a CaloCell container. Ask producers to create
    // vectors of CaloCells to be examined.

    ATH_MSG_DEBUG ("Executing");
    
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    if(!eventInfo.isValid()) {
      ATH_MSG_ERROR ("Error obtaining EventInfo object");
      return StatusCode::FAILURE;
    }
    
    std::vector<const CaloCell*> cells;
    CHECK(m_cellProducer->cells(cells, ctx));
    ATH_MSG_DEBUG(cells.size() <<"Cells read in");
 
    std::vector<const xAOD::eFexEMRoI*> rois;
    CHECK(m_roiAlgTool->RoIs(rois, ctx));
    ATH_MSG_DEBUG(rois.size() << " RoI(s) read in");

    // find cells in the neighborhood of RoIs.
    // A neighborhood is a collection of CellData objects which
    // contain cell eta, phi and Et.
    
    auto neighborhoodTOBs = std::make_unique<IOBitwise::IeEmNbhoodTOBContainer>();
    auto phimax = std::make_unique<std::vector<int>>();
    
    CHECK(findNeighborhoods_RowAware(rois, cells, *neighborhoodTOBs, *phimax));
    
    SG::WriteHandle<GlobalSim::IOBitwise::IeEmNbhoodTOBContainer> h_neighborhoodTOBs(m_neighKey, ctx);
    SG::WriteHandle<std::vector<int> > h_phimax(m_phimaxKey, ctx);

    auto dumper = GlobalSim::LArStripNeighborhoodDumper();
    if(m_dump || m_dumpTerse){
      if (m_dump) {
	CHECK(dumper.dump(name(), *eventInfo, *neighborhoodTOBs));
      }
      
      if (m_dumpTerse) {
	CHECK(dumper.dumpTerse(name(), *eventInfo, *neighborhoodTOBs));
      }
    }
    
    CHECK(h_neighborhoodTOBs.record(std::move(neighborhoodTOBs)));
    CHECK(h_phimax.record(std::move(phimax)));
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  Egamma1_LArStrip_Fex_RowAware::findNeighborhoods_RowAware(const std::vector<const xAOD::eFexEMRoI*>& rois,
							    const std::vector<const CaloCell*>& cells,
							    IOBitwise::IeEmNbhoodTOBContainer& neighborhoodTOBs,
							    std::vector<int>& phimax) const{
    
    for (const auto& roi : rois) {
      CHECK(findNeighborhood_RowAware(roi, cells, neighborhoodTOBs, phimax));
    }
    
    return StatusCode::SUCCESS;
  }
  

  StatusCode
  Egamma1_LArStrip_Fex_RowAware::findNeighborhood_RowAware(const xAOD::eFexEMRoI* roi,
							   const std::vector<const CaloCell*>& cells,
							   IOBitwise::IeEmNbhoodTOBContainer& neighborhoodTOBs,
							   std::vector<int>& phimax) const {
    
    // this member function constructs an LArStripNeighborhood.
    // 
    // A neighourhood is constructed from StripData objects constructed
    // from CalCells (strips) in the vicinity of an EM RoI the following manner:
    //
    // - the cell in the vicinity of the RoI is identified. The neighboorhood
    // strips are the strups centered on the maximum energy strip.
    //
    // In more detail:
    // A rectangular eta-phi subset of all strips is made.
    // This subset constains the cells needed for the maximum energy search
    // and for any subsequent neigborhood strip selection.
    //
    // The eta window for the initial selection ismade with half-width of
    // 0.05 + 8.5* DeltaEta where DeltaEta = 0.003125, the nominal strip width
    // in eta. A cut-oof off eta = +-1.4 is applied in the selection.
    // 
    // Phi selection consists of calculating a phi index for the the RoI,
    // then requiring selected strips to be have a phi indix within +=1 of
    // the RoI phi index.
    //
    // The strips used to identigy the max energy strip the are within +-1 of the 
    // as the phi index of the RoI, and eta that lies within 0.1 in eta to the RoI.
    //
     // The strips selected for the Neighborhood have eta  within +- 8.5 DeltaEta
    // of the max energy strip, and +- 1 of the max RoI strip phi Index.
 
    auto cells_near_roi = std::vector<const CaloCell*>();

    ATH_MSG_DEBUG("roi eta " << roi->eta() << " phi " << roi->phi());
    
    // lambda function to calculate the phi index of a eta-phi point
    auto phi_ind = [](const auto& c) {
      constexpr double dphi = std::numbers::pi/32;
      std::size_t iphi = 32 + int(std::floor(c->phi()/dphi));
      return iphi > 63 ?
	std::optional<std::size_t>() : std::make_optional(iphi);
    };

    const std::optional<std::size_t> roi_phi_index_opt = phi_ind(roi);
    if (not roi_phi_index_opt.has_value()) {
      return StatusCode::FAILURE; 
    }
    const std::size_t roi_phi_index = *roi_phi_index_opt;
    
    // obtain adjacent phi indices
    auto roi_phi_indices = *wrap5(roi_phi_index);

    // container for strips close to RoI in eta
    auto close = std::deque<std::vector<const CaloCell*>>(5);
    for (auto& v :close) {v.reserve(100);}
    
    //One ROI is 0.1 wide in eta, this is the half width
    constexpr double half_deta_roi{0.05};
    //One cell in the barrel is 0.003125, this is the half width of the 17 cell window
    constexpr double half_deta_neigh{8.5*0.003125};
    //To define the ±eta limit of our window, we need to sum the above
    constexpr double half_deta_fid{half_deta_roi + half_deta_neigh};
    
    
    double etalim_low = std::max(roi->eta()-half_deta_fid, -1.4);
    double etalim_high = std::min(roi->eta()+half_deta_fid, 1.4);


    // double loop. outer: all strips. inner: adjacent roi indices.
    for (const auto& cell : cells) {
      auto icell = *phi_ind(cell);
      std::size_t pos{0};
      for(const auto& iroi : roi_phi_indices) {
	auto c_eta = cell->eta();
	if (iroi == icell and c_eta >= etalim_low and c_eta < etalim_high) {
	  close[pos].push_back(cell);
	  break;
	}
	++pos;
      }
    }
    
    // select the cells within a a tower width of the RoI. Then find the
    // cell in this selction with the highest energy
    auto roi_cells = std::deque<std::vector<const CaloCell*>>(5);
    auto roi_max_it = std::vector<std::vector<const CaloCell*>::iterator>();
    
    for (std::size_t i{0ul}; i != close.size(); ++i) {
      roi_cells[i].reserve(close[i].size());
      std::copy_if(std::begin(close[i]),
		   std::end(close[i]),
		   std::back_inserter(roi_cells[i]),
		   [&roi](const auto& c) {
		     return std::abs(c->eta() - roi->eta()) < half_deta_roi;
		   });
      //Work out where the max is in the central block.
      if(i > 0 && i < 4){
	auto it = std::max_element(std::begin(roi_cells[i]),
				   std::end(roi_cells[i]),
				   [](const auto& l,const auto& r) {
				     return l->e() < r->e();
				   });
	ATH_MSG_DEBUG("max cell row "
		      << i << " position " 
		      << std::distance(std::begin(roi_cells[i]), it)
		      << " :" << **it);
	roi_max_it.push_back(it);
      }
    }
    
    auto max_row = std::max_element(std::begin(roi_max_it),
				    std::end(roi_max_it),
				    [](const auto& l,const auto& r) {
				      return (*l)->e() < (*r)->e();
				    });

    int max_row_pos = std::distance( roi_max_it.begin(), max_row );

    phimax.push_back(max_row_pos);
    
    ATH_MSG_DEBUG("max cell row "
		  << ' ' << std::distance( roi_max_it.begin(), max_row )+1);

    //If the maximum is not in the centre row, we need to re-seed    
    switch(max_row_pos)
    {
      case 0: 
        {
	  close.pop_back();
	  close.pop_back();
	  break;
	}
      case 1:
	{
	  close.pop_front();
	  close.pop_back();
	  break;
	}
      case 2:
	{
	  close.pop_front();
	  close.pop_front();
	  break;
	}
    }

    ATH_MSG_DEBUG("popped ");
    
    // set up Cell containers for the neighborhood. One container
    // per adjacent RoI phi indices.
    auto neigh_cells = std::vector<std::vector<const CaloCell*>>(3);

    const CaloCell* max_cell{*(*max_row)};
    const auto  max_cell_eta = max_cell->eta();

    ATH_MSG_DEBUG("Got the max cell");
    
    for (std::size_t iv{0ul}; iv != close.size(); ++iv) {
      std::copy_if(std::begin(close[iv]),
		   std::end(close[iv]),
		   std::back_inserter(neigh_cells[iv]),
		   [&max_cell_eta, &half_deta_neigh](const auto& c){
		     return abs(c->eta()-max_cell_eta) < half_deta_neigh;
		   });
    }

    ATH_MSG_DEBUG("Made our neighbourhood");
    
    auto max_neigh_cell_it = std::find(std::begin(neigh_cells[1]),
				  std::end(neigh_cells[1]),
				  max_cell);
    if (max_neigh_cell_it == std::end(neigh_cells[1])){
      ATH_MSG_ERROR("Lost the max cell");
      return StatusCode::FAILURE;
    }

    auto max_neigh_cell_pos{std::distance(std::begin(neigh_cells[1]),
					  max_neigh_cell_it)};

    ATH_MSG_DEBUG("Rediscovered our neighbourhood max");
    
    auto toStripData = [](const auto& fromCells){
      auto stripdata = std::vector<StripData>();
      stripdata.reserve(fromCells.size());
      std::transform(std::begin(fromCells),
		     std::end(fromCells),
		     back_inserter(stripdata),
		     [](const auto& c) {
		       return StripData(c->eta(),
					c->phi(),
					c->e());});
      return stripdata;
    };	   

    auto low = toStripData(neigh_cells[0]);
    auto center = toStripData(neigh_cells[1]);
    auto high = toStripData(neigh_cells[2]);

    Coords roi_c{roi->eta(), roi->phi()};
    Coords cell_c{max_cell->eta(), max_cell->phi()};
    
    ATH_MSG_DEBUG("Fill with strip data");

    LArStripNeighborhood neighborhood = LArStripNeighborhood(low, center, high, roi_c, cell_c, max_neigh_cell_pos);
    
    neighborhoodTOBs.push_back(std::make_unique<IOBitwise::eEmNbhoodTOB>(*roi, neighborhood));
    
    return StatusCode::SUCCESS;
  }
}
