/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// PlanarModuleStepper.h, ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDIGTOOLS_PLANARMODULESTEPPER_H
#define TRKDIGTOOLS_PLANARMODULESTEPPER_H

// Amg
#include "GeoPrimitives/GeoPrimitives.h" //Amg:: typedefs
// Trk
#include "TrkDigInterfaces/IModuleStepper.h"

// Gaudi & Athena
#include "AthenaBaseComps/AthAlgTool.h"
//
#include <vector>


namespace Trk {

    class DigitizationModule;
    class DigitizationStep;
    
    /** @class PlanarModuleStepper

      Module for fast, geometric 


      @author Andreas.Salzburger@cern.ch   
     */

    class PlanarModuleStepper : public AthAlgTool, virtual public IModuleStepper {

      public:
        /** Constructor */
        PlanarModuleStepper(const std::string&,const std::string&,const IInterface*);
        
        /** Destructor */
        virtual ~PlanarModuleStepper();

        /** AlgTool initialize method */
        StatusCode initialize() override;
        
        /** AlgTool finalize method */
        StatusCode finalize() override;
        
        /** calculate the steps caused by this track - full simulation interface */
        std::vector<DigitizationStep> cellSteps(const DigitizationModule& dmodule,
                                                const Amg::Vector3D& startPosition,
                                                const Amg::Vector3D& endPosition) const override;
        
        /** calculate the steps caused by this track - fast simulation interface */
        std::vector<DigitizationStep> cellSteps(const DigitizationModule& dmodule,
                                                const Amg::Vector2D& intersection,
                                                const Amg::Vector3D& direction) const override;


    };

} // end of namespace

#endif // TRKDIGTOOLS_PLANARMODULESTEPPER_H
