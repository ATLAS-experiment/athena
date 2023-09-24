/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONTITUENTS_LOADER_H
#define CONTITUENTS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "FlavorTagDiscriminants/GNNConfig.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"

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

    // The sequence getter takes in constituents and calculates arrays of
    // values which are better suited for inputs to the NNs
    template <typename T, typename U>
    class SequenceGetter{
      private:
        SG::AuxElement::ConstAccessor<T> m_getter;
        std::string m_name;
      public:
        SequenceGetter(const std::string& name):
          m_getter(name),
          m_name(name)
          {
          }
        std::pair<std::string, std::vector<double>> operator()(const xAOD::Jet&, const std::vector<const U*>& consts) const {
          std::vector<double> seq;
          for (const U* el: consts) {
            seq.push_back(m_getter(*el));
          }
          return {m_name, seq};
        }
    };


    // Virtual class to represent loader of any type of constituents
    class ConstituentsLoader {
        public:
            ConstituentsLoader(FTagConstituentsSequenceConfig cfg) {
              config = cfg;
            };
            virtual std::pair<std::string, input_pair> getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const = 0;

        private:
            FTagConstituentsSequenceConfig config;
    };
}


#endif