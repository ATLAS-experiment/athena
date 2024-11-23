/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimInput/FPGATrackSimOutputHeaderTool.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"

FPGATrackSimOutputHeaderTool::FPGATrackSimOutputHeaderTool(std::string const & algname, std::string const & name, IInterface const * ifc) :
  AthAlgTool(algname,name,ifc) {}


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
    // get the tree
    m_EventTree = (TTree*) m_infile->Get(m_treeName.value().c_str());
    if (!m_EventTree || m_EventTree->GetEntries() == -1) {
      ATH_MSG_FATAL ("Input file: " << m_inpath.value() << " has no entries");
      return StatusCode::FAILURE;
    }

    ATH_MSG_INFO ("Input file: " << path << " has " << m_EventTree->GetEntries() << " event entries.");
  }

  m_infile->cd();
  m_event = 0;
  return StatusCode::SUCCESS;
}

// Since now the list of branches to read gets configured by algorithms, we can't do this in initialize().
// We could use properties to pass the branch names to this tool, but the algorithms would need to know those
// names too, so it may make more sense to store them as properties on individual algorithms.

// Also... the properties would have to be a vector because we can have an arbitrary number of input and output headers.
// Perhaps we don't actually *need* an arbitrary number of input and output headers but I think it's best to allow for it.

StatusCode FPGATrackSimOutputHeaderTool::configureReadBranches() {

  // Don't do anything
  if (m_rwoption.value() != std::string("READ")) {
    ATH_MSG_FATAL("Called configureReadBranches() when ROOT file was not opened in READ mode.");
    return StatusCode::FAILURE;
  }

  // In read mode, we want to make sure the configured branches are actually available.
  // Configuration is via a function call (to allow different algorithms to set up different numbers of branches).
  for (unsigned i = 0; i < m_branchNameIns.size(); i++) {
    std::string branchName = m_branchNameIns.at(i);
    if (!m_EventTree->GetListOfBranches()->FindObject(branchName.c_str())) {
      ATH_MSG_FATAL("Configured input branch: " << branchName << " not found!");
      return StatusCode::FAILURE;
    }
    TBranch *branchIn = m_EventTree->GetBranch(branchName.c_str());
    branchIn->SetAddress(&(m_eventInputHeaders.at(i)));
    m_EventTree->SetBranchStatus(branchName.c_str(), 1);
  }

  // Do the same thing for the output branches. In fact we could maybe combine these loops by making a function.
  for (unsigned i = 0; i < m_branchNameOuts.size(); i++) {
    std::string branchName = m_branchNameOuts.at(i);
    if (!m_EventTree->GetListOfBranches()->FindObject(branchName.c_str())) {
      ATH_MSG_FATAL("Configured output branch: " << branchName << " not found!");
      return StatusCode::FAILURE;
    }
    TBranch *branchIn = m_EventTree->GetBranch(branchName.c_str());
    branchIn->SetAddress(&(m_eventOutputHeaders.at(i)));
    m_EventTree->SetBranchStatus(branchName.c_str(), 1);
  }

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::initialize()
{
 
  ATH_CHECK(m_tHistSvc.retrieve());

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
  } else if (m_rwoption.value()==std::string("RECREATE") || m_rwoption.value()==std::string("HEADER")) {

    // branches are NO LONGER created here, the user needs to do this.
    m_EventTree = new TTree(m_treeName.value().c_str(), "data");

    // For HEADER-- use THistSvc explicitly to set things up.
    if (m_rwoption.value() == std::string("HEADER")) {
      ATH_CHECK(m_tHistSvc->regTree(Form("/FPGATRACKSIMOUTPUT/%s", m_treeName.value().c_str()), m_EventTree));
    }

  } else {
    ATH_MSG_ERROR ("RWstatus = " << m_rwoption.value() << " is not allowed!");
    return StatusCode::FAILURE;
  }

  m_event    = 0; // in file
  m_totevent = 0; // total counter
  return StatusCode::SUCCESS;
}

// Create a new input or output branch and return a pointer to the header object so code can write to it.
// These functions work for both reading and writing.
FPGATrackSimLogicalEventInputHeader* FPGATrackSimOutputHeaderTool::addInputBranch(const std::string& branchName, bool write) {
  m_eventInputHeaders.push_back(new FPGATrackSimLogicalEventInputHeader());
  FPGATrackSimLogicalEventInputHeader* inputHeader = m_eventInputHeaders.at(m_eventInputHeaders.size() - 1);
  m_branchNameIns.push_back(branchName);
  if (write) {
    m_EventTree->Branch(branchName.c_str(), "FPGATrackSimLogicalEventInputHeader", inputHeader);
  }
  return inputHeader;
}

FPGATrackSimLogicalEventOutputHeader* FPGATrackSimOutputHeaderTool::addOutputBranch(const std::string& branchName, bool write) {
  m_eventOutputHeaders.push_back(new FPGATrackSimLogicalEventOutputHeader());
  FPGATrackSimLogicalEventOutputHeader* outputHeader = m_eventOutputHeaders.at(m_eventOutputHeaders.size() - 1);
  m_branchNameOuts.push_back(branchName);
  if (write) {
    m_EventTree->Branch(branchName.c_str(), "FPGATrackSimLogicalEventOutputHeader", outputHeader);
  }
  return outputHeader;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimOutputHeaderTool::finalize()
{
  ATH_MSG_INFO ("finalize: closing files");

  if (m_rwoption.value() == std::string("RECREATE")) {
    ATH_MSG_INFO ("Contains " << m_EventTree->GetEntries() << " entries, over " << m_event << " events run");
    // close the output files, but check that it exists (for athenaMP)
    m_infile->Write();
  }

  if (m_rwoption.value() != std::string("HEADER")) {
    m_infile->Close();
  }

  // deleting pointers
  for (auto* header : m_eventInputHeaders) delete header;
  for (auto* header : m_eventOutputHeaders) delete header;
  
  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// This version of writeData assumes that the header objects were created using addInputBranch and addOutputBranch
// and so don't need to be passed back to the tool.
StatusCode FPGATrackSimOutputHeaderTool::writeData() {

  if (m_rwoption.value() == std::string("READ")) {
    ATH_MSG_WARNING ("Asked to write file in READ  mode");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG ("Writing data in TTree");

  m_EventTree->Fill();

  // Reset any input headers that we wrote out (with debugging prints).
  for (unsigned i = 0; i < m_eventInputHeaders.size(); i++) {
    ATH_MSG_DEBUG("Wrote event " << m_event << " in input header (" << m_branchNameIns.at(i) << ") event " <<  m_eventInputHeaders.at(i)->event());
    m_eventInputHeaders.at(i)->reset();
  }

  // Reset any output headers that we wrote out (With debugging prints).
  for (unsigned i = 0; i < m_eventOutputHeaders.size(); i++) {
    ATH_MSG_DEBUG("Wrote event " << m_event << " in output header (" << m_branchNameOuts.at(i) << ")");
    ATH_MSG_DEBUG("n.roads_1st = "  << m_eventOutputHeaders.at(i)->nFPGATrackSimRoads_1st());
    ATH_MSG_DEBUG("n.roads_2nd = "  << m_eventOutputHeaders.at(i)->nFPGATrackSimRoads_2nd());
    ATH_MSG_DEBUG("n.tracks_1st = " << m_eventOutputHeaders.at(i)->nFPGATrackSimTracks_1st());
    ATH_MSG_DEBUG("n.tracks_2nd = " << m_eventOutputHeaders.at(i)->nFPGATrackSimTracks_2nd());
    m_eventOutputHeaders.at(i)->reset();
  }

  // Only return FAILURe if there were no input AND output headers (i.e. something is misconfigured).
  if (m_eventInputHeaders.size() == 0 and m_eventOutputHeaders.size() == 0) {
    ATH_MSG_ERROR("Tried to fill output ROOT file with no configured input or output headers.");
    return StatusCode::FAILURE;
  }
  
  m_event++;

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Since the branches are now created using the same set of functions the user already has a pointer to the object.
// So in this function we just need to reset those objects, test last, and... call GetEntry I think.
StatusCode FPGATrackSimOutputHeaderTool::readData(bool &last)
{
  if (m_rwoption.value() != std::string("READ")) {
    ATH_MSG_WARNING ("Asked to read file that is not in READ mode");
    return StatusCode::SUCCESS;
  }

  for (auto* header : m_eventInputHeaders) {
    header->reset();
  }

  for (auto* header : m_eventOutputHeaders) {
    header->reset();
  }
  
  ATH_MSG_DEBUG ("Asked Event " << m_event << " in this file; current total is " << m_totevent);
  last = false;
  if (m_event >= m_EventTree->GetEntries()) {
    if (++m_file < m_inpath.value().size()) {
      ATH_CHECK(openFile(m_inpath.value().at(m_file)));
      // If opening a new file we need to update the branch addresses.
      ATH_CHECK(configureReadBranches());
    }
    else {
      last = true;
      return StatusCode::SUCCESS;
    } 
  }

  // Read the objects. I removed some of the debug messages here, they could be readded.
  for (const std::string& branchName : m_branchNameIns) {
    int statIn = m_EventTree->GetBranch(branchName.c_str())->GetEntry(m_event);
    if (statIn <= 0) ATH_MSG_WARNING("Error in reading from branch " << branchName);
  }

  for (const std::string& branchName : m_branchNameOuts) {
    int statOut = m_EventTree->GetBranch(branchName.c_str())->GetEntry(m_event);
    if (statOut <= 0) ATH_MSG_WARNING("Error in reading from branch " << branchName);
  }
  
  m_event++;
  m_totevent++;


  return StatusCode::SUCCESS;
}

