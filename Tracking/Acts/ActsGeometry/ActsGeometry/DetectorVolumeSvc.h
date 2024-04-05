/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ACTSDETECTORVOLUMESVC_H
#define ACTSGEOMETRY_ACTSDETECTORVOLUMESVC_H

// ATHENA
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ToolHandle.h"

// PACKAGE
#include "ActsGeometryInterfaces/IDetectorVolumeSvc.h"
#include "ActsGeometryInterfaces/IDetectorVolumeBuilderTool.h"
#include "CxxUtils/CachedValue.h"


// ACTS
#include "Acts/Detector/Detector.hpp"


namespace ActsTrk{

    class DetectorVolumeSvc : public extends<AthService, IDetectorVolumeSvc> {
      public:

      DetectorVolumeSvc( const std::string& name, ISvcLocator* pSvcLocator );

      StatusCode initialize() override;

      DetectorPtr detector() const override;

      unsigned int populateAlignmentStore(AlignmentStore& store) const override;

      const ActsGeometryContext& getNominalContext() const override;

    private:
        std::shared_ptr<const Acts::Experimental::Detector> buildDetector() const;
        
        ToolHandleArray<IDetectorVolumeBuilderTool> m_builderTools{this, "DetectorBuilders", {}};
        
        CxxUtils::CachedValue<DetectorPtr> m_detector{};
        ActsGeometryContext m_nomContext{};
    };
}



#endif