#pragma once

#include <GaudiKernel/MsgStream.h>

class BeamPipeDetectorManager;

namespace InDetDD {
class SiDetectorManager;
}

class IMessageSvc;
class ActsElementVector;

namespace Acts {
class GeometryContext;
class TrackingGeometry;
namespace Experimental {
class BlueprintNode;
}

}  // namespace Acts

namespace ActsTrk {


class ActsBlueprintConstruction {
 public:
  struct Config {
    const BeamPipeDetectorManager* beamPipeMgr = nullptr;
    const InDetDD::SiDetectorManager* itkPixelManager = nullptr;
    const InDetDD::SiDetectorManager* itkStripManager = nullptr;
    std::string graphviz;
    bool objDebugOutput = false;
    ActsElementVector* elementStore =nullptr;
    bool doEndcapLayerMerging = true;
  };

  ActsBlueprintConstruction(Config cfg, IMessageSvc* msgSvc, MsgStream& msg,
                            MSG::Level lvl)
      : m_msgSvc(msgSvc), m_msg(msg), m_lvl(lvl), m_cfg(std::move(cfg)) {
    m_msg << lvl;
  }

  std::shared_ptr<const Acts::TrackingGeometry> buildBlueprintGeometry(
      const Acts::GeometryContext& gctx);

 private:
  void addBeamPipeToBlueprint(const Acts::GeometryContext& gctx,
                              Acts::Experimental::BlueprintNode& blueprint);
  void addITkPixelToBlueprint(const Acts::GeometryContext& gctx,
                              Acts::Experimental::BlueprintNode& blueprint);
  void addITkStripToBlueprint(const Acts::GeometryContext& gctx,
                              Acts::Experimental::BlueprintNode& blueprint);
  void addHGTDToBlueprint(const Acts::GeometryContext& gctx,
                          Acts::Experimental::BlueprintNode& blueprint);

  inline MsgStream& msg() { return msgStream(); }
  inline MsgStream& msg(const MSG::Level lvl) { return msgStream(lvl); }
  inline bool msgLvl(const MSG::Level lvl) const { return msgLevel(lvl); }

  inline MsgStream& msgStream() { return m_msg; }

  inline MsgStream& msgStream(const MSG::Level lvl) { return m_msg << lvl; }

  inline MSG::Level msgLevel() const { return m_lvl; }

  bool msgLevel(MSG::Level lvl) const { return msgLevel() <= lvl; }

  IMessageSvc* m_msgSvc;
  MsgStream& m_msg;
  MSG::Level m_lvl;
  Config m_cfg;
};

}  // namespace ActsTrk
