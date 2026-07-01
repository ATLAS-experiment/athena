#include "FPGATrackSimDumpOutputStatAlg.h"

#include "FPGATrackSimObjects/FPGATrackSimLogicalEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimLogicalEventOutputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "TH2F.h"


FPGATrackSimDumpOutputStatAlg::FPGATrackSimDumpOutputStatAlg (const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator) {}


StatusCode FPGATrackSimDumpOutputStatAlg::initialize()
{
  ATH_MSG_INFO ( "FPGATrackSimDumpOutputStatAlg::initialize()");  
  ATH_CHECK( m_readOutputTool.retrieve());

  // TODO: This tool needs to be completely rewritten.
  // It currently is assuming that there's (at least) one input branch and one output branch in the ROOT files it runs over.
  // The changes we're making to split up the "main algorithm" will mean that's no longer the case.
  if (m_inputBranchName != "") {
    m_eventInputHeader = m_readOutputTool->addInputBranch(m_inputBranchName, false);
  }
  if (m_outputBranchName != "") {
    m_eventOutputHeader = m_readOutputTool->addOutputBranch(m_outputBranchName, false);
  }

  // Hook up the branches for reading.
  ATH_CHECK(m_readOutputTool->configureReadBranches());

  // Eventually add some histograms for monitoring
  ATH_CHECK(BookHistograms());
  
   // TO DO: must register the histograms to the TFile in the Tool
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimDumpOutputStatAlg::BookHistograms(){
  //m_hits_r_vs_z = new TH2F("h_hits_r_vs_z", "r/z ITK hit map; z[mm];r[mm]", 3500, 0., 3500., 450, 0., 450.);
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimDumpOutputStatAlg::execute(const EventContext& /*ctx*/) {
  ATH_MSG_DEBUG ("Running on event ");   
  
  bool last=false;
  ATH_CHECK (m_readOutputTool->readData(last));
  if (last) return StatusCode::SUCCESS;

  // Assuming we read in an input header... do this.
  if (m_eventInputHeader != nullptr) {
    ATH_MSG_VERBOSE (&m_eventInputHeader);
  }

  // fill histograms, assuming we read in an output header (and if there are any to fill).
  if (m_eventOutputHeader != nullptr) {
    for (const auto& track : m_eventOutputHeader->getFPGATrackSimTracks_1st()) {
      ATH_MSG_DEBUG (track);
      //commented out for future debugging
      //float r= std::sqrt(hit.getX()*hit.getX() + hit.getY()*hit.getY());
      //m_hits_r_vs_z->Fill(hit.getZ(), r);
    }

    for (const auto &track : m_eventOutputHeader->getFPGATrackSimTracks_2nd()) {
      ATH_MSG_DEBUG (track);
    }
  }

  // TODO: I suppose the reason for this was that new histograms would get appended to the ROOT file.
  // But... we don't currently make any, and so I'm not sure there's any reason to just copy the input objects to a new file.
  //ATH_CHECK (m_writeOutputTool->writeData(&eventInputHeader_1st, &eventInputHeader_2nd, &eventOutputHeader) );

  m_event += 1;

  return StatusCode::SUCCESS;
}

StatusCode FPGATrackSimDumpOutputStatAlg::finalize() {
    ATH_MSG_INFO("Processed " << m_event << " events.");
    return StatusCode::SUCCESS;
}
