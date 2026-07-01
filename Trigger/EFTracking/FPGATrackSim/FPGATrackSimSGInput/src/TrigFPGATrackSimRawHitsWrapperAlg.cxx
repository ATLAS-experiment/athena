/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimTowerInputHeader.h"

#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"

#include "TrigFPGATrackSimRawHitsWrapperAlg.h"


TrigFPGATrackSimRawHitsWrapperAlg::TrigFPGATrackSimRawHitsWrapperAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{}


StatusCode TrigFPGATrackSimRawHitsWrapperAlg::initialize()
{
  ATH_MSG_DEBUG("TrigFPGATrackSimRawHitsWrapperAlg::initialize()");


  ATH_CHECK(m_hitInputTool.retrieve());
  ATH_MSG_DEBUG("Setting FPGATrackSim_SGHitInput tool");

  ATH_MSG_INFO("Creating output file: " << m_outpath);
  TNamed meta("metadata",m_metaData.value().c_str());
  m_outfile = TFile::Open(m_outpath.value().c_str(), "recreate");
  meta.Write();
  m_eventHeader = new FPGATrackSimEventInputHeader();

  ATH_MSG_DEBUG("instantiating tree");
  m_EventTree = new TTree("FPGATrackSimEventTree", "data");

  ATH_MSG_DEBUG("Setting branch");
  m_EventTree->Branch("FPGATrackSimEventInputHeader",
    "FPGATrackSimEventInputHeader",
    &m_eventHeader, 6400, 1);

  ATH_MSG_DEBUG("Finished initialize");

  return StatusCode::SUCCESS;
}


StatusCode TrigFPGATrackSimRawHitsWrapperAlg::execute(const EventContext& ctx) {
  ATH_MSG_DEBUG("Running on event ");

  ATH_CHECK(m_eventHeader != nullptr);
  ATH_CHECK(m_hitInputTool->readData(m_eventHeader, ctx));
  m_EventTree->Fill();
  return StatusCode::SUCCESS;

}


StatusCode TrigFPGATrackSimRawHitsWrapperAlg::finalize()
{
  // close the output files, but check that it exists (for athenaMP)
  if (m_outfile) {
    m_outfile->Write();
    m_outfile->Close();
  }
  delete m_eventHeader;
  return StatusCode::SUCCESS;
}
