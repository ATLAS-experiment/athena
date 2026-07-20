/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MaterialManipulation.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRALGS_MATERIALMANIPULATION_H
#define TRKDETDESCRALGS_MATERIALMANIPULATION_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
//Eigen
#include "GeoPrimitives/GeoPrimitives.h"

#include "TrkDetDescrInterfaces/ILayerMaterialManipulator.h"


namespace Trk {

    class LayerMaterialMap;
    

    /** @class MaterialManipulation

        A simple algorithm that reads in the material maps and writes out a new file,
        allowing to manipulate the material maps.

        @author Andreas.Salzburger@cern.ch      
     */


    class MaterialManipulation : public AthAlgorithm {

      public:

        /** Standard Athena-Algorithm Constructor */
        using AthAlgorithm::AthAlgorithm;

        /** standard Athena-Algorithm method */
        virtual StatusCode          initialize() override;
        
        /** standard Athena-Algorithm method */
        virtual StatusCode          execute(const EventContext& ctx) override;
        
        /** standard Athena-Algorithm method */
        virtual StatusCode          finalize() override;

    private:
                 
      //!< input material properties
      Gaudi::Property<std::string> m_inputLayerMaterialMapName
        {this, "LayerMaterialMapNameInput", "/GLOBAL/TrackingGeo/Input"};
      const LayerMaterialMap* m_inputLayerMaterialMap = nullptr;

      //!< output material properties
      Gaudi::Property<std::string> m_outputLayerMaterialMapName
	{this, "LayerMaterialMapNameOutput", "/GLOBAL/TrackingGeo/Output"};

      ToolHandle<ILayerMaterialManipulator> m_layerMaterialManipulator
	{this, "LayerMaterialManipulator", ""};

    };
}

#endif 
