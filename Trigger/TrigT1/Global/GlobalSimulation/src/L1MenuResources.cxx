/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "L1MenuResources.h"

#include "TrigConfData/L1Menu.h"
#include "TrigConfData/L1Threshold.h"
#include "TrigConfData/L1TopoAlgorithm.h" // VariableParameter declaration

#include <sstream>


namespace GlobalSim {

  L1MenuResources::L1MenuResources(const TrigConf::L1Menu& l1menu,
				   const std::string& confAlgName,
				   const std::string& confAlgTypeLabel
				   ):
    m_l1menu{l1menu},
    m_confAlgName{confAlgName},
    m_confAlgTypeLabel{confAlgTypeLabel}{
    std::vector<std::string> permittedTypeLabels{"MULTTOPO", "TOPO"};
    if (std::find(permittedTypeLabels.cbegin(),
		  permittedTypeLabels.cend(),
		  m_confAlgTypeLabel) == permittedTypeLabels.cend()) {
      throw std::runtime_error("L1TopoMenuResources unknown alg type label " +
			       m_confAlgTypeLabel);
    }

  }
		  


  std::map<std::string, int> L1MenuResources::isolationFW_CTAU() const {
    const TrigConf::L1ThrExtraInfo_cTAU &ctauExtraInfo =
      m_l1menu.thrExtraInfo().cTAU();
    
    const auto& LooseSel = TrigConf::Selection::WP::LOOSE;
    const auto& MediumSel = TrigConf::Selection::WP::MEDIUM;
    const auto& TightSel = TrigConf::Selection::WP::TIGHT;
    
    int CTAU_iso_fw_loose  =
      static_cast<int>(ctauExtraInfo.isolation(LooseSel, 0).isolation_fw());

    int CTAU_iso_fw_medium =
      static_cast<int>(ctauExtraInfo.isolation(MediumSel, 0).isolation_fw());
    
    int CTAU_iso_fw_tight  =
      static_cast<int>(ctauExtraInfo.isolation(TightSel, 0).isolation_fw());
    
    std::map<std::string, int> isolationMap;
    
    isolationMap[TrigConf::Selection::wpToString(LooseSel)] =
      CTAU_iso_fw_loose;
    
    isolationMap[TrigConf::Selection::wpToString(MediumSel)] =
      CTAU_iso_fw_medium;
    
    isolationMap[TrigConf::Selection::wpToString(TightSel)] =
      CTAU_iso_fw_tight;
    
    return isolationMap;
    
  }

  
  std::map<std::string, int>
  L1MenuResources::isolationFW_CTAU_jTAUCoreScale() const {
    
    const TrigConf::L1ThrExtraInfo_cTAU &ctauExtraInfo =
      m_l1menu.thrExtraInfo().cTAU();
    
    const auto& LooseSel = TrigConf::Selection::WP::LOOSE;
    const auto& MediumSel = TrigConf::Selection::WP::MEDIUM;
    const auto& TightSel = TrigConf::Selection::WP::TIGHT;
    
    int CTAU_iso_fw_loose  =
      static_cast<int>(ctauExtraInfo.isolation(LooseSel,
					       0).isolation_jTAUCoreScale_fw());

    int CTAU_iso_fw_medium =
      static_cast<int>(ctauExtraInfo.isolation(MediumSel,
					       0).isolation_jTAUCoreScale_fw());
    
    int CTAU_iso_fw_tight  =
      static_cast<int>(ctauExtraInfo.isolation(TightSel,
					       0).isolation_jTAUCoreScale_fw());
    
    std::map<std::string, int> isolationMap;
    
    isolationMap[TrigConf::Selection::wpToString(LooseSel)] =
      CTAU_iso_fw_loose;
    
    isolationMap[TrigConf::Selection::wpToString(MediumSel)] =
      CTAU_iso_fw_medium;
    
    isolationMap[TrigConf::Selection::wpToString(TightSel)] =
      CTAU_iso_fw_tight;
    
    return isolationMap;
    
  }


  const TrigConf::L1Threshold& L1MenuResources::threshold() const{
    auto& l1algo = m_l1menu.algorithm(m_confAlgName, m_confAlgTypeLabel);
    return m_l1menu.threshold(l1algo.outputs().at(0));
  }

 
  std::string L1MenuResources::to_string() const {
    
    auto& l1alg = m_l1menu.algorithm(m_confAlgName, m_confAlgTypeLabel);
    auto a_name = l1alg.name();
    auto parameters = l1alg.parameters();
    std::stringstream ss;
    ss<< "L1Menu resources for alg " << a_name;
    for (const auto& param : parameters){
      ss << " parameter: " << std::setw(20) << std::left << param.name()
	 << " value: " << std::setw(3) << std::left << param.value()
	 << " selection: " << param.selection() << '\n';
    }
    
    ss << "inputs [" << l1alg.inputs().size() << "]:\n";
    for (const auto& p : l1alg.inputs()) {
      ss << "  input: " << p << '\n';
    }
    
    ss << "outputs [" << l1alg.outputs().size() << "]:\n";
    for (const auto& p : l1alg.outputs()) {
      ss << "   output: " << p << '\n';
    }

    return ss.str();
  }
  
  const std::string& L1MenuResources::menuName() {return m_l1menu.name();}
}

