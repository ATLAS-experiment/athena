/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "Geo2G4Svc.h"
#include "Geo2G4AssemblyVolume.h"
#include "ExtParameterisedVolumeBuilder.h"

Geo2G4Svc::Geo2G4Svc(const std::string& name, ISvcLocator* svcLocator)
  : base_class(name,svcLocator)
{
}

Geo2G4Svc::~Geo2G4Svc()
{}

StatusCode Geo2G4Svc::initialize()
{
  ATH_MSG_VERBOSE ("Initializing the Geo2G4Svc");
  m_G4AssemblyFactory = std::make_unique<Geo2G4AssemblyFactory>();

  // Initialize builders
  const std::string nameBuilder = "Extended_Parameterised_Volume_Builder"; //TODO Configurable property??
  m_builders.emplace(nameBuilder,
                     std::make_unique<ExtParameterisedVolumeBuilder>(nameBuilder, m_G4AssemblyFactory.get()));

  this->SetDefaultBuilder(nameBuilder);
  if(msgLvl(MSG::VERBOSE)) {
    ATH_MSG_VERBOSE (nameBuilder << " --> set as default builder" );
    ATH_MSG_VERBOSE (nameBuilder << " --> ParamOn flag = " << GetDefaultBuilder()->GetParam());
    this->ListVolumeBuilders();
  }

  return StatusCode::SUCCESS;
}

void Geo2G4Svc::ListVolumeBuilders() const
{
  ATH_MSG_INFO("---- List of all Volume Builders registered with Geo2G4Svc ----");
  ATH_MSG_INFO("---------------------------------------------------------------");
  for (const auto& builder : m_builders)
    {
      ATH_MSG_INFO(" Volume Builder: "<<builder.second->GetKey());
    }
  ATH_MSG_INFO("---------------------------------------------------------------");
  ATH_MSG_INFO(" default builder is "<<GetDefaultBuilder()->GetKey());
}

VolumeBuilder* Geo2G4Svc::GetVolumeBuilder(std::string s) const
{
  const auto builderItr(m_builders.find(s));
  if (builderItr!=m_builders.end())
    {
      return builderItr->second.get();
    }
  else
    {
      ATH_MSG_ERROR ("Trying to retrieve a not existing builder "<<s);
      ATH_MSG_ERROR ("\treturning Default Builder");
    }
  return GetDefaultBuilder();
}
