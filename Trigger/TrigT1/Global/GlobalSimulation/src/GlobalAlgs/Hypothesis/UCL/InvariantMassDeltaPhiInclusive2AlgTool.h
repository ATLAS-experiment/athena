/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H
#define GLOBALSIM_INVARIANTMASSDELTAPHIINCLUSIVE2_H

/**
 * Algtool to run the Global InvariantMassDeltaPhiInclusive2
 */

#include "GenericTob.h"

#include "../../../IGlobalSimAlgTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

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

  };
}
    
#endif
