/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_TAUSELECTIONTOOL_H
#define TAUANALYSISTOOLS_TAUSELECTIONTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "PATCore/IAsgSelectionTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"

// Local include(s):
#include "TauAnalysisTools/ITauSelectionTool.h"
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/HelperFunctions.h"

// EDM include(s):
#include "xAODMuon/MuonContainer.h"
#include "xAODTau/TauJetContainer.h"

// ROOT include(s):
#include "TH1F.h"
#include "TFile.h"

namespace TauAnalysisTools
{

/// forward declarations
class TauSelectionCut;
class TauSelectionCutPt;
class TauSelectionCutAbsEta;
class TauSelectionCutAbsCharge;
class TauSelectionCutNTracks;
class TauSelectionCutJetIDWP;
class TauSelectionCutRNNJetScoreSigTrans;
class TauSelectionCutGNTauScoreSigTrans;
class TauSelectionCutRNNEleScore;
class TauSelectionCutEleIDWP;
class TauSelectionCutMuonOLR;


class TauSelectionTool : public virtual IAsgSelectionTool,
  public virtual ITauSelectionTool,
  public asg::AsgMetadataTool
{
  /// need to define cut classes to be friends to access protected variables,
  /// needed for access of cut thresholds
  friend class TauSelectionCut;
  friend class TauSelectionCutPt;
  friend class TauSelectionCutAbsEta;
  friend class TauSelectionCutAbsCharge;
  friend class TauSelectionCutNTracks;
  friend class TauSelectionCutJetIDWP;
  friend class TauSelectionCutRNNJetScoreSigTrans;
  friend class TauSelectionCutGNTauScoreSigTrans;
  friend class TauSelectionCutRNNEleScore;
  friend class TauSelectionCutEleIDWP;
  friend class TauSelectionCutMuonOLR;

  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( TauSelectionTool,
                   IAsgSelectionTool,
                   TauAnalysisTools::ITauSelectionTool )

  // declaration of classes as friends to access private member variables
  friend class TauEfficiencyCorrectionsTool;

public:
  /// Constructor for standalone usage
  TauSelectionTool( const std::string& name );

  virtual ~TauSelectionTool();

  /// Function initialising the tool
  virtual StatusCode initialize() override;

  /// Get an object describing the "selection steps" of the tool
  virtual const asg::AcceptInfo& getAcceptInfo() const override;

  /// Get the decision using a generic IParticle pointer
  virtual asg::AcceptData accept( const xAOD::IParticle* p ) const override;

  /// Get the decision for a specific TauJet object
  virtual asg::AcceptData accept( const xAOD::TauJet& tau ) const override;

  /// Set output file for control histograms
  virtual void setOutFile( TFile* fOutFile ) override;

  /// Write control histograms to output file
  virtual void writeControlHistograms() override;

private:

  // Execute at each event
  virtual StatusCode beginEvent() override;

  template<typename T, typename U>
  void FillRegionVector(std::vector<T>& vRegion, U tMin, U tMax) const;
  template<typename T, typename U>
  void FillValueVector(std::vector<T>& vRegion, U tVal) const;
  template<typename T>
  void PrintConfigRegion(const std::string& sCutName, std::vector<T>& vRegion) const;
  template<typename T>
  void PrintConfigValue(const std::string& sCutName, std::vector<T>& vRegion) const;
  template<typename T>
  void PrintConfigValue(const std::string& sCutName, T& sVal) const;

  // bitmask of tau selection cuts
  int m_iSelectionCuts;
  // vector of transverse momentum cut regions
  std::vector<float> m_vPtRegion;
  // vector of absolute eta cut regions
  std::vector<float> m_vAbsEtaRegion;
  // vector of absolute charge requirements
  std::vector<int> m_vAbsCharges;
  // vector of number of track requirements
  std::vector<unsigned> m_vNTracks;
  // vector of JetRNNSigTrans cut regions
  std::vector<float> m_vJetRNNSigTransRegion;
  // vector of GNTauSigTrans cut regions
  std::vector<float> m_vGNTauSigTransRegion;
  // JetID working point
  std::string m_sJetIDWP;
  int m_iJetIDWP;
  bool m_useGNTau=false;
  // vector of EleRNN cut regions
  std::vector<float> m_vEleRNNRegion;
  // EleID working point
  std::string m_sEleIDWP;
  int m_iEleIDWP;
  int m_iEleIDVersion;
  // do muon OLR
  bool m_bMuonOLR;

  float m_dPtMin;
  float m_dPtMax;
  float m_dAbsEtaMin;
  float m_dAbsEtaMax;
  float m_iAbsCharge;
  float m_iNTrack;
  float m_dJetRNNSigTransMin;
  float m_dJetRNNSigTransMax;
  float m_dGNTauSigTransMin;
  float m_dGNTauSigTransMax;
  float m_dEleRNNMin;
  float m_dEleRNNMax;

protected:
  TFile* m_fOutFile;//!
  std::shared_ptr<TH1F> m_hCutFlow;//!

private:
  std::string m_sConfigPath;

  SG::ReadHandleKey<xAOD::MuonContainer> m_muonContainerKey {this, "MuonContainerName", "Muons", "Muon container name"};
  SG::ReadHandleKey<xAOD::TauJetContainer> m_tauContainerKey {this, "TauContainerName", "TauJets", "Tau container name"};
  SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_eVetoDecorKey {this, "eVetoDecorName", "", "Name of eVeto decoration"};
  SG::ReadDecorHandleKey<xAOD::TauJetContainer> m_GNTauDecorKey {this, "GNTauDecorName", "", "Name of GnTauID decoration"};

  std::map<SelectionCuts, std::unique_ptr<TauAnalysisTools::TauSelectionCut>> m_cMap;

  void setupCutFlowHistogram();
  int convertStrToJetIDWP(const std::string& sJetIDWP) const;
  int convertStrToEleIDWP(const std::string& sEleIDWP) const;
  std::string convertJetIDWPToStr(int iJetIDWP) const;
  std::string convertEleIDWPToStr(int iEleIDWP) const;

protected:
  bool m_bCreateControlPlots;

  /// Object used to store selection information.
  asg::AcceptInfo m_aAccept;


}; // class TauSelectionTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_TAUSELECTIONTOOL_H
