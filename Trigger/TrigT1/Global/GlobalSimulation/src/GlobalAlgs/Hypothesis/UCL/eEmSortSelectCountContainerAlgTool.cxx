
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

    // Cut values for Select part of Algorithm. Not yet provided in VHDL

    m_EtMin = std::vector(AlgoConstants::NumSelect, 0);
    m_REtaMin = std::vector(AlgoConstants::NumSelect, 0);
    m_RHadMin = std::vector(AlgoConstants::NumSelect, 0);
    m_WsTotMin = std::vector(AlgoConstants::NumSelect, 0);

    // Cut values for the Count part of Algorithm.
    // All values set to EM5 for now
    
    m_count_EtMin =
      std::vector<std::vector<unsigned int>> (s_NumCnt,
					      std::vector<unsigned int>(s_NumEtaRanges,
									0x019));


    // Eta min is tricky:
    // It has been set to 0x100000000 in a 9 bit word. So it is negative.
    // But our words have > 9 bits.
    m_count_EtaMin = std::vector<std::vector<int>>(s_NumCnt,
					 std::vector<int>(s_NumEtaRanges, -0b011111111));
    
    m_count_EtaMax = std::vector<std::vector<int>>(s_NumCnt,
						   std::vector<int>(s_NumEtaRanges, 0b011111111));


    return StatusCode::SUCCESS;
  }

  StatusCode
  eEmSortSelectCountContainerAlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in input data (a FIFO) for GepAlgoHypothesis from the event store

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
    for (std::size_t i = 0; i != AlgoConstants::eEmNumSort; ++i) {

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

    

    // count the tobs according to various criteria.
    // limit the counts so that the number of output bits are not exeeded.
    // write the counts to the output port

    // find the number count values
    auto ntobs = count_tobs(selected_genericTobs);
    collector->collect("eEmSortSelectCount counts", ntobs);


    // limit the count values
    std::vector<std::pair<std::size_t, unsigned>> bounded_counts;
    bounded_counts.reserve(ntobs.size());
    for(std::size_t i = 0; i != ntobs.size(); ++i) {
      bounded_counts.push_back({
	  std::min(ntobs[i], AlgoConstants::max_counts[i]),
	  AlgoConstants::eEmCountOutWidth[i]});
    };

    // convert each limited count value to 0, 1 (type: short)
    std::vector<std::vector<short>> int_bit_vecs;
    int_bit_vecs.reserve(bounded_counts.size());


    std::transform(std::cbegin(bounded_counts),
		   std::cend(bounded_counts),
		   std::back_inserter(int_bit_vecs),
		   [](const auto& p) {
		     std::vector<short> bits(p.second, 0);
		     std::size_t i{0};
		     auto ntob = p.first; 
		     while (ntob) {
		       if (ntob&1) {bits.at(i) = 1;}
		       ntob >>= 1;
		       ++i;
		     }
		     std::reverse(std::begin(bits),
				  std::end(bits));
		     
		     return bits;});

    // flatten the vectors of count bits
    std::vector<short> int_bits;
    for (const auto& v : int_bit_vecs) {
      int_bits.insert(int_bits.end(), std::cbegin(v), std::cend(v));
    }

    // sanity check
    if (int_bits.size() != AlgoConstants::eEmNumTotalCountWidth or
	int_bits.size() != (ports_out->m_O_Multiplicity)->size()) {
      ATH_MSG_ERROR("incorrect number of count bits. Expected "
		    << AlgoConstants::eEmNumTotalCountWidth
		    << " obtained " << int_bits.size()
		    << " number bits in output ports "
		    << (ports_out->m_O_Multiplicity)->size());
      return StatusCode::FAILURE;
    }

    // convert the 0, 1 (as shorts) to bits in the output port
    auto& count_bits_out = ports_out->m_O_Multiplicity;
    for (std::size_t i = 0; i != int_bits.size(); ++i){
      if (int_bits[i] == 1) {
	count_bits_out->set(i);
      } else {
	count_bits_out->reset(i);
      }
    }


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

    // write the output port to the event store
    
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
  
    selectedTobs.reserve(AlgoConstants::NumSelect);
    
    for(std::size_t i = 0; i != AlgoConstants::NumSelect; ++i) {
      auto selector = [etMin = m_EtMin[i],
		       rEtaMin= m_REtaMin[i],
		       rHadMin = m_RHadMin[i],
		       wsTotMin = m_WsTotMin[i]](const auto& tob){
	return
	  bitSetToInt(tob->Et) >= etMin and
	  bitSetToInt(tob->REta) >= rEtaMin and
	  bitSetToInt(tob->RHad) >= rHadMin and
	  bitSetToInt(tob->WsTot) >= wsTotMin;
      };
   
      std::vector<eEmTobPtr> s_tobs;
      s_tobs.reserve(eEmTobs.size());
      std::copy_if(eEmTobs.cbegin(),
		   eEmTobs.cend(),
		   std::back_inserter(s_tobs),
		   selector);
      
      selectedTobs.push_back(s_tobs);
      
    }

    return StatusCode::SUCCESS;
  }

  std::vector<std::size_t>
  eEmSortSelectCountContainerAlgTool::count_tobs(const std::vector<std::vector<GenTobPtr>>& tobContainerVector) const 

  {
    auto counts = std::vector<std::size_t>(s_NumCnt, 0);
    
    for (std::size_t i = 0; i != s_NumCnt; ++i) {
      const auto& gTobs = tobContainerVector[s_CntSelN[i]];
	
      counts[i] = std::count_if(std::begin(gTobs),
				std::end(gTobs),
				[etMin_etaRegs = m_count_EtMin[i],
				 etaMin_etaRegs = m_count_EtaMin[i],
				 etaMax_etaRegs = m_count_EtaMax[i]](const auto& tob) {

				  // loop over eta regions
				  for(std::size_t j{0}; j != etMin_etaRegs.size(); ++j) {
				    if (tob->m_Et > etMin_etaRegs[j] and
					tob->m_Eta > etaMin_etaRegs[j] and
					tob->m_Eta <= etaMax_etaRegs[j]) {return true;}
				  }
				  return false;
				});
      
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

