/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSMATERIAL_ROOTMATERIALWRITERTOOL_H
#define ACTSMATERIAL_ROOTMATERIALWRITERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsMaterial/IMaterialWriterTool.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Material/TrackingGeometryMaterial.hpp"

class TFile;


namespace ActsTrk {

    /// @brief Material decorator from Root format
    ///
    /// This reads in material maps from a root file
    class RootMaterialWriterTool : public extends<AthAlgTool, IMaterialWriterTool> {
        public:
            RootMaterialWriterTool(const std::string& type,
                                   const std::string& name,
                                   const IInterface* parent);
            ~RootMaterialWriterTool() override;
            virtual StatusCode initialize() override;
            virtual void writeMaterial(const ActsTrk::GeometryContext& gctx,
                                       const Acts::TrackingGeometryMaterial& detMaterial) const override;
        private:
            /// The name of the output file
            Gaudi::Property<std::string> m_fileName{this, "FileName", "material-maps.root", "Output root file for the Material Map"};
            /// The output file
            mutable TFile* m_outputFile ATLAS_THREAD_SAFE {};
    };

}

#endif
