
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmSortSelectCountContainerAlgTool.h"
#include "AlgoDataTypes.h"  // bitSetToInt()
#include "DataCollector.h"

#include "../../../dump.h"
#include "../../../dump.icc"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {

  std::vector<eEmTobPtr>
  make_eEmTobs(const GlobalSim::GepAlgoHypothesisFIFO& fifo);

  
  eEmSortSelectCountContainerAlgTool::eEmSortSelectCountContainerAlgTool(const std::string& type,
									 const std::string& name,
									 const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode eEmSortSelectCountContainerAlgTool::initialize() {
       
    CHECK(m_HypoFIFOReadKey.initialize());
    CHECK(m_portsOutWriteKey.initialize());

    if (m_EtMin.size() != m_nSelect) {return StatusCode::FAILURE;}
    if (m_REtaMin.size() != m_nSelect) {return StatusCode::FAILURE;}
    if (m_RHadMin.size() != m_nSelect) {return StatusCode::FAILURE;}
    if (m_WsTotMin.size() != m_nSelect) {return StatusCode::FAILURE;}
    return StatusCode::SUCCESS;
  }

  StatusCode
  eEmSortSelectCountContainerAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in input data for GepAlgoHypothesis from the event store

    auto fifo =
      SG::ReadHandle<GlobalSim::GepAlgoHypothesisFIFO>(m_HypoFIFOReadKey,
						       ctx);
    CHECK(fifo.isValid());
     
    ATH_MSG_DEBUG("read in GepAlgoHypothesis fifo ");

    auto eEmTobs = make_eEmTobs(*fifo);

    auto collector = std::make_unique<DataCollector>();
    collector->collect("eEmTobs in", eEmTobs);
    
    // Select
    auto selected_eEmTobs = std::vector<std::vector<eEmTobPtr>>();
    CHECK(make_selectedTobs(eEmTobs, selected_eEmTobs));

    collector->collect("selected eEmTobs containers", selected_eEmTobs);
    
    // why isn't the selection done on Generic TOBs? - bacause the original
    // VHDL does not.

    auto selected_genericTobs =
      std::vector<std::vector<GenTobPtr>>(selected_eEmTobs.size(),
					  std::vector<GenTobPtr>());

    auto make_genericTob = [](const auto& tob) {
      return std::make_shared<GenericTob>(tob);
    };

    for (std::size_t i = 0; i != selected_eEmTobs.size(); ++i){
      std::transform(std::begin(selected_eEmTobs[i]),
		     std::end(selected_eEmTobs[i]),
		     std::back_inserter(selected_genericTobs[i]),
		     make_genericTob);
    }
      
    collector->collect("Selected generic tob containers",
		       selected_genericTobs);
    // Sort
    auto EtGreater = [] (const GenTobPtr& l, const GenTobPtr& r) {
      return l->m_Et > r->m_Et;
    };


    auto ports_out = std::make_unique<eEmSortSelectCountContainerPortsOut>();

    //sort selected tobs, and copy to output port
    for (std::size_t i = 0; i != selected_genericTobs.size(); ++i) {

      auto& sel =  selected_genericTobs[i];
      auto divider = std::begin(sel) +
	std::min(sel.size(), AlgoConstants::eEmSortOutWidth[i]);


      std::partial_sort(std::begin(sel),
			divider,
			std::end(sel),
			EtGreater);

      auto& outputTobs = ports_out->m_O_eEmGenTob;
      auto start_iter =
	std::begin(outputTobs) + AlgoConstants::eEmSortOutStart[i];
      std::copy(std::begin(sel),
		divider,
		start_iter);
    }

    collector->collect("Sorted generic tob containers",
		       selected_genericTobs);

    
    
    auto ntobs = count_tobs(selected_genericTobs);
    collector->collect("eEmSortSelectCount counts", ntobs);

    {
      std::stringstream ss;
      ss << *collector;
      ATH_MSG_DEBUG(ss.str());
    }

    {
      std::stringstream ss;
      ss << *ports_out;
      ATH_MSG_DEBUG(ss.str());
    }

    
    auto h_write =
      SG::WriteHandle<eEmSortSelectCountContainerPortsOut>(m_portsOutWriteKey,
							   ctx);

    CHECK(h_write.record(std::move(ports_out)));
    return StatusCode::SUCCESS;
  }
    

  eEmTobPtr to_eEmTob(const GepAlgoHypothesisPortsIn& ports_in) {

    auto tob = std::make_shared<eEmTob>();
    const auto& w_tob = ports_in.m_I_eEmTobs;

    std::size_t i;
    std::size_t j;

    for (i = 0; i != AlgoConstants::eFexEtBitWidth; ++i) {
      tob->Et[i] = (*w_tob)[i];
    }

    for (i = 16, j=0;
	 i != 16+ AlgoConstants::eFexPhiBitWidth;
	 ++i, ++j) {
      tob->Phi[j] = (*w_tob)[i];
    }

    for (i = 32, j=0;
	 i != 32+ AlgoConstants::eFexEtaBitWidth;
	 ++i, ++j) {
      tob->Eta[j] = (*w_tob)[i];
    }


    for (i = 48, j=0;
	 i != 48+ AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	tob->REta[j] = (*w_tob)[i];
      }
    

    for (i = 48+ AlgoConstants::eFexDiscriminantBitWidth, j=0;
	 i != 48 + 2*AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	tob->RHad[j] = (*w_tob)[i];
      }

    
    for (i = 48+ 2*AlgoConstants::eFexDiscriminantBitWidth, j=0;
	 i != 48 + 3*AlgoConstants::eFexDiscriminantBitWidth;
	 ++i, ++j)
      {
	tob->WsTot[j] = (*w_tob)[i];
      }

    tob->Overflow[0] = (*w_tob)[63];

    return tob;
  }

  
  std::vector<eEmTobPtr>
  make_eEmTobs(const GlobalSim::GepAlgoHypothesisFIFO& fifo) {

    auto eEmTobs = std::vector<eEmTobPtr>();
    eEmTobs.reserve(fifo.size());

    std::transform(fifo.cbegin(),
		   fifo.cend(),
		   std::back_inserter(eEmTobs),
		   to_eEmTob);

    return eEmTobs;
  }


  StatusCode
  eEmSortSelectCountContainerAlgTool::make_selectedTobs(const std::vector<eEmTobPtr>& eEmTobs,
							std::vector<std::vector<eEmTobPtr>>& selectedTobs) const {
  
    selectedTobs.reserve(m_nSelect);
    
    for(std::size_t i = 0; i != m_nSelect; ++i) {
      auto selector = [etMin = m_EtMin,
		       rEtaMin= m_REtaMin,
		       rHadMin = m_RHadMin,
		       wsTotMin = m_WsTotMin,
		       j=i](const auto& tob){
	return
	  bitSetToInt(tob->Et) >= etMin[j] and
	  bitSetToInt(tob->REta) >= rEtaMin[j] and
	  bitSetToInt(tob->RHad) >= rHadMin[j] and
	  bitSetToInt(tob->WsTot) >= wsTotMin[j];
      };
   
      std::vector<eEmTobPtr> s_tobs;
      s_tobs.reserve(eEmTobs.size());
      std::copy_if(eEmTobs.cbegin(),
		   eEmTobs.cend(),
		   std::back_inserter(s_tobs),
		   std::move(selector));
      
      selectedTobs.push_back(s_tobs);
      
    }

    return StatusCode::SUCCESS;
  }

  std::vector<std::size_t>
  eEmSortSelectCountContainerAlgTool::count_tobs(const std::vector<std::vector<GenTobPtr>>& tobContainerVector) const 

  {
    auto sz = tobContainerVector.size();
    
    auto counts = std::vector<std::size_t>(sz, 0);
    
    for (std::size_t i = 0; i != sz; ++i) {
      const auto& gTobs = tobContainerVector[i];
      counts[i] = std::count_if(std::begin(gTobs),
				std::end(gTobs),
				[etMin = m_etaReg_EtMin[i],
				 etaMin = m_etaReg_EtaMin[i],
				 etaMax = m_etaReg_EtaMax[i]](const auto& tob) {

				  return
				    tob->m_Et > etMin and
				    tob->m_Eta > etaMin and
				    tob->m_Eta <= etaMax;});
      
    }
    return counts;
  }
  
  std::string eEmSortSelectCountContainerAlgTool::toString() const {

    std::stringstream ss;
    ss << "eEmSortSelectCountContainerAlgTool.name: " << name() << '\n'
       << m_HypoFIFOReadKey << '\n'
       << '\n';
    return ss.str();
  }
}

