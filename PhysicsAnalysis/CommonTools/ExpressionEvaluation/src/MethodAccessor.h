/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _ExpressionEvaluation_MethodAccessor_h_
#define _ExpressionEvaluation_MethodAccessor_h_

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandle.h"
#include "ExpressionEvaluation/IProxyLoader.h"
#include "ExpressionEvaluation/IAccessor.h"
#include "RootUtils/TSMethodCall.h"
#include "NormalizedTypeNameUtil.h"

#include "TClass.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "CxxUtils/checker_macros.h"
#include "TVirtualCollectionProxy.h"
#include "ClingCallWrapper.h"
#include "TFunction.h"

#include <memory>
#include <map>
#include <stdexcept>
#include <cassert>
#include <cstdint> //for uint64_t etc.

namespace ExpressionParsing {

   /** Auxiliary class to handle method calls of vector containers (AuxVectorBase)
    */
   template <class T_Cont,typename T_src>
   class CollectionMethodHelper {
   public:
      CollectionMethodHelper(const RootUtils::ClingCallWrapper<T_src> &method_wrapper,
                             const TVirtualCollectionProxy &collection_proxy,
                             const void *data,
                             [[maybe_unused]] unsigned int n_elements)
         : m_methodWrapper(method_wrapper),
           m_collectionProxy( collection_proxy.Generate())
      {
         void* data_nc ATLAS_THREAD_SAFE = const_cast<void *>(data);  // required by TVirtualCollectionProxy
         m_collectionProxy->PushProxy(data_nc);
         assert( m_collectionProxy->Size() == n_elements);
      }

      std::size_t size(const T_Cont &/*cont*/) const {
         return m_collectionProxy->Size();
      }

      /// Get the scalar provided by the container
      T_src get(const T_Cont &/*cont*/) const {
         assert( m_collectionProxy->Size() == 1);
         T_src ret = m_methodWrapper((*m_collectionProxy)[0]);
         return ret;
      }

      /// Get the specified element of the vector provided by the container
      T_src get(const T_Cont &/*cont*/, std::size_t idx) const {
         void *element_data=(*m_collectionProxy)[idx];
         T_src ret = m_methodWrapper(element_data);
         return ret;
      }

      /** Auxiliary class to create the corresponding auxiliary helper object
       */
      class Kit {
      public:
         Kit(Kit &&) = default;
         Kit(RootUtils::ClingCallWrapper<T_src> &&method_wrapper,
             TVirtualCollectionProxy &collection_proxy)
            : m_methodWrapper(std::move(method_wrapper)),
              m_collectionProxy(&collection_proxy)
         {}

         CollectionMethodHelper<T_Cont,T_src> create(const EventContext& /*ctx*/,
                                                     SG::ReadHandle<T_Cont> &handle) const
        {
          return CollectionMethodHelper<T_Cont,T_src>(m_methodWrapper,
                                                      *m_collectionProxy, getVectorData(*handle), getContainerSize(*handle));
         }
      private:
         RootUtils::ClingCallWrapper<T_src> m_methodWrapper;
         const TVirtualCollectionProxy *m_collectionProxy;
      };

   private:
      RootUtils::ClingCallWrapper<T_src> m_methodWrapper;
      std::unique_ptr<TVirtualCollectionProxy> m_collectionProxy;
   };

   /** Auxiliary class to handle method calls of "scalar" containers (AuxElement).
    */
   template <class T_Cont,typename T_src>
   class MethodHelper {
   public:
      MethodHelper(const RootUtils::ClingCallWrapper<T_src> &method_wrapper,
                   const void *data)
         : m_methodWrapper(method_wrapper),
           m_data(data)
      {
      }

      std::size_t size(const T_Cont &cont) const {
         return getContainerSize(cont); //@TODO correct ? or 1 ?
      }

      /// Get the scalar provided by the container
      T_src get(const T_Cont &/*cont*/) const {
         void* data_nc ATLAS_THREAD_SAFE = const_cast<void *>(m_data);
         T_src ret = this->m_methodWrapper(data_nc);
         return ret;
      }

      /// Get the specified element of the vector provided by the container
      T_src get(const T_Cont &/*cont*/, [[maybe_unused]] std::size_t idx) const {
         assert( idx==0);
         void* data_nc ATLAS_THREAD_SAFE = const_cast<void *>(m_data);  // required by TMethodCall
         T_src ret = this->m_methodWrapper(data_nc);
         return ret;
      }

      /** Auxiliary class to create the corresponding auxiliary helper object
       */
      class Kit {
      public:
         Kit(Kit &&) = default;
         Kit(RootUtils::ClingCallWrapper<T_src> &&method_wrapper) : m_methodWrapper(std::move(method_wrapper)) {}

         MethodHelper<T_Cont,T_src> create(const EventContext& /*ctx*/, SG::ReadHandle<T_Cont> &handle) const {
            return MethodHelper<T_Cont,T_src>(m_methodWrapper,handle.cptr());
         }
      private:
         RootUtils::ClingCallWrapper<T_src>  m_methodWrapper;
      };
   private:
      RootUtils::ClingCallWrapper<T_src>  m_methodWrapper;
      const void  *m_data;
   };

   /** Class to create accessor which call methods of an AuxElement of an AuxVectorBase container (singleton).
    */
   class MethodAccessorFactory : public Singleton<MethodAccessorFactory> {
   private:
      class IMethodAccessorKit {
         friend class MethodAccessorFactory;
      public:
         virtual ~IMethodAccessorKit() {}
         virtual std::unique_ptr<IAccessor> create( const SG::ReadHandleKey<SG::AuxVectorBase> &key,
                                                    RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                                    TVirtualCollectionProxy *proxy=nullptr) const = 0;
         virtual std::unique_ptr<IAccessor> create( const SG::ReadHandleKey<SG::AuxElement> &key,
                                                    RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                                    TVirtualCollectionProxy *proxy=nullptr) const = 0;
      };

      /** Auxiliary class to create a specific accessor which calls a method of an AuxElement or AuxVectorBase.
       */
      template <class T_src>
      class MethodAccessorKit : public IMethodAccessorKit {
      public:
         MethodAccessorKit(ExpressionParsing::IAccessor::VariableType scalar_type,
                           ExpressionParsing::IAccessor::VariableType vector_type)
            : m_scalarType(scalar_type),
              m_vectorType(vector_type)
         {}

         /** create an accessor which called the specified method of an AuxVectorBase.*/
         virtual std::unique_ptr<IAccessor> create( const SG::ReadHandleKey<SG::AuxVectorBase> &key,
                                                    RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                                    TVirtualCollectionProxy *proxy) const override {
            if (!proxy) {
               const auto msg = "Cannot use method access of types SG::AuxVectorBase without a collection proxy.";
               throw std::logic_error(msg);
            }
            return createAccessor<SG::AuxVectorBase, VectorHelper>(key,std::move(method_wrapper),proxy,m_vectorType);
         }

         /** create an accessor which called the specified method of an AuxElement.*/
         virtual std::unique_ptr<IAccessor> create( const SG::ReadHandleKey<SG::AuxElement> &key,
                                                    RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                                    TVirtualCollectionProxy *proxy=nullptr) const override {
            if (proxy) {
               return createAccessor<SG::AuxElement,VectorHelper>(key,std::move(method_wrapper),proxy, m_vectorType);
            }
            else {
               return createAccessor<SG::AuxElement,ScalarHelper>(key,std::move(method_wrapper),proxy, m_scalarType );
            }
            // @TODO really vectorType and GenScalarAccessor if collection proxy is given ?
         }
      private:
         template <class T_Aux, class T_ScalarVectorHelper>
         std::unique_ptr<IAccessor> createAccessor( const SG::ReadHandleKey<T_Aux> &key,
                                                    RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                                    TVirtualCollectionProxy *proxy,
                                                    ExpressionParsing::IAccessor::VariableType variable_type) const {
            if (proxy) {
               return std::make_unique<GenAccessor<T_Aux,
                                                   typename CollectionMethodHelper<T_Aux,T_src>::Kit,
                                                   T_ScalarVectorHelper> >(key,
                                                                           typename CollectionMethodHelper<T_Aux,T_src>::Kit(
                                                                                        RootUtils::getClingCallWrapperChecked<T_src>(std::move(method_wrapper)),
                                                                                        *proxy),
                                                                           variable_type);
            }
            else {
               return std::make_unique<GenAccessor<T_Aux,
                                                   typename MethodHelper<T_Aux,T_src>::Kit,
                                                   T_ScalarVectorHelper> >(key,
                                                                           typename MethodHelper<T_Aux,T_src>::Kit(
                                                                                       RootUtils::getClingCallWrapperChecked<T_src>(std::move(method_wrapper))),
                                                                           variable_type);
            }
         }
         ExpressionParsing::IAccessor::VariableType m_scalarType;
         ExpressionParsing::IAccessor::VariableType m_vectorType;
      };

      template <typename T>
      static std::pair<std::string, std::pair<std::unique_ptr<IMethodAccessorKit>, TInterpreter::EReturnType> > keyAndIntMethodKit() {
         return std::make_pair(RootUtils::getNormalizedTypeName<T>(),
                               std::make_pair(std::make_unique<MethodAccessorKit<T> >(IProxyLoader::VT_INT,IProxyLoader::VT_VECINT),
                                              TInterpreter::EReturnType::kLong));
      }
      template <typename T>
      static std::pair<std::string, std::pair<std::unique_ptr<IMethodAccessorKit>, TInterpreter::EReturnType> > keyAndDoubleMethodKit() {
         return std::make_pair(RootUtils::getNormalizedTypeName<T>(),
                               std::make_pair(std::make_unique<MethodAccessorKit<T> >(IProxyLoader::VT_DOUBLE, IProxyLoader::VT_VECDOUBLE),
                                              TInterpreter::EReturnType::kDouble));
      }
   public:
      MethodAccessorFactory() {
         m_kits.insert(keyAndIntMethodKit<bool>());
         m_kits.insert(keyAndIntMethodKit<std::int8_t>());
         m_kits.insert(keyAndIntMethodKit<std::int16_t>());
         m_kits.insert(keyAndIntMethodKit<std::int32_t>());
         m_kits.insert(keyAndIntMethodKit<std::int64_t>());
         m_kits.insert(keyAndIntMethodKit<std::uint8_t>());
         m_kits.insert(keyAndIntMethodKit<std::uint16_t>());
         m_kits.insert(keyAndIntMethodKit<std::uint32_t>());
         m_kits.insert(keyAndIntMethodKit<std::uint64_t>());
         m_kits.insert(keyAndDoubleMethodKit<double>());
         m_kits.insert(keyAndDoubleMethodKit<float>());
      }

      /** Create an accessor which calls the specified method of an AuxElement.
       */
      std::unique_ptr<IAccessor> create(const SG::ReadHandleKey<SG::AuxElement> &key,
                                        RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                        TVirtualCollectionProxy *proxy=nullptr) const {
         return getKit(method_wrapper).create(key,std::move(method_wrapper), proxy);
      }

      /** Create an accessor which calls the specified method of an AuxVectorBase.
       */
      std::unique_ptr<IAccessor> create(const SG::ReadHandleKey<SG::AuxVectorBase> &key,
                                        RootUtils::ClingCallWrapperUncheckedReturnValue<> &&method_wrapper,
                                        TVirtualCollectionProxy *proxy=nullptr) const {
         return getKit(method_wrapper).create(key,std::move(method_wrapper), proxy);
      }

   private:

      /** Get an specific class which creates the accessor for the given method.
       */
      const IMethodAccessorKit &getKit(const RootUtils::ClingCallWrapperUncheckedReturnValue<> &method_wrapper) const {

         std::map<std::string, std::pair<std::unique_ptr<IMethodAccessorKit>, TInterpreter::EReturnType> >::const_iterator
            iter = m_kits.find(method_wrapper.getReturnTypeNormalizedName());
         if (iter == m_kits.end()) {
            const std::string amsg = "ExpressionParsing::MethodAccessorFactory: no kit for return type " + method_wrapper.getReturnTypeNormalizedName()
                 + " wrapper " + typeid(method_wrapper).name();
            throw std::runtime_error(amsg);
         }
         return *(iter->second.first);
      }
      std::map<std::string, std::pair<std::unique_ptr<IMethodAccessorKit>, TInterpreter::EReturnType> > m_kits;
   };

}
#endif
