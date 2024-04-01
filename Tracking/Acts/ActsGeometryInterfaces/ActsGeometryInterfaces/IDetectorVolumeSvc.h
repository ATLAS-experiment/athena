/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IACTSDETECTORVOLUMESVC_H
#define ACTSGEOMETRYINTERFACES_IACTSDETECTORVOLUMESVC_H

#include "GaudiKernel/IService.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"

#include "Acts/Detector/Detector.hpp"

#include <memory>


namespace ActsTrk{
    /** @brief Interface of the service providing the Acts::Detector which implements
     *         the navigation delegate paradigm and eventually replace the legacy tracking geometry
     *
     */
    class IDetectorVolumeSvc : virtual public IService {
    public:


        DeclareInterfaceID(IDetectorVolumeSvc, 1, 0);

        virtual ~IDetectorVolumeSvc() = default;
        
        /// Returns the pointer to the Acts::Detector representing all ATLAS tracking layers
        /// If the method is called for the first time, the detector is instantiated assuming perfect
        /// alignment of the layers.
        using DetectorPtr = std::shared_ptr<const Acts::Experimental::Detector>; 
        virtual DetectorPtr detector() const = 0;

        /// Caches the final transformations in the alignment store for a given sub detector type
        /// (defined by an internal flag in the Store). Returns the number of added elements
        using AlignmentStore = ActsGeometryContext::AlignmentStore;
        virtual unsigned int populateAlignmentStore(AlignmentStore& store) const = 0;
        /// Returns an empty nominal context without any alignment caches
        virtual const ActsGeometryContext& getNominalContext() const = 0;
    };
  
}


#endif
