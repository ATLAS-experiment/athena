/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ExpressionEvaluation/xAODVariableProxyLoaders.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "CxxUtils/checker_macros.h"

#include "TClass.h"

#include <memory>

namespace ExpressionParsing {

  TMethodWrapper::TMethodWrapper(const std::type_info &elementTypeinfo, 
      const std::string &methodName)
    : m_valid(false)
  {
    TClass *cls = TClass::GetClass(elementTypeinfo);
    if (!cls) {
      cls = TClass::GetClass(SG::normalizedTypeinfoName(elementTypeinfo).c_str());
      if (!cls) return;
    }

    m_methodCall.setProto (cls, methodName, "");

    m_valid = m_methodCall.call() != nullptr;
  }

  TMethodWrapper::~TMethodWrapper()
  {
  }

  IAccessor::VariableType TMethodWrapper::variableType()
  {
    TMethodCall* mc = m_methodCall.call();
    if (!mc) return IProxyLoader::VT_UNK;
    switch (mc->ReturnType()) {
      case TMethodCall::kLong:
        return IProxyLoader::VT_INT;
      case TMethodCall::kDouble: 
        return IProxyLoader::VT_DOUBLE;
      case TMethodCall::kString:
      case TMethodCall::kOther:
      case TMethodCall::kNone:
      default:                   return IProxyLoader::VT_UNK;
    }
  }

  bool TMethodWrapper::isValid(const SG::AuxElement *) const
  {
    return m_valid;
  }

  bool TMethodWrapper::isValid(const SG::AuxVectorData *) const
  {
    return m_valid;
  }

  int TMethodWrapper::getIntValue(const SG::AuxElement *auxElement) const
  {
    long retLong;
    auto aux_nc ATLAS_THREAD_SAFE = const_cast<SG::AuxElement*>(auxElement);  // required by TMethodCall
    m_methodCall.call()->Execute(aux_nc, retLong);
    return (int) retLong;
  }

  double TMethodWrapper::getDoubleValue(const SG::AuxElement *auxElement) const
  {
    double retDouble;
    auto aux_nc ATLAS_THREAD_SAFE = const_cast<SG::AuxElement*>(auxElement);  // required by TMethodCall
    m_methodCall.call()->Execute(aux_nc, retDouble);
    return retDouble;
  }

  std::vector<int> TMethodWrapper::getVecIntValue(const SG::AuxVectorData *)
  {
    throw std::runtime_error("TMethodWrapper doesn't deal with containers");
  }

  std::vector<double> TMethodWrapper::getVecDoubleValue(const SG::AuxVectorData *)
  {
    throw std::runtime_error("TMethodWrapper doesn't deal with containers");
  }



  // ********************************************************************************
  TMethodCollectionWrapper::TMethodCollectionWrapper(const std::type_info &containerTypeinfo, 
      const std::string &methodName)
    : m_collectionProxy(nullptr),
      m_valid(false)
  {
    TClass *containerClass = TClass::GetClass(containerTypeinfo);
    if (!containerClass) {
      containerClass = TClass::GetClass(SG::normalizedTypeinfoName(containerTypeinfo).c_str());
      if (!containerClass) return;
    }

    m_collectionProxy = containerClass->GetCollectionProxy();
    if (!m_collectionProxy) return;

    TClass *elementClass = m_collectionProxy->GetValueClass();
    if (!elementClass) return;

    m_methodCall.setProto (elementClass, methodName, "");

    m_valid = m_methodCall.call() != nullptr;
  }

  TMethodCollectionWrapper::~TMethodCollectionWrapper()
  {
  }

  IAccessor::VariableType TMethodCollectionWrapper::variableType()
  {
    TMethodCall* mc = m_methodCall.call();
    if (!mc) return IProxyLoader::VT_UNK;
    switch (mc->ReturnType()) {
      case TMethodCall::kLong:
        return IProxyLoader::VT_VECINT;
      case TMethodCall::kDouble: 
        return IProxyLoader::VT_VECDOUBLE;
      case TMethodCall::kString:
      case TMethodCall::kOther:
      case TMethodCall::kNone:
      default:                   return IProxyLoader::VT_UNK;
    }
  }

  bool TMethodCollectionWrapper::isValid(const SG::AuxElement *) const
  {
    return m_valid;
  }

  bool TMethodCollectionWrapper::isValid(const SG::AuxVectorData *) const
  {
    return m_valid;
  }

  int TMethodCollectionWrapper::getIntValue(const SG::AuxElement *) const
  {
    throw std::runtime_error("TMethodCollectionWrapper doesn't deal with scalars");
  }

  double TMethodCollectionWrapper::getDoubleValue(const SG::AuxElement *) const
  {
    throw std::runtime_error("TMethodCollectionWrapper doesn't deal with scalars");
  }

  std::vector<int> TMethodCollectionWrapper::getVecIntValue(const SG::AuxVectorData *auxVectorData)
  {
    long retLong;
    auto data_nc ATLAS_THREAD_SAFE = const_cast<SG::AuxVectorData*>(auxVectorData);  // required by TVirtualCollectionProxy
    m_collectionProxy->PushProxy(data_nc);
    const UInt_t N = m_collectionProxy->Size();
    std::vector<int> result(N);
    for (UInt_t i = 0; i < N; ++i) {
      void *element = (*m_collectionProxy)[i];
      m_methodCall.call()->Execute(element, retLong);
      result[i] = (int) retLong;
    }
    m_collectionProxy->PopProxy();
    return result;
  }

  std::vector<double> TMethodCollectionWrapper::getVecDoubleValue(const SG::AuxVectorData *auxVectorData)
  {
    double retDouble;
    auto data_nc ATLAS_THREAD_SAFE = const_cast<SG::AuxVectorData*>(auxVectorData);  // required by TVirtualCollectionProxy
    m_collectionProxy->PushProxy(data_nc);
    const UInt_t N = m_collectionProxy->Size();
    std::vector<double> result(N);
    for (UInt_t i = 0; i < N; ++i) {
      void *element = (*m_collectionProxy)[i];
      m_methodCall.call()->Execute(element, retDouble);
      result[i] = (double) retDouble;
    }
    m_collectionProxy->PopProxy();
    return result;
  }



  // ********************************************************************************
  xAODProxyLoader::xAODProxyLoader() :
    m_accessorCache(accessorCache_t::Updater_t())
  {
  }

  xAODProxyLoader::~xAODProxyLoader()
  {
    xAODProxyLoader::reset();
  }

  void xAODProxyLoader::reset()
  {
    for (auto x : m_accessorCache) delete x.second;
    // m_accessorCache.clear();  // not supported by ConcurrentStrMap
  }

  template <class TYPE, class AUX>
  bool xAODProxyLoader::try_type(const std::string& varname, const std::type_info* ti, const AUX* data) const
  {
    if (*ti == typeid(TYPE)) {
      auto accWrap = std::make_unique<AccessorWrapper<TYPE>>(varname);
      if (accWrap && accWrap->isValid(data)) {
        m_accessorCache.insert_or_assign(varname, accWrap.release());
        return true;
      } else if (accWrap) {
        const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
        const SG::auxid_t auxid = r.findAuxID(varname);
        throw std::runtime_error("Unsupported aux element type '"+r.getTypeName(auxid)+"' for '"+varname+"'");
      }
    }
    return false;
  }

  template <class AUX>
  IAccessor::VariableType xAODProxyLoader::try_all_known_types(const std::string& varname,
                                                                  const AUX* data, bool isVector) const
  {
    const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
    const SG::auxid_t auxid = r.findAuxID(varname);
    if (auxid != SG::null_auxid) {
      const std::type_info *ti = r.getType(auxid);
      // Try integer types:
      if ( try_type<int>(varname, ti, data) ||
           try_type<bool>(varname, ti, data) ||
           try_type<unsigned int>(varname, ti, data) ||
           try_type<char>(varname, ti, data) ||
           try_type<uint8_t>(varname, ti, data) ||
           try_type<unsigned short>(varname, ti, data) ||
           try_type<short>(varname, ti, data) ) {
        return isVector ? IProxyLoader::VT_VECINT : IProxyLoader::VT_INT;
      }
      // Try floating point types:
      if ( try_type<float>(varname, ti, data) ||
           try_type<double>(varname, ti, data) ) {
        return isVector ? IProxyLoader::VT_VECDOUBLE : IProxyLoader::VT_DOUBLE;
      }
    }
    return VT_UNK;
  }



  // ********************************************************************************
  xAODElementProxyLoader::xAODElementProxyLoader(const SG::AuxElement *auxElement)
    : m_auxElement(auxElement)
  {
  }

  void xAODElementProxyLoader::setData(const SG::AuxElement *auxElement)
  {
    m_auxElement = auxElement;
  }

  IAccessor::VariableType xAODElementProxyLoader::variableTypeFromString(const std::string &varname) const
  {
    // Try TMethodWrapper
    auto container = m_auxElement->container();
    const std::type_info &containerTypeinfo = typeid(*container);
    TClass *containerClass = TClass::GetClass(containerTypeinfo);
    if (!containerClass) {
      containerClass = TClass::GetClass(SG::normalizedTypeinfoName(containerTypeinfo).c_str());
    }
    if (containerClass) {
      std::unique_ptr<TMethodWrapper> accWrap;
      if( !strcmp(containerClass->GetName(),"SG::AuxElementStandaloneData") ) { /* special case where the element type is the aux element */
        accWrap = std::make_unique<TMethodWrapper>( typeid(*m_auxElement) , varname );
      } else {
        TVirtualCollectionProxy* collProxy = containerClass->GetCollectionProxy();
        if(collProxy) {
          const std::type_info &elementTypeinfo = *(collProxy->GetValueClass()->GetTypeInfo());
          accWrap = std::make_unique<TMethodWrapper>(elementTypeinfo, varname);
        }
      }
      if (accWrap && accWrap->isValid(m_auxElement)) {
        const IAccessor::VariableType vtype = accWrap->variableType();
        m_accessorCache.insert_or_assign(varname, accWrap.release());
        return vtype;
      }
    }

    return try_all_known_types(varname, m_auxElement, false);
  }

  int xAODElementProxyLoader::loadInt([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    return m_accessorCache.at(varname)->getIntValue(m_auxElement);
  }

  double xAODElementProxyLoader::loadDouble([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    return m_accessorCache.at(varname)->getDoubleValue(m_auxElement);
  }

  std::vector<int> xAODElementProxyLoader::loadVecInt([[maybe_unused]] const EventContext& ctx,const std::string &) const
  {
    throw std::runtime_error("xAODElementProxyLoader can't load vector types");
  }

  std::vector<double> xAODElementProxyLoader::loadVec([[maybe_unused]] const EventContext& ctx,const std::string &) const
  {
    throw std::runtime_error("xAODElementProxyLoader can't load vector types");
  }



  // ********************************************************************************
  xAODVectorProxyLoader::xAODVectorProxyLoader(const SG::AuxVectorData *auxVectorData)
    : m_auxVectorData(auxVectorData)
  {
  }

  void xAODVectorProxyLoader::setData(const SG::AuxVectorData *auxVectorData)
  {
    m_auxVectorData = auxVectorData;
  }

  IAccessor::VariableType xAODVectorProxyLoader::variableTypeFromString(const std::string &varname) const
  {
    auto accWrap = std::make_unique<TMethodCollectionWrapper>(typeid(*m_auxVectorData), varname);
    if (accWrap && accWrap->isValid(m_auxVectorData)) {
      const IAccessor::VariableType vtype = accWrap->variableType();
      m_accessorCache.insert_or_assign(varname, accWrap.release());
      return vtype;
    }

    IAccessor::VariableType vtype = try_all_known_types(varname, m_auxVectorData, true);

    // Before giving up completely, check the size of the vector. If it's
    // 0, it may be that it's empty on *all* events of the current input
    // file. Meaning that dynamic variables will be missing from each event.
    if( vtype==VT_UNK && m_auxVectorData->size_v() == 0 ) {
       // Let's claim a vector<double> type, that seems to be the safest bet.
       // Even if the variable should actually be vector<int>, this is a
       // simple conversion at least.
       vtype = VT_VECDOUBLE;
    }

    return vtype;
  }

  int xAODVectorProxyLoader::loadInt([[maybe_unused]] const EventContext& ctx,const std::string &) const
  {
    throw std::runtime_error("xAODVectorProxyLoader can't load scalar types");
  }

  double xAODVectorProxyLoader::loadDouble([[maybe_unused]] const EventContext& ctx,const std::string &) const
  {
    throw std::runtime_error("xAODVectorProxyLoader can't load scalar types");
  }

  std::vector<int> xAODVectorProxyLoader::loadVecInt([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {
    return m_accessorCache.at(varname)->getVecIntValue(m_auxVectorData);
  }

  std::vector<double> xAODVectorProxyLoader::loadVec([[maybe_unused]] const EventContext& ctx,const std::string &varname) const
  {/*
     // Check whether we have an accessor already:
     std::map< std::string, BaseAccessorWrapper* >::const_iterator itr;
     if( ( itr = m_accessorCache.find( varname ) ) == m_accessorCache.end() ) {
        // For an empty container let's not bother too much:
        if( m_auxVectorData->size_v() == 0 ) {
           return std::vector< double >();
        }
        // If it's not empty, then let the variableTypeFromString function
        // figure out the variable type, and create the accessor:
        if( variableTypeFromString( varname ) == VT_UNK ) {
           throw std::runtime_error( "Couldn't find variable type for " +
                                     varname );
        }
        // Update the iterator:
        itr = m_accessorCache.find( varname );
     }
     // Now do the "regular thing". Note that even if the type turns out
     // to be an integer type, the accessor wrapper does the conversion
     // reasonably anyway, behind the scenes.
     return itr->second->getVecDoubleValue(m_auxVectorData);*/
    return m_accessorCache.at(varname)->getVecDoubleValue(m_auxVectorData);
  }

}
