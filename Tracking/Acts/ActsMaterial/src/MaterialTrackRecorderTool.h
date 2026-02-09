/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_MATERIALTRACKRECORDERTOOL_H
#define ACTSGEOMETRY_MATERIALTRACKRECORDERTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "MaterialTrackRecorder.h"

namespace ActsTrk
{
    /// @class MaterialTrackRecorder
    ///
    /// @brief Handles the MaterialTrackRecorder G4UA

    class MaterialTrackRecorderTool : public G4UA::UserActionToolBase<MaterialTrackRecorder>
    {
        public:
            MaterialTrackRecorderTool(const std::string& type, const std::string& name,const IInterface* parent);

        protected:
            virtual std::unique_ptr<MaterialTrackRecorder> makeAndFillAction(G4UA::G4AtlasUserActions&) override final;

        private:
            /// The name of the recorded material track collection
            Gaudi::Property<std::string> m_materialTrackCollectionName
            {this, "MaterialTrackCollectionName", "OutputMaterialTracks", "Name of the output recorded material track collection"};

            /// The list of material excluded from recording
            Gaudi::Property<std::vector<std::string>> m_excludeMaterials
            {this, "ExcludeMaterials", {"Air", "Vacuum"}, "Material you want to exclude from recording"};

    };
}

#endif

