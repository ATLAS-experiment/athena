/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FLAVORTAGINFERENCE_GNNDATALOADER_H
#define FLAVORTAGINFERENCE_GNNDATALOADER_H

#include "FlavorTagInference/GNNOptions.h"
#include "FlavorTagInference/DataPrepUtilities.h"
#include "FlavorTagInference/SaltModelEDMLoaderBase.h"
#include "FlavorTagInference/DataPrepUtilities.h"
#include "FlavorTagInference/ISaltModel.h"

namespace FlavorTagInference {

    class GNNDataLoader : public SaltModelEDMLoaderBase {
    public:
        GNNDataLoader(ISaltModelPtr salt_model, const GNNOptions& opts);
        FTagOptions ftag_options;
        FTagDataDependencyNames data_dependency_names;
    private:
        GNNOptions m_gnn_options;
        std::string getVecInputName(const SaltModelVersion salt_model_version, const ConstituentsInputConfig& constituents_config) const;
    };
} // namespace FlavorTagInference

#endif
