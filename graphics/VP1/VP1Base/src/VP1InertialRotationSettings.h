/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */
#ifndef VP1_INERTIAL_ROTATION_SETTINGS_H
#define VP1_INERTIAL_ROTATION_SETTINGS_H

#include <QAction>
#include <QMenu>
#include <QSettings>
#include <functional>

// Keep mode-specific user preferences independent of portable scene settings.
namespace VP1InertialRotationSettings {
inline const char* key()
{
  return qgetenv("VP1_REMOTE_MODE") == "1" ? "VP1/Remote/ContinueRotation"
                                         : "VP1/Normal/ContinueRotation";
}

inline bool enabled()
{
  return QSettings().value(key(), qgetenv("VP1_REMOTE_MODE") != "1").toBool();
}

inline QAction* addAction(QMenu* menu, const std::function<void(bool)>& apply)
{
  auto* action = menu->addAction("Continue rotating after mouse release");
  action->setCheckable(true);
  action->setToolTip("Keep spinning after a mouse drag. Disable for more responsive remote viewing.");
  QObject::connect(action, &QAction::triggered, menu, [apply](bool checked) {
    apply(checked);
    QSettings().setValue(key(), checked);
  });
  return action;
}
}
#endif
