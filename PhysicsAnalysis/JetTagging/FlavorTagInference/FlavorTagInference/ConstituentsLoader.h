/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This is a virtual class to represent loader of any type of constituents.
  It defines the interface for loading constituents from a jet 
  and extracting their features for the NN evaluation.
*/

#ifndef CONTITUENTS_LOADER_H
#define CONTITUENTS_LOADER_H

// local includes
#include "FlavorTagInference/FlipTagEnums.h"
#include "FlavorTagInference/SaltModel.h"
#include "FlavorTagInference/FTagDataDependencyNames.h"
#include "FlavorTagInference/StringUtils.h"

// EDM includes
#include "xAODJet/Jet.h"

// STL includes
#include <string>
#include <vector>
#include <set>
#include <tuple>

namespace FlavorTagInference {

    enum class ConstituentsEDMType {CHAR, UCHAR, INT, FLOAT, DOUBLE, CUSTOM_GETTER};
    enum class ConstituentsSortOrder {
        ABS_D0_SIGNIFICANCE_DESCENDING,
        D0_SIGNIFICANCE_DESCENDING,
        PT_DESCENDING,
        ABS_D0_DESCENDING,
        UNDEFINED
    };
    enum class ConstituentsSelection {
        ALL,
        IP3D_2018,
        DIPS_TIGHT_UPGRADE,
        DIPS_LOOSE_UPGRADE,
        DIPS_LOOSE_202102,
        LOOSE_202102_NOIP,
        R22_DEFAULT,
        R22_LOOSE,
        TAUTRACK_CLASSIFIED
    };
    enum class ConstituentsType {
        FLOW_ELEMENT,
        TRACK,
        HIT,
        ELECTRON,
        TAUTRACK,
        TAUCLUSTER,
        UNKNOWN
    };

    struct InputVariableConfig {
        std::string name;
        ConstituentsEDMType type;
        bool flip_sign;
    };

    struct ConstituentsInputConfig {
        std::string name;
        std::string output_name;
        ConstituentsType type{ConstituentsType::UNKNOWN};
        ConstituentsSortOrder order{ConstituentsSortOrder::UNDEFINED};
        size_t max_n_constituents = std::numeric_limits<size_t>::max();
        ConstituentsSelection selection = ConstituentsSelection::ALL;
        std::vector<InputVariableConfig> inputs;
    };

    ConstituentsInputConfig createConstituentsLoaderConfig(
      const std::string & name,
      const std::vector<std::string> & input_variables,
      FlipTagConfig flip_config
    );

    // Virtual class to represent loader of any type of constituents
    class IConstituentsLoader {
        public:
            IConstituentsLoader(const ConstituentsInputConfig& cfg)
              : m_config (cfg)
            {
            };
            virtual ~IConstituentsLoader() = default;
            virtual std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(const xAOD::IParticle& jet) const = 0;
            virtual const FTagDataDependencyNames& getDependencies() const = 0;
            virtual const std::set<std::string>& getUsedRemap() const = 0;
            virtual const std::string& getName() const = 0;
            virtual const ConstituentsType& getType() const = 0;

        protected:
            FTagDataDependencyNames m_deps;
            ConstituentsInputConfig m_config;
            std::set<std::string> m_used_remap;
            std::string m_name;
    };
}


#endif
