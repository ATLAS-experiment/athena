/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RandomSurfaceBuilder.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_RANDOMSURFACEBUILDER_H
#define TRKDETDESCRTOOLS_RANDOMSURFACEBUILDER_H

// Trk
#include "TrkDetDescrInterfaces/ISurfaceBuilder.h"
// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// STL
#include <algorithm>

namespace Trk {

    class Surface;

    /** @class RandomSurfaceBuilder

      The RandomSurfaceBuilder is a simple Tool that helps to construct
      randomly poisitioned surfaces - mainly for validation studies.

      @author Andreas.Salzburger@cern.ch   
     */

    class RandomSurfaceBuilder : public AthAlgTool, virtual public ISurfaceBuilder {

      public:
        /** Constructor */
        RandomSurfaceBuilder(const std::string&,const std::string&,const IInterface*);
        
        /** Destructor */
        virtual ~RandomSurfaceBuilder();

        /** AlgTool initialize method */
        StatusCode initialize();
        
        /** AlgTool finalize method */
        StatusCode finalize();

        /** SurfaceBuilder interface method - provide a vector of surfaces - */
        const std::vector< const Surface* >* surfaces() const; 
      
        /** SurfaceBuilder interface method - provice a single surface */
        const Surface* surface( ) const;
        
     private :
        /** random number engine used */
        SmartIF<IRndmGenSvc>                    m_rndmSvc;
        Rndm::Numbers*                          m_flatDist = nullptr;

        Gaudi::Property<size_t> m_numberOfSurfaces{this, "NumberOfSurfaces", 100};
        Gaudi::Property<std::vector<double>> m_worldDimensions
	  {this, "WorldDimensions", {}, "the active dimensions"};
    };

} // end of namespace

#endif // TRKDETDESCRTOOLS_RANDOMSURFACEBUILDER_H

