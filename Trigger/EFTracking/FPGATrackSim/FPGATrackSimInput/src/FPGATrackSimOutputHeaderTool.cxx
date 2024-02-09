/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimOutputHeaderTool.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"

FPGATrackSimOutputHeaderTool::FPGATrackSimOutputHeaderTool(std::string const & algname, std::string const & name, IInterface const * ifc) :
  base_class(algname, name, ifc) {}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::openFile(std::string const & path)
{
  // close old file (I don't think we delete the pointer. Does ROOT handle that?)
  if (m_infile && m_infile->IsOpen()) m_infile->Close();

  // open new file
  ATH_MSG_DEBUG ("Opening file " << path.c_str() << " in " << m_rwoption.value() << " mode.");
  m_infile = TFile::Open(path.c_str(), m_rwoption.value().c_str());

  if (!m_infile) {
    ATH_MSG_FATAL("Could not open input file: " << path);
    return StatusCode::FAILURE;
  }

  if (m_rwoption.value() == std::string("READ")) {
    // get the tree, work on backwards compatability, so try two of them
    m_EventTree = (TTree*) m_infile->Get("FPGATrackSimLogicalEventTree");
    if (!m_EventTree || m_EventTree->GetEntries() == -1) {
      ATH_MSG_FATAL ("Input file: " << m_inpath.value() << " has no entries");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO ("Input file: " << path << " has " << m_EventTree->GetEntries() << " event entries.");

    // branches
    if (!m_EventTree->GetListOfBranches()->FindObject(m_branchNameIn_1st.c_str())) {
      ATH_MSG_FATAL ("Branch: " << m_branchNameIn_1st << " not found!");
      return StatusCode::FAILURE;
    }
    TBranch *branchIn_1st = m_EventTree->GetBranch(m_branchNameIn_1st.c_str());
    branchIn_1st->SetAddress(&m_eventInputHeader_1st);
    m_EventTree->SetBranchStatus(m_branchNameIn_1st.c_str(), 1);

    if (m_runSecondStage) {
      if (!m_EventTree->GetListOfBranches()->FindObject(m_branchNameIn_2nd.c_str())) {
        ATH_MSG_FATAL ("Branch: " << m_branchNameIn_2nd << " not found!");
        return StatusCode::FAILURE;
      }
      TBranch *branchIn_2nd = m_EventTree->GetBranch(m_branchNameIn_2nd.c_str());
      branchIn_2nd->SetAddress(&m_eventInputHeader_2nd);
      m_EventTree->SetBranchStatus(m_branchNameIn_2nd.c_str(), 1);
    }

    if (!m_EventTree->GetListOfBranches()->FindObject(m_branchNameOut.c_str())) {
      ATH_MSG_FATAL ("Branch: " << m_branchNameOut << " not found!");
      return StatusCode::FAILURE;
    }
    TBranch *branchOut = m_EventTree->GetBranch(m_branchNameOut.c_str());
    branchOut->SetAddress(&m_eventOutputHeader);
    m_EventTree->SetBranchStatus(m_branchNameOut.c_str(),1);
  }
  m_infile->cd();
  m_event = 0;
  return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::initialize()
{
  m_eventInputHeader_1st  = new FPGATrackSimLogicalEventInputHeader();
  m_eventOutputHeader = new FPGATrackSimLogicalEventOutputHeader();
  if (m_runSecondStage) m_eventInputHeader_2nd = new FPGATrackSimLogicalEventInputHeader();
 
  // set defaults
  m_branchNameIn_1st  = "FPGATrackSimLogicalEventInputHeader_1st";
  m_branchNameIn_2nd  = "FPGATrackSimLogicalEventInputHeader_2nd";
  m_branchNameOut     = "FPGATrackSimLogicalEventOutputHeader";

  if( m_rwoption.value()!=std::string("HEADER"))
  {
    if (m_inpath.value().empty())
    {      
        ATH_MSG_ERROR("Empty input file list");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(openFile(m_inpath.value().front())); 
  }

  if (m_rwoption.value() == std::string("READ")) {
    ATH_MSG_DEBUG ("Initialized in READ MODE");
  }
  else if (m_rwoption.value()==std::string("RECREATE") || m_rwoption.value()==std::string("HEADER")) {
    ATH_MSG_INFO ("Creating empty branches in output file");   
    ATH_CHECK(openFile(m_inpath.value().front())); 
    m_EventTree = new TTree("FPGATrackSimLogicalEventTree","data");
    
    m_EventTree->Branch(m_branchNameIn_1st.c_str(),
            "FPGATrackSimLogicalEventInputHeader", // class name
            &m_eventInputHeader_1st);
    if (m_runSecondStage) {
      m_EventTree->Branch(m_branchNameIn_2nd.c_str(),
            "FPGATrackSimLogicalEventInputHeader", // class name
            &m_eventInputHeader_2nd);
    }
    m_EventTree->Branch(m_branchNameOut.c_str(),
            "FPGATrackSimLogicalEventOutputHeader", // class name
            &m_eventOutputHeader);
  }
  else {
    ATH_MSG_ERROR ("RWstatus = " << m_rwoption.value() << " is not allowed!");
    return StatusCode::FAILURE;
  }

  m_event    = 0; // in file
  m_totevent = 0; // total counter
  return StatusCode::SUCCESS;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::finalize()
{
  ATH_MSG_INFO ("finalize: closing files");

  if (m_rwoption.value() == std::string("RECREATE")) {
    m_EventTree->Print();
    ATH_MSG_INFO ("Contains " << m_EventTree->GetEntries() << " entries, over " << m_event << " events run");
    // close the output files, but check that it exists (for athenaMP)
    m_infile->Write();
  }

  if (m_rwoption.value() != std::string("HEADER")) {
    m_infile->Close();
  }
  //deleting pointers
  delete m_eventInputHeader_1st ;
  delete m_eventOutputHeader;
  
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
StatusCode FPGATrackSimOutputHeaderTool::writeData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader)
{

  if (m_rwoption.value() == std::string("READ")) {
    ATH_MSG_WARNING ("Asked to write file in READ  mode");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG ("Writing data in TTree");
  if (INheader_1st)
    *m_eventInputHeader_1st = *INheader_1st; //copy object
  else
    ATH_MSG_INFO ("m_eventInputHeader_1st is empty");

  if (m_runSecondStage) {
    if (INheader_2nd)
      *m_eventInputHeader_2nd = *INheader_2nd;
    else
      ATH_MSG_INFO("m_eventInputHeader_2nd is empty");
  }

  if (OUTheader)
    *m_eventOutputHeader = *OUTheader;
  else
    ATH_MSG_INFO ("m_eventOutputHeader is empty");

  m_EventTree->Fill();
  
  if (m_eventInputHeader_1st) {
    ATH_MSG_DEBUG ("Wrote Event " << m_event << " in first stage header event " << m_eventInputHeader_1st->event());
  } else {
    ATH_MSG_ERROR("m_eventInputHeader_1st is null in FPGATrackSimOutputHeaderTool::writeData");
    return StatusCode::FAILURE;
  }
  if (m_runSecondStage && m_eventInputHeader_2nd) {
    ATH_MSG_DEBUG ("Wrote Event " << m_event << " in second stage header event " << m_eventInputHeader_2nd->event());
  }
  if (m_eventOutputHeader) {
    ATH_MSG_DEBUG ("n.roads_1st = "  << m_eventOutputHeader->nFPGATrackSimRoads_1st());
    ATH_MSG_DEBUG ("n.roads_2nd = "  << m_eventOutputHeader->nFPGATrackSimRoads_2nd());
    ATH_MSG_DEBUG ("n.tracks_1st = " << m_eventOutputHeader->nFPGATrackSimTracks_1st());
    ATH_MSG_DEBUG ("n.tracks_2nd = " << m_eventOutputHeader->nFPGATrackSimTracks_2nd());
  } else {
    ATH_MSG_ERROR("m_eventOutputHeader is null in FPGATrackSimOutputHeaderTool::writeData");
    return StatusCode::FAILURE;
  }
  
  m_event++;

  m_eventInputHeader_1st->reset();
  if (m_runSecondStage and m_eventInputHeader_2nd) m_eventInputHeader_2nd->reset();
  m_eventOutputHeader->reset();

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::readData(FPGATrackSimLogicalEventInputHeader* INheader_1st, FPGATrackSimLogicalEventInputHeader* INheader_2nd, FPGATrackSimLogicalEventOutputHeader* OUTheader, bool &last) 
{
  if (m_rwoption.value() != std::string("READ")) {
    ATH_MSG_WARNING ("Asked to read file that is not in READ mode");
    return StatusCode::SUCCESS;
  }

  m_eventInputHeader_1st->reset();
  m_eventOutputHeader->reset();
  if (m_runSecondStage) m_eventInputHeader_2nd->reset();
  
  ATH_MSG_DEBUG ("Asked Event " << m_event << " in this file; current total is " << m_totevent);
  last = false;
  if (m_event >= m_EventTree->GetEntries()) {
    if (++m_file < m_inpath.value().size()) {
      ATH_CHECK(openFile(m_inpath.value().at(m_file)));
    }
    else {
      last = true;
      return StatusCode::SUCCESS;
    } 
  }

  
  Int_t statIn_1st = m_EventTree->GetBranch(m_branchNameIn_1st.c_str())->GetEntry(m_event);
  Int_t statOut    = m_EventTree->GetBranch(m_branchNameOut.c_str())->GetEntry(m_event);

  if (statIn_1st <= 0) ATH_MSG_WARNING("Error in reading from branch " << m_branchNameIn_1st);
  if (statOut    <= 0) ATH_MSG_WARNING("Error in reading from branch " << m_branchNameOut);

  if (m_runSecondStage) {
    Int_t statIn_2nd = m_EventTree->GetBranch(m_branchNameIn_2nd.c_str())->GetEntry(m_event);
    if (statIn_2nd <= 0) ATH_MSG_WARNING("Error in reading from branch " << m_branchNameIn_2nd);
  }
  

  ATH_MSG_DEBUG ("Asked in first stage header event: " << m_eventInputHeader_1st->event());
  if (m_runSecondStage) ATH_MSG_DEBUG ("Asked in second stage header event: " << m_eventInputHeader_2nd->event());

  ATH_MSG_DEBUG ("n.roads_1st = "  << m_eventOutputHeader->nFPGATrackSimRoads_1st());
  ATH_MSG_DEBUG ("n.tracks_1st = " << m_eventOutputHeader->nFPGATrackSimTracks_1st());

  if (m_runSecondStage) {
    ATH_MSG_DEBUG ("n.roads_2nd = "  << m_eventOutputHeader->nFPGATrackSimRoads_2nd());
    ATH_MSG_DEBUG ("n.tracks_2nd = " << m_eventOutputHeader->nFPGATrackSimTracks_2nd());
  }
  
  *INheader_1st = *m_eventInputHeader_1st; //copy object to the external pointer
  *OUTheader    = *m_eventOutputHeader;
  if (m_runSecondStage) *INheader_2nd = *m_eventInputHeader_2nd;

  m_event++;
  m_totevent++;


  return StatusCode::SUCCESS;
}

