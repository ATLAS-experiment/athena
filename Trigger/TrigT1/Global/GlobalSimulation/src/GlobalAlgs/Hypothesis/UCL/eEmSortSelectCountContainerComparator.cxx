
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eEmSortSelectCountContainerComparator.h"
#include "hexStrToBinStr.h" 

#include "../../../dump.h"
#include "../../../dump.icc"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {

  eEmSortSelectCountContainerComparator::eEmSortSelectCountContainerComparator(const std::string& type,
									 const std::string& name,
									 const IInterface* parent) :
    base_class(type, name, parent){
  }
  
  StatusCode eEmSortSelectCountContainerComparator::initialize() {
       
    CHECK(m_HypoFIFOReadKey.initialize());
    CHECK(m_portsOutReadKey.initialize());
    CHECK(m_eEmSortSelectCountExpectationsReadKey.initialize());

    return StatusCode::SUCCESS;
  }

  struct TestCounts {
    std::size_t nInputPorts{0};
    std::size_t nOutputTobs{0};
    std::size_t nOutputTobs_pass{0};
    std::size_t nCounts{0};
    std::size_t nCounts_pass{0};
  };
  std::ostream& operator << (std::ostream& os, const TestCounts& tc) {
    os << "test counts. nInputPorts: " << tc.nInputPorts  
       << " nOutputTobs: " << tc.nOutputTobs
       << " nOutputTobs_pass: " << tc.nOutputTobs_pass
       << " nCounts: " << tc.nCounts
       << " nCounts_pass: " << tc.nCounts_pass;
    return os;
  }
      
				      
  StatusCode
  eEmSortSelectCountContainerComparator::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

  
    // read in input data (a FIFO) for GepAlgoHypothesis from the event store

    auto fifo =
      SG::ReadHandle<GlobalSim::GepAlgoHypothesisFIFO>(m_HypoFIFOReadKey,
						       ctx);
    CHECK(fifo.isValid());
     
    ATH_MSG_DEBUG("read in GepAlgoHypothesis fifo ");

    auto ports_out =
      SG::ReadHandle<GlobalSim::eEmSortSelectCountContainerPortsOut>(
								     m_portsOutReadKey,
								     ctx);
    CHECK(ports_out.isValid());

    auto tcounts = TestCounts();
    tcounts.nInputPorts = fifo->size();
    {
      std::stringstream ss;
      ss << "eEmTobs from FIFO:\n";
      for (const auto& i : *fifo) {
	ss << eEmInputTOBToString(*(i.m_I_eEmTobs)) << '\n';
      }

      ATH_MSG_DEBUG(ss.str());
    }

    auto expectations =
      SG::ReadHandle<GlobalSim::eEmSortSelectCountExpectations>(m_eEmSortSelectCountExpectationsReadKey, ctx);
								 
    CHECK(expectations.isValid());

    const auto& exp_tob_bitstr = expectations->m_expected_tob_bits;
    auto sz = exp_tob_bitstr.size();

    // break down the bit string into 8 character chunks
    std::vector<std::bitset<32>> exp_tobs;
    exp_tobs.reserve(sz/8);  

    std::stringstream ss;

    for (std::size_t i = 2; i < sz; i += 8) {
      ss << std::hex
	 << std::string("0x" + std::string(std::cbegin(exp_tob_bitstr)+i,
					   std::cbegin(exp_tob_bitstr)+i+8));
      unsigned n;
      ss >> n;
      exp_tobs.push_back(std::bitset<32>(n));
      ss.clear();
  
    }

  
    auto ntobs = exp_tobs.size();
    if (ntobs != ports_out->m_O_eEmGenTob.size()) {
      ATH_MSG_ERROR("exp_tobs size " << exp_tobs.size() <<
		    " port tobs size : " << ports_out->m_O_eEmGenTob.size());
      return StatusCode::FAILURE;
    }

    //for some reason the tobs are delivered in reverse order. Fix this.
    std::reverse(std::begin(exp_tobs), std::end(exp_tobs));


    {

      auto fnd_tobs = ports_out->m_O_eEmGenTob;

      std::size_t idx{0};

      // compare only sorted TOBS - the  the ordering of the unsorted TOBS
      // may differ in the VHDL code
      auto ss = std::stringstream();
      for (;idx != fnd_tobs.size(); ++idx) {

	
	const auto& f_tob = fnd_tobs[idx];
	bool pass{(f_tob->as_bits().to_ulong()  == exp_tobs[idx].to_ulong())};
	ss << '\n' << idx << " found    " <<  f_tob->as_bits() << ' '
	   << std::hex << f_tob->as_bits().to_ulong() 
	   << " expected " <<  exp_tobs[idx] << ' '
	   << exp_tobs[idx].to_ulong()
	   << ' ' << std::boolalpha
	   << pass
	   << '\n';
	tcounts.nOutputTobs += 1;
	if (pass) {tcounts.nOutputTobs_pass +=1;}
      }
      ATH_MSG_DEBUG(ss.str());
    }

    
    {
      const auto& mult_bits = ports_out-> m_O_Multiplicity;
      
      auto found = mult_bits->to_string();
      auto exp_str = hexStrToBinStr((*expectations).m_expected_multiplicity_bits);
      ATH_MSG_DEBUG("\nmultiplicity bits. found: " << found
		    << " expected: "
		    << exp_str);
      
      tcounts.nCounts +=1;
      if (found == exp_str) {
	tcounts.nCounts_pass +=1;
      }
      
      ATH_MSG_DEBUG(tcounts);
    }
    
    return StatusCode::SUCCESS;
  }

  std::string
  eEmSortSelectCountContainerComparator::toString() const {
    
    std::stringstream ss;
    ss << "eEmSortSelectCountContainerComparator.name: " << name() << '\n'
       << m_HypoFIFOReadKey << '\n'
       << m_portsOutReadKey
       << '\n';
    return ss.str();
  }
}

