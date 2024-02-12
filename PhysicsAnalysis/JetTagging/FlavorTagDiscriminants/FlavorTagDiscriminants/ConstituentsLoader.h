/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONTITUENTS_LOADER_H
#define CONTITUENTS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"

// STL includes
#include <string>
#include <vector>

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

    struct FTagConstituentsInputConfig {
        std::string name;
        ConstituentsEDMType type;
        bool flip_sign;
    };

    struct FTagConstituentsSequenceConfig {
        std::string name;
        ConstituentsSortOrder order;
        ConstituentsSelection selection;
        std::vector<FTagConstituentsInputConfig> inputs;
    };

    // Virtual class to represent loader of any type of constituents
    class ConstituentsLoader {
        public:
            ConstituentsLoader(FTagConstituentsSequenceConfig cfg) {
              config = cfg;
            };
            virtual ~ConstituentsLoader() {
            };
            virtual std::pair<std::string, input_pair> getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const = 0;
            virtual FTagDataDependencyNames getDependencies() const = 0;
            virtual std::set<std::string> getUsedRemap() const = 0;

        protected:
            FTagDataDependencyNames deps;
            FTagConstituentsSequenceConfig config;
            std::set<std::string> used_remap;
    };
}


#endif