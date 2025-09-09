/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

// Local include(s).
#include <EventLoop/AlgorithmMemoryModule.h>
#include <EventLoop/AlgorithmStateModule.h>
#include <EventLoop/AlgorithmTimerModule.h>
#include <EventLoop/BatchInputModule.h>
#include <EventLoop/DirectInputModule.h>
#include <EventLoop/EventCountModule.h>
#include <EventLoop/FactoryPreloadModule.h>
#include <EventLoop/FileExecutedModule.h>
#include <EventLoop/GridReportingModule.h>
#include <EventLoop/LeakCheckModule.h>
#include <EventLoop/MemoryMonitorModule.h>
#include <EventLoop/StopwatchModule.h>
#include <EventLoop/PostClosedOutputsModule.h>
#include <EventLoop/TEventModule.h>
#include <EventLoop/WorkerConfigModule.h>

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT(EL::Detail::AlgorithmMemoryModule)
DECLARE_COMPONENT(EL::Detail::AlgorithmStateModule)
DECLARE_COMPONENT(EL::Detail::AlgorithmTimerModule)
DECLARE_COMPONENT(EL::Detail::BatchInputModule)
DECLARE_COMPONENT(EL::Detail::DirectInputModule)
DECLARE_COMPONENT(EL::Detail::EventCountModule)
DECLARE_COMPONENT(EL::Detail::FactoryPreloadModule)
DECLARE_COMPONENT(EL::Detail::FileExecutedModule)
DECLARE_COMPONENT(EL::Detail::GridReportingModule)
DECLARE_COMPONENT(EL::Detail::LeakCheckModule)
DECLARE_COMPONENT(EL::Detail::MemoryMonitorModule)
DECLARE_COMPONENT(EL::Detail::StopwatchModule)
DECLARE_COMPONENT(EL::Detail::PostClosedOutputsModule)
DECLARE_COMPONENT(EL::Detail::TEventModule)
DECLARE_COMPONENT(EL::Detail::WorkerConfigModule)
