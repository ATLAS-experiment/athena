/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_GEPPI0ALG_H
#define TRIGL0GEPPERF_GEPPI0ALG_H

/*
  This algorithm provides a framework in which to study alternative
  pi0 detection algorithms or, more precisely, algorithms which take
  a seed cell and a neighborhood of cells all lying in the EMB1 sampling
  layer of the LArEM calorimeter.

  The AlgTool delivers cell collections to the Algorithm. There may
  a single collection if all the cells are read in, or many such collections
  as would happen when a cell collection is produced for each CaloCluster
  in the event.

  A seed cell for a each input cell collection is identified.

  The algorithm also reads in all Calorimeter Cells.

  The algorithms are implemented in separate merthods. They receive
  as inputs the seeds, and the EMB1  cells.

  Output is currently done in debug mode (m_debug = true) by outputing
  text files.

*/

#include "./ICaloCellsProducer.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "CaloConditions/CaloNoise.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "Identifier/Identifier.h"
#include <vector>

class GepPi0Alg: public AthReentrantAlgorithm { 
 public:

  typedef std::vector<std::vector<const CaloCell*>> CellVectors;
  typedef std::vector<std::vector<float>> PathsSignif;

  
  GepPi0Alg(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode  initialize() override;   
  virtual StatusCode  execute(const EventContext& ) const override;    

 private: 

  // label for debug output
  mutable std::atomic<size_t> m_id{0l};
  
  // key to get all calo cells
  SG::ReadHandleKey<CaloCellContainer> m_allCaloCellsKey {
    this, "caloCells", "AllCalo", "key to read in a CaloCell constainer"};

  // tool to get vector of vector of cal cells, eg one vector per cluster
  ToolHandle<ICaloCellsProducer> m_cellsProducer{this,
      "caloCellsProducer",
      "EMB1CellsFromCaloCells",
      "AlgTool to provide vectors of CaloCells ot GepPi0Alg"};

  // Provide access to a noise tool. Allows calculation of S/N for a CaloCell
  SG::ReadCondHandleKey<CaloNoise>
  m_totalNoiseKey{this,
		  "totalNoiseKey",
		  "totalNoise",
		  "SG Key of CaloNoise data object"};

  // CaloCell_ID helps with with intercell navigation
  const CaloCell_ID* m_calocell_id{nullptr};
  
  // CaloDetDescrMAnager provides descriptors with cell geometry information.
    SG::ReadCondHandleKey<CaloDetDescrManager>
  m_caloMgrKey{this,
	       "caloDetDescrManager",
	       "CaloDetDescrManager",
	       "SG Key of the CaloDetDescrManager in  the Consition Store"};

  // Neighborhood generation
  // 0.25: about seed strip +- 1 strip in eta
  Gaudi::Property<float> m_neigh_half_eta{
    "neigh_half_eta",
    {0.004},
    "+- neighborhood extent in eta"};
  
  // 0.1: about seed strip +- 6 strips in phi
  Gaudi::Property<float> m_neigh_half_phi{
    this,
    "neigh_half_phi",
    {0.6},
    "+- neighborhood extent in phi"};

  // Allow the user to set the desired algorithm(s)
  Gaudi::Property<std::vector<std::string>> m_strategies{
    this,
    "pi0strategies",
    {"TWINPEAKS", "CRAWL"},
    "list of pi0 detection strategies to be run"};

  // paramaters for seed cell identification
  Gaudi::Property<float> m_seed_signifcut{
    this, "seed_signif_cut", {2.0}, "twin peak min peak e/noise"};


  // parameters for crawl algorithm
  Gaudi::Property<float> m_tp_signifcut{
    this, "tp_signif_cut", {2.0}, "twin peak min peak e/noise"};

  Gaudi::Property<float> m_crawl_signifcut{
    this, "crawl_signif_cut", {2.0}, "crawl min peak e/noise"};
  
  Gaudi::Property<int> m_er_neta{
    this, "er_neta", {1}, "crawl number of steps eta"};

  Gaudi::Property<int> m_er_nphi{
    this, "er_nphi", {10}, "crawl number of steps phi"};

  
  Gaudi::Property<bool> m_dump{
    this, "dump", false, "flag to trigger writing out debug information"};


  // The different algorithms are encapsulated in private methods
  // twin peaks exhaustively looks for local peaks in the neighborhood
  // of a seed
  StatusCode twinpeaks_strategy(const CaloCell* seed,
				const std::vector<const CaloCell*>& laremCells,
				const std::vector<const CaloCell*>& emb1Cells,
				std::size_t iseed,
				const CaloNoise*,
				const CaloDetDescrManager*
				) const;

  // sort will sort the cells in a neighborhood of a seed by e, and examine
  // this for cells which are local maxima.

  StatusCode sort_strategy(const CaloCell* seed,
			   const std::vector<const CaloCell*>& laremCells,
			   const std::vector<const CaloCell*>& emb1Cells,
			   std::size_t iseed,
			   const CaloNoise*,
			   const CaloDetDescrManager*) const;

  // the crawl algorithm finds the binary string found by comparing the
  // energy of at location along a path with the previous location.
  // The strings are then tested against a regex to determine whether
  // a particular pattern is present. The default regex identifies '010'
  // within the string, or '01' at the end of the string.
  
  StatusCode crawl_strategy(const CaloCell* seed,
			    const std::vector<const CaloCell*>& laremCells,
			    const std::vector<const CaloCell*>& emb1Cells,
			    std::size_t iseed,
			    const CaloNoise*,
			    const CaloDetDescrManager*) const;


  

  // Find a list of cell identifiers in the neighborhood of a seed cell.
  // The neighborhood is rectangular in eta-phi.
  StatusCode neighborhood(const CaloCell*,
			   const std::vector<const CaloCell*>& laremCells,
			  std::vector<const CaloCell*>& neighs,
			  const CaloDetDescrManager*) const;

  // methods to output details of the different algorithms
  StatusCode dump_crawl(std::size_t iseed,
			const CaloCell* seed,
			float seed_signif,
			const CellVectors&,
			const PathsSignif&,
			const std::vector<std::string>& paths_pat) const;
  
  StatusCode dump_twinpeaks(std::size_t iseed,
			    const CaloCell* seed,
			    double seed_signif,
			    const std::vector<const CaloCell*>& neighborhood,
			    const std::vector<const CaloCell*>& peaks,
			    const std::vector<double>& peak_signifs) const;
}; 

#endif




