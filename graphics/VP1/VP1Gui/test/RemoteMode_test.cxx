/* Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration */
// Exercise real widgets without an X server, detector geometry or GL rendering.
#include "VP1InertialRotationSettings.h"
#include "VP1Base/VP1SnapshotStatus.h"
#include "VP1Gui/VP1MainWindow.h"
#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool condition, const char* message)
{
  if (!condition) throw std::runtime_error(message);
}

void testRotationSettings()
{
  QMenu menu;
  bool applied = false;
  int updates = 0;
  auto* action = VP1InertialRotationSettings::addAction(&menu, [&](bool enabled) {
    applied = enabled;
    ++updates;
  });
  check(action->isCheckable(), "Rotation action must be a checkbox");
  check(action->text() == "Continue rotating after mouse release", "Incorrect checkbox label");
  qputenv("VP1_REMOTE_MODE", "1");
  check(!VP1InertialRotationSettings::enabled(), "Remote default must disable spinning");
  action->setChecked(VP1InertialRotationSettings::enabled());
  action->trigger();
  check(applied && updates == 1, "Checkbox must enable viewer spinning");
  check(VP1InertialRotationSettings::enabled(), "Remote preference was not persisted");

  qputenv("VP1_REMOTE_MODE", "0");
  check(VP1InertialRotationSettings::enabled(), "Normal default must enable spinning");
  action->setChecked(VP1InertialRotationSettings::enabled());
  action->trigger();
  check(!applied && updates == 2, "Checkbox must disable viewer spinning");
  check(!VP1InertialRotationSettings::enabled(), "Normal preference was not persisted");

  qputenv("VP1_REMOTE_MODE", "1");
  check(VP1InertialRotationSettings::enabled(), "Normal setting contaminated remote preference");
  action->setChecked(VP1InertialRotationSettings::enabled());
  action->trigger();
  check(!applied && updates == 3, "Remote checkbox must disable viewer spinning");
  check(!VP1InertialRotationSettings::enabled(), "Changed remote preference was not persisted");
  qputenv("VP1_REMOTE_MODE", "0");
  check(!VP1InertialRotationSettings::enabled(), "Remote setting contaminated normal preference");
}

void testNotice(bool remote, bool batch, const char* reason = "forwarded X11 display",
                const char* snapshots = "Active: GLX compatibility enabled.",
                const char* coin = "OpenGL 1.1+ features allowed for indirect contexts.")
{
  qputenv("VP1_REMOTE_MODE", remote ? "1" : "0");
  qputenv("VP1_REMOTE_REASON", reason);
  qputenv("VP1_SNAPSHOT_WORKAROUND_MESSAGE", snapshots);
  qputenv("VP1_COIN_INDIRECT_RENDERING_MESSAGE", coin);
  if (batch) qputenv("VP1_BATCHMODE", "1");
  else qunsetenv("VP1_BATCHMODE");
  VP1MainWindow window(nullptr, nullptr);
  QApplication::processEvents();
  const auto notices = window.findChildren<QMessageBox*>();
  check(notices.size() == (remote && !batch ? 1 : 0), "Incorrect startup notice visibility");
  if (!notices.isEmpty()) {
    const auto* notice = notices.front();
    check(notice->text().contains(reason), "Notice lacks detection reason");
    check(notice->text().contains(snapshots), "Notice lacks snapshot compatibility status");
    check(notice->text().contains(coin), "Notice lacks effective Coin OpenGL policy");
    check(notice->text().contains("--no-remote"), "Notice lacks override flag");
    check(notice->text().contains("OFF by default"), "Notice lacks changed default");
    check(notice->textFormat() == Qt::PlainText, "Environment must be displayed as plain text");
    check(notice->isVisible(), "Remote notice was not opened");
  }
  bool indicator = false;
  for (const auto* label : window.statusBar()->findChildren<QLabel*>())
    indicator |= label->text() == "REMOTE MODE";
  check(indicator == remote, "Incorrect persistent remote indicator");
}

void testSnapshotFailureHints()
{
  qputenv("VP1_SNAPSHOT_WORKAROUND_STATUS", "active");
  check(VP1SnapshotStatus::failureHint().contains("already active"), "Active shim needs diagnostic guidance");
  check(!VP1SnapshotStatus::failureHint().contains("--fix-snapshots"), "Do not suggest enabling an active shim");
  qputenv("VP1_SNAPSHOT_WORKAROUND_STATUS", "unavailable");
  qputenv("VP1_SNAPSHOT_WORKAROUND_MESSAGE", "Unavailable: missing compiler.");
  check(VP1SnapshotStatus::failureHint().contains("missing compiler"), "Unavailable shim needs failure reason");
  qputenv("VP1_SNAPSHOT_WORKAROUND_STATUS", "disabled");
  check(VP1SnapshotStatus::failureHint().contains("--no-fix-snapshots"), "Explain snapshot opt-out");
  qputenv("VP1_SNAPSHOT_WORKAROUND_STATUS", "not-requested");
  check(VP1SnapshotStatus::failureHint().contains("--fix-snapshots"), "Normal-mode failures need opt-in hint");
}
}

int main(int argc, char** argv)
{
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication application(argc, argv);
  // Isolate the rotation preferences from the user's settings.
  QTemporaryDir settingsDirectory;
  if (!settingsDirectory.isValid()) {
    std::cerr << "Could not create isolated settings directory\n";
    return 1;
  }
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsDirectory.path());
  QCoreApplication::setOrganizationName("VP1Tests");
  QCoreApplication::setApplicationName("RemoteMode");
  try {
    testRotationSettings();
    testSnapshotFailureHints();
    testNotice(false, false);
    testNotice(true, false);
    testNotice(true, true);
    testNotice(true, false, "requested with --remote");
    testNotice(true, false, "SSH session", "Unavailable: missing compiler.");
    testNotice(true, false, "SSH session", "Disabled by --no-fix-snapshots.");
    testNotice(true, false, "SSH session", "Active.", "Conservative Coin policy: COIN_FULL_INDIRECT_RENDERING=0.");
    testNotice(true, false, "SSH session", "Active.", "OpenGL 1.0 only: COIN_FORCE_GL1_0_ONLY=1 overrides full features.");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  std::cout << "Rotation preferences and startup notice checks passed\n";
  return 0;
}
