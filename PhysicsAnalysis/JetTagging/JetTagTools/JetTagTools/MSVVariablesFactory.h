// -*- c++ -*-

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BTAGTOOL_MSVVARIABLESFACTORY_C
#define BTAGTOOL_MSVVARIABLESFACTORY_C

/******************************************************
    @class  MSVVariableFactory
    
********************************************************/

#include "AthenaBaseComps/AthAlgTool.h"
#include "JetTagTools/IMSVVariablesFactory.h"
#include <utility> 

class StoreGateSvc;

namespace Analysis {

  class MSVVariablesFactory : public AthAlgTool , virtual public IMSVVariablesFactory  {
    
  public:
    
    MSVVariablesFactory(const std::string& name,
			      const std::string& n, const IInterface* p);
    virtual ~MSVVariablesFactory() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    
    virtual StatusCode fillMSVVariables
    (const xAOD::Jet &, xAOD::BTagging* BTag,
     const Trk::VxSecVKalVertexInfo* myInfoVKal,
     xAOD::VertexContainer* btagVertex, const xAOD::Vertex& PV,
     std::string basename) const override ;
    virtual StatusCode createMSVContainer
    (const xAOD::Jet &, const Trk::VxSecVKalVertexInfo* myInfoVKal,
     xAOD::VertexContainer* btagVertex, const xAOD::Vertex& PV) const override;
   
  private:
    double get3DSignificance(const xAOD::Vertex* priVertex,
                             std::vector<const xAOD::Vertex*>& secVertex,
                             const Amg::Vector3D jetDirection) const;
   
    
  };
  
}//end Analysis namespace

#endif
