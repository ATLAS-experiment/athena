/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "Acts/Utilities/Logger.hpp"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/CommonMessaging.h"
#include "GaudiKernel/INamedInterface.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "AthenaKernel/getMessageSvc.h"
#include <memory>

#include <optional>

class ActsAthenaPrintPolicy final : public Acts::Logging::OutputPrintPolicy
{
public:

 ActsAthenaPrintPolicy(std::shared_ptr<MsgStream> msg, const std::string& name);

  void
  flush(const Acts::Logging::Level& lvl, const std::string& input) override;

  virtual 
    const std::string& 
    name() const override;

  virtual 
    std::unique_ptr<Acts::Logging::OutputPrintPolicy> 
    clone(const std::string& name) const override;

private:
  IMessageSvc* m_svc{};
  std::shared_ptr<MsgStream> m_msg{};
  std::string m_name{};
};

class ActsAthenaFilterPolicy final : public Acts::Logging::OutputFilterPolicy {
public:
  ActsAthenaFilterPolicy(std::shared_ptr<MsgStream> msg) : m_msg(msg) {}

  bool doPrint(const Acts::Logging::Level& lvl) const override;

  virtual 
    Acts::Logging::Level 
    level() const override;

  virtual 
    std::unique_ptr<Acts::Logging::OutputFilterPolicy> 
    clone(Acts::Logging::Level level) const override;

private:
  std::shared_ptr<MsgStream> m_msg;
};


std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(IMessageSvc *svc, const std::string& name, 
    int level, std::optional<std::string> parent_name);

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name);

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const AthMessaging* parent, const std::string& name);

std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name, 
    std::optional<std::string> parent_name);

// problem: string literal does not play well with std::optional
std::unique_ptr<const Acts::Logger>
makeActsAthenaLogger(const CommonMessagingBase* parent, const std::string& name, 
    const std::string& parent_name); 

    

