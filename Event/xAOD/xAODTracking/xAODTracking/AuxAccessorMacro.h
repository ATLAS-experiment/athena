/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_AUXACCESSORMACRO_H
#define XAODTRACKING_AUXACCESSORMACRO_H

/*
#define DEFINE_API(__TYPE, __GETTER, __SETTER)                           \
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SurfaceBackend_v1, __TYPE, __GETTER, \
                                       __SETTER)                          \
  __TYPE* SurfaceBackend_v1::__GETTER##Ptr() {                              \
    static const SG::Accessor<__TYPE> acc(#__GETTER);         \
    return &(acc(*this));                                                 \
  }                                                                       \
  const __TYPE* SurfaceBackend_v1::__GETTER##Ptr() const {                  \
    static const SG::ConstAccessor<__TYPE> acc(#__GETTER);    \
    return &(acc(*this));                                                 \
  }  
*/
#define DEFINE_API(__CL, __TYPE, __GETTER, __SETTER)                           \
  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(__CL, __TYPE, __GETTER, \
                                       __SETTER)                          \
  __TYPE* __CL::__GETTER##Ptr() {                              \
    static const SG::Accessor<__TYPE> acc(#__GETTER);         \
    return &(acc(*this));                                                 \
  }                                                                       \
  const __TYPE* __CL::__GETTER##Ptr() const {                  \
    static const SG::ConstAccessor<__TYPE> acc(#__GETTER);    \
    return &(acc(*this));                                                 \
  }   

#endif // XAODTRACKING_AUXACCESSORMACRO_H