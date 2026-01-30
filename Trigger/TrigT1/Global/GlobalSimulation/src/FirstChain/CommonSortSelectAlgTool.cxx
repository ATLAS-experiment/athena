/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "./CommonSortSelectAlgTool.h"
#include "./CommonSelector.h"
#include "./eEmSelector.h"
#include "../IO//CommonTOB.h"

#include <fstream>

namespace GlobalSim {

  CommonSortSelectAlgTool::CommonSortSelectAlgTool(const std::string& type,
				 const std::string& name,
				 const IInterface* parent) :
    base_class(type, name, parent) {
  }
  
  
  // Initialize function running before first event
  StatusCode CommonSortSelectAlgTool::initialize() {

    CHECK(m_inTOBContainerKey.initialize());
    CHECK(m_outTOBContainerKey.initialize());

    // create the selector function object
    m_c_selector = std::make_unique<CommonSelector>(m_et_low_str,
						    m_et_high_str,
						    m_eta_low_str,
						    m_eta_high_str,
						    m_phi_low_str,
						    m_phi_high_str
						    );
 
    return StatusCode::SUCCESS;
  }

  
  StatusCode CommonSortSelectAlgTool::run(const EventContext& ctx) const {
    auto tobs =
      SG::ReadHandle<GlobalSim::IOBitwise::CommonTOBContainer>(m_inTOBContainerKey,
								ctx);

    CHECK(tobs.isValid());

    // copy selected tobs to the output container

    auto out_tobs =
      std::make_unique<IOBitwise::CommonTOBContainer>(tobs->size());

    
    auto tmp_tobs =
      std::vector<IOBitwise::CommonTOB*>(tobs->size());

    std::transform(std::begin(*tobs),
		   std::end(*tobs),
		   std::back_inserter(tmp_tobs),
		   [](const auto& tobptr) {
		     return new IOBitwise::CommonTOB(*tobptr);
		   });

    std::copy_if(std::begin(tmp_tobs),
		 std::end(tmp_tobs),
		 std::back_inserter(*out_tobs),
		 [&selector=m_c_selector](const auto& tob) {
		   return selector->select(*tob);});

    
    ATH_MSG_DEBUG("no of TOBS in, selected: " << tobs->size()
		  << " " << out_tobs->size());

    std::size_t maxTOBs{m_maxTOBs};
    out_tobs->resize(std::min(maxTOBs, out_tobs->size()));

    std::sort(std::begin(*out_tobs),
	      std::end(*out_tobs),
	      [](const auto& l, const auto& r) {
		return l->et_bits().to_ulong() <  r->et_bits().to_ulong();
	      });

    SG::WriteHandle<IOBitwise::CommonTOBContainer>
      h_out(m_outTOBContainerKey, ctx);
    
    CHECK(h_out.record(std::move(out_tobs)));

    return StatusCode::SUCCESS;
  }

  std::string CommonSortSelectAlgTool::toString() const {
    std::stringstream ss;
    ss <<name () << ": " <<m_menu_name << ' '
       << "CommonSortSelectAlgTool read, select, and sort CommonTOBS\n"
       << m_c_selector->to_string() << '\n';
    return ss.str();
  }

}
