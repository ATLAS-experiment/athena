/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMODELR4_ToroidDetectorTool_H
#define MUONGEOMODELR4_ToroidDetectorTool_H

#include "GeoModelUtilities/GeoModelTool.h"


namespace MuonGMR4{
    class ToroidDetectorManager;

    class ToroidDetectorTool final : public GeoModelTool {
        public:
            // Constructor
            using GeoModelTool::GeoModelTool;
            // Destructor
            virtual ~ToroidDetectorTool() override final;

            // build the geometry
            virtual StatusCode create() override final;

            // Dereference tree tops and drop readout objects
            virtual StatusCode clear() override final;
        private:
            /** @brief Under what name the detector manager can be found in store gate */
            Gaudi::Property<std::string> m_mgrName{this, "ManagerName", "Toroid"};
            /** @brief Name of the tree tops */
            Gaudi::Property<std::vector<std::string>> m_treeTops{this, "TreeTops", {}};
            /** @brief Reference to the detector manger */
            ToroidDetectorManager* m_manager{nullptr};
    };
}
#endif
