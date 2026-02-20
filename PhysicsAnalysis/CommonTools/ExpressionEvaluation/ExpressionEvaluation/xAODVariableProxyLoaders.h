/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// xAODVariableProxyLoaders.h, (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////
// Author: Thomas Gillam (thomas.gillam@cern.ch)
// ExpressionParsing library
/////////////////////////////////////////////////////////////////

#ifndef XAOD_VARIABLE_PROXY_LOADERS_H
#define XAOD_VARIABLE_PROXY_LOADERS_H

#include "ExpressionEvaluation/RelayProxyLoader.h"

#include "AthContainers/AuxElement.h"
#include "RootUtils/TSMethodCall.h"
#include "TMethodCall.h"
#include "TVirtualCollectionProxy.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/ConcurrentStrMap.h"
#include "CxxUtils/SimpleUpdater.h"

#include <unordered_map>
#include <string>
#include <typeinfo>

namespace ExpressionParsing {

  class BaseAccessorWrapper {
    public:
      virtual ~BaseAccessorWrapper() { }
      virtual bool isValid(const SG::AuxElement *auxElement) const = 0;
      virtual bool isValid(const SG::AuxVectorData *auxVectorData) const = 0;

      virtual int getIntValue(const SG::AuxElement *auxElement) const = 0;
      virtual double getDoubleValue(const SG::AuxElement *auxElement) const = 0;
      virtual std::vector<int> getVecIntValue(const SG::AuxVectorData *auxVectorData) = 0;
      virtual std::vector<double> getVecDoubleValue(const SG::AuxVectorData *auxVectorData) = 0;
  };


  template <typename T> class AccessorWrapper : public BaseAccessorWrapper { 
    public:
      AccessorWrapper(const std::string &elementName) :
        m_elementName(elementName), m_acc(elementName) 
      {
      }

      virtual ~AccessorWrapper() { }

      virtual bool isValid(const SG::AuxElement *auxElement) const {
        return m_acc.isAvailable(*auxElement);
      }

      virtual bool isValid(const SG::AuxVectorData *auxVectorData) const {
        // TPSG: A bit hacky - implementation copied from AuxElement.icc
        SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
        SG::auxid_t auxid = r.findAuxID(m_elementName);
        if (!auxVectorData || !auxVectorData->getConstStore()) return false;

        // At this point we just have to cross our fingers and hope...
        if (auxVectorData->size_v() == 0) return true;

        const SG::auxid_set_t& ids = auxVectorData->getConstStore()->getAuxIDs();
        if( ids.find (auxid) != ids.end() ) return true;
        return auxVectorData->getConstStore()->getData( auxid ) != 0;
      }

      virtual int getIntValue(const SG::AuxElement *auxElement) const {
        return (int) m_acc(*auxElement);
      }

      virtual double getDoubleValue(const SG::AuxElement *auxElement) const {
        return (double) m_acc(*auxElement);
      }

      virtual std::vector<int> getVecIntValue(const SG::AuxVectorData *auxVectorData) {
        size_t N = auxVectorData->size_v();
        std::vector<int> result(N);
        for (size_t i = 0; i < N; ++i) {
          result[i] = (int) m_acc(*auxVectorData, i);
        }
        return result;
      }

      virtual std::vector<double> getVecDoubleValue(const SG::AuxVectorData *auxVectorData) {
        size_t N = auxVectorData->size_v();
        std::vector<double> result(N);
        for (size_t i = 0; i < N; ++i) {
          result[i] = (double) m_acc(*auxVectorData, i);
        }
        return result;
      }

    private:
      std::string m_elementName;
      typename SG::AuxElement::ConstAccessor<T> m_acc;
  };


  class TMethodWrapper : public BaseAccessorWrapper {
    public:
      TMethodWrapper(const std::type_info &elementTypeinfo, const std::string &methodName);
      TMethodWrapper(const TMethodWrapper&) = delete;
      TMethodWrapper& operator= (const TMethodWrapper&) = delete;
      virtual ~TMethodWrapper();
      IAccessor::VariableType variableType();
      virtual bool isValid(const SG::AuxElement *auxElement) const;
      virtual bool isValid(const SG::AuxVectorData *auxVectorData) const;

      virtual int getIntValue(const SG::AuxElement *auxElement) const;
      virtual double getDoubleValue(const SG::AuxElement *auxElement) const;
      virtual std::vector<int> getVecIntValue(const SG::AuxVectorData *auxVectorData);
      virtual std::vector<double> getVecDoubleValue(const SG::AuxVectorData *auxVectorData);

    private:
      mutable RootUtils::TSMethodCall m_methodCall ATLAS_THREAD_SAFE;
      bool m_valid;
  };


  class TMethodCollectionWrapper : public BaseAccessorWrapper {
    public:
      TMethodCollectionWrapper(const std::type_info &containerTypeinfo, const std::string &methodName);
      TMethodCollectionWrapper(const TMethodCollectionWrapper&) = delete;
      TMethodCollectionWrapper& operator= (const TMethodCollectionWrapper&) = delete;
      virtual ~TMethodCollectionWrapper();
      IAccessor::VariableType variableType();
      virtual bool isValid(const SG::AuxElement *auxElement) const;
      virtual bool isValid(const SG::AuxVectorData *auxVectorData) const;

      virtual int getIntValue(const SG::AuxElement *auxElement) const;
      virtual double getDoubleValue(const SG::AuxElement *auxElement) const;
      virtual std::vector<int> getVecIntValue(const SG::AuxVectorData *auxVectorData);
      virtual std::vector<double> getVecDoubleValue(const SG::AuxVectorData *auxVectorData);

    private:
      TVirtualCollectionProxy *m_collectionProxy;
      mutable RootUtils::TSMethodCall m_methodCall ATLAS_THREAD_SAFE;
      bool m_valid;
  };


  class xAODProxyLoader : public RelayProxyLoader {
  public:
    xAODProxyLoader();
    virtual ~xAODProxyLoader();

    /// Disallow copy construction and assignment.
    xAODProxyLoader(const xAODProxyLoader&) = delete;
    xAODProxyLoader& operator=(const xAODProxyLoader&) = delete;

    virtual void reset();

  protected:
    template<class TYPE, class AUX>
    bool try_type(const std::string& varname, const std::type_info* ti, const AUX* data) const;

    template<class AUX>
    IAccessor::VariableType try_all_known_types(const std::string& varname, const AUX* data, bool isVector) const;

    using accessorCache_t = CxxUtils::ConcurrentStrMap<BaseAccessorWrapper*, CxxUtils::SimpleUpdater>;
    mutable accessorCache_t m_accessorCache ATLAS_THREAD_SAFE;
  };


  class xAODElementProxyLoader : public xAODProxyLoader {
    public:
      xAODElementProxyLoader() = default;
      xAODElementProxyLoader(const SG::AuxElement *auxElement);

      void setData(const SG::AuxElement *auxElement);

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      const SG::AuxElement *m_auxElement{nullptr};
  };


  class xAODVectorProxyLoader : public xAODProxyLoader {
    public:
      xAODVectorProxyLoader() = default;
      xAODVectorProxyLoader(const SG::AuxVectorData *auxVectorData);

      void setData(const SG::AuxVectorData *auxElement);

      virtual IAccessor::VariableType variableTypeFromString(const std::string &varname) const override;

      virtual int loadInt(const EventContext& ctx,const std::string &varname) const override;
      virtual double loadDouble(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<int> loadVecInt(const EventContext& ctx,const std::string &varname) const override;
      virtual std::vector<double> loadVec(const EventContext& ctx,const std::string &varname) const override;

    private:
      const SG::AuxVectorData *m_auxVectorData{nullptr};
  };

}

#endif // XAOD_VARIABLE_PROXY_LOADERS_H
