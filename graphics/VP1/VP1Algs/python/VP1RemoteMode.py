# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Launch-time remote-session detection, independent of Athena and OpenGL."""
import os
import re
import socket


def remoteSessionReasons(environment, hostname):
    """Return human-readable evidence; no display connection or GL queries."""
    reasons = []
    if environment.get("SSH_CONNECTION") or environment.get("SSH_CLIENT"):
        reasons.append("SSH session")
    if any(environment.get(key) for key in
           ("VNCDESKTOP", "VNCSESSION", "XRDP_SESSION", "XRDP_SOCKET_PATH",
            "NXSESSIONID", "X2GO_SESSION")):
        reasons.append("remote desktop session")
    display = environment.get("DISPLAY", "")
    match = re.fullmatch(r"(?:tcp/)?(.*):(\d+)(?:\.\d+)?", display)
    if match and not display.startswith("/"):
        host, number = match.groups()
        host = host.lower()
        local_names = {"", "unix", "localhost", "127.0.0.1", "[::1]",
                       hostname.lower(), hostname.lower().split(".")[0]}
        if host not in local_names and not host.endswith("/unix"):
            reasons.append("X11 display on another host")
        elif host in {"localhost", "127.0.0.1", "[::1]"} and int(number) >= 10:
            reasons.append("forwarded X11 display")
    return reasons


def addRemoteArguments(group):
    """Share exact options between the early launcher and Athena's parser."""
    remote = group.add_mutually_exclusive_group()
    remote.add_argument("--remote", action="store_true",
                        help="Force remote mode, bypassing detection, and automatically enable --fix-snapshots "
                             "on supported systems unless --no-fix-snapshots is given.")
    remote.add_argument("--no-remote", action="store_true",
                        help="Use normal viewer defaults and disable automatic snapshot-workaround activation; "
                             "an explicit --fix-snapshots still enables the workaround.")
    snapshots = group.add_mutually_exclusive_group()
    snapshots.add_argument("--fix-snapshots", action="store_true",
                           help="Explicitly enable the Linux GLX snapshot workaround (requires a C compiler).")
    snapshots.add_argument("--no-fix-snapshots", action="store_true",
                           help="Disable automatic snapshot-workaround activation while retaining remote mode.")


def _positiveCoinSetting(value):
    # Coin uses atoi(), so leading whitespace/signs and a numeric prefix count.
    match = re.match(r"\s*([+-]?\d+)", value)
    return bool(match and int(match.group(1)) > 0)


def _configureCoinPolicy(remote):
    full = "COIN_FULL_INDIRECT_RENDERING"
    force = os.environ.get("COIN_FORCE_GL1_0_ONLY", "")
    forced_gl1 = _positiveCoinSetting(force)
    if remote and not forced_gl1:
        os.environ.setdefault(full, "1")
    if forced_gl1:
        message = (f"OpenGL 1.0 only: COIN_FORCE_GL1_0_ONLY={force!r} "
                   "overrides COIN_FULL_INDIRECT_RENDERING.")
    elif full not in os.environ:
        message = "Coin default policy for indirect contexts (COIN_FULL_INDIRECT_RENDERING unset)."
    elif _positiveCoinSetting(os.environ[full]):
        message = (f"OpenGL 1.1+ features allowed for indirect contexts ({full}={os.environ[full]!r}); "
                   "driver capabilities still apply.")
    else:
        message = f"Conservative Coin policy for indirect contexts ({full}={os.environ[full]!r}; value preserved)."
    os.environ["VP1_COIN_INDIRECT_RENDERING_MESSAGE"] = message


def remoteBootstrapKey(no_remote=False, force_remote=False):
    """Scope the early decision to this exec-preserved PID and explicit overrides."""
    return f"{os.getpid()}:{int(no_remote)}:{int(force_remote)}"


def configureRemoteMode(no_remote=False, force_remote=False, announce=True, reuse_bootstrap=False):
    """Set the process-wide mode before constructing any VP1 viewers."""
    if no_remote and force_remote:
        raise ValueError("--remote and --no-remote cannot be used together")
    if reuse_bootstrap and os.environ.get("VP1_REMOTE_BOOTSTRAP") == remoteBootstrapKey(no_remote, force_remote):
        reasons = os.environ.get("VP1_REMOTE_REASON", "").split(", ") if os.environ.get("VP1_REMOTE_MODE") == "1" else []
    elif force_remote:
        reasons = ["requested with --remote"]
    elif no_remote:
        reasons = []
    else:
        reasons = remoteSessionReasons(os.environ, socket.gethostname())
    os.environ["VP1_REMOTE_MODE"] = "1" if reasons else "0"
    os.environ["VP1_REMOTE_REASON"] = ", ".join(reasons)
    _configureCoinPolicy(bool(reasons))
    if reasons and announce:
        snapshot_status = os.environ.get("VP1_SNAPSHOT_WORKAROUND_MESSAGE",
                                         "Not initialized by the vp1 launcher.")
        print("\n" + "=" * 72 + "\n"
              "VP1 REMOTE MODE: " + ", ".join(reasons) + ".\n"
              "Continuous rotation after mouse release is OFF by default\n"
              "(your saved remote preference takes precedence).\n"
              "Change it in the 3D viewer menu: Continue rotating after mouse release.\n"
              "Snapshot compatibility: " + snapshot_status + "\n"
              "Coin OpenGL policy: " + os.environ["VP1_COIN_INDIRECT_RENDERING_MESSAGE"] + "\n"
              "To use normal mode, restart VP1 with --no-remote.\n"
              + "=" * 72 + "\n", flush=True)
    return bool(reasons)
