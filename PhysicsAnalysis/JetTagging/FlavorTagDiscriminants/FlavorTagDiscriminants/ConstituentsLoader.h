/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a virtual class to represent loader of any type of constituents.
  It defines the interface for loading constituents from a jet 
  and extracting their features for the NN evaluation.
*/

#ifndef CONTITUENTS_LOADER_H
#define CONTITUENTS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/SaltModel.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "FlavorTagDiscriminants/StringUtils.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"

// STL includes
#include <string>
#include <vector>
#include <set>
#include <tuple>

namespace FlavorTagDiscriminants {

    enum class ConstituentsEDMType {CHAR, UCHAR, INT, FLOAT, DOUBLE, CUSTOM_GETTER};
    enum class ConstituentsSortOrder {
        ABS_D0_SIGNIFICANCE_DESCENDING,
        D0_SIGNIFICANCE_DESCENDING,
        PT_DESCENDING,
        ABS_D0_DESCENDING
    };
    enum class ConstituentsSelection {
        ALL,
        IP3D_2018,
        DIPS_TIGHT_UPGRADE,
        DIPS_LOOSE_UPGRADE,
        DIPS_LOOSE_202102,
        LOOSE_202102_NOIP,
        R22_DEFAULT,
        R22_LOOSE
    };
    enum class ConstituentsType {
        FLOW_ELEMENT,
        TRACK,
        HIT,
        ELECTRON
    };

    struct InputVariableConfig {
        std::string name;
        ConstituentsEDMType type;
        bool flip_sign;
    };

    struct ConstituentsInputConfig {
        std::string name;
        std::string output_name;
        ConstituentsType type;
        ConstituentsSortOrder order;
        ConstituentsSelection selection;
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
            IConstituentsLoader(ConstituentsInputConfig cfg) {
              m_config = cfg;
            };
            virtual ~IConstituentsLoader() = default;
            virtual std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>> getData(
                const xAOD::Jet& jet, 
                [[maybe_unused]] const SG::AuxElement& btag) const = 0;
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