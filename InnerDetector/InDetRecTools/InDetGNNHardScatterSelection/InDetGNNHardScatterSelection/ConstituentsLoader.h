/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a virtual class to represent loader of any type of constituents.
  It defines the interface for loading constituents from a vertex
  and extracting their features for the NN evaluation.
*/

#ifndef INDET_CONSTITUENTS_LOADER_H
#define INDET_CONSTITUENTS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/OnnxUtil.h"

// EDM includes
#include "xAODTracking/Vertex.h"

// STL includes
#include <string>
#include <vector>

namespace InDetGNNHardScatterSelection {

    enum class ConstituentsEDMType {CHAR, UCHAR, INT, FLOAT, DOUBLE, CUSTOM_GETTER};
    enum class ConstituentsSortOrder {
        PT_DESCENDING
    };
    enum class ConstituentsSelection {
        ALL
    };
    enum class ConstituentsType {
        IPARTICLE,
        TRACK,
        ELECTRON,
        MUON,
        JET,
        PHOTON
    };

    struct InputVariableConfig {
        std::string name;
        ConstituentsEDMType type{ConstituentsEDMType::CHAR};
    };

    struct ConstituentsInputConfig {
        std::string name;
        std::string output_name;
        std::string link_name;
        ConstituentsType type{ConstituentsType::IPARTICLE};
        ConstituentsSortOrder order{ConstituentsSortOrder::PT_DESCENDING};
        ConstituentsSelection selection{ConstituentsSelection::ALL};
        std::vector<InputVariableConfig> inputs;
    };

    ConstituentsInputConfig createConstituentsLoaderConfig(
      std::string name,
      std::vector<std::string> input_variables
    );

    // Virtual class to represent loader of any type of constituents
    class IConstituentsLoader {
        public:
            IConstituentsLoader(const ConstituentsInputConfig & cfg):m_config(cfg) { }
            virtual ~IConstituentsLoader() = default;
            virtual std::tuple<std::string, FlavorTagDiscriminants::Inputs, std::vector<const xAOD::IParticle*>> getData(
                const xAOD::Vertex& vertex) const = 0;
            virtual std::string getName() const = 0;
            virtual ConstituentsType getType() const = 0;

        protected:
            ConstituentsInputConfig m_config;
            std::string m_name;
    };
}

#endif
