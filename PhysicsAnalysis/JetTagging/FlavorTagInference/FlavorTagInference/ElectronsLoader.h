/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general Electrons from the jet
  and extract their features for the NN evaluation. For now it supports only neutral flow objects.
  Charged flow objects have experimental support and are not recommended for use.
*/

#ifndef ELECTRONS_LOADER_H
#define ELECTRONS_LOADER_H

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODBase/IParticle.h"
#include "xAODBase/IParticleContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODJet/JetFwd.h"

// STL includes
#include <string>
#include <vector>
#include <functional>

namespace FlavorTagInference {

ConstituentsInputConfig createElectronsLoaderConfig(
    std::pair<std::string, std::vector<std::string>> iparticle_names
);
// Subclass for Electrons loader inherited from abstract IConstituentsLoader class
class ElectronsLoader final : public IConstituentsLoader {
 public:
  ElectronsLoader(const ConstituentsInputConfig&, const FTagOptions& options);
  std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(
      const xAOD::IParticle& jet ) const override;
  const FTagDataDependencyNames& getDependencies() const override;
  const std::set<std::string>& getUsedRemap() const override;
  const std::string& getName() const override;
  const ConstituentsType& getType() const override;

 protected:
  // typedefs
  typedef xAOD::IParticle Jet;
  typedef std::pair<std::string, double> NamedVar;
  typedef std::pair<std::string, std::vector<double>> NamedSeq;
  // electrons typedefs
  typedef std::vector<const xAOD::Electron*> Electrons;
  typedef std::function<double(const xAOD::Electron*, const Jet&)> ElectronSortVar;

  // filter function
  typedef std::function<bool(const Jet&, const xAOD::Electron*)> ElectronFilter;

  // usings for Electron getter
  using AE = SG::AuxElement;
  using IPC = xAOD::IParticleContainer;
  using PartLinks = std::vector<ElementLink<IPC>>;
  using IPV = std::vector<const xAOD::Electron*>;

  ElectronSortVar electronSortVar(ConstituentsSortOrder);

  Electrons getElectronsFromJet(const xAOD::IParticle& jet) const;
  std::pair<ElectronFilter,std::set<std::string>> electronFilter(ConstituentsSelection config);

  ElectronSortVar m_electronSortVar;
  ElectronFilter m_electronFilter;
  getter_utils::SeqGetter<xAOD::Electron> m_seqGetter;
  std::function<IPV(const Jet&)> m_associator;
};
}  // namespace FlavorTagInference

#endif
