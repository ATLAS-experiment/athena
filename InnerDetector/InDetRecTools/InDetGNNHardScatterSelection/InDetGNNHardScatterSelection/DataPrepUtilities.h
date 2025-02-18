/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_DATA_PREP_UTILITIES_H
#define INDET_DATA_PREP_UTILITIES_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/SaltModel.h"

// EDM includes
#include "xAODTracking/Vertex.h"

// external libraries
#include "lwtnn/lightweight_network_config.hh"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>


namespace InDetGNNHardScatterSelection {

  enum class EDMType {CHAR, UCHAR, INT, FLOAT, DOUBLE, CUSTOM_GETTER};

  // Structures to define GNNTool input.
  //
  struct HSGNNInputConfig
  {
    std::string name;
    EDMType type;
    std::string default_flag;
  };


  // _____________________________________________________________________
  // Internal code

  namespace internal {
    // typedefs
    typedef std::pair<std::string, double> NamedVar;
    typedef xAOD::Vertex Vertex;

    typedef std::function<NamedVar(const Vertex&)> VarFromVertex;

    // ___________________________________________________________________
    // Getter functions
    //
    // internally we want a bunch of std::functions that return pairs
    // to populate the lwtnn input map.
    //
    template <typename T>
    class VertexVarGetter {
      private:
        typedef SG::AuxElement AE;
        AE::ConstAccessor<T> m_getter;
        AE::ConstAccessor<char> m_default_flag;
        std::string m_name;
      public:
        VertexVarGetter(const std::string& name, const std::string& default_flag):
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
    class VertexVarGetterNoDefault {
      private:
        typedef SG::AuxElement AE;
        AE::ConstAccessor<T> m_getter;
        std::string m_name;
      public:
        VertexVarGetterNoDefault(const std::string& name):
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
      VarFromVertex varFromVertex(const std::string& name,
                                  EDMType,
                                  const std::string& defaultflag);
    }

    typedef SG::AuxElement::Decorator<float> OutputSetterFloat;
    typedef std::vector<std::pair<std::string, OutputSetterFloat>> OutNodeFloat;

  }


  // higher level configuration functions
  namespace dataprep {
    typedef std::vector<std::pair<std::regex, std::string> > StringRegexes;

    // Get the configuration structures based on the lwtnn NN
    // structure.
    std::tuple<
      std::vector<HSGNNInputConfig>,
      std::vector<ConstituentsInputConfig>>
    createGetterConfig( lwt::GraphConfig& graph_config );

    // return the scalar getter functions for NNs
    std::vector<internal::VarFromVertex>
    createVertexVarGetters(
      const std::vector<HSGNNInputConfig>& inputs);
  }
}
#endif
