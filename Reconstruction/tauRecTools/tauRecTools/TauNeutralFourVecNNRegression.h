/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TauNeutralFourVecNNRegression_H
#define TAURECTOOLS_TauNeutralFourVecNNRegression_H

// base class include(s)
#include "tauRecTools/TauRecToolBase.h"

// xAOD include(s)
#include "xAODTau/TauJet.h"
#include "xAODTau/TauJetContainer.h"

#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/ReadDecorHandle.h"

// lwtnn include(s)
#include "lwtnn/LightweightGraph.hh"
#include "lwtnn/parse_json.hh"
#include "lwtnn/Exceptions.hh"

// standard library include(s)
#include <memory>
#include <vector>
#include <set>
#include <map>

/**
 * @brief Tau pi0 four-vector reconstruction using a neural network
 *
 * @author S. Thiele
 *
 */

class TauNeutralFourVecNNRegression : public TauRecToolBase
{
public:
  ASG_TOOL_CLASS2(TauNeutralFourVecNNRegression, TauRecToolBase, ITauToolBase)

  explicit TauNeutralFourVecNNRegression(const std::string &name = "TauNeutralFourVecNNRegression");
  virtual ~TauNeutralFourVecNNRegression();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(xAOD::TauJet &xTau) const override;

private:
    // properties of the tool
    // Gaudi::Property<std::string> m_outputName{this, "OutputName", "TauNeutralFourVec"}; // not needed, since we decorate three individual values, instead of one vector, so there's not just 1 Output
  Gaudi::Property<std::string> m_outputPrefix{this, "OutputPrefix", "neutralFourVecNN_"};
  Gaudi::Property<std::string> m_weightFile_1p1n{this, "WeightFile_1p1n", ""};
  Gaudi::Property<std::string> m_weightFile_1pXn{this, "WeightFile_1pXn", ""};
  Gaudi::Property<std::string> m_weightFile_3pXn{this, "WeightFile_3pXn", ""};
  Gaudi::Property<std::size_t> m_maxTauTracks{this, "MaxTauTracks", 3};
  Gaudi::Property<std::size_t> m_maxNeutralPFOs{this, "MaxNeutralPFOs", 8};
  Gaudi::Property<std::size_t> m_maxShotPFOs{this, "MaxShotPFOs", 6};
  Gaudi::Property<std::size_t> m_maxConvTracks{this, "MaxConvTracks", 4};
  Gaudi::Property<float> m_neutralPFOPtCut{this, "NeutralPFOPtCut", 1.5};
  // Gaudi::Property<std::array<std::string, 3>> m_fourVecDimNames{this, "FourVecDimNames", {"E", "eta", "phi"}};
  /**
   * @brief retrieve the input variables from a TauJet
   * @param xTau a TauJet object
   * @param inputSeqMap a map that contain several sequences
   * 
   * each sequence contains its input variables stored in a vector
   * this map is used by the lwtnn graph
   */
  virtual StatusCode getInputs(const xAOD::TauJet &xTau,
                               std::map<std::string, std::map<std::string, std::vector<double>>> &inputSeqMap) const;

  SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_decayModeName {this,
      "decayModeNameKey", 
      "TauJets.NNDecayMode",
      "Decoration for Tau Decay Mode"};

  /// lwtnn graph
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_1p1n; //!
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_1pXn; //!
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_3pXn; //!
};
#endif // TAURECTOOLS_TauNeutralFourVecNNRegression_H
