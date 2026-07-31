/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

/*
  This algorithm outputs Calorimeter strip data in the region of
  eFex RoIs.
*/

#include "Egamma1_OnlineMapNbhood.h"
#include "../IO/LArStripNeighborhoodDumper.h"
#include "../IO/eEmNbhoodTOB.h"

#include "CaloEvent/CaloCell.h"
#include "CaloIdentifier/CaloIdManager.h"

#include "Identifier/IdentifierHash.h"

#include "xAODEventInfo/EventInfo.h"

#include <fstream>
#include <vector>
#include <algorithm>
#include <optional>
#include <ranges>

namespace GlobalSim {

  Egamma1_OnlineMapNbhood::Egamma1_OnlineMapNbhood(const std::string& name, ISvcLocator* pSvcLocator ) : 
    AthReentrantAlgorithm(name, pSvcLocator){
  }
  
  //This is not a good hashId. Use this as a dummy/halt point throughout the code.
  constexpr IdentifierHash hashIdDummy = 999999;
  //The maximum number of cells (i.e. the ceiling of the IdentifierHash's
  constexpr int maxIdentifierHash = 187650;

  //Before all events, setup the required variables and tools.
  StatusCode Egamma1_OnlineMapNbhood::initialize() {
    ATH_MSG_DEBUG ("Initializing " << name());

    //Input keys
    CHECK(m_eventInfoKey.initialize());
    CHECK(m_gblLArCellContainerKey.initialize());
    CHECK(m_roiAlgTool.retrieve());
    //Output keys
    CHECK(m_neighKey.initialize());
    CHECK(m_eFEXetaKey.initialize());
    CHECK(m_eFEXphiKey.initialize());
    CHECK(m_FailedeFEXetaKey.initialize());
    CHECK(m_FailedeFEXphiKey.initialize());
    
    // retrieve ID helpers
    ATH_CHECK(detStore()->retrieve(m_calocell_id, "CaloCell_ID"));

    //Launch the calorimeter ID managers
    const CaloIdManager* caloIdMgr = nullptr;
    StatusCode sc = detStore()->retrieve(caloIdMgr);
    if (sc.isFailure()) {
      ATH_MSG_ERROR(" Unable to retrieve CaloIdManager from DetectoreStore");
      return StatusCode::FAILURE;
    }
    m_larem_id = caloIdMgr->getEM_ID();
    
    return StatusCode::SUCCESS;
  }

  // For each event, read in the event info,  GlobalLArCell and eFeXRoI containers.
  // Ask producers to create windows around each incoming RoI in the event.
  StatusCode Egamma1_OnlineMapNbhood::execute(const EventContext& ctx) const {
    
    ATH_MSG_DEBUG ("Executing");

    //Get EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey, ctx);
    if(!eventInfo.isValid()) {
      ATH_MSG_ERROR ("Error obtaining EventInfo object");
      return StatusCode::FAILURE;
    }

    //Get GlobalLArCells
    auto h_gblLArCells = SG::makeHandle(m_gblLArCellContainerKey, ctx);
    CHECK(h_gblLArCells.isValid());
    const auto & gblLArCells = *h_gblLArCells;
    ATH_MSG_DEBUG(gblLArCells.size() <<"Cells read in");

    //Get eFeXRoIs
    std::vector<const xAOD::eFexEMRoI*> rois_in;
    CHECK(m_roiAlgTool->RoIs(rois_in, ctx));
    ATH_MSG_DEBUG(rois_in.size() << " RoI(s) read in");

    //Remove the RoIs in the barrel/endcap crack region (hopefully temporary)
    std::vector<const xAOD::eFexEMRoI*> rois;
    for(auto roi:rois_in){
      //Kill rois in the crack
      if(std::abs(roi->eta()) >= 1.37 && std::abs(roi->eta()) <= 1.52) continue;
      ATH_MSG_DEBUG ("Roi et " << roi->et());
      rois.push_back(roi);
    }
    
    // find GlobalLArCells in the neighborhood of RoIs.
    // A neighborhood is a collection of CellData objects which
    // contain cell eta, phi and Et.

    //Setup a container of TOBs with associated GlobalLArCell windows
    auto neighborhoodTOBs = std::make_unique<IOBitwise::eEmNbhoodTOBContainer>();
    //Flags for window finding success/failure
    std::vector<bool> successes;

    //Finw the windows for all of the valid incoming RoIs
    CHECK(findNeighborhoods_OnlineMap(rois, gblLArCells, successes, *neighborhoodTOBs));

    //Setup output for eFeXRoI eta/phi debug variables
    SG::WriteHandle<std::vector<float> > h_eFEXeta(m_eFEXetaKey, ctx);
    CHECK(h_eFEXeta.record(std::make_unique<std::vector<float> >()));
    SG::WriteHandle<std::vector<float> > h_eFEXphi(m_eFEXphiKey, ctx);
    CHECK(h_eFEXphi.record(std::make_unique<std::vector<float> >()));

    SG::WriteHandle<std::vector<float> > h_FailedeFEXeta(m_FailedeFEXetaKey, ctx);
    CHECK(h_FailedeFEXeta.record(std::make_unique<std::vector<float> >()));
    SG::WriteHandle<std::vector<float> > h_FailedeFEXphi(m_FailedeFEXphiKey, ctx);
    CHECK(h_FailedeFEXphi.record(std::make_unique<std::vector<float> >()));

    
    //Fill RoI eta/phi variables for valid windows
    for(int i=0; auto success:successes){
      if(success){
	h_eFEXeta->push_back(rois[i]->eta());
	h_eFEXphi->push_back(rois[i]->phi());
      } else {
	h_FailedeFEXeta->push_back(rois[i]->eta());
        h_FailedeFEXphi->push_back(rois[i]->phi());
      }
      i++;
    }

    //Setup the write out of the resultant TOBs
    SG::WriteHandle<GlobalSim::IOBitwise::eEmNbhoodTOBContainer> h_neighborhoodTOBs(m_neighKey, ctx);

    //Dump the window/RoI information to local files (for DEBUG)
    auto dumper = GlobalSim::LArStripNeighborhoodDumper();
    if(m_dump || m_dumpTerse){
      if (m_dump) {
        CHECK(dumper.dump(name(), *eventInfo, *neighborhoodTOBs));
      }

      if (m_dumpTerse) {
        CHECK(dumper.dumpTerse(name(), *eventInfo, *neighborhoodTOBs));
      }
    }

    //Write out the TOBs
    CHECK(h_neighborhoodTOBs.record(std::move(neighborhoodTOBs)));
    
    return StatusCode::SUCCESS;
  }

  //This member function steps through all RoIs to produce windows for each RoI
  //There are cases where the window finding can fail (no seed cell found). The
  //success or failure of the algorithm is recorded here for debug purposes.
  StatusCode
  Egamma1_OnlineMapNbhood::findNeighborhoods_OnlineMap(const std::vector<const xAOD::eFexEMRoI*>& rois,
						       const GlobalSim::GlobalLArCellContainer& cells,
						       std::vector<bool>& successes,
						       IOBitwise::eEmNbhoodTOBContainer& neighborhoodTOBs) const{
    
    for (const auto& roi : rois) {
      ATH_MSG_DEBUG("roi et " << roi->et());
      bool success = false;
      ATH_CHECK(findNeighborhood_OnlineMap(roi, cells, success, neighborhoodTOBs));
      successes.push_back(success);
    }
    
    return StatusCode::SUCCESS;
  }
  
  // This member function constructs an LArStripNeighborhood.
  // 
  // A neighourhood is constructed from StripData objects constructed
  // from GlobalLArCells (strips) in the vicinity of an EM RoI the following manner:
  // The strip seed cell in the vicinity of the RoI is identified. An initial
  // neighbourhood of strips centered on the seed is formed. The maximum energy
  // cell within this window is then found. A final neighbourhood is then centered on
  // this cell.
  //
  // In more detail:
  // The RoI eta/phi coordinate (a supercell coordinate) is used to identify
  // the seed cell in the strips (EM1) to form the window. This is done by forming
  // a box in eta/phi space for each cell (using the cells known size and centre)
  // and asking if the RoI's eta/phi position falls in it.
  // As the granularity of the strips changes through the detector the search occurs in
  // differing numbers of strips for each RoI tower range (the crack 1.37-152 is currently
  // not dealth with).
  // There are (8,6,4,1) strips per RoI supercell. First, the inner eta central strip
  // is searched for first, e.g. +eta 4/8 or -eta 6/8. If a cell is found in the collection
  // its ID is returned. If no cell is found then the search continues inwards, then finally
  // outwards in eta. If no strips are found then the window finding halts. This halting is
  // an issue with this method, and a fix is needed.
  //
  // A 17x3 eta/phi window is then formed around this seed cell. First the seed cell is found
  // from a map of CellID->GlobalLArCells. Then the +-8 cells in eta are found using the
  // m_larem_id->get_neighbours function. If a cell isn't present in the collection a dummy
  // with the correct cellID is added to the list. If the window is at the edge of the detector
  // a dummy with cellID 999999 is added, to allow for full sized windows at the edge. Once the
  // central row has been filled, the process is repeated to fill the phi rows above and below.
  //
  // Once an initial window has been formed the maximum energy cell in this window is found.
  // If the cell maxima for two rows are equal, then the central row is taken.
  //
  // This maximum energy cell is then used to repeat the first window finding step to find
  // the final neighbourhood for this eFeXRoI, and an eEmNbhoodTOB is added to the list.
  StatusCode
  Egamma1_OnlineMapNbhood::findNeighborhood_OnlineMap(const xAOD::eFexEMRoI* roi,
						      const GlobalSim::GlobalLArCellContainer& cells,
						      bool& success,
						      IOBitwise::eEmNbhoodTOBContainer& neighborhoodTOBs) const {
    
    // Form the window, and set the vector sizes.
    auto window = std::vector<std::vector<std::shared_ptr<const GlobalLArCell>>>(3);
    window[0].resize(17);
    window[1].resize(17);
    window[2].resize(17);

    //Setup variables
    Identifier CellID;
    float etaCheck = roi->eta();
    float phiCheck = roi->phi();
    bool found = false;
    int strip = 0;

    //Tower position to find strip granularity
    int iEta = roi->iEta();
    int iPhi = roi->iPhi();
    ATH_MSG_DEBUG("Where is this RoI? eta: " << etaCheck << " phi: " << phiCheck << " iEta" << iEta << " iPhi " << iPhi);
    //There are 1 strips per 0.025 supercell at the edge.
    //Shouldn't need to shift these?
    if(iEta == -25 || iEta == 24) {
      //Just use the eta/phi as is, becasue there is only one strip here.
      found = findSeedCell(etaCheck, phiCheck, cells, CellID);
    }
    //There are 4 strips per 0.025 in these towers
    else if((iEta > -25 && iEta <= -21) || (iEta < 24 && iEta >= 20)){
      strip = 4;
    }
    //There are 6 strips per 0.025 in these towers
    else if((iEta > -21 && iEta <= -19) || (iEta < 20 && iEta >= 18)) {
      strip = 6;
    }
    //There are 8 strips per 0.025 in these towers, apart from the crack region where
    //here be dragons. The crack region is not currently dealt with.
    else if((iEta > -19 && iEta <=-1) || (iEta < 18 && iEta >=0)) {
      strip = 8;
    } else {
      //We shouldn't get here, but just in case...
      ATH_MSG_WARNING("Where are we? eta " << etaCheck << " phi " << phiCheck);
    }
    //
    if(strip != 0){
      //Send the number of strips information along with the eta/phi supercell.
      found = findHalfStrips(strip, etaCheck, phiCheck, cells, CellID);
    }

    //If we successfully found the seed cell, then continue.
    if(found){
      int ibec     = abs(m_larem_id->barrel_ec(CellID));   // 1 barrel 2 EC OW  3 EC IW
      int sampling = m_larem_id->sampling(CellID);
      int eta      = m_larem_id->eta(CellID);
      int phi      = m_larem_id->phi(CellID);
      int region   = m_larem_id->region(CellID);
      
      ATH_MSG_DEBUG("Where is this Seed? B/E? " << ibec << " sampling " << sampling << " eta " << eta << " phi " << phi << " region " << region);
            
      //Need the hash for the neighbours
      IdentifierHash hashId=m_calocell_id->calo_cell_hash(CellID);
      IdentifierHash hashIdMax=hashId;
      //Find the initial 17x3 eta/phi window
      ATH_CHECK(findWindow(hashId, cells, window));
      //Find the maxima in the window (only once!)
      ATH_CHECK(findMaxima(hashIdMax, window));
      /*Find the new 17x3 eta/phi window around this maxima
        Don't need to if the max is the seed already */
      if(hashIdMax != hashId){
	ATH_CHECK(findWindow(hashIdMax, cells, window));
      }
      
      //Rediscover the maximum energy cell
      auto it = std::ranges::max_element(std::begin(window[1]),
				 std::end(window[1]),
				 [](const auto& l,const auto& r) {
				   return l->getEnergy() < r->getEnergy();
				 });
      
      std::shared_ptr<const GlobalLArCell> max_cell{*it};    

      //Function to fill the StripData for the neighbourhood
      auto toStripData = [](const auto& fromCells){
	auto stripdata = std::vector<StripData>();
	stripdata.reserve(fromCells.size());
	std::ranges::transform(std::begin(fromCells),
		       std::end(fromCells),
		       back_inserter(stripdata),
		       [](const auto& c) {
			 return StripData(c->eta(),
					  c->phi(),
					  c->getEnergy());});
	return stripdata;
      };

      //Check that we didn't lose the maximum energy cell
      auto max_neigh_cell_it = std::ranges::find(window[1],
						 max_cell);
      if (max_neigh_cell_it == std::end(window[1])){
	ATH_MSG_ERROR("Lost the max cell");
	return StatusCode::FAILURE;
      }

      //Find the position of the maximum energy cell in the neighbourhood.
      auto max_neigh_cell_pos{std::distance(std::begin(window[1]),
					    max_neigh_cell_it)};

      //Fill the StripData vectors
      auto low = toStripData(window[0]);
      auto center = toStripData(window[1]);
      auto high = toStripData(window[2]);

      //Fill the neighbourhood seed roi, and maximum cell positions.
      Coords roi_c{roi->eta(), roi->phi()};
      Coords cell_c{max_cell->eta(), max_cell->phi()};

      //Form the neighbourhood
      LArStripNeighborhood neighborhood = LArStripNeighborhood(low, center, high, roi_c, cell_c, max_neigh_cell_pos);

      //Add the neighbourhood to the list.
      neighborhoodTOBs.push_back(std::make_unique<IOBitwise::eEmNbhoodTOB>(*roi, neighborhood));
      success = true;
    } else {
      //If we didn't find the window, then tell us about it.
      ATH_MSG_DEBUG("Didn't find the RoI eta: " << roi->eta() << " phi: " << roi->phi());
    }    
    return StatusCode::SUCCESS;
  }

  // Function to step through midpoints of the LAr strips, depending on how many strips
  // there are in the current tower.
  bool Egamma1_OnlineMapNbhood::findHalfStrips(int strip,
					      float eta,
					      float phi,
					      const GlobalSim::GlobalLArCellContainer& cells,
					      Identifier& CellID) const {
    //Initialise variables
    bool found = false;
    //For each supercell there are different numbers of strips, this finds the midpoints.
    float half_strip = 0.025/strip/2;
    //Define the "inwards" direction 
    //int sign = 1;
    //if(eta < 0) sign = -1;
    int sign = std::copysign(1, eta);
    int halt = strip-1;
    //Starting at the inner centre strip, step through the allowed strips to find the
    //seed cell.
    for(int i=1;i<=halt;i+=2){
      found = findSeedCell(eta-(i*half_strip*sign), phi, cells, CellID);
      if(found) {
	ATH_MSG_DEBUG("We found it! Going in by " << (i+1)/2 << " eta: " << eta-(i*half_strip) << " phi: " << phi);
	return found;
      }
    }
    //If we didn't find the strip going inwards, look outwards instead.
    if(!found){
      for(int i=1;i<=halt;i+=2){
	found = findSeedCell(eta+(i*half_strip*sign), phi, cells, CellID);
	if(found) {
	  ATH_MSG_DEBUG("We found it! Going out " << (i+1)/2 << " eta: " << eta+(i*half_strip) << " phi: " << phi);
	  return found;
	}
      }
    }
    //Return if we failed to find a strip.
    return found;
  }

  // Function to locate the seed cell for a window from an input eta/phi position.
  // Loops over the input strips in layer 1 and asks if the input eta/phi position is within
  // their physical location (i.e. centre +- half width in eta/phi).
  // Once found it returns the CellID of the located cell for future retrieval.
  bool Egamma1_OnlineMapNbhood::findSeedCell(float eta, float phi,
					     const GlobalSim::GlobalLArCellContainer& cells,
					     Identifier& CellID) const {


    CellID = cells.getIDFromLoc(eta, phi);
    //Failing to find a match will return a default CellID
    if((CellID.get_identifier32()).get_compact() != 0xffffffff){
      ATH_MSG_DEBUG("Found the seed from MAP: " << CellID << " eta "<< eta << " phi " << phi);
      return true;
    }
    ATH_MSG_DEBUG("Didn't find the seed from MAP: eta "<< eta << " phi " << phi);
    return false;
  }

  // Function to draw a window around a seed cell starting from the hashID of the seed cell
  // and a container of all known GlobalLArCells. Returns the window.
  StatusCode Egamma1_OnlineMapNbhood::findWindow(IdentifierHash hashId,
						 const GlobalSim::GlobalLArCellContainer& cells,
						 std::vector<std::vector<std::shared_ptr<const GlobalLArCell>>>& window) const {

    // If we pass in a Dummy ID we cannot start the process, fail gracefully for now.
    if(hashId == hashIdDummy) return StatusCode::SUCCESS;

    //Setup container for found neighbours.
    std::vector<IdentifierHash> neighbourList;
    //Setup next/previous hashIDs
    IdentifierHash hashIdNext;
    IdentifierHash hashIdPrev;
    //Initialise them to the input for the first loop.
    hashIdNext = hashIdPrev = hashId;
    //Position of the cell in each 17 long eta row. Defined by position in loop.
    int shift = 0;

    ATH_MSG_DEBUG("Seed Hash: " << hashId);
    //Start loop to fill the central row of the 17x3 eta/phi window.
    for(int etaOffset = 0; etaOffset <=8; etaOffset++){
      ATH_MSG_DEBUG("Offset: " << etaOffset);
      //Rediscover the centre cell for the first step.
      if(etaOffset == 0){
	//This retrieves the cell from a hash map in the GlobalLArCell container.
	auto cell = cells.getCellFromHash(hashId);
	//Sanity check.
	if(cell != nullptr){
	  //Fill the central neighbourhood position.
	  window[1][8] = cell;
	  ATH_MSG_DEBUG("Found the middle cell " << cell->getID() << " at eta " << cell->eta() << " phi " << cell->phi());
	} else {
	  //We didn't find the cell in the hash map. Put a dummy in the window.
	  ATH_MSG_DEBUG("Putting in a dummy middle cell with hashId " << hashId);
	  window[1][8] = std::make_shared<GlobalLArCell>(hashId,"",0);
	  //Potential issue if you cannot find the cell... but this cell would be the last in eta?
	}
      } else {
        neighbourList.clear();
        if (hashIdNext != hashIdDummy)
	  m_larem_id->get_neighbours(hashIdNext,LArNeighbours::nextInEta,neighbourList);
	//If we are not at the edge, and were not previously at the edge. Then...
	if(!neighbourList.empty()){
	  //Use the previous hashIDNext to look for the next cell in eta
	  m_larem_id->get_neighbours(hashIdNext,LArNeighbours::nextInEta,neighbourList);
	  ATH_MSG_DEBUG("Next in eta " << neighbourList[0] << " HashId " << hashId);
	  //Grab the next cell hashID in eta and store it for the next step.
	  hashIdNext = neighbourList[0];
	  //Try to retrieve the cell from the hash map
	  auto cellNext = cells.getCellFromHash(hashIdNext);
	  //Define how steps we are away from the central cell
	  shift = 8+etaOffset;
	  //If the hash map contained the next cell.
	  if(cellNext != nullptr){
	    ATH_MSG_DEBUG("Found a cell " << cellNext->getID() << " at eta " << cellNext->eta() << " phi " << cellNext->phi());
	    //Put the found cell in the window
	    window[1][shift] = cellNext;
	  } else {
	    //We didn't find the cell in the hash map. Put a dummy in the window.
	    ATH_MSG_DEBUG("Putting in a next dummy with hashIdNext " << hashIdNext);
	    window[1][shift] = std::make_shared<GlobalLArCell>(hashIdNext,"",0);
	    //Potential issue if you cannot find the cell... but this cell would be the last in eta?
	  }
	} else {
	  //We are off the edge of the detector. Put in a Dummy cell to keep the window going.
	  shift = 8+etaOffset;
	  ATH_MSG_DEBUG("No next in eta: Putting in an empty DUMMY");
	  window[1][shift] = std::make_shared<GlobalLArCell>(hashIdDummy,"",0);
	}
	//Now go inwards, as above, but with prevInEta, and no need to pad for the edge of the detector.
	m_larem_id->get_neighbours(hashIdPrev,LArNeighbours::prevInEta,neighbourList);
	ATH_MSG_DEBUG("Previous in eta " << neighbourList[0] << " hashId " << hashId);
	hashIdPrev = neighbourList[0];
	auto cellPrev = cells.getCellFromHash(hashIdPrev);
	shift = 8-etaOffset;
	if(cellPrev != nullptr){
	  ATH_MSG_DEBUG("Found a cell " << cellPrev->getID() << " at eta " << cellPrev->eta() << " phi " << cellPrev->phi());
	  window[1][shift] = cellPrev;
	} else {
	  ATH_MSG_DEBUG("Putting in a previous dummy with hasIdPrev " << hashIdPrev);
	  window[1][shift] = std::make_shared<GlobalLArCell>(hashIdPrev,"",0);
	}
      }
    }

    //We now have the central row. Loop to fill the phi rows above and below in phi.
    for(int phiOffset = 0; phiOffset <=1; phiOffset++){
      ATH_MSG_DEBUG("Offset: " << phiOffset);
      //First do nextInPhi
      if(phiOffset == 0){
	shift = 0;
	//Loop over the middle row.
	for(auto& windowCell:window[1]){
	  ATH_MSG_DEBUG("Cell ID " << shift << " is " << windowCell->getID());
	  //If this isn't a Dummy cell
	  if(windowCell->getID() != hashIdDummy){
	    //Find the cell hashID from its offline ID.
	    IdentifierHash hashIdCurrent = m_calocell_id->calo_cell_hash(static_cast<Identifier>(windowCell->getID()));
	    //There are cases where the CellID is the hashID. Update the hash to this.
	    if(windowCell->getID() <= maxIdentifierHash) hashIdCurrent = windowCell->getID();
	    //Proceed as in eta, finding next in Phi.
	    m_larem_id->get_neighbours(hashIdCurrent,LArNeighbours::nextInPhi,neighbourList);
	    ATH_MSG_DEBUG("Next in phi " << neighbourList[0] << " hashId " << hashIdCurrent);
	    //Find the hashID and use it to look up the cell in the hash map.
	    hashIdCurrent = neighbourList[0];
	    auto cellCurrent = cells.getCellFromHash(hashIdCurrent);
	    //If we find the cell in the hash map
	    if(cellCurrent != nullptr){
	      //Put the cell in the window.
	      ATH_MSG_DEBUG("Found a cell " << cellCurrent->getID() << " at eta " << cellCurrent->eta() << " phi " << cellCurrent->phi());
	      window[0][shift] = cellCurrent;
	    } else {
	      //Put a dummy cell. No need to get the ID right here.
	      ATH_MSG_DEBUG("No next in phi: Last HashIdCurrent " << hashIdCurrent);
	      window[0][shift] = std::make_shared<GlobalLArCell>(hashIdDummy,"",0);
	    }
	    shift++;
	  } else {
	    //We found a Dummy in the middle row, put a dummy in next phi too.
	    ATH_MSG_DEBUG("No next in phi: Putting in a DUMMY");
	    window[0][shift] = std::make_shared<GlobalLArCell>(hashIdDummy,"",0);
	    shift++;
	  }
	}
      } else {
	//Repeat the above for the previous row in Phi
	shift = 0;
	for(auto& windowCell:window[1]){
	  ATH_MSG_DEBUG("Cell ID " << shift << " is " << windowCell->getID());
	  if(windowCell->getID() != hashIdDummy){
	    IdentifierHash hashIdCurrent = m_calocell_id->calo_cell_hash(static_cast<Identifier>(windowCell->getID()));
	    if(windowCell->getID() <= maxIdentifierHash) hashIdCurrent = windowCell->getID();
	    m_larem_id->get_neighbours(hashIdCurrent,LArNeighbours::prevInPhi,neighbourList);
	    ATH_MSG_DEBUG("Previous in phi " << neighbourList[0] << " hashId " << hashIdCurrent);
	    hashIdCurrent = neighbourList[0];
	    auto cellCurrent = cells.getCellFromHash(hashIdCurrent);
	    if(cellCurrent != nullptr){
	      ATH_MSG_DEBUG("Found a cell " << cellCurrent->getID() << " at eta " << cellCurrent->eta() << " phi " << cellCurrent->phi());
	      window[2][shift] = cellCurrent;
	    } else {
	      ATH_MSG_DEBUG("No prev in phi: Putting in a DUMMY");
	      window[2][shift] = std::make_shared<GlobalLArCell>(hashIdDummy,"",0);
	    }
	    shift++;
	  } else {
	    ATH_MSG_DEBUG("No prev in phi: Putting in a DUMMY");
            window[2][shift] = std::make_shared<GlobalLArCell>(hashIdDummy,"",0);
            shift++;
          }
	}
      }
    }
    return StatusCode::SUCCESS;
  }
  // Function to find the maximum energy cell within a neighbourhood.
  // Return its hashID to be used to seed another neighbourhood window.
  StatusCode Egamma1_OnlineMapNbhood::findMaxima(IdentifierHash& hashId,
						 std::vector<std::vector<std::shared_ptr<const GlobalLArCell>>>& window) const {

    //Vector to hold each row's maximum
    std::vector<std::shared_ptr<const GlobalLArCell>> maxima;
    //Loop over the rows and find each maximum energy cell.
    for(uint row = 0;row < window.size();row++){
      auto max_cell = Egamma1_OnlineMapNbhood::findMax(window[row]);
      maxima.push_back(max_cell);
      ATH_MSG_DEBUG("Found a max cell " << max_cell->getID() << " et " << max_cell->getEnergy() << " eta " << max_cell->eta() << " phi " << max_cell->phi()); 
    }
    //If we have two equal maxima, then take the middle row to be consistent.
    if((maxima[0]->getEnergy() == maxima[1]->getEnergy() && maxima[1]->getEnergy() >= maxima[2]->getEnergy())
       || (maxima[1]->getEnergy() == maxima[2]->getEnergy() && maxima[1]->getEnergy() >= maxima[0]->getEnergy())){
      ATH_MSG_DEBUG("Found the max cell " << maxima[1]->getID() << " et " << maxima[1]->getEnergy() << " eta " << maxima[1]->eta() << " phi " << maxima[1]->phi());
      hashId = m_calocell_id->calo_cell_hash(static_cast<Identifier>(maxima[1]->getID()));
      return StatusCode::SUCCESS;
    } else {
      //Otherwise just take the maximum cell.
      auto maximum_cell = Egamma1_OnlineMapNbhood::findMax(maxima);
      ATH_MSG_DEBUG("Found the max cell " << maximum_cell->getID() << " et " << maximum_cell->getEnergy() << " eta " << maximum_cell->eta() << " phi " << maximum_cell->phi()); 
      hashId = m_calocell_id->calo_cell_hash(static_cast<Identifier>(maximum_cell->getID()));
      return StatusCode::SUCCESS;
    }
  } 

  //Function to find the maximum energy cell in a vector of GlobalLArCells.
  std::shared_ptr<const GlobalLArCell> Egamma1_OnlineMapNbhood::findMax(std::vector<std::shared_ptr<const GlobalLArCell>>& row) const {
    auto it = std::max_element(std::begin(row),
                               std::end(row),
                               [](const auto& l,const auto& r) {
                                 return l->getEnergy() < r->getEnergy();
                               });
    
    std::shared_ptr<const GlobalLArCell> max_cell{*it};
    return max_cell;
  }
}
