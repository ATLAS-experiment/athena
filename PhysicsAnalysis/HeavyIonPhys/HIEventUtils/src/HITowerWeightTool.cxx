/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "HIEventUtils/HITowerWeightTool.h"
#include "PathResolver/PathResolver.h"

HITowerWeightTool::HITowerWeightTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent),
    m_h3W(nullptr),
    m_h3Eta(nullptr),
    m_h3Phi(nullptr),
    m_h3Mag(nullptr),
    m_h3EtaPhiResponse(nullptr),
    m_h3EtaPhiOffset(nullptr)
{
}


float HITowerWeightTool::getWeight(float eta, float phi, int sample) const
{
  return m_h3W->GetBinContent(m_h3W->FindFixBin(eta,phi,sample));
}
float HITowerWeightTool::getWeightEta(float eta, float phi, int sample) const
{
  return m_h3Eta->GetBinContent(m_h3Eta->FindFixBin(eta,phi,sample));
}
float HITowerWeightTool::getWeightPhi(float eta, float phi, int sample) const
{
  return m_h3Phi->GetBinContent(m_h3Phi->FindFixBin(eta,phi,sample));
}
float HITowerWeightTool::getWeightMag(float eta, float phi, int sample) const
{
  return m_h3Mag->GetBinContent(m_h3Mag->FindFixBin(eta,phi,sample));
}


float HITowerWeightTool::getEtaPhiResponse(float eta, float phi, int runIndex) const
{
  if(runIndex<=0)  return 1;

  int eb=std::as_const(m_h3EtaPhiResponse)->GetXaxis()->FindFixBin(eta);
  int pb=std::as_const(m_h3EtaPhiResponse)->GetYaxis()->FindFixBin(phi);
  return m_h3EtaPhiResponse->GetBinContent(eb,pb,runIndex);
}


float HITowerWeightTool::getEtaPhiOffset(float eta, float phi, int runIndex) const
{
  if(runIndex<=0) return 0;

  int eb=std::as_const(m_h3EtaPhiOffset)->GetXaxis()->FindFixBin(eta);
  int pb=std::as_const(m_h3EtaPhiOffset)->GetYaxis()->FindFixBin(phi);
  return m_h3EtaPhiOffset->GetBinContent(eb,pb,runIndex)*std::cosh(eta);
}


int HITowerWeightTool::getRunIndex(const EventContext& ctx) const
{
  if (!m_applycorrection){
    ATH_MSG_DEBUG("Using unit weights and doing no eta-phi correction.");
    return 0;
  }

  unsigned int run_number=ctx.eventID().run_number();

  auto itr=m_runMap.find(run_number);
  if(itr==m_runMap.end())
  {
    //trying generic run numbers <=> no run dependence
    for(auto run_number : m_defaultRunNumbers)
    {
      auto itrg=m_runMap.find(run_number);
      if(itrg!=m_runMap.end())
      {
        ATH_MSG_DEBUG("Using run " << run_number << " generic calibration for eta-phi correction.");
        return itrg->second;
      }
    }

    if(m_defaultRunNumbers.empty())
    {
      ATH_MSG_WARNING("No calibration for " << run_number << " is avaliable and no generic run numbers were set. Doing no eta-phi correction.");
    }
    else
    {
      std::string str_defaultRunNumbers="";
      for(auto run_number : m_defaultRunNumbers)
      {
        str_defaultRunNumbers+=std::to_string(run_number)+", ";
      }
      str_defaultRunNumbers.resize(str_defaultRunNumbers.length()-2);

      ATH_MSG_WARNING("No calibration for " << run_number << " is avaliable; no generic calibration for runs "<<str_defaultRunNumbers<<". Doing no eta-phi correction.");
    }

    return 0;
  }
  else 
  {
    return itr->second;
  }
}


StatusCode HITowerWeightTool::initialize()
{
  std::string local_path = static_cast<std::string>(m_configDir)+m_inputFile;
  std::string full_path=PathResolverFindCalibFile(local_path);
  ATH_MSG_INFO("Reading input file "<< m_inputFile << " from " << full_path);
  TFile* f=TFile::Open(full_path.c_str());
  if(f==nullptr)
  {
    ATH_MSG_FATAL("Cannot read config file " << full_path );
    return StatusCode::FAILURE;
  }

  m_h3W=(TH3F*)f->GetObjectChecked("h3_w","TH3F");
  if(m_h3W==nullptr)
  {
    ATH_MSG_FATAL("Cannot find TH3F m_h3W in config file " << full_path );
    return StatusCode::FAILURE;
  }

  m_h3Eta=(TH3F*)f->GetObjectChecked("h3_eta","TH3F");
  if(m_h3Eta==nullptr)
  {
    ATH_MSG_FATAL("Cannot find TH3F m_h3Eta in config file " << full_path );
    return StatusCode::FAILURE;
  }

  m_h3Phi=(TH3F*)f->GetObjectChecked("h3_phi","TH3F");
  if(m_h3Phi==nullptr)
  {
    ATH_MSG_FATAL("Cannot find TH3F m_h3Phi in config file " << full_path );
    return StatusCode::FAILURE;
  }

  m_h3Mag=(TH3F*)f->GetObjectChecked("h3_R","TH3F");
  if(m_h3Mag==nullptr)
  {
    ATH_MSG_FATAL("Cannot find TH3F m_h3Mag in config file " << full_path );
    return StatusCode::FAILURE;
  }

  m_h3W->SetDirectory(0);
  m_h3Eta->SetDirectory(0);
  m_h3Phi->SetDirectory(0);
  m_h3Mag->SetDirectory(0);

  if(m_applycorrection)
  {
    ATH_MSG_DEBUG("ApplyCorrection is set to true, now loading h3_eta_phi_response, h3_eta_phi_offset, and h1_run_index.");
    
    m_h3EtaPhiResponse=(TH3F*)f->GetObjectChecked("h3_eta_phi_response","TH3F");
    if(m_h3EtaPhiResponse==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F h3_eta_phi_response in config file " << full_path );
      return StatusCode::FAILURE;
    }
    m_h3EtaPhiResponse->SetDirectory(0);
    m_h3EtaPhiOffset=(TH3F*)f->GetObjectChecked("h3_eta_phi_offset","TH3F");
    if(m_h3EtaPhiOffset==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F h3_eta_phi_offset in config file " << full_path );
      return StatusCode::FAILURE;
    }
    m_h3EtaPhiOffset->SetDirectory(0);

    TH1I* h1_run_index=(TH1I*)f->GetObjectChecked("h1_run_index","TH1I");
    if(h1_run_index==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F h1_run_index in config file " << full_path );
      return StatusCode::FAILURE;
    }
    for(int xbin=1; xbin<=h1_run_index->GetNbinsX(); xbin++) {
      m_runMap.emplace_hint(m_runMap.end(),std::make_pair(h1_run_index->GetBinContent(xbin),xbin));
    }
  }
  else
  {
    ATH_MSG_DEBUG("ApplyCorrection is set to false, not loading h3_eta_phi_response, h3_eta_phi_offset, and h1_run_index.");
  }

  f->Close();
  return StatusCode::SUCCESS;
}
