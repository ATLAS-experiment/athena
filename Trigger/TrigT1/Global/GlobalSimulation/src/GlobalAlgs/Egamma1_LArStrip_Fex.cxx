/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm outputs Calorimeter strip data in the region of
  eFex RoIs.
*/

#include "Egamma1_LArStrip_Fex.h"
#include "CaloEvent/CaloCell.h"

#include "../IO/LArStripNeighborhoodDumper.h"

#include "xAODEventInfo/EventInfo.h"

#include <fstream>
#include <vector>
#include <algorithm>
#include <optional>

namespace GlobalSim {

  std::optional<std::vector<std::size_t>> wrap3(std::size_t center) {
    if (center > 63) {
      return std::optional<std::vector<std::size_t>>{};
    }

    if (center == 63) {
      return std::make_optional(std::vector<std::size_t>({62ul, 63ul, 0ul}));
    }
    
    if (center == 0) {
      return std::make_optional(std::vector<std::size_t>({63ul, 0ul, 1ul}));
    }
    
    return std::make_optional(std::vector<std::size_t>(
						       {center-1,
							center,
							center+1}));
  }

  Egamma1_LArStrip_Fex::Egamma1_LArStrip_Fex(const std::string& name, ISvcLocator* pSvcLocator ) : 
    AthReentrantAlgorithm(name, pSvcLocator){
  }


  StatusCode Egamma1_LArStrip_Fex::initialize() {
    ATH_MSG_INFO ("Initializing " << name());

    CHECK(m_cellProducer.retrieve());
    CHECK(m_roiAlgTool.retrieve());
    CHECK(m_neighKey.initialize());
    CHECK(m_eventInfoKey.initialize());

  
    return StatusCode::SUCCESS;
  }


  StatusCode Egamma1_LArStrip_Fex::execute(const EventContext& ctx) const {
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
    ATH_MSG_DEBUG(cells.size() <<" cells read in");
    
    std::vector<const xAOD::eFexEMRoI*> rois;
    CHECK(m_roiAlgTool->RoIs(rois, ctx));
    ATH_MSG_DEBUG(rois.size() << " RoI(s) read in");

    // find cells in the neighborhood of RoIs.
    // A neighborhood is a collection of CellData objects which
    // contain cell eta, phi and Et.
    
    auto neighborhoods = std::make_unique<LArStripNeighborhoodContainer>();
    
    CHECK(findNeighborhoods(rois, cells, *neighborhoods));
    
    SG::WriteHandle<GlobalSim::LArStripNeighborhoodContainer> h_write(m_neighKey, ctx);

    auto dumper = GlobalSim::LArStripNeighborhoodDumper();
    if (m_dump) {
      CHECK(dumper.dump(name(), *eventInfo, *neighborhoods));
    }

    if (m_dumpTerse) {
      CHECK(dumper.dumpTerse(name(), *eventInfo, *neighborhoods));
    }
 
    
    CHECK(h_write.record(std::move(neighborhoods)));

    ATH_MSG_INFO(evtStore()->dump());
    
    return StatusCode::SUCCESS;
  }
  
  StatusCode
  Egamma1_LArStrip_Fex::findNeighborhoods(const std::vector<const xAOD::eFexEMRoI*>& rois,
					  const std::vector<const CaloCell*>& cells,
					  LArStripNeighborhoodContainer& neighborhoods) const{
    
    for (const auto& roi : rois) {
      CHECK(findNeighborhood(roi, cells, neighborhoods));
    }
    
    return StatusCode::SUCCESS;
  }
  

  StatusCode
  Egamma1_LArStrip_Fex::findNeighborhood(const xAOD::eFexEMRoI* roi,
					 const std::vector<const CaloCell*>& cells,
					 LArStripNeighborhoodContainer& neighborhoods) const {
    
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
    // 0.05 + 8.5* DeltaEta where DeltaEta = 0.0031, the nominal strip width
    // in eta. A cut-oof off eta = +-1.4 is applied in the selection.
    // 
    // Phi selection consists of calculating a phi index for the the RoI,
    // then requiring selected strips to be have a phi indix within +=1 of
    // the RoI phi index.
    //
    // The strips used to identigy the max energy strip have the same phi index
    // as the RoI, and eta that lies witun 0.1 in eta to the RoI.
    //
    // The strips selected for the Neighborhood have eta  within +- 8.5 DeltaEta
    // of the max energy strip, and +- 1 of the RoI strip phi Index.
 
    
    
    
    auto cells_near_roi = std::vector<const CaloCell*>();

    // lambda function to calculate the phi index of a eta-phi point
    auto phi_ind = [](const auto& c) {
      constexpr double dphi = std::numbers::pi/32;
      std::size_t iphi = 32 + int(std::floor(c->phi()/dphi));
      return iphi > 63 ?
	std::optional<std::size_t>() : std::make_optional(iphi);
    };

    // calculate roi phi index
    auto roi_phi_index = *phi_ind(roi);

    // obtain adjacent phi indices
    auto roi_phi_indices = *wrap3(roi_phi_index);
    for (const auto& i : roi_phi_indices) {
      ATH_MSG_DEBUG("roi index " << i);
    }

    // container for strips close to RoI in eta
    auto close = std::vector<std::vector<const CaloCell*>>(3);
    for (auto& v :close) {v.reserve(100);}
    
    
    constexpr double half_deta_roi{0.05};
    constexpr double half_deta_neigh{8.5*0.0031};
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
    
    
    for (const auto& v : close) {ATH_MSG_DEBUG("size close " << v.size());}

    // select the cells within a a tower width of the RoI. Then find the
    // cell in this selction with the highest energy
    auto roi_cells = std::vector<const CaloCell*> ();
    roi_cells.reserve(close[1].size());
    std::copy_if(std::begin(close[1]),
		 std::end(close[1]),
		 std::back_inserter(roi_cells),
		 [&roi](const auto& c) {
		   return std::abs(c->eta() - roi->eta()) < half_deta_roi;
		 });
    
    
    auto it = std::max_element(std::begin(roi_cells),
			       std::end(roi_cells),
			       [](const auto& l,const auto& r) {
				 return l->e() < r->e();
			       });
    
    if (it == std::end(roi_cells)) {
      ATH_MSG_ERROR("max_cell not found");
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("max cell pos "
		  << std::distance(std::begin(roi_cells), it)
		  << ' ' << **it);

    // set up Cell containers for the neighborhood. One container
    // per adjacent RoI phi indices.
    auto neigh_cells = std::vector<std::vector<const CaloCell*>>(3);

    const CaloCell* max_cell{*it};
    const auto  max_cell_eta = max_cell->eta();
    
    
    for (std::size_t iv{0ul}; iv != close.size(); ++iv) {
      std::copy_if(std::begin(close[iv]),
		   std::end(close[iv]),
		   std::back_inserter(neigh_cells[iv]),
		   [&max_cell_eta, &half_deta_neigh](const auto& c){
		     return abs(c->eta()-max_cell_eta) < half_deta_neigh;
		   });
    }

    auto max_neigh_cell_it = std::find(std::begin(neigh_cells[1]),
				  std::end(neigh_cells[1]),
				  max_cell);
    if (max_neigh_cell_it == std::end(neigh_cells[1])){
      ATH_MSG_ERROR("Lost the max cell");
      return StatusCode::FAILURE;
    }

    auto max_neigh_cell_pos{std::distance(std::begin(neigh_cells[1]),
					  max_neigh_cell_it)};

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
    
    
    neighborhoods.push_back(std::make_unique<LArStripNeighborhood>(low,
								   center,
								   high,
								   roi_c,
								   cell_c,
								   max_neigh_cell_pos));
    
    return StatusCode::SUCCESS;
  }
  
}
