/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DATA_PREP_UTILITIES_H
#define DATA_PREP_UTILITIES_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "FlavorTagDiscriminants/OnnxUtil.h"
#include "FlavorTagDiscriminants/ConstituentsLoader.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"

// external libraries
#include "lwtnn/lightweight_network_config.hh"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>


namespace FlavorTagDiscriminants {

  enum class EDMType {CHAR, UCHAR, INT, FLOAT, DOUBLE, CUSTOM_GETTER};

  // Structures to define DL2/GNNTool input.
  //
  struct FTagInputConfig
  {
    std::string name;
    EDMType type;
    std::string default_flag;
  };

  // other DL2/GNNTool options
  struct FTagOptions {
    FTagOptions();
    std::string track_prefix;
    FlipTagConfig flip;
    std::string track_link_name;
    std::map<std::string,std::string> remap_scalar;
    TrackLinkType track_link_type;
    float default_output_value;
    std::string invalid_ip_key;
  };


  // _____________________________________________________________________
  // Internal code

  namespace internal {
    // typedefs
    typedef std::pair<std::string, double> NamedVar;
    typedef xAOD::Jet Jet;
    typedef xAOD::BTagging BTagging;
    typedef std::vector<const xAOD::TrackParticle*> Tracks;

    // getter functions
    typedef std::function<NamedVar(const SG::AuxElement&)> VarFromBTag;
    typedef std::function<NamedVar(const Jet&)> VarFromJet;

    // ___________________________________________________________________
    // Getter functions
    //
    // internally we want a bunch of std::functions that return pairs
    // to populate the lwtnn input map. We define a functor here to
    // deal with the b-tagging cases.
    //
    template <typename T>
    class BVarGetter {
      private:
        typedef SG::AuxElement AE;
        AE::ConstAccessor<T> m_getter;
        AE::ConstAccessor<char> m_default_flag;
        std::string m_name;
      public:
        BVarGetter(const std::string& name, const std::string& default_flag):
          m_getter(name),
          m_default_flag(default_flag),
          m_name(name)
          {
          }
        NamedVar operator()(const SG::AuxElement& btag) const {
          T ret_value = m_getter(btag);
          bool is_default = m_default_flag(btag);
          if constexpr (std::is_floating_point<T>::value) {
            if (std::isnan(ret_value) && !is_default) {
              throw std::runtime_error(
                "Found NAN value for '" + m_name
                + "'. This is only allowed when using a default"
                " value for this input");
            }
          }
          return {m_name, is_default ? NAN : ret_value};
        }
    };

    template <typename T>
    class BVarGetterNoDefault {
      private:
        typedef SG::AuxElement AE;
        AE::ConstAccessor<T> m_getter;
        std::string m_name;
      public:
        BVarGetterNoDefault(const std::string& name):
          m_getter(name),
          m_name(name)
          {
          }
        NamedVar operator()(const SG::AuxElement& btag) const {
          T ret_value = m_getter(btag);
          if constexpr (std::is_floating_point<T>::value) {
            if (std::isnan(ret_value)) {
              throw std::runtime_error(
                "Found NAN value for '" + m_name + "'.");
            }
          }
          return {m_name, ret_value};
        }
    };

    // Filler functions
    //
    // factory functions to produce callable objects that build inputs
    namespace get {
      VarFromBTag varFromBTag(const std::string& name,
                              EDMType,
                              const std::string& defaultflag);
    }

    typedef SG::AuxElement::Decorator<float> OutputSetterFloat;
    typedef std::vector<std::pair<std::string, OutputSetterFloat>> OutNodeFloat;

  }


  // higher level configuration functions
  namespace dataprep {
    typedef std::vector<std::pair<std::regex, std::string> > StringRegexes;
    StringRegexes getNameFlippers(const FlipTagConfig& flip_config);

    // Get the configuration structures based on the lwtnn NN
    // structure.
    std::tuple<
      std::vector<FTagInputConfig>,
      std::vector<ConstituentsInputConfig>,
      FTagOptions>
    createGetterConfig( lwt::GraphConfig& graph_config,
      FlipTagConfig flip_config,
      std::map<std::string, std::string> remap_scalar,
      TrackLinkType track_link_type);

    // return the scalar getter functions for NNs
    std::tuple<
      std::vector<internal::VarFromBTag>,
      std::vector<internal::VarFromJet>,
      FTagDataDependencyNames>
    createBvarGetters(
      const std::vector<FTagInputConfig>& inputs);

    // return the decorators for the NNs
    std::tuple<
      std::map<std::string, internal::OutNodeFloat>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createDecorators(
      const lwt::GraphConfig& config,
      const FTagOptions& options);

    // return a function to check if IP is invalid
    std::tuple<
      std::function<char(const internal::Tracks&)>,
      std::vector<SG::AuxElement::Decorator<char>>,
      FTagDataDependencyNames,
      std::set<std::string>>
    createIpChecker(
      const lwt::GraphConfig&, const FTagOptions&);

    // check that all the remapping was used
    void checkForUnusedRemaps(
      const std::map<std::string, std::string>& requested,
      const std::set<std::string>& used);
  }
}
#endif
