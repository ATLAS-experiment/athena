/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAMPTOOLS_SHAREDWRITERTOOL_H
#define ATHENAMPTOOLS_SHAREDWRITERTOOL_H

#include "AthenaMPToolBase.h"

class IConversionSvc;

class SharedWriterTool final : public AthenaMPToolBase
{
 public:
  SharedWriterTool(const std::string& type
		   , const std::string& name
		   , const IInterface* parent);

  virtual ~SharedWriterTool() override;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;

  // _________IAthenaMPTool_________   
  virtual int makePool ATLAS_NOT_THREAD_SAFE (int maxevt, int nprocs, const std::string& topdir) override;
  virtual StatusCode exec ATLAS_NOT_THREAD_SAFE () override;

  virtual void subProcessLogs(std::vector<std::string>&) override;
  virtual AthenaMP::AllWorkerOutputs_ptr generateOutputReport() override;

  // _____ Actual working horses ________
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> bootstrap_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> exec_func() override;
  virtual std::unique_ptr<AthenaInterprocess::ScheduledWork> fin_func() override;

 private:
  SharedWriterTool();
  SharedWriterTool(const SharedWriterTool&);
  SharedWriterTool& operator= (const SharedWriterTool&);

  // Properties
  Gaudi::Property<bool>  m_nMotherProcess{
      this, "MotherProcess", false,
      "Expect mother process to write event data. The default is false."};
  Gaudi::Property<bool> m_debug{
      this, "Debug", false,
      "Are we running in debug mode? The default is false"};

  int  m_rankId;          // Each worker has its own unique RankID from the range (0,...,m_nprocs-1)

  std::unique_ptr<AthenaInterprocess::SharedQueue>  m_sharedRankQueue;
  SmartIF<IConversionSvc>           m_cnvSvc;

};

#endif
