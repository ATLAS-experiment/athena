/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "PhotonVertexSelection/PhotonPointingTool.h"
#include "PhotonVertexSelection/PhotonVertexHelpers.h"

// EDM includes
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEgamma/EgammaDefs.h"
#include "xAODMetaData/FileMetaData.h"

// Framework includes
#include "egammaUtils/ShowerDepthUtil.h"
#include "egammaUtils/egPhotonWrtPoint.h"
#include "PathResolver/PathResolver.h"

// ROOT includes
#include "TFile.h"
#include "TH1F.h"

// std includes
#include <algorithm>

namespace CP {

//____________________________________________________________________________
PhotonPointingTool::PhotonPointingTool(const std::string &name)
  : asg::AsgMetadataTool(name)
    , m_zCorrection(nullptr)
{
  declareProperty("isSimulation", m_isMC);
  declareProperty("zOscillationFileMC", m_zOscFileMC = "PhotonVertexSelection/v1/pointing_correction_mc.root");
  declareProperty("zOscillationFileData", m_zOscFileData = "PhotonVertexSelection/v1/pointing_correction_data.root");
  declareProperty("zvertex", m_zvertexDecorName = "zvertex");
  declareProperty("errz", m_errzDecorName = "errz");
  declareProperty("HPV_zvertex", m_HPV_zvertexDecorName = "HPV_zvertex");
  declareProperty("HPV_errz", m_HPV_errzDecorName = "HPV_errz");
  declareProperty("ContainerName", m_ContainerName = "Photons");
}

//____________________________________________________________________________
PhotonPointingTool::~PhotonPointingTool()
{
  SafeDelete(m_zCorrection);
}

//____________________________________________________________________________
StatusCode PhotonPointingTool::initialize()
{
  ATH_MSG_INFO("Initializing PhotonVertexSelectionTool..." << name());

  ATH_CHECK( m_evtInfo.initialize() );

  if(m_ContainerName.empty()) m_ContainerName = "Photons";

  m_zvertex = m_ContainerName + "." + m_zvertexDecorName;
  m_errz = m_ContainerName + "." + m_errzDecorName;
  m_HPV_zvertex = m_ContainerName + "." + m_HPV_zvertexDecorName;
  m_HPV_errz = m_ContainerName + "." + m_HPV_errzDecorName;

  ATH_CHECK(m_zvertex.initialize());
  ATH_CHECK(m_errz.initialize());
  ATH_CHECK(m_HPV_zvertex.initialize());
  ATH_CHECK(m_HPV_errz.initialize());

  // Determine if this is MC or data
  std::string dataType("");
  m_isMC = true;
  if (inputMetaStore()->contains<xAOD::FileMetaData>("FileMetaData")) {
    const xAOD::FileMetaData* fmd = nullptr;
    ATH_CHECK(inputMetaStore()->retrieve(fmd, "FileMetaData"));
    std::string simType("");
    const bool s = fmd->value(xAOD::FileMetaData::simFlavour, simType);
    if (!s) {
      ATH_MSG_DEBUG("no sim flavour from metadata: must be data");
      m_isMC = false;
    }
  } else {
    ATH_MSG_WARNING("Failed to retrieve FileMetaData : assuming to be MC");
  }


  std::string filepath = PathResolverFindCalibFile(m_isMC ? m_zOscFileMC :
                                                   m_zOscFileData);
  TFile *file = TFile::Open(filepath.c_str(), "READ");

  if (file == nullptr) {
    ATH_MSG_WARNING("Couldn't find file for z-correction: " << filepath.c_str());
    ATH_MSG_WARNING("Failed to initialize.");
    return StatusCode::FAILURE;
  }

  TH1F *temp = nullptr;
  file->GetObject("dz_trk_pointing_vs_etas2", temp);

  if (temp == nullptr) {
    ATH_MSG_WARNING("Couldn't find 'dz_trk_pointing_vs_etas2' histogram in file: " << filepath.c_str());
    ATH_MSG_WARNING("Failed to initialize.");
    return StatusCode::FAILURE;
  }

  bool status = TH1::AddDirectoryStatus();
  TH1::AddDirectory(false);
  m_zCorrection = dynamic_cast<TH1F*>(temp->Clone("zCorrection"));
  SafeDelete(file);
  TH1::AddDirectory(status);

  return StatusCode::SUCCESS;
}

//____________________________________________________________________________
StatusCode PhotonPointingTool::updatePointingAuxdata(const xAOD::EgammaContainer &egammas) const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // create the decorators
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> s_zvertex(m_zvertex, ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> s_errz(m_errz, ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> s_HPV_zvertex(m_HPV_zvertex, ctx);
  SG::WriteDecorHandle<xAOD::EgammaContainer, float> s_HPV_errz(m_HPV_errz, ctx);

  bool isDecorAvailable = s_zvertex.isAvailable();

  // Loop over photons and add calo pointing auxdata
  std::pair<float, float> result;
  for (const auto *egamma: egammas) {
    if(egamma==nullptr){
      ATH_MSG_DEBUG("Passed Egamma was a nullptr  -- skipping");
      continue;
    }
    // Get calo pointing variables
    result = getCaloPointing(egamma);

    // Set photon auxdata with new value
    if(isDecorAvailable){
      ATH_MSG_DEBUG("Called updatePointingAuxdata but decoration is already available");
    }
    else{
      s_zvertex(*egamma) = result.first;
      s_errz(*egamma)    = result.second;
    }

    // Get conv pointing variables
    if (egamma->type() == xAOD::Type::Photon) {
      const xAOD::Egamma *eg     = static_cast<const xAOD::Egamma*>(egamma);
      const xAOD::Photon *photon = dynamic_cast<const xAOD::Photon*>(eg);

      if (photon && xAOD::EgammaHelpers::numberOfSiTracks(photon))
        result = getConvPointing(photon);
    }

    // Set photon auxdata with new value
    if(!isDecorAvailable){
      s_HPV_zvertex(*egamma) = result.first;
      s_HPV_errz(*egamma)    = result.second;
    }

  }

  return StatusCode::SUCCESS;
}

  //____________________________________________________________________________
  float PhotonPointingTool::getCorrectedZ(float zPointing, float etas2) const
  {
    if (fabs(etas2) < 1.37) return zPointing;
    return zPointing - m_zCorrection->Interpolate(etas2);
  }

  //____________________________________________________________________________
  std::pair<float, float> PhotonPointingTool::getCaloPointing(const xAOD::Egamma *egamma) const
  {
    if (egamma == nullptr) {
      ATH_MSG_WARNING("Passed Egamma was a nullptr, returning (0,0).");
      return std::make_pair(0,0);
    }

    // Get the EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfo);
    if(!eventInfo.isValid()){
      ATH_MSG_WARNING("Couldn't retrieve EventInfo from TEvent, egamma won't be decorated.");
      return std::make_pair(0, 0);
    }

    // Beam parameters
    double d_beamSpot   = hypot(eventInfo->beamPosY(), eventInfo->beamPosX());
    double phi_beamSpot = atan2(eventInfo->beamPosY(), eventInfo->beamPosX());

    // Photon variables
    const xAOD::CaloCluster *cluster = egamma->caloCluster();
    if (cluster == nullptr) {
      ATH_MSG_WARNING("Couldn't retrieve CaloCluster from Photon, egammas won't be decorated.");
      return std::make_pair(0, 0);
    }

    float cl_e     = cluster->e();
    float etas1    = cluster->etaBE(1);
    float etas2    = cluster->etaBE(2);
    float phis2    = cluster->phiBE(2);
    float cl_theta = 2.0*atan(exp(-1.0*cluster->eta()));

    // Shower depths
    std::pair<float, float> RZ1 = CP::ShowerDepthUtil::getRZ(etas1, 1);
    std::pair<float, float> RZ2 = CP::ShowerDepthUtil::getRZ(etas2, 2);

    // Calo cluster pointing calculation
    double r0_with_beamSpot = d_beamSpot*cos(phis2 - phi_beamSpot);

    float s_zvertex = 0, s_errz = 0;
    s_zvertex = (RZ1.second * (RZ2.first - r0_with_beamSpot) -
                 RZ2.second * (RZ1.first - r0_with_beamSpot)) /
                (RZ2.first - RZ1.first);
    s_zvertex = getCorrectedZ(s_zvertex, etas2);

    s_errz = 0.5 * (RZ2.first + RZ1.first) * (0.060 / sqrt(cl_e * 0.001)) /
             (sin(cl_theta) * sin(cl_theta));

    return std::make_pair(s_zvertex, s_errz);
  }

  //____________________________________________________________________________
  std::pair<float, float> PhotonPointingTool::getConvPointing(const xAOD::Photon *photon) const
  {
    if (photon == nullptr) {
      ATH_MSG_WARNING("Passed Egamma was a nullptr, returning (0,0).");
      return std::make_pair(0,0);
    }

    // Get the EventInfo
    SG::ReadHandle<xAOD::EventInfo> eventInfo(m_evtInfo);
    if(!eventInfo.isValid()){
      ATH_MSG_WARNING("Couldn't retrieve EventInfo from TEvent, photons won't be decorated.");
      return std::make_pair(0, 0);
    }

    // Beam parameters
    double d_beamSpot   = hypot(eventInfo->beamPosY(), eventInfo->beamPosX());
    double phi_beamSpot = atan2(eventInfo->beamPosY(), eventInfo->beamPosX());

    // Photon variables
    const xAOD::CaloCluster *cluster = photon->caloCluster();
    if (cluster == nullptr) {
      ATH_MSG_WARNING("Couldn't retrieve CaloCluster from Photon, photons won't be decorated.");
      return std::make_pair(0, 0);
    }

    float etas1    = cluster->etaBE(1);
    float phis2    = cluster->phiBE(2);

    const xAOD::Vertex *conv = photon->vertex();
    if (cluster == nullptr) {
      ATH_MSG_WARNING("Couldn't retrieve conversion Vertex from Photon, photons won't be decorated.");
      return std::make_pair(0, 0);
    }

    float conv_x = conv->x();
    float conv_y = conv->y();
    float conv_z = conv->z();

    // Shower depths
    std::pair<float, float> RZ1 = CP::ShowerDepthUtil::getRZ(etas1, 1);

    // Photon conversion
    double conv_r = hypot(conv_x, conv_y);

    // HPV pointing calculation
    double phi_Calo         = atan2(sin(phis2), cos(phis2));
    double r0_with_beamSpot = d_beamSpot*cos(phi_Calo - phi_beamSpot);

    float s_zvertex = (RZ1.second*(conv_r - r0_with_beamSpot) - conv_z*(RZ1.first - r0_with_beamSpot)) / (conv_r - RZ1.first);

    float dist_vtx_to_conv = hypot(conv_r - r0_with_beamSpot, conv_z - s_zvertex);
    float dist_conv_to_s1 = hypot(RZ1.first - conv_r, RZ1.second - conv_z);

    float error_etaS1 = 0.001; // FIXME is there a tool which provides a better value?
    float s_errz = 0.0;

    if ((cluster->inBarrel() && !cluster->inEndcap()) ||
        (cluster->inBarrel() &&  cluster->inEndcap() && cluster->eSample(CaloSampling::EMB1) > cluster->eSample(CaloSampling::EME1))) {
      // Barrel case
      float error_Z_Calo_1st_Sampling_barrel = error_etaS1*RZ1.first*fabs(cosh(etas1));
      s_errz = error_Z_Calo_1st_Sampling_barrel*dist_vtx_to_conv/dist_conv_to_s1;
    } else {
      // Endcap case
      float error_R_Calo_1st_Sampling_endcap = error_etaS1*cosh(etas1)*RZ1.first*RZ1.first/fabs(RZ1.second);
      s_errz = error_R_Calo_1st_Sampling_endcap*fabs(sinh(etas1))*dist_vtx_to_conv/dist_conv_to_s1;
    }

    return std::make_pair(s_zvertex, s_errz);
  }

} // namespace CP
