/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm creates clusters from CaloCells, and writes them out
   as Caloclusters. The clustering strategy is carried out by helper objects.
   The strategy used is chosen according to string set at configure time. *
*/

#include "./GepPi0Alg.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include <algorithm>
#include <queue>
#include <regex>
#include <sstream>
#include <fstream>

GepPi0Alg::GepPi0Alg(const std::string& name, ISvcLocator* pSvcLocator ) : 
  AthReentrantAlgorithm(name, pSvcLocator){
}


StatusCode GepPi0Alg::initialize() {
  ATH_MSG_INFO ("Initializing " << name());

  ATH_CHECK(m_totalNoiseKey.initialize());
  ATH_CHECK(m_caloMgrKey.initialize());
  CHECK(m_allCaloCellsKey.initialize());
  
  CHECK(m_cellsProducer.retrieve());
  
  CHECK(detStore()->retrieve (m_calocell_id, "CaloCell_ID"));

  return StatusCode::SUCCESS;
}


StatusCode GepPi0Alg::execute(const EventContext& ctx) const {
  // Read in a CaloCell container. Ask producers to create
  // vectors of CaloCells to be examined.

  ATH_MSG_DEBUG ("Executing " << name());

  // obtain all LAREM CaloCells, then those in EMB1
  auto h_allCaloCells = SG::makeHandle(m_allCaloCellsKey, ctx);
  CHECK(h_allCaloCells.isValid());
  
  const auto & allCaloCells = *h_allCaloCells;

 

  // obatain LAREM CaloCells
  if(! allCaloCells.hasCalo(m_calocell_id->LAREM)){
    ATH_MSG_ERROR("CaloCellCollection does not contain LAREM cells");
    return StatusCode::FAILURE;
  }

  auto cellbegin = allCaloCells.beginConstCalo(CaloCell_ID::LAREM);
  auto cellend = allCaloCells.endConstCalo(CaloCell_ID::LAREM);
  ATH_MSG_DEBUG ("Size from SubCalo Iters " << std::distance(cellbegin,
							     cellend));

  std::vector<const CaloCell*>
    laremCells(allCaloCells.beginConstCalo(CaloCell_ID::LAREM),
	       allCaloCells.endConstCalo(CaloCell_ID::LAREM));

  // Simple checks on the LAREM cell hashes. Will use laremCells vector to
  // obtain CaloCells goven their hashes. 
  auto front_idhash = m_calocell_id->calo_cell_hash(laremCells.front()->ID());
  auto back_idhash = m_calocell_id->calo_cell_hash(laremCells.back()->ID());

  // LAREM cell hashes go from 0-173311. check that this is the case
  // and thar the cells are in hash oeder to allow for indexing.

  constexpr unsigned int LAREMHASH_LO{0};
 
  if (front_idhash != LAREMHASH_LO) {
    ATH_MSG_ERROR("front LARM identifier hash != 0, is " << front_idhash);
    return StatusCode::FAILURE;
  }

  constexpr unsigned int LAREMHASH_HI{173311};

  if (back_idhash != LAREMHASH_HI) {
    ATH_MSG_ERROR("back LARM identifier hash != 173311, is " << back_idhash);
    return StatusCode::FAILURE;
  }

  if(!std::is_sorted(laremCells.begin(),
		     laremCells.end(),
		     [&cc_id=m_calocell_id](const auto& l, const auto& r) {
		       return
			 cc_id->calo_cell_hash(l->ID()) <
			 cc_id->calo_cell_hash(r->ID());
		     })){
								   
    ATH_MSG_ERROR("LARM CaloCells are not sorted by identifier hash");
    return StatusCode::FAILURE;
  }
		     
  
  ATH_MSG_DEBUG("LAREM front, back, diff  idhash v size " << 
		front_idhash << " " << 
		back_idhash << " " << 
		back_idhash-front_idhash << " " <<
		laremCells.size()
		);


  // Now select the cells in the EMB1 sampling layer from laremCells.
  std::vector<const CaloCell*> allEMB1Cells;

  auto EMB1Selector = [&calocell_id=m_calocell_id](const auto& cell) {
    auto ha = calocell_id->calo_cell_hash(cell->ID());
    return calocell_id->calo_sample(ha) == CaloCell_Base_ID::EMB1;
  };

  std::copy_if(allCaloCells.beginConstCalo(CaloCell_ID::LAREM),
	       allCaloCells.endConstCalo(CaloCell_ID::LAREM),
	       std::back_inserter(allEMB1Cells),
	       EMB1Selector);

  
  SG::ReadCondHandle<CaloNoise> totalNoiseHdl{m_totalNoiseKey, ctx};
  if (!totalNoiseHdl.isValid()) {return StatusCode::FAILURE;}
  const CaloNoise* totalNoiseCDO = *totalNoiseHdl;
 

  // ask tool for emb1CellVecs. The length of emb1CellVecs varies according
  // to the concrete tool. Examples: If the tool obtains all cells, then the
  // length of calCellsVecs is 1. If the tool obtains CellCollections
  // from clusters, then the number of vectors will be equal to the
  // number of clusters.

  std::vector<std::vector<const CaloCell*>> emb1CellVecs;

  CHECK(m_cellsProducer->cells(emb1CellVecs, ctx));
  
  // check cells in the EMB1 layer

  for (const auto& cvec : emb1CellVecs) {
    if (std::find_if_not(cvec.cbegin(),
			 cvec.cend(),
			 EMB1Selector) != cvec.cend()){
      ATH_MSG_ERROR("cell is not in  EMB1  ");
      return StatusCode::FAILURE;
    }
  }

  // one seed cell per emb1Cell collection
  std::vector<const CaloCell*> seeds(emb1CellVecs.size());

  auto signif = [&totalNoiseCDO,
		 &cut=m_seed_signifcut](const auto& c) -> float {
    float noise = totalNoiseCDO->getNoise(c->ID(),
					  c->gain());
    auto sfc = c->e()/noise;
    return sfc < cut ? 0.:sfc; 
  };
    
  auto findSeed = [&signif](const auto& cellVec) -> const CaloCell* {
    return *std::max_element(cellVec.cbegin(),
			     cellVec.cend(),
			     [&signif](const auto& c1,
				       const auto& c2) {
			       return signif(c1) < signif(c2);});};


  for (const auto& v: emb1CellVecs) {
    ATH_MSG_DEBUG("Looking for seed in " << v.size() << " cells");
  }

  std::transform(emb1CellVecs.cbegin(),
		 emb1CellVecs.cend(),
		 seeds.begin(),
		 findSeed);

  std::size_t iseed{0};
  for(const auto& seed : seeds) {
    
    ATH_MSG_DEBUG("seed: eta "<< seed->eta()
		  << " phi " << seed->phi()
		  << " significance "<< signif(seed)
		  );

    SG::ReadCondHandle<CaloDetDescrManager>
      caloMgrHandle{m_caloMgrKey, ctx};
    
    ATH_CHECK(caloMgrHandle.isValid());
    const CaloDetDescrManager* caloMgr = *caloMgrHandle;

    // run the requested algorithms
    for (const auto& strategy : m_strategies) {
      if (strategy == "TWINPEAKS") {
	try {
	  CHECK(twinpeaks_strategy(seed,
				   laremCells,
				   allEMB1Cells,
				   iseed,
				   totalNoiseCDO,
				   caloMgr));
	} catch (std::runtime_error& e) {
	  ATH_MSG_ERROR("Error running twin peaks - " << e.what());
	  return StatusCode::FAILURE;
	}
      } else if (strategy == "SORT") {
	CHECK(sort_strategy(seed,
			    laremCells,
			    allEMB1Cells,
			    iseed,
			    totalNoiseCDO,
			    caloMgr));
      } else if (strategy == "CRAWL") {
	try {
	  CHECK(crawl_strategy(seed,
			       laremCells,
			       allEMB1Cells,
			       iseed,
			       totalNoiseCDO,
			       caloMgr));
	  
	} catch (std::runtime_error& e) {
	  ATH_MSG_ERROR("Error running crawl: " << e.what());
	  return StatusCode::FAILURE;
	}
      } else {
	ATH_MSG_ERROR ("Unknown pi0 strategy " << strategy);
	return StatusCode::FAILURE;
      }
    }
    ++iseed;
  }
  ++m_id;
  return StatusCode::SUCCESS;
}


StatusCode
GepPi0Alg::twinpeaks_strategy(const CaloCell* seed,
			      const std::vector<const CaloCell*>& laremCells,
			      const std::vector<const CaloCell*>& allEMB1Cells,
			      std::size_t iseed,
			      const CaloNoise* totalNoiseCDO,
			      const CaloDetDescrManager* caloMgr) const {

  // look for a local maximum in a neighborhood
  // the siginicance (e/noise) of a maximum must be over a threshold
  // (for now, set to 2.0)
  ATH_MSG_DEBUG("twin peaks - start");
  
  
  if (!seed) {return StatusCode::SUCCESS;}
  if (allEMB1Cells.empty()) {return StatusCode::SUCCESS;}
  
  
  // find the neighborhood as a collection of Identifiers
  std::vector<const CaloCell*> neigh_cells;
  CHECK(neighborhood(seed, laremCells, neigh_cells, caloMgr));

  
    
  ATH_MSG_DEBUG("twin peaks - neighborhood cells size " << neigh_cells.size()
		<< " input cell collection size " << allEMB1Cells.size());
  
  
  auto signif = [&totalNoiseCDO](const auto& cell){
    return cell->e()/totalNoiseCDO->getNoise(cell->ID(), cell->gain());
  };
  
  // apply a cut on the signiificance
  auto new_end = std::partition(neigh_cells.begin(),
				neigh_cells.end(),
				[&signif,
				 &signif_cut=m_tp_signifcut](const auto& ncell){
				  return signif(ncell) > signif_cut;
				});
  
  neigh_cells.resize(new_end - neigh_cells.begin());
  
  ATH_MSG_DEBUG("twin peaks -  neighborhood size after signif cut: "
		<< neigh_cells.size());
  
  
  // ---------
  // create a lambda to look for significant cells in the neighborhood
  // which are adjacent to a choosen cell (center_cell).
  auto is_local_max = [neigh_cells,  // copy
		       &cc_id=m_calocell_id,
		       &signif](const auto& center_cell) mutable {
    
    // obtain the hashes of the cells adjacent to center_cell.
    // some of these will be in the neighborhood. For cells
    // at the edge of the neighborhood, there may be
    // adjacent cells not in the neighborhood. For now,
    // such extra-neighborhood cells will be ignored.
    
    std::vector<IdentifierHash> adj_hashes;
    auto rc = cc_id->get_neighbours(cc_id->calo_cell_hash(center_cell->ID()), 
				    LArNeighbours::all2D,
				    adj_hashes);
    if (rc != 0) {
      throw std::runtime_error("twin peaks - error obtaining cell neighbors");
    }
    
    // obtain the identifiers of cells immediately adjacent to center_cell
    std::vector<Identifier> adj_idents;;
    std::transform(adj_hashes.cbegin(),
		   adj_hashes.cend(),
		   std::back_inserter(adj_idents),
		   [&cc_id](const auto& hash){
		     return cc_id->cell_id(hash);
		   });

    // obtain neighborhood cells immediately adjacent to center_cell

    auto id_cell_match = [&adj_idents](const CaloCell* n_cell)->bool {
      return std::find(adj_idents.begin(),
		       adj_idents.end(),
		       n_cell->ID()) != adj_idents.end();};

    auto adj_neigh_cell_end = std::partition(neigh_cells.begin(),
					     neigh_cells.end(),
					     id_cell_match);

    for (auto iter = neigh_cells.begin();
	 iter != adj_neigh_cell_end;
	 ++iter) {
      if (signif(*iter) > signif(center_cell)) {return false;}
    }

    return true;
  };
  

  auto peaks_end = std::partition(neigh_cells.begin(),
				  neigh_cells.end(),
				  std::move(is_local_max));


  auto n_peaks = peaks_end - neigh_cells.begin();
  ATH_MSG_DEBUG("twin peaks - number of local maxima " << n_peaks);

  auto n_peaks_noseed = n_peaks;
  if (std::find(neigh_cells.begin(), peaks_end, seed) != peaks_end){--n_peaks_noseed;} 
  ATH_MSG_DEBUG("twin peaks - number of local maxima ommiting seed " << n_peaks_noseed);

  ATH_MSG_DEBUG("twin peaks - seed eta " << seed->eta()
		<< " phi " << seed->phi()
		<< " signif " <<signif(seed));
  for (auto iter = neigh_cells.begin(); iter != peaks_end; ++iter) {
    ATH_MSG_DEBUG("  twin peaks - peak eta " << (*iter)->eta()
		  << " phi " << (*iter)->phi()
		  << " signif " <<signif(*iter));
    
    ATH_MSG_DEBUG("  twin peaks - distance from seed  deta "
		  << seed->eta() - (*iter)->eta() << " dphi "
		  << seed->phi() - (*iter)->phi());
  }

  ATH_MSG_DEBUG("twin peaks: neighborhood size after signif cut "
		<< neigh_cells.size()); 


  for (const auto& c: neigh_cells) {
    if (!c) {throw std::runtime_error("Neighbor pointer is nullptr");}
    
    
    ATH_MSG_DEBUG("twin peaks - neigh cell eta " << c->eta()
		  << " phi " << c->phi()
		  << " signif " << signif(c));
  }


  
  if (m_dump) {

    std::vector<const CaloCell*> peaks_cells(neigh_cells.begin(), peaks_end);

    std::vector<double> peaks_signif;
    std::transform(neigh_cells.begin(),
		   peaks_end,
		   std::back_inserter(peaks_signif),
		   signif);
    
    CHECK(dump_twinpeaks(iseed,
			 seed,
			 signif(seed),
			 neigh_cells,
			 peaks_cells,
			 peaks_signif));
  }
  
  ATH_MSG_DEBUG("twin peaks - finished");
  
  return StatusCode::SUCCESS;
}


StatusCode
GepPi0Alg::sort_strategy(const CaloCell* seed,
			 const std::vector<const CaloCell*>& laremCells,
			 const std::vector<const CaloCell*>& allEMB1Cells,
			 std::size_t iseed,
			 const CaloNoise*,
			 const CaloDetDescrManager* caloMgr) const {
  
  if (!seed) {return StatusCode::SUCCESS;}
  if (allEMB1Cells.empty()) {return StatusCode::SUCCESS;}

  std::vector<const CaloCell*> neigh_cells;
  CHECK(neighborhood(seed, laremCells, neigh_cells, caloMgr));
  
  ATH_MSG_DEBUG("sort_strategy  seed idx " << iseed <<
		"nbhd size :" << neigh_cells.size());

  
  return StatusCode::SUCCESS;
}

StatusCode
GepPi0Alg::crawl_strategy(const CaloCell* seed,
			  const std::vector<const CaloCell*>& laremCells,
			  const std::vector<const CaloCell*>& allEMB1Cells,
			  std::size_t iseed,
			  const CaloNoise* totalNoiseCDO,
			  const CaloDetDescrManager* caloMgr) const {

  if (!seed) {return StatusCode::SUCCESS;}
  if (allEMB1Cells.empty()) {return StatusCode::SUCCESS;}


  // path_str is a string composed of '0' and '1' characters.
  // '0' indicates an energy decreases
  // '1' indicates the energy increases along the path.

  // a path is calculated for a number of contiguous eta steps.
  // for each eta value, a path is formed in the two phi directions.
  //
  // The first step is to determine the CaloCell hashes along
  // the paths, starting from the location of the seed cell.

  ATH_MSG_DEBUG("crawl: start");
  ATH_MSG_DEBUG("crawl - seed eta " << seed->eta()
		<< " phi " << seed->phi());

  std::vector<std::string> paths(2*m_er_neta +1);

  auto seed_hash = m_calocell_id->calo_cell_hash(seed->ID());

  std::vector<IdentifierHash> phi_starts;

  // start point for +- phi paths about the seed eta value
  phi_starts.push_back(seed_hash);

  
  std::vector<LArNeighbours::neighbourOption>
    dirs {LArNeighbours::prevInEta, LArNeighbours::nextInEta};

  // start point for +- phi paths above and below the seed eta

  for (const auto& dir : dirs) {
    auto cur_hash = seed_hash;
    for (int i = 0; i < m_er_neta; ++i){
      std::vector<IdentifierHash> v_adj;
      int rc = m_calocell_id->get_neighbours(cur_hash,
					     dir,
					     v_adj);
      if (rc != 0) {break;} // out of bounds

      if (v_adj.size() != 1) {
	// this happens when the the cells no longer form an regular
	// array. Emprically: at around eta = +- 1.4, the phi spacing
	// becomes small.
	// The eta of these cells are almost, but not exactly the same.
	// The get_neighbours returns multiple cell identifiers in this case.
	//
	// When this happens, issue a warning, and break of search in this
	// eta direction for cells to start the phi crawl.
	for (const auto& ha : v_adj) {
	  const auto *dde = caloMgr->get_element(m_calocell_id->cell_id(ha));
	  ATH_MSG_DEBUG("cell eta " << dde-> eta() << " phi " << dde->phi() <<
			" z " << dde->z());
	}
	ATH_MSG_WARNING("unexpected number of identifier hashes: "
			<< v_adj.size());
	
	break;
      }

      cur_hash = v_adj[0];
      phi_starts.push_back(cur_hash);
    }
  }

  ATH_MSG_DEBUG("crawl - number of 1-dim path start points "
		<< phi_starts.size());
  for (const auto& h: phi_starts) {
    auto eta = caloMgr->get_element(m_calocell_id->cell_id(h))->eta();
    ATH_MSG_DEBUG ("crawl - starting etas " << h << " eta " << eta);
  }

  std::vector<std::vector<IdentifierHash>> paths_hash;
  dirs =  {LArNeighbours::prevInPhi,
	   LArNeighbours::nextInPhi};
  
  for (const auto& dir : dirs){

    auto calc_phihashes = [&dir,
			   &cc_id=m_calocell_id,
			   nphi=m_er_nphi](auto hash){ //hash: pass by value
      std::vector<IdentifierHash> path;
      std::vector<IdentifierHash> adjacent;
      path.push_back(hash);
      for (int i = 0; i<nphi; ++i) {
	
	int rc = cc_id->get_neighbours(hash, dir, adjacent);
	if (rc != 0) {break;} // out of bounds
	auto size = adjacent.size();
	if (size != 1) {
	  throw std::runtime_error("unexpected number of identifier hashes: " + std::to_string(size));
	}

	hash = adjacent[0];
	path.push_back(hash);
	
      }
      return path;
    };
    
    std::transform(phi_starts.cbegin(),
		   phi_starts.cend(),
		   std::back_inserter(paths_hash),
		   std::move(calc_phihashes));
  };

   ATH_MSG_DEBUG("crawl - number of 1-dim paths (hashes) "
		<< paths_hash.size());

   // step 2 - obtain the paths in terms of CaloCells
   // std::vector<std::vector<const CaloCell*>> paths_cell;
   CellVectors  paths_cell;
   paths_cell.reserve(paths_hash.size());

   
  for (const auto& p_h : paths_hash) {
    std::vector<const CaloCell*> path;
    path.reserve(p_h.size());
    std::transform(p_h.cbegin(), p_h.cend(),
		   std::back_inserter(path),
		   [&laremCells](const auto& hash){
		     return laremCells.at(hash);});
    paths_cell.push_back(std::move(path));
  }


  
  // now have paths of cells.
  // step 3: create strings of energy up-down patterns


  auto signif = [&totalNoiseCDO,
		 &signif_min = m_crawl_signifcut](const auto& cell){
    auto e = cell->e();
    if (e <= 0){return 0.;}
    auto signif = cell->e()/totalNoiseCDO->getNoise(cell->ID(), cell->gain());
    return signif < signif_min ? 0:signif;
  };


  // std::vector<std::vector<float>> paths_signif;
  PathsSignif paths_signif;
  paths_signif.reserve(paths_cell.size());

  // should probably set signif to 0 if below a threhold (eg 2.)
  for (const auto& p_c : paths_cell) {
    std::vector<float> path_signif;
    std::transform(p_c.cbegin(),
		   p_c.cend(),
		   std::back_inserter(path_signif),
		   signif);
    paths_signif.push_back(std::move(path_signif));
  }

  for (const auto& p_s: paths_signif) {
    std::stringstream ss;
    ss <<"crawl - path signf ";
    for (const auto& s : p_s) {
      ss << std::setw(7) << std::setprecision(3)<< s << " ";
    }
    ATH_MSG_DEBUG (ss.str());
  }

  
  std::vector<std::string> paths_pat;
  paths_signif.reserve(paths_signif.size());

  for (const auto& p_s: paths_signif) {
    std::string pat{"0"}; // padding to keep the string size as path size
    if (!p_s.empty()) {
      for (std::size_t i = 1; i != p_s.size(); ++i) {
	if (p_s[i] > p_s[i-1]) {
	  pat += '1';
	} else {
	  pat += '0';
	}
      }

      paths_pat.push_back(std::move(pat));
    }
  }
  

  ATH_MSG_DEBUG("crawl - no of  patterns " << paths_pat.size());

  for(const auto& p: paths_pat) {ATH_MSG_DEBUG("crawl - pattern " << p);}

  std::vector<std::regex> peaks_re{
    std::regex("01*(1)0"), std::regex("01*(1)$")};
  
  std::vector<const CaloCell*> peak_cells;
  std::size_t idx{0};

  for (const auto& s : paths_pat) {

    for (const auto& regex : peaks_re) {
      std::smatch m;
      if(std::regex_search(s, m, regex)){
	auto pos = m.position(1);
	ATH_MSG_DEBUG("crawl - match at idx " << idx << " pos " << pos);
	peak_cells.push_back(paths_cell[idx][pos]);
	break;
      }
    }
    ++idx;
  }


  auto n_peaks = peak_cells.size();
  ATH_MSG_DEBUG("crawl - number of local maxima " << n_peaks);
  ATH_MSG_DEBUG("crawl - seed eta " << seed->eta()
		<< " phi " << seed->phi()
		<< " signif " <<signif(seed));
  for (const auto& pc : peak_cells){
    ATH_MSG_DEBUG("  crawl - peak eta " << pc->eta()
		  << " phi " << pc->phi()
		  << " signif " <<signif(pc));
    
    ATH_MSG_DEBUG("  crawl - distance from seed  deta "
		  << seed->eta() - pc->eta() << " dphi "
		  << seed->phi() - pc->phi());
  }

  if (m_dump) {CHECK(dump_crawl(iseed,
				seed,
				signif(seed),
				paths_cell,
				paths_signif,
				paths_pat));}
  
  ATH_MSG_DEBUG("crawl: finished");
  
  return StatusCode::SUCCESS;
}

StatusCode
GepPi0Alg::dump_crawl(std::size_t i,
		      const CaloCell* seed,
		      float seed_signif,
		      const CellVectors& paths_cell,
		      const std::vector<std::vector<float>>& signifs,
		      const std::vector<std::string>& paths_pat) const {
  std::stringstream ss;
  ss << "crawl. cell source " << i << '\n';
  ss << "seed " << seed->eta() << " " << seed->phi() << " "
     << seed->e() << " " << seed_signif << '\n';
  ss << "path etas:\n";
  for (const auto& path: paths_cell) {
    for (const auto& c : path){
      ss << c->eta() << " ";
    }
    ss << '\n';
  }
  ss << "path phis:\n";
  for (const auto& path: paths_cell) {
    for (const auto& c : path){
      ss << c->phi() << " ";
    }
    ss << '\n';
  }
  
  ss << "path es:\n";
  for (const auto& path: paths_cell) {
    for (const auto& c : path){
      ss << c->e() << " ";
    }
    ss << '\n';
  }
  
  ss << "path signifs:\n";
  for (const auto& path: signifs) {
    for (const auto& sig : path){
      ss << sig << " ";
    }
    ss << '\n';
  }

  ss << "path patterns:\n";
  for (const auto& pat: paths_pat) {
    ss << pat << '\n';
  }


  std::stringstream fn;
  fn << "crawl_" << m_id << "_" << i << ".txt";
  std::ofstream out(fn.str());
  out << ss.str();

  return StatusCode::SUCCESS;
}


StatusCode
GepPi0Alg::dump_twinpeaks(std::size_t i,
			  const CaloCell* seed,
			  double seed_signif,
			  const std::vector<const CaloCell*>& neighborhood,
			  const std::vector<const CaloCell*>& peaks,
			  const std::vector<double>& peak_signifs) const {
  std::stringstream ss;
  ss << "twinpeaks. cell source " << i << '\n';
  ss << "seed " << seed->eta() << " " << seed->phi() << " "
     << seed->e() << " " << seed_signif << '\n';
  ss << "neighborhood etas:\n";
  for (const auto& c: neighborhood) {
    ss << c->eta() << " ";
  }
  ss << '\n';

  ss << "neighborhood phis:\n";
  for (const auto& c : neighborhood){
    ss << c->phi() << " ";
  }
  ss << '\n';
  
  ss << "neighborhood es:\n";
  for (const auto& c : neighborhood){
    ss << c->e() << " ";
  }
  ss << '\n';
  
  ss << "peak etas:\n";
  for (const auto& c : peaks){
    ss << c->eta() << " ";
  }
  ss << '\n';

  ss << "peak phis:\n";
  for (const auto& c : peaks){
    ss << c->phi() << " ";
  }
  ss << '\n';

  ss << "peak es:\n";
  for (const auto& c : peaks){
    ss << c->e() << " ";
  }
  ss << '\n';


  ss << "peak signifs:\n";
  for (const auto& s : peak_signifs){
    ss << s << " ";
  }
  ss << '\n';

  std::stringstream fn;
  fn << "twinpeak_" << m_id << "_" << i << ".txt";
  std::ofstream out(fn.str());
  out << ss.str();

  return StatusCode::SUCCESS;
}
  


StatusCode
GepPi0Alg::neighborhood(const CaloCell* seed,
			const std::vector<const CaloCell*>& laremCells,
			std::vector<const CaloCell*>& neigh_cells,
			const CaloDetDescrManager* caloMgr) const {

  // find the cells in the neighborhood od a seed cell. The neighborhood
  // cells are those within a Euclidean distance in eta-phi of the seed.
  // The algorithm works by iteratively finding immediately adjacent cells
  // to cells already established as being in the neighborhood. If these
  // cells are within range, they are added to the neighborhood

  auto seedID = seed->ID();

  auto seed_eta = seed->eta();
  auto seed_phi = seed->phi();
  
  std::queue<Identifier> to_process;
  to_process.push(seedID);

  auto accept = [&seed_eta, &seed_phi,
		 &calocell_id=m_calocell_id,
		 &neigh_half_eta = m_neigh_half_eta,
		 &neigh_half_phi = m_neigh_half_phi,
		 &caloMgr](const auto& c_id){
    auto dde = caloMgr->get_element(c_id);
    auto c_eta = dde->eta();
    auto c_phi = dde->phi();

    auto hash = calocell_id->calo_cell_hash(c_id);
    if (calocell_id->calo_sample(hash) != CaloCell_Base_ID::EMB1) {return false;}

    auto deta = seed_eta-c_eta;
    if( std::abs(deta) >  neigh_half_eta) {return false;}

    auto dphi = seed_phi-c_phi;
    return std::abs(dphi) < neigh_half_phi;

  };

  std::vector<Identifier> mask;
  auto add_neighbors_to_queue = [&to_process,
			&cc_id=m_calocell_id,
			&mask] (const auto& desc) {
    auto hash = cc_id->calo_cell_hash(desc);
    std::vector<IdentifierHash> neigh_hashes;
    int rc = cc_id->get_neighbours(hash,
				   LArNeighbours::all2D,
				   neigh_hashes);
    if (rc != 0) {
      return StatusCode::FAILURE;
    }
    
    for (const auto& nh :neigh_hashes) {
      auto cid = cc_id->cell_id(nh);
      if (std::find(mask.cbegin(), mask.cend(), cid) == mask.cend()) {
	mask.push_back(cid);
	to_process.push(cc_id->cell_id(nh));
      }
    }
    return StatusCode::SUCCESS;
 
  };

  CHECK(add_neighbors_to_queue(seedID));
  std::vector<Identifier> neigh_idents;

  // iterative procedure:
  while (!to_process.empty()){
    const auto& ident = to_process.front();
    if (accept(ident)){
      ATH_MSG_DEBUG ("  accepting neigh " << ident);
      const auto *dde = caloMgr->get_element(ident);
      auto c_eta = dde->eta();
      auto c_phi = dde->phi();
      ATH_MSG_DEBUG("   neigh eta: " << c_eta
		    << " phi " << c_phi);
      neigh_idents.push_back(ident);
      add_neighbors_to_queue(ident);
    }
    to_process.pop();
  }

  std::sort(neigh_idents.begin(), neigh_idents.end());
  auto end = std::unique(neigh_idents.begin(), neigh_idents.end());
  auto it{neigh_idents.begin()};
  
  for(;it!= end; ++it){
    neigh_cells.push_back(laremCells.at(m_calocell_id->calo_cell_hash(*it)));
  }
  return StatusCode::SUCCESS;
}
