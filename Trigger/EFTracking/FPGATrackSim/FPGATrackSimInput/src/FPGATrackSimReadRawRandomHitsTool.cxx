/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimInput/FPGATrackSimReadRawRandomHitsTool.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"


 FPGATrackSimReadRawRandomHitsTool::FPGATrackSimReadRawRandomHitsTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
   base_class(algname,name,ifc) {}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
StatusCode FPGATrackSimReadRawRandomHitsTool::initialize(){

   // open input file
  ATH_MSG_INFO ( "Opening input file: "  << m_inpath.value() );
  m_infile = TFile::Open(m_inpath.value().c_str(),"READ");
  if (m_infile == nullptr){
    ATH_MSG_FATAL ("Could not open input file: " << m_inpath.value() );
    return StatusCode::FAILURE;
  }

  //get the tree, try old and new versions for backwards compatability
  m_EventTree = (TTree*) m_infile->Get("FPGATrackSimEventTree");
  if (m_EventTree == nullptr || m_EventTree->GetEntries() == -1 ){
    ATH_MSG_FATAL ("Input file: " << m_inpath << " has no entries");
    return StatusCode::FAILURE;
  }

  std::string branchName="FPGATrackSimEventInputHeader";
  if(!m_EventTree->GetListOfBranches()->FindObject(branchName.c_str())){
    ATH_MSG_FATAL ("Branch: " << branchName << " not found!");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO ( "Getting branch and set EventHeader" );
  TBranch *branch  = m_EventTree->GetBranch(branchName.c_str());
  branch->Print();

  m_eventHeader = new FPGATrackSimEventInputHeader();
  branch->SetAddress(&m_eventHeader);

  return StatusCode::SUCCESS;
}



StatusCode FPGATrackSimReadRawRandomHitsTool::readData(FPGATrackSimEventInputHeader* header, bool &last) const
{
  return readData(header, last, false); // by default with this tool don't reset data
}

StatusCode FPGATrackSimReadRawRandomHitsTool::readData(FPGATrackSimEventInputHeader* header, bool &last, bool doReset) const
{
  std::lock_guard<std::mutex> lock(m_readMutex);
  
  last = false;
  m_EventTree->GetEntry(m_entry++);

  // Truth Info
  FPGATrackSimOptionalEventInfo optional;
  unsigned long int mbc = 0; // to scale up barcodes!

  // --- Copy old data
  if (doReset)
    {
      header->reset(); //reset things!
    }
  else // not resetting, start by copying over truth information from old header
    {
      for (const auto& truthtrack : header->optional().getTruthTracks())
        {
	  if (truthtrack.getBarcode() > mbc) mbc = truthtrack.getBarcode();
      	  optional.addTruthTrack(truthtrack);
        }

      // now we got the max bar code, copy the offline tracks
      for (const auto& offlinetrack : header->optional().getOfflineTracks()) 
	      { 
      	  optional.addOfflineTrack(offlinetrack);
      	}
    }

  // --- Copy new data
  for (auto truthtrack : m_eventHeader->optional().getTruthTracks())
    {
      truthtrack.setBarcode(truthtrack.getBarcode() + mbc);
      optional.addTruthTrack(truthtrack);
    }
  for (auto offlinetrack : m_eventHeader->optional().getOfflineTracks()) 
    {
      offlinetrack.setBarcode(offlinetrack.getBarcode() + mbc);
      optional.addOfflineTrack(offlinetrack);
    }

  header->setOptional(optional);

  // copy Hits
  for (auto rawhit : m_eventHeader->hits())
    {
      FPGATrackSimMultiTruth origtruth = rawhit.getTruth();
      FPGATrackSimMultiTruth mt;
      FPGATrackSimMultiTruth::Barcode uniquecode(rawhit.getEventIndex(),rawhit.getBarcode()+mbc);
      mt.maximize(uniquecode, rawhit.getBarcodePt());
      rawhit.setBarcode(rawhit.getBarcode() + mbc);
      rawhit.setTruth(mt);
      header->addHit(rawhit);
    }

  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimReadRawRandomHitsTool::writeData(FPGATrackSimEventInputHeader* /*header*/) const {
  ATH_MSG_FATAL("This tool is being forced to write things. But it is only designed to read things. Don't worry, everything is fine");
  return StatusCode::FAILURE;// this tool is not designed to write things
}


StatusCode FPGATrackSimReadRawRandomHitsTool::finalize(){
  delete m_eventHeader;
  return StatusCode::SUCCESS;
}
