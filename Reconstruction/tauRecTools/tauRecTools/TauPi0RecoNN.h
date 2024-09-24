/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAURECTOOLS_TAUPi0RecoNN_H
#define TAURECTOOLS_TAUPi0RecoNN_H

// base class include(s)
#include "tauRecTools/TauRecToolBase.h"

// xAOD include(s)
#include "xAODTau/TauJet.h"

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

class TauPi0RecoNN : public TauRecToolBase
{
public:
  ASG_TOOL_CLASS2(TauPi0RecoNN, TauRecToolBase, ITauToolBase)

  explicit TauPi0RecoNN(const std::string &name = "TauPi0RecoNN");
  virtual ~TauPi0RecoNN();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(xAOD::TauJet &xTau) const override;

private:
    // properties of the tool
    std::string m_OutputPrefix;                   //!
    std::string m_weightFile_1p1n;                //!
    std::string m_weightFile_1pXn;                //!
    std::string m_weightFile_3pXn;                //!
    std::size_t m_maxTauTracks;                   //!
    std::size_t m_maxNeutralPFOs;                 //!
    std::size_t m_maxShotPFOs;                    //!
    std::size_t m_maxConvTracks;                  //!
    float m_neutralPFOPtCut;                      //!
    std::string m_DecayModeName;                  //!
    // std::array<std::string, 3> m_FourVecDimNames; //!
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
  /// lwtnn graph
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_1p1n; //!
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_1pXn; //!
  std::unique_ptr<const lwt::LightweightGraph> m_lwtGraph_3pXn; //!
};
#endif // TAURECTOOLS_TauPi0RecoNN_H
