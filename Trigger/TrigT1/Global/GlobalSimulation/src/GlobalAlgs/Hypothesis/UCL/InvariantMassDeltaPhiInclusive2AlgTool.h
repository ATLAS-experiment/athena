/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H
#define GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H

/**
 * AlgTool to run the Global InvariantMassDeltaPhiInclusive2
 */

#include "GenericTob.h"
#include "InvariantMassResult.h"
#include "../../../IGlobalSimAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include <bitset>

namespace GlobalSim {
  class InvariantMassDeltaPhiInclusive2AlgTool: public extends<AthAlgTool,
							   IGlobalSimAlgTool> {
    
  public:
    
    InvariantMassDeltaPhiInclusive2AlgTool(const std::string& type,
					   const std::string& name,
					   const IInterface* parent);
    
    virtual ~InvariantMassDeltaPhiInclusive2AlgTool() = default;
    
    virtual StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;

    using TobContainer = std::vector<std::string>;
    using TobContainerPtr = std::unique_ptr<TobContainer>;
    
  private:
 
    Gaudi::Property<bool>
    m_enableDump{this,
	"enableDump",
	  {false},
	"flag to enable dumps"};
    
    Gaudi::Property<std::vector<int>> m_minEt1Cuts
      {this,
       "minEt1Cuts",
       {},
       "Min Et for Tobs 1"
      };
    
    Gaudi::Property<std::vector<int>> m_minEt2Cuts
      {this,
       "minEt2Cuts",
       {},
       "Min Et for Tobs 2"
      };

    Gaudi::Property<bool> m_applyEtaCuts
      {this,
       "applyEtaCuts",
       {true},
       "Apply eta cuts if set true"
      };
        
    Gaudi::Property<std::vector<int>> m_minEta1Cuts
      {this,
       "minEta1Cuts",
       {},
       "Min Eta for Tobs 1"
      };
           
    Gaudi::Property<std::vector<int>> m_maxEta1Cuts
      {this,
       "maxEta1Cuts",
       {},
       "Max Eta for Tobs 1"
      };
   
    Gaudi::Property<std::vector<int>> m_minEta2Cuts
      {this,
       "minEta2Cuts",
       {},
       "Min Eta for Tobs 2"
      };
           
    Gaudi::Property<std::vector<int>> m_maxEta2Cuts
      {this,
       "maxEta2Cuts",
       {},
       "Max Eta for Tobs 2"
      };

    Gaudi::Property<std::vector<int>> m_minInvMassSqrCuts
      {this,
       "minInvMassSqrCuts",
       {},
       "minimum invariant mass squared"
      };

    
    Gaudi::Property<std::vector<int>> m_maxInvMassSqrCuts
      {this,
       "maxInvMassSqrCuts",
       {},
       "maximum invariant mass squared"
      };
        
    Gaudi::Property<std::vector<int>> m_minDeltaPhiCuts
      {this,
       "minDeltaPhiCuts",
       {},
       "minimum DeltaPhi"
      };
 
    Gaudi::Property<std::vector<int>> m_maxDeltaPhiCuts
      {this,
       "maxDeltaPhiCuts",
       {},
       "maximum DeltaPhi"
      };

    
    Gaudi::Property<int> m_maxTob1
      {this,
       "maxTob1",
       {6},
       "maximum number of Tobs from 1st list to consider"
      };

     
    Gaudi::Property<int> m_maxTob2
      {this,
       "maxTob2",
       {6},
       "maximum number of Tobs from 2nd list to consider"
      };
  
    SG::ReadHandleKey<GenericTobContainer>
    m_tobsInReadKey1 {
      this,
      "GenericTobContainerReadKey1",
      "genericTobContainer1",
      "key to read a container of Generic TOBS"};

    SG::ReadHandleKey<GenericTobContainer>
    m_tobsInReadKey2 {
      this,
      "GenericTobContainerReadKey2",
      "genericTobContainer2",
      "key to read a container of Generic TOBS"};

    
    SG::WriteHandleKey<InvariantMassResult>
    m_resultsWriteKey {
      this,
      "ResultsKey",
      "yesultsKey",
      "key to write a bitset of results"};
    

    using AcceptFlags =  std::vector<std::vector<bool>>;
    
    StatusCode
    selectTobs1(const GenericTobContainer&,
		AcceptFlags&) const;

    StatusCode
    selectTobs2(const GenericTobContainer&,
		AcceptFlags&) const;
    
    StatusCode
    setAcceptFlags(const GenericTobContainer&,
		   std::vector<bool>&,
		   int minEt,
		   int minEta,
		   int maxEta) const;

    
    StatusCode
    setAcceptFlags(const GenericTobContainer&,
		   std::vector<bool>&,
		   int minEt) const;
    

    constexpr static std::size_t s_inputWidth1{6};
    constexpr static std::size_t s_inputWidth2{6};
  };

  
}
    
#endif
