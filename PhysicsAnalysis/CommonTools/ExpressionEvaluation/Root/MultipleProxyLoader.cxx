/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MultipleProxyLoader.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////


#include "ExpressionEvaluation/MultipleProxyLoader.h"

#include <stdexcept>
#include <iostream>
#include <sstream>

namespace ExpressionParsing {
  MultipleProxyLoader::MultipleProxyLoader() :
    m_varnameToProxyLoader(proxyCache_t::Updater_t())
  {
  }

  MultipleProxyLoader::~MultipleProxyLoader()
  {
  }


  IProxyLoader* MultipleProxyLoader::push_back(std::unique_ptr<IProxyLoader> proxyLoader)
  {
    m_proxyLoaders.push_back(std::move(proxyLoader));
    return m_proxyLoaders.back().get();
  }

  void MultipleProxyLoader::reset()
  {
    for (const auto &proxyLoader : m_proxyLoaders) {
      proxyLoader->reset();
    }
  }

  IProxyLoader::VariableType MultipleProxyLoader::variableTypeFromString(const std::string &varname) const
  {
    auto itr = m_varnameToProxyLoader.find(varname);
    if (itr != m_varnameToProxyLoader.end()) {
      return itr->second->variableTypeFromString(varname);
    }

    IProxyLoader::VariableType result;
    for (const auto &proxyLoader : m_proxyLoaders) {
      try {
        result = proxyLoader->variableTypeFromString(varname);
        if (result == VT_UNK) continue;
      } catch (const std::runtime_error &) {
        continue;
      }
      m_varnameToProxyLoader.emplace(varname, proxyLoader.get());
      return result;
    }
    std::stringstream msg;
    msg << "MultipleProxyLoader: unable to find valid proxy loader for " << varname << "."
        << " If it is an xAOD element or container which is read from the input file"
        << " this problem may occur if it is not accessed anywhere in the job via read handles."
        << " The problem can be mitigated by providing the missing information in the property "
        << " ExtraDataForDynamicConsumers of the sequence which has the property "
        << " ProcessDynamicDataDependencies set to True."
        << " The property takes a list of strings of the form \'type/container-name\' e.g."
        << " \'xAOD::TrackParticleContainer/InDetTrackParticles\'.";
    throw std::runtime_error(msg.str());
  }

  int MultipleProxyLoader::loadIntVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadIntVariableFromString(varname);
  }

  double MultipleProxyLoader::loadDoubleVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadDoubleVariableFromString(varname);
  }

  std::vector<int> MultipleProxyLoader::loadVecIntVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVecIntVariableFromString(varname);
  }

  std::vector<double> MultipleProxyLoader::loadVecDoubleVariableFromString(const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVecDoubleVariableFromString(varname);
  }
}
