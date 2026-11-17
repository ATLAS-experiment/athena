/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */
#ifndef VP1_SNAPSHOT_STATUS_H
#define VP1_SNAPSHOT_STATUS_H
#include <QString>

namespace VP1SnapshotStatus {
inline QString description()
{
  const auto message = qgetenv("VP1_SNAPSHOT_WORKAROUND_MESSAGE");
  return message.isEmpty() ? QStringLiteral("Not initialized by the vp1 launcher.")
                           : QString::fromLocal8Bit(message);
}

inline QString failureHint()
{
  const auto status = qgetenv("VP1_SNAPSHOT_WORKAROUND_STATUS");
  if (status == "active")
    return QStringLiteral("The GLX snapshot compatibility workaround is already active. "
                          "Restart with COIN_DEBUG_GLGLUE=1 for diagnostics, keep the terminal log "
                          "and contact the VP1 developers at hn-atlas-vp1-help@cern.ch.");
  if (status == "unavailable")
    return description() + QStringLiteral(" Check the startup diagnostics and install the required "
        "compiler/OpenGL/X11 development headers where applicable. "
        "Keep the log if rendering still fails.");
  if (status == "disabled")
    return QStringLiteral("Snapshot compatibility was disabled by --no-fix-snapshots. "
                          "Restart without that flag or explicitly use --fix-snapshots.");
  return QStringLiteral("Try restarting VP1 with --fix-snapshots (for example, vp1 --fix-snapshots). "
                        "Usage and requirements are documented in graphics/VP1/README.md, "
                        "section Temporary workaround for remote snapshot rendering.");
}
}
#endif
