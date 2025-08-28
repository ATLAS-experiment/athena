/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_DITAUSELECTIONTOOL_H
#define TAUANALYSISTOOLS_DITAUSELECTIONTOOL_H

/*
  author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  contact email: antonio.de.maria@cern.ch
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/PropertyWrapper.h"
#include "PATCore/IAsgSelectionTool.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "xAODTau/DiTauJetContainer.h"

// Local include(s):
#include "TauAnalysisTools/IDiTauSelectionTool.h"
#include "TauAnalysisTools/Enums.h"
#include "TauAnalysisTools/HelperFunctions.h"

// ROOT include(s):
#include "TH1F.h"
#include "TFile.h"

namespace TauAnalysisTools
{

/// forward declarations
class DiTauSelectionCut;
class DiTauSelectionCutPt;
class DiTauSelectionCutAbsEta;
class DiTauSelectionCutNSubjets;
class DiTauSelectionCutAbsCharge;
class DiTauSelectionCutOmniScore;

class DiTauSelectionTool : public virtual IAsgSelectionTool,
  public virtual IDiTauSelectionTool,
  public asg::AsgMetadataTool
{
  /// need to define cut classes to be friends to access protected variables,
  /// needed for access of cut thresholds
  friend class DiTauSelectionCut;
  friend class DiTauSelectionCutPt;
  friend class DiTauSelectionCutAbsEta;
  friend class DiTauSelectionCutNSubjets;
  friend class DiTauSelectionCutAbsCharge;
  friend class DiTauSelectionCutOmniScore;

  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( DiTauSelectionTool,
                   IAsgSelectionTool,
                   TauAnalysisTools::IDiTauSelectionTool )

public:
  /// Constructor for standalone usage
  DiTauSelectionTool( const std::string& name );

  virtual ~DiTauSelectionTool();

  /// Function initialising the tool
  virtual StatusCode initialize() override;

  /// Get an object describing the "selection steps" of the tool
  virtual const asg::AcceptInfo& getAcceptInfo() const override;

  /// Get the decision using a generic IParticle pointer
  virtual asg::AcceptData accept( const xAOD::IParticle* p ) const override;

  /// Get the decision for a specific TauJet object
  virtual asg::AcceptData accept( const xAOD::DiTauJet& tau ) const override;

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

  // vector of transverse momentum cut regions
  std::vector<float> m_vPtRegion;
  // vector of absolute eta cut regions
  std::vector<float> m_vAbsEtaRegion;
  // vector of number of subjets cut regions
  std::vector<float> m_vNSubjetsRegion;
  // vector of absolute charge requirements
  std::vector<int> m_vAbsCharges;
  // vector of OmniScore cut regions
  std::vector<float> m_vOmniScoreRegion;

  bool m_useOmniScore=false;

  Gaudi::Property<std::string> m_sConfigPath{this, "ConfigPath", ""};
  Gaudi::Property<int> m_iSelectionCuts{this, "SelectionCuts", NoDiTauCut};
  Gaudi::Property<float> m_dPtMin{this, "PtMin", NAN};
  Gaudi::Property<float> m_dPtMax{this, "PtMax", NAN};
  Gaudi::Property<float> m_dAbsEtaMin{this, "AbsEtaMin", NAN};
  Gaudi::Property<float> m_dAbsEtaMax{this, "AbsEtaMax", NAN};
  Gaudi::Property<float> m_dNSubjetsMin{this, "NSubjetsMin", NAN};
  Gaudi::Property<float> m_dNSubjetsMax{this, "NSubjetsMax", NAN};
  Gaudi::Property<float> m_iAbsCharge{this, "AbsCharge", NAN};
  Gaudi::Property<float> m_dOmniScoreMin{this, "OmniScoreMin", NAN};
  Gaudi::Property<float> m_dOmniScoreMax{this, "OmniScoreMax", NAN};

  Gaudi::Property<std::vector<float>> m_vecPtRegion{this, "PtRegion", {}};
  Gaudi::Property<std::vector<float>> m_vecAbsEtaRegion{this, "AbsEtaRegion",{}};
  Gaudi::Property<std::vector<float>> m_vecNSubjetsRegion{this, "NSubjetsRegion", {}};
  Gaudi::Property<std::vector<int>> m_vecAbsCharges{this, "AbsCharges", {}};
  Gaudi::Property<std::vector<float>> m_vecOmniScoreRegion{this, "OmniScoreRegion", {}};

protected:
  TFile* m_fOutFile;//!
  std::shared_ptr<TH1F> m_hCutFlow;//!

private:

  std::map<DiTauSelectionCuts, std::unique_ptr<TauAnalysisTools::DiTauSelectionCut>> m_cMap;

  void setupCutFlowHistogram();

protected:
  
  Gaudi::Property<bool> m_bCreateControlPlots{this, "CreateControlPlots", false};
  SG::ReadDecorHandleKey<xAOD::DiTauJetContainer> m_OmniScoreDecorKey {this, "OmniScoreDecorName", "", "Name of OmniScore decoration"};

  /// Object used to store selection information.
  asg::AcceptInfo m_aAccept;


}; // class DiTauSelectionTool

} // namespace TauAnalysisTools

#endif // TAUANALYSISTOOLS_DITAUSELECTIONTOOL_H
