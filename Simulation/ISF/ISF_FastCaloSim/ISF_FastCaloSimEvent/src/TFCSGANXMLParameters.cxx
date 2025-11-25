/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TFCSGANXMLParameters.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// Class header include
#include "ISF_FastCaloSimEvent/TFCSGANXMLParameters.h"

TFCSGANXMLParameters::TFCSGANXMLParameters() = default;

TFCSGANXMLParameters::~TFCSGANXMLParameters() = default;

void TFCSGANXMLParameters::InitialiseFromXML(
    int pid, int etaMid, const std::string& FastCaloGANInputFolderName) {

  m_fastCaloGANInputFolderName = FastCaloGANInputFolderName;
  std::string xmlFullFileName = FastCaloGANInputFolderName + "/binning.xml";

  // Parse the XML file
  xmlDocPtr doc = xmlParseFile(xmlFullFileName.c_str());
  if (!doc) {
    ATH_MSG_WARNING("Failed to parse XML file: " << xmlFullFileName);
    return;
  }

  for (xmlNodePtr nodeRoot = doc->children; nodeRoot != nullptr;
       nodeRoot = nodeRoot->next) {
    if (xmlStrEqual(nodeRoot->name, BAD_CAST "Bins")) {
      for (xmlNodePtr nodeParticle = nodeRoot->children;
           nodeParticle != nullptr; nodeParticle = nodeParticle->next) {
        if (xmlStrEqual(nodeParticle->name, BAD_CAST "Particle")) {
          int nodePid = std::stoi(reinterpret_cast<const char*>(
              xmlGetProp(nodeParticle, BAD_CAST "pid")));

          if (nodePid == pid) {
            for (xmlNodePtr nodeBin = nodeParticle->children;
                 nodeBin != nullptr; nodeBin = nodeBin->next) {
              if (xmlStrEqual(nodeBin->name, BAD_CAST "Bin")) {
                int nodeEtaMin = std::stoi(reinterpret_cast<const char*>(
                    xmlGetProp(nodeBin, BAD_CAST "etaMin")));
                int nodeEtaMax = std::stoi(reinterpret_cast<const char*>(
                    xmlGetProp(nodeBin, BAD_CAST "etaMax")));
                int regionId = std::stoi(reinterpret_cast<const char*>(
                    xmlGetProp(nodeBin, BAD_CAST "regionId")));

                if (std::abs(etaMid) > nodeEtaMin &&
                    std::abs(etaMid) < nodeEtaMax) {

                  m_symmetrisedAlpha =
                      ReadBooleanAttribute("symmetriseAlpha", nodeParticle);
                  m_ganVersion = std::stod(reinterpret_cast<const char*>(
                      xmlGetProp(nodeBin, BAD_CAST "ganVersion")));
                  m_latentDim = std::stod(reinterpret_cast<const char*>(
                      xmlGetProp(nodeParticle, BAD_CAST "latentDim")));

                  for (xmlNodePtr nodeLayer = nodeBin->children;
                       nodeLayer != nullptr; nodeLayer = nodeLayer->next) {
                    if (xmlStrEqual(nodeLayer->name, BAD_CAST "Layer")) {
                      std::vector<double> edges;
                      std::string s(reinterpret_cast<const char*>(
                          xmlGetProp(nodeLayer, BAD_CAST "r_edges")));

                      std::istringstream ss(s);
                      std::string token;

                      while (std::getline(ss, token, ',')) {
                        edges.push_back(std::stod(token));
                      }

                      int binsInAlpha = std::stoi(reinterpret_cast<const char*>(
                          xmlGetProp(nodeLayer, BAD_CAST "n_bin_alpha")));
                      int layer = std::stoi(reinterpret_cast<const char*>(
                          xmlGetProp(nodeLayer, BAD_CAST "id")));

                      std::string name = "hist_pid_" + std::to_string(nodePid) +
                                         "_region_" + std::to_string(regionId) +
                                         "_layer_" + std::to_string(layer);
                      int xBins = static_cast<int>(edges.size()) - 1;

                      if (xBins <= 0) {
                        ATH_MSG_DEBUG(
                            "No bins defined in r for layer "
                            << layer
                            << ", setting to 1 bin to avoid empty histogram");
                        xBins = 1;  // Remove warning and set a default bin
                        edges.push_back (edges.back()+1);
                      } else {
                        m_relevantlayers.push_back(layer);
                      }

                      double minAlpha = -M_PI;
                      if (m_symmetrisedAlpha && binsInAlpha > 1) {
                        minAlpha = 0;
                      }
                      // Create histogram and add to binning map
                      m_binning.emplace(
                          layer,
                          TH2D(name.c_str(), name.c_str(), xBins, edges.data(),
                               binsInAlpha, minAlpha, M_PI));
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Free XML document after parsing
  xmlFreeDoc(doc);
}

bool TFCSGANXMLParameters::ReadBooleanAttribute(const std::string& name,
                                                xmlNodePtr node) {
  std::string attribute =
      reinterpret_cast<const char*>(xmlGetProp(node, BAD_CAST name.c_str()));
  return attribute == "true";
}

void TFCSGANXMLParameters::Print() const {
  ATH_MSG_INFO("Parameters taken from XML");
  ATH_MSG_INFO("  symmetrisedAlpha: " << m_symmetrisedAlpha);
  ATH_MSG_INFO("  ganVersion:" << m_ganVersion);
  ATH_MSG_INFO("  latentDim: " << m_latentDim);
  ATH_MSG(INFO) << "  relevantlayers: ";
  for (const auto& l : m_relevantlayers) {
    ATH_MSG(INFO) << l << " ";
  }
  ATH_MSG(INFO) << END_MSG(INFO);

  for (const auto& element : m_binning) {
    int layer = element.first;
    const TH2D* h = &element.second;

    // attempt to debug intermittent ci issues described in
    // https://its.cern.ch/jira/browse/ATLASSIM-7031
    if (h->IsZombie()) {
      ATH_MSG_WARNING("Histogram pointer for layer "
                      << layer << " is broken. Skipping.");
      continue;
    }

    int xBinNum = h->GetNbinsX();
    const TAxis* x = h->GetXaxis();

    if (xBinNum == 1) {
      ATH_MSG_INFO("layer " << layer << " not used");
      continue;
    }
    ATH_MSG_INFO("Binning along r for layer " << layer);
    ATH_MSG(INFO) << "0,";
    // First fill energies
    for (int ix = 1; ix <= xBinNum; ++ix) {
      ATH_MSG(INFO) << x->GetBinUpEdge(ix) << ",";
    }
    ATH_MSG(INFO) << END_MSG(INFO);
  }
}
