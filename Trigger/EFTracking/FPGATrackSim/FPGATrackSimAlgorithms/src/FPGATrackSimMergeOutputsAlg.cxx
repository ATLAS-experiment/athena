/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "FPGATrackSimMergeOutputsAlg.h"
#include "FPGATrackSimAlgorithms/FPGATrackSimOverlapRemovalTool.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"

#include "TH2F.h"
#include "TTree.h"
#include "TFile.h"

FPGATrackSimMergeOutputsAlg::FPGATrackSimMergeOutputsAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator) {
}


StatusCode FPGATrackSimMergeOutputsAlg::initialize()
{

  ATH_MSG_INFO ( "FPGATrackSimMergeOutputsAlg::initialize()");

  ATH_CHECK(m_FPGATrackKey.initialize());
  ATH_CHECK(m_FPGAHitKey.initialize());  
  ATH_CHECK(m_overlapRemovalTool.retrieve());
  
  unsigned nfiles = m_inpaths.size();

  m_files.resize(nfiles);
  m_trees.resize(nfiles);
  
  m_eventOutputHeaders.resize(nfiles);
  
  bool regionsFound[N]; // did we find this region in ANY file yet?
  for (unsigned iregion = 0; iregion < N; iregion++) {
    regionsFound[iregion] = false;
  }

  bool foundDP=false;

  for (unsigned ifile = 0; ifile < nfiles; ifile++) { // loop over paths to files
    m_files[ifile] = new TFile(m_inpaths[ifile].c_str(),"READ");
    if (!m_files[ifile]->IsOpen() || m_files[ifile]->IsZombie()) {
      ATH_MSG_ERROR("Could not open" << m_inpaths[ifile]);
      return StatusCode::FAILURE;
    }
    
    if (!foundDP) { // only needed for one file
      m_dataprep_tree = (TTree*)(m_files[ifile]->Get("FPGATrackSimDataPrepTree"));
      if (m_dataprep_tree) {
        if (m_dataprep_tree->GetEntries() > 0) {
          foundDP = true;
          m_dataprep = new FPGATrackSimLogicalEventInputHeader();
          
          TBranch *dpb = m_dataprep_tree->GetBranch("LogicalEventInputHeader_PostCluster");
          if (!dpb) {
            ATH_MSG_ERROR("Could not get LogicalEventInputHeader_PostCluster file " << m_inpaths[ifile]);
            return StatusCode::FAILURE;
          }
          dpb->SetAddress(&m_dataprep);
          m_dataprep_tree->SetBranchStatus("LogicalEventInputHeader_Pre*",0);      
        }
      }
    }

    
    m_trees[ifile].resize(N);
    m_eventOutputHeaders[ifile].resize(N);

    for (unsigned iregion = 0; iregion < N; iregion++) {
      m_trees[ifile][iregion] = (TTree*)(m_files[ifile]->Get(Form("FPGATrackSimLogicalEventTree_reg%d",iregion)));
      if (!m_trees[ifile][iregion]) {
        continue;
      }
      else if (regionsFound[iregion]) {
        ATH_MSG_ERROR("Found two files with region number " << iregion << " and I do not know which one to use!");
        return StatusCode::FAILURE;
      }
      else {
	      regionsFound[iregion] = true;
      }

      m_eventOutputHeaders[ifile][iregion] = new FPGATrackSimLogicalEventOutputHeader();
      TBranch *b = m_trees[ifile][iregion]->GetBranch("LogicalEventOutputHeader");
      if (!b) {
	      ATH_MSG_ERROR("Could not get LogicalEventOutputHeader in file " << m_inpaths[ifile]);
	      return StatusCode::FAILURE;
      }
      b->SetAddress(&m_eventOutputHeaders[ifile][iregion]);      

      m_trees[ifile][iregion]->SetBranchStatus("LogicalEventSecond*",0);
      m_trees[ifile][iregion]->SetBranchStatus("LogicalEventFirstP*",0);      
      m_trees[ifile][iregion]->SetBranchStatus("LogicalEventSpace*",0);
      m_trees[ifile][iregion]->SetBranchStatus("LogicalEventStrip*",0);        
      
    }
  }

  if (!foundDP) {
    ATH_MSG_ERROR("Did not find the DP tree");
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimMergeOutputsAlg::execute() {
  ATH_MSG_DEBUG ("Running on event ");

  const EventContext& ctx = getContext();
  SG::WriteHandle<FPGATrackSimHitCollection> FPGAHits_Handle (m_FPGAHitKey, ctx); // all the clusters across the regions

  SG::WriteHandle<FPGATrackSimTrackCollection> FPGATracks_Handle (m_FPGATrackKey, ctx); // all the tracks across the regions
  // Do this now so we can deference it
  ATH_CHECK(FPGATracks_Handle.record (std::make_unique<FPGATrackSimTrackCollection>()));
  FPGATrackSimTrackCollection *FPGATracks = FPGATracks_Handle.ptr();

  // get the hits
  ATH_CHECK(FPGAHits_Handle.record (std::make_unique<FPGATrackSimHitCollection>()));
  m_dataprep_tree->GetEntry(m_evtloop); 
  for (const auto & tower : m_dataprep->towers()) {    
    const std::vector<FPGATrackSimHit> hits = tower.hits();
    for (auto& hit : hits) {
      FPGAHits_Handle->push_back(std::make_unique<FPGATrackSimHit>(std::move(hit)));
    }
  }

  for (unsigned ivec = 0; ivec < m_eventOutputHeaders.size(); ivec++) {
    for (unsigned iregion = 0; iregion < N; iregion++) {
      if (!m_trees[ivec][iregion]) continue;
      if (m_evtloop >= m_trees[ivec][iregion]->GetEntries()) {
	ATH_MSG_ERROR("Tried reading in more events than the tree had available!");
	return StatusCode::FAILURE;
      }
      m_trees[ivec][iregion]->GetEntry(m_evtloop);
      // Time to load up these tracks! Only bother using ones that already passed OLR
      std::vector<FPGATrackSimTrack> const tracks = m_eventOutputHeaders[ivec][iregion]->getFPGATrackSimTracks_1st();
      m_alltracks += tracks.size();
      for (const auto &track : tracks) {
        if (track.passedOR()) FPGATracks->push_back(track);
      }
    }
  }
  
  // Now run overlap removal on all the tracks that already passed overlap removal between other regions
  ATH_CHECK(m_overlapRemovalTool->runOverlapRemoval(*FPGATracks));
  for (const auto &track : *FPGATracks) {
    if (track.passedOR()) m_tracksPassOR++;
  }
  // Increase evtloop
  m_evtloop++;
  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimMergeOutputsAlg::finalize() {
    ATH_MSG_INFO("Processed " << m_evtloop << " events.");
    ATH_MSG_INFO("Average number of tracks per event = " << (m_alltracks/m_evtloop));
    ATH_MSG_INFO("Average number of tracks per event passing global OLR = " << (m_tracksPassOR/m_evtloop));    
    return StatusCode::SUCCESS;
}
