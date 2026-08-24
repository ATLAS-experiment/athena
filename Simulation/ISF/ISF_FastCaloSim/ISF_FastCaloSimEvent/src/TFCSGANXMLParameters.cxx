/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TFCSGANXMLParameters.cxx, (c) ATLAS Detector software             //
///////////////////////////////////////////////////////////////////

// Class header include
#include "ISF_FastCaloSimEvent/TFCSGANXMLParameters.h"

#include "XMLCoreParser/XMLCoreParser.h"
#include "XMLCoreParser/XMLCoreNode.h"

#include "CxxUtils/hexdump.h"
#include "CxxUtils/StringUtils.h"
#include <format>

TFCSGANXMLParameters::TFCSGANXMLParameters() = default;

TFCSGANXMLParameters::~TFCSGANXMLParameters() = default;

void TFCSGANXMLParameters::InitialiseFromXML(
    int pid, int etaMid, const std::string& FastCaloGANInputFolderName) {

  m_fastCaloGANInputFolderName = FastCaloGANInputFolderName;
  std::string xmlFullFileName = FastCaloGANInputFolderName + "/binning.xml";

  // Parse the XML file
  XMLCoreParser p;
  std::unique_ptr<XMLCoreNode> doc = p.parse (xmlFullFileName);

  if (!doc) {
    ATH_MSG_WARNING("Failed to parse XML file: " << xmlFullFileName);
    return;
  }

  for (const XMLCoreNode* nodeParticle : doc->get_children ("Bins/Particle")) {
    if (nodeParticle->get_int_attrib ("pid") == pid) {
      for (const XMLCoreNode* nodeBin : nodeParticle->get_children ("Bin")) {
        int nodeEtaMin = nodeBin->get_int_attrib ("etaMin");
        int nodeEtaMax = nodeBin->get_int_attrib ("etaMax");
        int regionId = nodeBin->get_int_attrib ("regionId");

        if (std::abs(etaMid) > nodeEtaMin &&
            std::abs(etaMid) < nodeEtaMax)
        {
          m_symmetrisedAlpha = nodeBin->has_attrib ("symmetriseAlpha") &&
            nodeBin->get_attrib ("symmetriseAlpha") == "true";
          m_ganVersion = nodeBin->get_int_attrib ("ganVersion");
          m_latentDim = nodeParticle->get_int_attrib ("latentDim");

          for (const XMLCoreNode* nodeLayer : nodeBin->get_children ("Layer")) {
            const std::string &rEdgesStr = nodeLayer->get_attrib("r_edges");
            std::vector<double> edges = CxxUtils::tokenizeDouble(rEdgesStr, ",");

            int binsInAlpha = nodeLayer->get_int_attrib ("n_bin_alpha");
            int layer = nodeLayer->get_int_attrib ("id");

            const std::string name = std::format("hist_pid_{}_region_{}_layer_{}", pid, regionId, layer);
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
            auto itr = m_binning.emplace(
                                         layer,
                                         TH2D(name.c_str(), name.c_str(), xBins, edges.data(),
                                              binsInAlpha, minAlpha, M_PI));
            itr.first->second.SetDirectory(nullptr);
            ROOT::Internal::MarkTObjectAsNotOnHeap(itr.first->second);
          }
        }
      }
    }
  }
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


void TFCSGANXMLParameters::fixHists()
{
  for (auto &[layer, h] : m_binning) {
    // The histograms we've read in are in an STL container.
    // Rarely, ROOT can falsely set the kIsOnHeap flag on one of them.
    // In that case, it will try to delete the histogram when the
    // file is closed, which will lead to a crash later on.
    // Make sure kIsOnHeap is clear, and also make sure that nobody else
    // thinks that they own one of these histograms.
    // See ATLASSIM-7031.
    h.SetDirectory (nullptr);
    h.ResetBit (TObject::kIsOnHeap);
  }
}
