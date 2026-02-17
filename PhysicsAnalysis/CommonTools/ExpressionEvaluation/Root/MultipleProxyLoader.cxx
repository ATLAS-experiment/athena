/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MultipleProxyLoader.cxx, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////


#include "ExpressionEvaluation/MultipleProxyLoader.h"
#include "AthContainers/CurrentContext.h"

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

  IAccessor::VariableType MultipleProxyLoader::variableType(const std::string &varname) const {
    const EventContext& ctx = Gaudi::Hive::currentContext();
    std::pair< IProxyLoader::VariableType, const IAccessor &>
       ret = getAccessorFromString(ctx, varname);
    return ret.first;
  }

  std::pair<IAccessor::VariableType, const IAccessor & >
  MultipleProxyLoader::getAccessorFromString(const EventContext &ctx, const std::string &varname) const
  {
    auto itr = m_varnameToProxyLoader.find(varname);
    if (itr != m_varnameToProxyLoader.end()) {
      return {itr->second->variableType(varname),*itr->second};
    }

    for (const auto &proxyLoader : m_proxyLoaders) {
       try {
          std::pair<IAccessor::VariableType, const IAccessor &>
             result = proxyLoader->getAccessorFromString(ctx, varname);
          if (result.first == VT_UNK) continue;
          if (result.first != VT_VECEMPTY) {
             // do not cache the result for an "empty vector", since in such cases there is
             // not enough information available to decide the correct accessor.
             m_varnameToProxyLoader.emplace(varname, &result.second);
          }
          return result;
       } catch (const std::runtime_error &) {
          continue;
       }
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

  int MultipleProxyLoader::loadInt(const EventContext& ctx,const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadInt(ctx,varname);
  }

  double MultipleProxyLoader::loadDouble(const EventContext& ctx,const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadDouble(ctx,varname);
  }

  std::vector<int> MultipleProxyLoader::loadVecInt(const EventContext& ctx,const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVecInt(ctx,varname);
  }

  std::vector<double> MultipleProxyLoader::loadVec(const EventContext& ctx,const std::string &varname) const
  {
    return m_varnameToProxyLoader.at(varname)->loadVec(ctx,varname);
  }
}
