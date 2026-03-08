/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ROOTUTILS_CLINGCALLWRAPPER_H
#define ROOTUTILS_CLINGCALLWRAPPER_H

#include "NormalizedTypeNameUtil.h"

#include <type_traits>
#include <array>
#include <string>
#include <sstream>
#include <stdexcept>

#include "TClass.h"
#include "TMethod.h"
#include "TROOT.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TGenericClassInfo.h"

#include "CxxUtils/checker_macros.h"

namespace RootUtils {
template<typename T>
constexpr auto getPtr(typename std::remove_reference<T>::type& arg) noexcept {
   // the interface requires that the argument list is a list of non-const void pointers
   // It is assumed that the argument types have been checked before hand.
   auto non_const_arg ATLAS_THREAD_SAFE = const_cast<std::remove_cvref<T>::type *>( &arg);
   return non_const_arg;
}

template <typename ...T_MethodArgs>
class ClingCallWrapperUncheckedReturnValue;
template <typename T_ReturnValue, typename ...T_MethodArgs>
class ClingCallWrapper;

template <typename ...T_MethodArgs>
ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> getClingCallWrapper(TClass *the_class, const std::string &method_name, bool is_const);

template <typename T_ReturnValue, typename ...T_MethodArgs>
ClingCallWrapper<T_ReturnValue, T_MethodArgs...> getClingCallWrapperChecked(ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> &&call_wrapper);

/// @brief Intermediate helper class wrapping a cling method call with arbitrary return type.
///
/// The arguments of the function call are defined by the template parameters,
/// but the return type can be arbitrary.
// @TODO add access policy to also handle non-const methods securely
template <typename ...T_MethodArgs>
class ClingCallWrapperUncheckedReturnValue {
   template<typename ...T_MethodArgsFunc>
   friend ClingCallWrapperUncheckedReturnValue<T_MethodArgsFunc...> getClingCallWrapper(TClass *the_class, const std::string &method_name, bool is_const);

   template <typename T_ReturnValue, typename ...T_MethodArgsFunc>
   friend ClingCallWrapper<T_ReturnValue, T_MethodArgsFunc...> getClingCallWrapperChecked(ClingCallWrapperUncheckedReturnValue<T_MethodArgsFunc...> &&call_wrapper);

protected:
   // @brief The function prototype of the function returned by TMethod::InterfaceMethod:
   // @param obj pointer to the object.
   // @param nargs number of method arguments (excluding the this pointer).
   // @param arg_list array of void pointers, pointing to each of the nargs arguments.
   // @param ret_val pointer where the return value is to be stored.
   using TClingCallFuncWrapper_t = void (*)(void* /*obj*/, int /* nargs*/, void** /*arg_list*/, void* /*ret_val*/);

   ClingCallWrapperUncheckedReturnValue(TClingCallFuncWrapper_t method_wrapper, const TMethod *method)
      : m_methodWrapper(method_wrapper),
        m_method(method) {}

   template<typename T_ReturnType>
   T_ReturnType call(const void *object, T_MethodArgs...args) const {
      T_ReturnType return_value;
      std::array<void *, sizeof...(args)> arg_list{getPtr<T_MethodArgs>(args)...};

      // it was already checked that the method is declared const
      // the interface however requires a non const void pointer to the object independent of
      // of the method being const or non-const.
      void *non_const_obj ATLAS_THREAD_SAFE=const_cast<void *>(object);
      m_methodWrapper(non_const_obj, arg_list.size(),arg_list.data(),&return_value);
      return return_value;
   }
public:
   /// @brief Test whether the function return type and the template parameter are the same.
   /// @return true if the template parameter and the return type are the same.
   template<typename T_ReturnType>
   bool isReturnTypeMatching() const {
      return ROOT::Internal::GetDemangledTypeName(typeid(T_ReturnType)) == m_method->GetReturnTypeNormalizedName();
   }

   /// @brief the normalized name of the function return type.
   std::string getReturnTypeNormalizedName() const {
      return m_method->GetReturnTypeNormalizedName();
   }
private:
   TClingCallFuncWrapper_t m_methodWrapper = nullptr;
   const TMethod *m_method = nullptr;
   TMethodCall::EReturnType m_returnType{};
};

/// @brief Helper class to wrap a cling method call of an object's method with defined return type and argument list.
/// @tparam T_ReturnValue the type of the return value
/// @tparam ..T_MethofArgs the types of the method argument (excluding the this pointer)
template <typename T_ReturnValue, typename ...T_MethodArgs>
class ClingCallWrapper :  ClingCallWrapperUncheckedReturnValue<T_MethodArgs...>
{
template <typename T_ReturnValueFunc, typename ...T_MethodArgsFunc>
friend ClingCallWrapper<T_ReturnValueFunc, T_MethodArgsFunc...> getClingCallWrapperChecked(ClingCallWrapperUncheckedReturnValue<T_MethodArgsFunc...> &&call_wrapper);

protected:
   /// @brief create a cling method wrapper for a method with defined argument list and return type (move).
   ClingCallWrapper(ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> &&call_wrapper)
      : ClingCallWrapperUncheckedReturnValue<T_MethodArgs...>(std::move(call_wrapper))
   {}
   /// @brief create a cling method wrapper for a method with defined argument list and return type (copy).
   ClingCallWrapper(const ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> &call_wrapper)
      : ClingCallWrapperUncheckedReturnValue<T_MethodArgs...>(call_wrapper)
   {}
public:
   /// @brief call the wrapped cling method.
   /// @param object the this pointer to an object which matches the TClass used to create the call_wrapper.
   /// @param ...args the arguments pass to the method call.
   /// @return the return value of the called method.
   /// @note the object  type  is unchecked. If the pointer does not match the TClass that
   //        was used the create this wrapper, the result will be undefined.
   T_ReturnValue operator()(const void *object, T_MethodArgs...args) const {
      return this->template call<T_ReturnValue>(object, args...);
   }
};

/// @brief helper template to turn an argument list into a prototype string.
///
/// Creates a prototype string which can be used to query a method of
/// a TClass using GetMethodWithPrototype.
template <typename ...T_MethodArgs>
struct ArgGen {
static std::string addArgumentPrototype(std::string &arg_proto_type);
};

// specialization for an empty argument list.
template <>
struct ArgGen<> {
static std::string addArgumentPrototype([[maybe_unused]] std::string &arg_proto_type) {
   return std::string();
}
};

// implementation for an non-empty argument list.
template <typename T_First, typename ...T_MethodArgs>
struct ArgGen<T_First, T_MethodArgs...> {
static void  addArgumentPrototype(std::string &arg_proto_type) {
   if (!arg_proto_type.empty()) {
      arg_proto_type+=",";
   }
   else {
      constexpr unsigned int avTypeNameLength = 20;
      arg_proto_type.reserve(sizeof...(T_MethodArgs) * avTypeNameLength);
   }
   arg_proto_type += ROOT::Internal::GetDemangledTypeName(typeid(T_First));
   ArgGen<T_MethodArgs...>::addArgumentPrototype(arg_proto_type);
}
};

/// @brief template function to create a argument list prototype from a type  list.
/// @tparam ...T_MethodArgs the types of the argument list.
/// @return a string which can be used to query a method of a TClass using GetMethodWithPrototype.
template <typename ...T_MethodArgs>
std::string makeArgumentPrototype() {
   std::string ret;
   ArgGen<T_MethodArgs...>::addArgumentPrototype(ret);
   return ret;
}

/// @brief helper method to wrap a method of a certain name with defined arguments, but arbitrary return
///        type implemented by the given TClass.
/// @tparam ...T_MethodArgs the arguments of the function in question (excluding the this pointer).
/// @param the_class the TClass of the object.
/// @param method_name the bare method name (without argument list and attributes e.g. "pt").
/// @param is_const true if the method must be const.
/// @return intermediate helper object which wraps a cling function call for the given class and argument.
/// Will lock the global ROOT mutex, may throw an exception if the requirements are not fulfilled i.e.
/// non-static method, public and const depending on is_const, or there is no function of the given name
/// matching the arguments. The resulting object cannot be called yet. It is expected that it is
/// passed to @ref getClingCallWrapperChecked to get wrapper for a function with a specific return type.
template <typename ...T_MethodArgs>
ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> getClingCallWrapper(TClass *the_class, const std::string &method_name, bool is_const) {
   R__LOCKGUARD(gROOTMutex);
   const TMethod *method = the_class->GetMethodWithPrototype(method_name.c_str(),
                                                             makeArgumentPrototype<T_MethodArgs...>().c_str(),
                                                             is_const,
                                                             ROOT::kConversionMatch);
   if (!method) {
      std::stringstream amsg; amsg << "Failed to get method " << method_name << "(" << makeArgumentPrototype<T_MethodArgs...>() << ").";
      throw std::runtime_error(amsg.str());
   }
   MethodInfo_t *method_info=gInterpreter->MethodInfo_Factory(method->GetDeclId());
   static constexpr unsigned int flags = (kIsStatic|kIsPublic);
   unsigned int method_properties = gInterpreter->MethodInfo_Property(method_info);
   if ( (method_properties &  flags) != kIsPublic ) {
      std::runtime_error("Method is not public or is static.");
   }
   if (is_const && (method_properties & kIsConstant) != kIsConstant) {
      std::runtime_error("Method is not constant.");
   }
   using TheClingCallWrapper_t = ClingCallWrapperUncheckedReturnValue<T_MethodArgs...>;

   return TheClingCallWrapper_t(reinterpret_cast<TheClingCallWrapper_t::TClingCallFuncWrapper_t>(method->InterfaceMethod()),method);
}

/// @brief wrap a cling method call for a method with defined arguments and return types.
/// @tparam T_ReturnValue the type of the method return value.
/// @tparam ...T_MethodArgs the types of the arguments of the method.
/// @param call_wrapper a wrapped cling method call with arbitrary return type.
/// @return a wrapped cling method call for t
template <typename T_ReturnValue, typename ...T_MethodArgs>
ClingCallWrapper<T_ReturnValue, T_MethodArgs...> getClingCallWrapperChecked(ClingCallWrapperUncheckedReturnValue<T_MethodArgs...> &&call_wrapper) {
   if (!call_wrapper.template isReturnTypeMatching<T_ReturnValue>()) {
      std::stringstream amsg;
      amsg << "Return type mismatch. Desired "
           << getNormalizedTypeName<T_ReturnValue>()
           << " != is " << call_wrapper.getReturnTypeNormalizedName();
      throw std::runtime_error(amsg.str());
   }
   return ClingCallWrapper<T_ReturnValue, T_MethodArgs...>(std::forward<decltype(call_wrapper)>(call_wrapper));
}
}
#endif
