/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration 
*/

#ifndef TrigBtagEmulationTool_H
#define TrigBtagEmulationTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrigBtagEmulationTool/ITrigBtagEmulationTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "src/JetManagerTool.h"
#include "src/TrigBtagEmulationChain.h"
#include "FlavorTagDiscriminants/DL2HighLevel.h"

#include <TLorentzVector.h>

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

namespace Trig {

class TrigBtagEmulationTool : 
  public extends<AthAlgTool, Trig::ITrigBtagEmulationTool> {
public:
  TrigBtagEmulationTool(const std::string& type, 
			const std::string& name, 
			const IInterface* parent);
  virtual ~TrigBtagEmulationTool() = default;
	
  virtual StatusCode initialize() override;

  // Interface
  virtual const EmulContext& populateJetManagersTriggerObjects() const override;  
  virtual bool isPassed(const std::string& chain) const override;
  virtual bool isPassed(const std::string& chain, const EmulContext&) const override;
  virtual std::unordered_map<std::string, std::vector<std::pair<const xAOD::Jet*, bool>>> getEmulatedJets(std::string) const override;

private:
  bool isPassed(const std::string&, const TrigBtagEmulationChain&, const EmulContext&) const;
  bool evaluate_L1(const TrigBtagEmulationChain&, const EmulContext&) const;
  bool evaluate_L1(const std::string&, const EmulContext&) const;
  bool evaluate_HLT(const TrigBtagEmulationChain&, const EmulContext&) const;
  bool evaluate_HLT(const TrigBtagEmulationChain&, const EmulContext&, std::unordered_map<std::string, std::vector<bool>>&) const;

  bool evaluate_preselection(const TrigBtagEmulationChain& chain,
			     const std::vector<TrigBtagEmulationJet>& preselJets) const;
  bool evaluate_dijetmass(const TrigBtagEmulationChain& chain,
			  const std::vector<TrigBtagEmulationJet>& preselJets) const;

  void parse_preselection(const std::string& presel_part,
			  int& presel_multiplicity,
			  double& presel_ptcut,
			  double& presel_eta_min,
			  double& presel_eta_max) const;
  void parse_dijetmass(const std::string& dijetmass,
		       double& dijet_mass,
		       double& dijet_minjetpt,
		       double& dijet_dphi,
		       double& dijet_deta) const;

  bool allocate_jets_to_chainParts(const std::vector<std::vector<bool>>& chainPart_jet_matrix,
				   const std::vector<std::string>& chainPartNames,
				   const std::vector<int>& multiplicities) const;

  std::vector<std::vector<bool>>
  evaluate_HLT_chainParts(const TrigBtagEmulationChain &chain,
                          const std::vector<TrigBtagEmulationJet> &jets,
                          int idx_begin, int idx_end,
                          const std::vector<TrigBtagEmulationJet> *splitJets = nullptr,
                          const std::vector<TrigBtagEmulationJet> *gscJets = nullptr
                          ) const;

  bool isPassedBTagger(const TrigBtagEmulationJet& jet,
		       const std::string& btagger) const;
  
  StatusCode addEmulatedChain(const std::string& name, 
			      const std::vector< std::string >& definition);

  StatusCode retrieveTriggerObjects(const Trig::JetManagerTool&,
				    EmulContext&) const;

  StatusCode indexRun2TriggerObjects(EmulContext&) const;

private:
  PublicToolHandle<Trig::TrigDecisionTool> m_trigDec {this, "TrigDecisionTool", "",""};
  
  // jet Managers
  ToolHandle< Trig::JetManagerTool > m_manager_PFlow_cnt {this, "JM_PFlow_CNT", "",""};
  ToolHandle< Trig::JetManagerTool > m_manager_EMTopo_presel {this, "JM_EMTopo_PRESEL", "",""};
  ToolHandle< Trig::JetManagerTool > m_manager_a4tcemsubjesJet_cnt {this, "JM_a4tcemsubjes_CNT", "",""};
  ToolHandle< Trig::JetManagerTool > m_manager_SplitJet_cnt {this, "JM_Split_CNT", "",""};
  ToolHandle< Trig::JetManagerTool > m_manager_GSCJet_cnt {this, "JM_GSC_CNT", "",""};

  // Flavour Tagging
  Gaudi::Property< std::map<std::string, double> > m_tagger_wp {this, "WorkingPoints", {}};
  Gaudi::Property< std::map<std::string, std::string> > m_remapping {this, "FTD_Remapping", {}};

  // LHC period Run2 or Run3
  Gaudi::Property<int> m_LHCPeriod {this, "LHCPeriod", 3, "LHC Period Run2 or Run3"};

  // EMULATED CHAINS
  Gaudi::Property< std::map< std::string, std::vector< std::string >>> m_emulatedChainDefinitions {this, "EmulatedChainDefinitions", {}, ""};

  std::unordered_map< std::string, std::unique_ptr<TrigBtagEmulationChain> > m_emulatedChains;  
  std::unique_ptr<FlavorTagDiscriminants::DL2HighLevel> m_dl2;
};

} // namespace

#endif
