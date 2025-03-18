/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IITkCablingTool_h
#define IITkCablingTool_h
/**
 * @file IITkCablingTool.h
 * Header file for abstract interface to ITkStrip cabling tool
 * Based on code by Susumu Oda
 * @author Shaun Roe
 * @author Edson Carquin
 * @date 10 October, 2024
 **/

//InnerDetector
//to ensure clients can actually use the conversion constructors, include these here (otherwise could be forward declared)
#include "ITkStripCabling/ITkStripOnlineId.h"

//Gaudi includes
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

//standard includes
#include <cstdint> //C++11
#include <vector>

//fwd declarations
class Identifier;
class IdentifierHash;

/**
 * @class IITkStripCablingTool
 * Client interface to the cabling, providing conversions between online and offline identifiers
 **/
class IITkStripCablingTool: virtual public IAlgTool {
 public:
  /// no-op destructor
  virtual ~IITkStripCablingTool() = default;

  /// Creates the InterfaceID and interfaceID() method
  DeclareInterfaceID(IITkStripCablingTool, 1, 0);

  /// return the online Id, given a hash (used by simulation encoders)
  virtual ITkStripOnlineId getOnlineIdFromHash(const IdentifierHash& hash) const = 0;
  virtual ITkStripOnlineId getOnlineIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const = 0;

  /// return the rob/rod Id, given a hash (used by simulation encoders)
  virtual std::uint32_t getRobIdFromHash(const IdentifierHash& hash) const = 0;
  virtual std::uint32_t getRobIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const = 0;
  
  /// fill a users vector with all the RodIds
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector) const = 0;
  virtual void getAllRods(std::vector<std::uint32_t>& usersVector, const EventContext& ctx) const = 0;
};

#endif // IITkStripCablingTool_h
