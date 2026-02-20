/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_STG_XMeansperSector_H
#define DQM_ALGORITHMS_STG_XMeansperSector_H

#include "dqm_core/Algorithm.h"
#include <string>
#include <iosfwd>

namespace dqm_algorithms {
	
	class STG_XMeansperSector : public dqm_core::Algorithm {
	public:
		
		STG_XMeansperSector();
		
		virtual ~STG_XMeansperSector() = default;
		virtual dqm_core::Algorithm*  clone() override final;
		virtual dqm_core::Result* execute( const std::string& name, 
		                                       const TObject& data,
		                                       const dqm_core::AlgorithmConfig& config ) override final;
		using dqm_core::Algorithm::printDescription;
		virtual void                  printDescription(std::ostream& out);
		
	protected:
		std::string m_name{"STG_XMeansperSector"};
	};
} //namespace dqm_algorithms

#endif
