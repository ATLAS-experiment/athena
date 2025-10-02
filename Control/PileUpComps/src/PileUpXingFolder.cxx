/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/IProperty.h"
#include <exception>
#include "PileUpXingFolder.h"
PileUpXingFolder::PileUpXingFolder( const std::string& type,
                                    const std::string& name,
                                    const IInterface* parent) :
  base_class(type, name, parent),
  m_folder( "SG::Folder/SGF", this )
{
  m_cacheRefreshFrequency.verifier().setBounds(0.0, 1.0);
}

StatusCode PileUpXingFolder::initialize() {
  if (!(m_folder.retrieve()).isSuccess()) return StatusCode::FAILURE;
  return (dynamic_cast<IProperty&>(*m_folder)).setProperty(m_itemList);
}
