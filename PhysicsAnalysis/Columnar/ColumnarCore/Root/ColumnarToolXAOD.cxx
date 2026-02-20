/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <ColumnarCore/ColumnarTool.h>

#include <AsgDataHandles/VarHandleKey.h>
#include <AsgTools/AsgTool.h>

//
// method implementations
//

namespace columnar
{
  ColumnarTool<ColumnarModeXAOD> ::
  ColumnarTool ()
    : m_sharedData (std::make_shared<SharedData>(this))
  {
  }

  ColumnarTool<ColumnarModeXAOD> ::
  ColumnarTool (ColumnarTool<ColumnarModeXAOD>* val_parent)
  {
    if (val_parent)
    {
      m_sharedData = val_parent->m_sharedData;
      if (auto *tool = dynamic_cast<asg::AsgTool*>(this))
        m_sharedData->m_fulltools.insert (tool);
      m_sharedData->m_subtools.insert(this);
    } else
      m_sharedData = std::make_shared<SharedData>(this);
  }

  ColumnarTool<ColumnarModeXAOD> ::
  ~ColumnarTool ()
  {
    m_sharedData->m_subtools.erase(this);
    if (auto *tool = dynamic_cast<asg::AsgTool*>(this))
      m_sharedData->m_fulltools.erase (tool);
  }

  StatusCode ColumnarTool<ColumnarModeXAOD> ::
  initializeColumns ()
  {
    for (auto& [key_weak, addMTDependency] : m_sharedData->m_keys)
    {
      if (auto key = key_weak.lock(); key && !key->key().empty())
      {
        // we don't have a message stream here, and the key should have
        // already logged the error, so not logging anything here
        if (key->initialize().isFailure())
          return StatusCode::FAILURE;
#ifndef XAOD_STANDALONE
        if (addMTDependency)
        {
          for (auto* tool : m_sharedData->m_fulltools)
            tool->addDependency (key->fullKey(), key->mode());
        }
#endif
      }
    }
    m_sharedData->m_keys.clear();
    return StatusCode::SUCCESS;
  }

  void ColumnarTool<ColumnarModeXAOD> ::
  addSubtool (ColumnarTool<ColumnarModeXAOD>& subtool)
  {
    auto sharedData = subtool.m_sharedData;
    for (auto* tool : sharedData->m_subtools)
      tool->m_sharedData = m_sharedData;
    m_sharedData->m_subtools.insert (sharedData->m_subtools.begin(), sharedData->m_subtools.end());
    m_sharedData->m_fulltools.insert (sharedData->m_fulltools.begin(), sharedData->m_fulltools.end());
  }

  void ColumnarTool<ColumnarModeXAOD> ::
  callEvents (ObjectRange<ContainerId::eventContext,ColumnarModeXAOD> /*events*/) const
  {
    throw std::runtime_error ("tool didn't implement callEvents");
  }

  void ColumnarTool<ColumnarModeXAOD> ::
  internalAddKey (std::weak_ptr<SG::VarHandleKey> key, bool addMTDependency)
  {
    m_sharedData->m_keys.emplace_back (key, addMTDependency);
  }

  ColumnarTool<ColumnarModeXAOD>::SharedData ::
  SharedData (ColumnarTool<ColumnarModeXAOD>* self)
    : m_subtools ({self})
  {
    if (auto *tool = dynamic_cast<asg::AsgTool*>(self))
      m_fulltools.insert (tool);
  }
}
