/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// StepEngine.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXENINGE_STEPENGINE_H
#define TRKEXENINGE_STEPENGINE_H

#ifndef TRKEXENINGE_OUTPUTHELPER 
#define TRKEXENINGE_OUTPUTHELPER 
#define OH_CHECKFOUND(object) ( object ? "found" : "not found")
#endif

// Gaudi
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
// Trk
#include "TrkExInterfaces/IExtrapolationEngine.h"
#include "TrkExInterfaces/ExtrapolationMacros.h"
#include "TrkExInterfaces/IPropagator.h"
#include "TrkExUtils/ExtrapolationCell.h"
#include "TrkExUtils/TargetSurfaces.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkNeutralParameters/NeutralParameters.h"

#include "CxxUtils/checker_macros.h"

namespace Trk {

  class IMaterialEffectsEngine;
  class INavigationEngine;        
    
  /** @class StepEngine
    
    Extrapolation engine for arbitrary tracking geometry environment. Intended as primary choice for Calo/MS.  
    
    @author sarka.todorova -at- cern.ch
  
  */
  class ATLAS_NOT_THREAD_SAFE StepEngine : public AthAlgTool, virtual public IExtrapolationEngine {

      public:
          
        /** @enum ResolveLayerType 
            - use for code readability
        */
        enum ResolveLayerType {
          StartLayer                = 0,
          NavigationLayer           = 1,
          PassThroughLayer          = 2,
          SubStructureLayer         = 3,
          DestinationLayer          = 4,
          StartAndDestinationLayer  = 6,
          UndefinedLayer            = 5
        };          
        
        /** Constructor */
        StepEngine(const std::string&,const std::string&,const IInterface*);
        
        /** Destructor */
        virtual ~StepEngine();

        /** AlgTool initialize method */
        virtual StatusCode initialize() override;
        
        /** AlgTool finalize method */
        virtual StatusCode finalize() override;
        
        using IExtrapolationEngine::extrapolate;
        
        /** charged extrapolation - public interface */
        virtual ExtrapolationCode extrapolate(ExCellCharged& ecCharged,
                                              const Surface* sf = 0,
                                              const BoundaryCheck& bcheck = true) const override;


        /** neutral extrapolation - public interface */
        virtual ExtrapolationCode extrapolate(ExCellNeutral& ecNeutral,
                                              const Surface* sf = 0,
                                              const BoundaryCheck& bcheck = true) const override;
                         
        /** define for which GeometrySignature this extrapolator is valid - this is GLOBAL */
        virtual GeometryType geometryType() const override;
                         
     private:
        /** main loop extrapolation method */
        template <class T> Trk::ExtrapolationCode targetSurfacesT(ExtrapolationCell<T>& eCell,
							       Trk::TargetSurfaceVector& ts,
							       bool trueOrderedIntersections, 
							       const Surface* sf = 0,
							       const BoundaryCheck& bcheck = true ) const;

	template<class T> Trk::ExtrapolationCode resolveFrameBoundaryT(ExtrapolationCell<T>& eCell,
								       Amg::Vector3D position,
                                                                       unsigned int index ) const;
        /** distance calculations */
        void evaluateDistance(Trk::TargetSurface& tt, const Amg::Vector3D& pos, const Amg::Vector3D& mom,
                              Trk::TargetSurfaceVector&ts, bool trueOrdered) const;
                                                           
	/** handle extrapolation step */
	Trk::ExtrapolationCode handleIntersection(ExCellCharged& ecCharged, 
						  Trk::TargetSurfaceVector& solutions) const;

        ToolHandle<IPropagator> m_propagator
	  {this, "Propagator", "", "the used propagation engine"};
        ToolHandle<IMaterialEffectsEngine> m_materialEffectsEngine
	  {this, "MaterialEffectsEngine", "", "the material effects updated"};
        ToolHandle<INavigationEngine> m_navigationEngine
	  {this, "NavigationEngine", "", "access to tracking geometry"};

        // local variables
	double m_tolerance = 0.01;

        // target surfaces
        mutable TargetSurfaces                             m_targetSurfaces;

        // debugging mode & switch         
        BooleanProperty m_debugAndFix{this, "DebugAndFixMode", true};
        mutable bool m_debugCall = false;       // mode

        StringProperty m_sopPrefix_prop{this, "OutputPrefix", ""};
        StringProperty m_sopPostfix_prop{this, "OutputPostfix", ""};

    };

  inline GeometryType StepEngine::geometryType() const 
      { return Trk::Dense; }


} // end of namespace

//!< define the templated function    
#include "StepEngine.icc"  

#endif // TRKEXINTERFACES_IStepEngine_H

