/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
 
#include "L0MuonSmearingAlg.h"

#include "xAODTrigger/MuonRoIAuxContainer.h"

#include "TruthTrackSmearer.h"

#include "TH1.h"

namespace L0Muon {

L0MuonSmearingAlg::L0MuonSmearingAlg(const std::string& name, ISvcLocator* pSvcLocator)
: AthReentrantAlgorithm(name, pSvcLocator) {}


L0MuonSmearingAlg::~L0MuonSmearingAlg() {}


StatusCode L0MuonSmearingAlg::initialize() {
  ATH_MSG_INFO("Initializing " << name() << "...");

  ATH_CHECK(m_inputTruthParticleKey.initialize());
  ATH_CHECK(m_outputMuonRoIKey.initialize());

  ATH_CHECK(m_rndmSvc.retrieve());

  ATH_MSG_INFO("########## L0MuonSmearingAlg Configurations are ########## ");
  ATH_MSG_INFO("------- InputTruthParticleKey: " << m_inputTruthParticleKey.key());
  ATH_MSG_INFO("------- OutputMuonRoIKey: "<< m_outputMuonRoIKey.key());
  ATH_MSG_INFO("########## L0MuonSmearingAlg Configurations: That's it. ########## ");

  // configure the Smearer
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
  m_mySmearer = std::make_unique<TruthTrackSmearer>(rngWrapper);

  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
  return StatusCode::SUCCESS;
}


StatusCode L0MuonSmearingAlg::finalize() {
  ATH_MSG_INFO ("Finalizing " << name() << "...");
  return StatusCode::SUCCESS;
}


StatusCode L0MuonSmearingAlg::execute(const EventContext& ctx) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");

  SG::ReadHandle<xAOD::TruthParticleContainer> inputTruth_handle(m_inputTruthParticleKey, ctx);
  const xAOD::TruthParticleContainer* inputTruth = inputTruth_handle.cptr();
  if (not inputTruth) {
    ATH_MSG_FATAL("Unable to retrieve input truth particle of " << m_inputTruthParticleKey.key());
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<xAOD::MuonRoIContainer> outputRoI_handle(m_outputMuonRoIKey, ctx);
  ATH_CHECK(outputRoI_handle.record(std::make_unique<xAOD::MuonRoIContainer>(), std::make_unique<xAOD::MuonRoIAuxContainer>()));
  auto outputRoIs = outputRoI_handle.ptr();

  int n_input_tracks=0;
  int n_output_tracks=0;
  ATH_MSG_DEBUG("Found " << inputTruth->size() << " input truth particles of " << m_inputTruthParticleKey.key());

  for (const auto* part : *inputTruth) {
    // only muon is used
    if (part->pdgId() != 13 && part->pdgId() != -13) continue;
    // Minimum pT cut for 2 GeV
    if (part->pt() < 2000.) continue;   // MeV
    // only undecayed physical particle
    if (part->status() != 1) continue;
    // eta within 3.0
    if (std::abs(part->eta()) > 3.0) continue;

    ATH_MSG_DEBUG("New Truth Muon: phi=" << part->phi()
                              << " eta=" << part->eta()
                               << " pT=" << part->pt() / 1000.
                        << "(GeV) q/pT=" << part->charge() * 1000. / part->pt()
                     << "(1/GeV) PDGID=" << part->pdgId()
                           << " status=" << part->status());
 	  n_input_tracks++;

    if (!m_monTool.empty()) {
      auto track_input_eta = Monitored::Scalar<float>("track_input_eta", part->eta());
 	    auto track_input_pt = Monitored::Scalar<float>("track_input_pt", part->pt() / 1000.);
 	    auto track_input_phi = Monitored::Scalar<float>("track_input_phi", part->phi());
      auto track_input_curv = Monitored::Scalar<float>("track_input_curv", part->charge() * 1000. / part->pt());
      auto monitorIt = Monitored::Group(m_monTool, track_input_eta, track_input_pt, track_input_phi, track_input_curv);
    }

	  float qoverPt = part->charge() / part->pt();   // in MeV
    L0MuonTrack otrack;
    if (m_mySmearer->emulateL0MuonTrack(qoverPt, part->eta(), part->phi(), otrack) == false) {   // smearing here
      ATH_MSG_DEBUG("Killed by the efficiency: q/pt=" << qoverPt * 1000. << " (1/GeV)");
      continue;   // false means the truth muon is killed by the emulated efficiency
    }

 	  n_output_tracks++;

    ATH_MSG_DEBUG("L0Muon Track:   phi=" << otrack.phi()
                              << " eta=" << otrack.eta()
                              << " pT=" << 1. / std::abs(otrack.invpt() * 1000.)
                              << "(GeV) q/pT=" << 1000. * otrack.invpt() << "(1/GeV)");

    if (!m_monTool.empty()) {
      auto track_output_eta = Monitored::Scalar<float>("track_output_eta", otrack.eta());
      auto track_output_pt = Monitored::Scalar<float>("track_output_pt", 1./ std::abs(otrack.invpt() * 1000.));
      auto track_output_phi = Monitored::Scalar<float>("track_output_phi", otrack.phi());
      auto track_output_curv = Monitored::Scalar<float>("track_output_curv", otrack.invpt() * 1000.);
      auto monitorIt = Monitored::Group(m_monTool, track_output_eta, track_output_pt, track_output_phi, track_output_curv);
    }

    // RoI creation
    outputRoIs->push_back(std::make_unique<xAOD::MuonRoI>()); 

    // To fill RoI contents, below converts from floating to bit-wise values.
    static constexpr float PHI_ROISIZE = TMath::TwoPi() / static_cast<float>(xAOD::MuonRoI::PHI_MASK);  // phi divided by 9 bits
    static constexpr float ETA_ROISIZE = 5.4 / static_cast<float>(xAOD::MuonRoI::ETA_MASK);             // eta divided by 14 bits
    static constexpr float PT_ROISIZE = 127.5 / static_cast<float>(xAOD::MuonRoI::PT_MASK);             // 0.5 GeV width for 0-128 GeV (8 bits)

    uint32_t phiword = uint32_t((otrack.phi()+TMath::Pi()) / PHI_ROISIZE);  // phi defined from 0 to 2pi in MuonRoI, instead of -pi to pi
    uint32_t etaword = uint32_t((otrack.eta() + 2.7) / ETA_ROISIZE);        // -2.7 to 2.7 in MuonRoI.
    uint32_t ptword = static_cast<uint32_t>(1. / std::abs(otrack.invpt() * 1000.) / PT_ROISIZE);
    if (ptword > 0xff) ptword = 0xff;

    // to still input eta and phi for initialize, not calculated from RoIword
    float roi_eta = etaword * ETA_ROISIZE - 2.7;
    float roi_phi = phiword * PHI_ROISIZE - TMath::Pi();

    // construct roiWord (23th bit is charge information.)
    uint32_t roiword = (ptword<<24) | ((otrack.invpt() > 0)<<23) | (phiword<<14) | etaword;

    uint32_t extraword = (static_cast<uint32_t>(0x1)<<31) | (((ptword>>1) + 0x2) & 0xf);  // for the time being...

    std::string emu_thr_name = "L0_MUx";
    float thrvalue = static_cast<float>(((ptword>>1) + 0x2) & 0xf);
    outputRoIs->back()->initialize(roiword, roi_eta, roi_phi, emu_thr_name, thrvalue, extraword);

    ATH_MSG_DEBUG("L0MuonRoI: phi = " << roi_phi << " (0x" << std::hex << phiword << std::dec << "), "
                      << "eta = " << roi_eta << " (0x" << std::hex << etaword << std::dec << "), "
                      << "pT = " << outputRoIs->back()->pt() << " (GeV) (0x" << std::hex << ptword << std::dec << "), "
                      << "q/pT= " << ((otrack.invpt() > 0) ? 1. : -1.) / outputRoIs->back()->pt());

    if (!m_monTool.empty()) {
      auto roi_output_eta = Monitored::Scalar<float>("roi_output_eta", outputRoIs->back()->eta());
      auto roi_output_phi = Monitored::Scalar<float>("roi_output_phi", outputRoIs->back()->phi());
      auto roi_output_pt = Monitored::Scalar<float>("roi_output_pt", outputRoIs->back()->pt());
      auto roi_output_curv = Monitored::Scalar<float>("roi_output_curv", (outputRoIs->back()->getCharge() > 0 ? 1. : -1.)/outputRoIs->back()->pt());

      auto delta_eta = Monitored::Scalar<float>("delta_eta", outputRoIs->back()->eta() - part->eta());  
      auto delta_phi = Monitored::Scalar<float>("delta_phi", TVector2::Phi_mpi_pi(outputRoIs->back()->phi() - (part->phi())));
      auto delta_pt = Monitored::Scalar<float>("delta_pt", (outputRoIs->back()->pt() - part->pt()/1000.) / (part->pt()/1000.));
      auto delta_curv = Monitored::Scalar<float>("delta_curv", ((outputRoIs->back()->getCharge() == 1 ? 1. : -1.)/outputRoIs->back()->pt() - (part->charge()*1000./part->pt())) / (part->charge()*1000./part->pt()));

      auto monitorIt = Monitored::Group(m_monTool, roi_output_eta, roi_output_pt, roi_output_phi, roi_output_curv,
                                                   delta_eta, delta_phi, delta_pt, delta_curv);
    }
  }

  ATH_MSG_DEBUG ("Result: Number of input truth muons= " << n_input_tracks << " , smeared tracks= "<< n_output_tracks);

  return StatusCode::SUCCESS;
}


}   // end of namespace
