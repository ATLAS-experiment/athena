/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InvariantMassDeltaPhiInclusive2AlgTool.h"
#include "AlgoDataTypes.h"  // bitSetToInt()
#include "DataCollector.h"

#include "../../../dump.h"
#include "../../../dump.icc"

#include "AthenaMonitoringKernel/Monitored.h"
#include "AthenaMonitoringKernel/MonitoredCollection.h"

#include <sstream>
#include <algorithm>

namespace GlobalSim {
  
  InvariantMassDeltaPhiInclusive2AlgTool::InvariantMassDeltaPhiInclusive2AlgTool(const std::string& type,
									 const std::string& name,
									 const IInterface* parent) :
    base_class(type, name, parent){
  }

  bool cutChecker(const std::vector<int>& vals, std::size_t sz) {
    if (vals.size() != sz) {return false;}
    return std::all_of(std::cbegin(vals),
		       std::cend(vals),
		       [](const auto& v){return v >= 0;});
  } 
    
  StatusCode InvariantMassDeltaPhiInclusive2AlgTool::initialize() {
       
    CHECK(m_tobsInReadKey1.initialize());
    CHECK(m_tobsInReadKey2.initialize());
    CHECK(m_resultsWriteKey.initialize());

    
    if(!cutChecker(m_minEt1Cuts, s_NumResultBits)){
      ATH_MSG_ERROR("minimum Et1 cuts error");
      return StatusCode::FAILURE;
    }

    if(!cutChecker(m_minEt2Cuts, s_NumResultBits)){
      ATH_MSG_ERROR("minimum Et2 cuts error");
      return StatusCode::FAILURE;
    }

    if (m_applyEtaCuts) {
      
      if(!cutChecker(m_minEta1Cuts, s_NumResultBits)){
	ATH_MSG_ERROR("minimum Eta1 cuts error");
	return StatusCode::FAILURE;
      }
	
      if(!cutChecker(m_maxEta1Cuts, s_NumResultBits)){
	ATH_MSG_ERROR("maximum Eta1 cuts error");
	return StatusCode::FAILURE;
      }
	
      if(!cutChecker(m_minEta2Cuts, s_NumResultBits)){
	ATH_MSG_ERROR("minimum Eta2 cuts error");
	return StatusCode::FAILURE;
      }
	
      if(!cutChecker(m_maxEta2Cuts, s_NumResultBits)){
	ATH_MSG_ERROR("maximum Eta2 cuts error");
	return StatusCode::FAILURE;
      }
    }
    
    if(!cutChecker(m_minInvMassSqrCuts, s_NumResultBits)){
      ATH_MSG_ERROR("minimum invariant mass cuts error");
      return StatusCode::FAILURE;
    }
    
    if(!cutChecker(m_maxInvMassSqrCuts, s_NumResultBits)){
      ATH_MSG_ERROR("maximum invariant mass cuts error");
      return StatusCode::FAILURE;
    }
    
    if(!cutChecker(m_minDeltaPhiCuts, s_NumResultBits)){
      ATH_MSG_ERROR("minimum Delta phi cuts error");
      return StatusCode::FAILURE;
    }
        
    if(!cutChecker(m_maxDeltaPhiCuts, s_NumResultBits)){
      ATH_MSG_ERROR("maximum Delta phi cuts error");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::run(const EventContext& ctx) const {
    ATH_MSG_DEBUG("run()");

    auto tobs1 =
      SG::ReadHandle<GenericTobContainer>(m_tobsInReadKey1,
					  ctx);

    auto tobs2 =
      SG::ReadHandle<GenericTobContainer>(m_tobsInReadKey2,
					  ctx);

    auto ss = std::stringstream();
    ss << "Tobs 1 in\n";
    for (const auto& tob: *tobs1) {
      ss << *tob << '\n';
    }

    ss << "Tobs 2 in\n";
    for (const auto& tob: *tobs1) {
      ss << *tob << '\n';
    }
    ATH_MSG_DEBUG(ss.str());
    
    // n selections on Tobs according to the s_NumResult cut categories

    std::size_t maxCount1 = std::max(tobs1->size(), s_inputWidth1);
    auto acceptFlagsEtEta1 =
      AcceptFlags(s_NumResultBits, std::vector<bool>(maxCount1, false));
    
    CHECK(selectTobs1(*tobs1, acceptFlagsEtEta1));

    std::size_t maxCount2 = std::max(tobs2->size(), s_inputWidth2); 
    auto acceptFlagsEtEta2 =
      AcceptFlags(s_NumResultBits, std::vector<bool>(maxCount2, false));
    CHECK(selectTobs1(*tobs2,  acceptFlagsEtEta2));
    
    CHECK(selectTobs2(*tobs2, acceptFlagsEtEta2));

    auto result = std::make_unique<InvariantMassResult>();

    for (auto isel{0U}; isel != s_NumResultBits; ++isel) {
      for (auto i1{0U}; i1 != maxCount1; ++i1) {
	if (!acceptFlagsEtEta1[isel][i1]) {continue;}
	for (auto i2{0U}; i2 != maxCount2; ++i2) {
	  if (!acceptFlagsEtEta1[isel][i2]) {continue;}
	  (*result)[isel] = true;
	}
      }
    }

    auto h_write =
      SG::WriteHandle<InvariantMassResult>(m_resultsWriteKey, ctx);
    CHECK(h_write.record(std::move(result)));

    return StatusCode::SUCCESS;
  }

  std::string
  InvariantMassDeltaPhiInclusive2AlgTool::toString() const {

    std::stringstream ss;
    ss << "name: " << name() << '\n'
       << " tobs in 1 read key " << m_tobsInReadKey1
       << " tobs in 2 read key " << m_tobsInReadKey2
       << '\n';
    
    return ss.str();
  }

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::selectTobs1(const GenericTobContainer& tobs,
						      AcceptFlags& flags) const {

    
    
    for (std::size_t isel{0}; isel != s_NumResultBits; ++isel) {
      if (m_applyEtaCuts) {
	CHECK (setAcceptFlags(tobs,
			      flags[isel],
			      m_minEt1Cuts[isel],
			      m_minEta1Cuts[isel],
			      m_maxEta1Cuts[isel]));
      } else {
	CHECK (setAcceptFlags(tobs,
			      flags[isel],
			      m_minEt1Cuts[isel]));
      }
    }
    
    return StatusCode::SUCCESS;
  }

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::selectTobs2(const GenericTobContainer& tobs,
						      AcceptFlags& flags) const {

    
    for (std::size_t isel{0}; isel != s_NumResultBits; ++isel) {
      if (m_applyEtaCuts) {
	CHECK (setAcceptFlags(tobs,
			      flags[isel],
			      m_minEt2Cuts[isel],
			      m_minEta2Cuts[isel],
			      m_maxEta2Cuts[isel]));
      } else {
	CHECK (setAcceptFlags(tobs,
			      flags[isel],
			      m_minEt2Cuts[isel]));
      }
    }
    return StatusCode::SUCCESS;
  }
  

  /* FIXME
  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::selectTobs(const GenericTobContainer& tobs,
						     GenericTobContainer& selectedTobs,
						     int minEt,
						     int minEta,
						     int maxEta) const {
    
    auto tobSelector = [minEt,
			minEta,
			maxEta] (const auto& t) {
      return  t->Et() > minEt and t->Eta() >= minEta and t->Eta() <= maxEta;
    };
    
    std::copy_if(std::cbegin(tobs),
		 std::cend(tobs),
		 std::back_inserter(selectedTobs),
		 tobSelector);


    return StatusCode::SUCCESS;
  }

  

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::selectTobs(const GenericTobContainer& tobs,
						     GenericTobContainer& selectedTobs,
						     int minEt) const {
 
    auto tobSelector = [minEt] (const auto& t) {
      return t->Et() > minEt;
    };
      
    std::copy_if(std::cbegin(tobs),
		 std::cend(tobs),
		 std::back_inserter(selectedTobs),
		 tobSelector);
    
    
    return StatusCode::SUCCESS;
  }

  */
  
  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::setAcceptFlags(const GenericTobContainer& tobs,
							 std::vector<bool>& flags,
							 int minEt,
							 int minEta,
							 int maxEta) const {
    
    auto tobSelector = [minEt,
			minEta,
			maxEta] (const auto& t) {
      return  t->Et() > minEt and t->Eta() >= minEta and t->Eta() <= maxEta;
    };
    
    std::transform(std::cbegin(tobs),
		   std::cend(tobs),
		   std::back_inserter(flags),
		   tobSelector);


    return StatusCode::SUCCESS;
  }

  

  StatusCode
  InvariantMassDeltaPhiInclusive2AlgTool::setAcceptFlags(const GenericTobContainer& tobs,
							 std::vector<bool>& flags,
							 int minEt) const {
 
    auto tobSelector = [minEt] (const auto& t) {
      return t->Et() > minEt;
    };
      
    std::transform(std::cbegin(tobs),
		  std::cend(tobs),
		  std::back_inserter(flags),
		  tobSelector);
    
    
    return StatusCode::SUCCESS;
  }
}

