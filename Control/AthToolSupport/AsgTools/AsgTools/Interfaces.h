/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ASGTOOLS_INTERFACES_H
#define ASGTOOLS_INTERFACES_H

///
/// Standalone versions of Gaudi interface declarations.
//
#if defined(XAOD_STANDALONE)

/// This can't be a no-op, because the Gaudi version needs to be
/// followed by a semicolon, so we need a statement that requires to
/// be followed by a semi-colon.
#define DeclareInterfaceID(iface, major, minor) \
  static_assert(true)


/// See GaudiKernel/extend_interfaces.h
template <typename... Interfaces>
struct extend_interfaces : virtual public Interfaces... {
};


/// See GaudiKernel/extends.h
template <typename BASE, typename... Interfaces>
struct extends : public BASE, virtual public extend_interfaces<Interfaces...> {
  using base_class = extends;
  using BASE::BASE;
};


/// See GaudiKernel/implements.h
template <typename... Interfaces>
struct implements : virtual public extend_interfaces<Interfaces...> {
  using base_class = implements<Interfaces...>;
};

#endif
#endif
