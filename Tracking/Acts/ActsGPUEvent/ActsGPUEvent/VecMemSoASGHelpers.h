// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#ifndef ACTSGPUEVENT_VECMEMSOASGHELPERS_H
#define ACTSGPUEVENT_VECMEMSOASGHELPERS_H

// Framework include(s).
#include "AthenaKernel/BaseInfo.h"

/// Helper macro setting up all the standard conversion rules for VecMem SoA
/// types.
#define SG_ADD_VECMEM_SOA_CONVERSIONS(COLLECTION, CNVNAME)                    \
  SG_BASES(COLLECTION::buffer, COLLECTION::view);                             \
  SG_BASES(COLLECTION::data, COLLECTION::view);                               \
  SG_BASES(COLLECTION::const_data, COLLECTION::const_view);                   \
  namespace ActsTrk {                                                         \
  struct Device##CNVNAME##ViewConversion                                      \
      : public SG::CopyConversion<COLLECTION::view, COLLECTION::const_view> { \
    virtual void convert(const COLLECTION::view& view,                        \
                         COLLECTION::const_view& const_view) const override { \
      const_view = view;                                                      \
    }                                                                         \
  };                                                                          \
  struct Device##CNVNAME##BufferConversion                                    \
      : public SG::CopyConversion<COLLECTION::buffer,                         \
                                  COLLECTION::const_view> {                   \
    virtual void convert(const COLLECTION::buffer& buffer,                    \
                         COLLECTION::const_view& const_view) const override { \
      const_view = buffer;                                                    \
    }                                                                         \
  };                                                                          \
  struct Device##CNVNAME##DataConversion                                    \
      : public SG::CopyConversion<COLLECTION::data,                         \
                                  COLLECTION::const_view> {                   \
    virtual void convert(const COLLECTION::data& data,                    \
                         COLLECTION::const_view& const_view) const override { \
      const_view = data;                                                    \
    }                                                                         \
  };                                                                          \
  struct Device##CNVNAME##HostConversion                                      \
      : public SG::CopyConversion<COLLECTION::host, COLLECTION::const_data> { \
    virtual void convert(const COLLECTION::host& host,                        \
                         COLLECTION::const_data& const_data) const override { \
      const_data = vecmem::get_data(host);                                    \
    }                                                                         \
  };                                                                          \
  }                                                                           \
  SG_ADD_COPY_CONVERSION(COLLECTION::view,                                    \
                         ActsTrk::Device##CNVNAME##ViewConversion);           \
  SG_ADD_COPY_CONVERSION(COLLECTION::buffer,                                  \
                         ActsTrk::Device##CNVNAME##BufferConversion);         \
  SG_ADD_COPY_CONVERSION(COLLECTION::data,                                    \
                         ActsTrk::Device##CNVNAME##DataConversion);           \
  SG_ADD_COPY_CONVERSION(COLLECTION::host,                                    \
                         ActsTrk::Device##CNVNAME##HostConversion)

#endif  // ACTSGPUEVENT_VECMEMSOASGHELPERS_H
