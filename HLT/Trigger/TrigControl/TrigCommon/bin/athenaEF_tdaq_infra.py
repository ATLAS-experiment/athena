#!/usr/bin/env python3
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Start a private TDAQ online infrastructure for testing the athenaEF OH
# publication path (athenaEF -M) in an offline job.
#
# athenaEF decides the online environment: partition name, webis host/port and OH server name. 
# TDAQ_WEBDAQ_BASE has to be available.
# This script only reports readiness.
#
# The script first sources the TDAQ release via cm_setup.sh and re-execs itself.
# exec preserves the PID, so the PR_SET_PDEATHSIG that athenaEF set on this process stays valid across the environment setup.
#
# Started applications:
#   ipc_server            initial partition
#   ipc_server -p <part>  job partition
#   is_server             DF, RunParams and the OH server
#   rdb_server            ISRepository, only if --schema is given
#   webproxy              Used by WebdaqHistSvc on webdaq-port
#
# IS publication (WebdaqInfoSvc) needs rdb_server serving the ISRepository (needed by webproxy to resolve the object type).
# OH publication does not need it, so no --schema means no rdb_server.
#
# NB: webproxy MUST be used, NOT webis_server: webproxy deserialises OH POSTs as
# binary TBufferFile (matching webdaq::oh::put), while webis_server expects
# JSON - it answers 201 anyway and then silently drops every publication.
#
# Once all servers are up the script prints ATHENAEF_INFRA_READY on stdout, then
# stays alive polling them and exits non-zero if any of them dies. 
# Server log files are written to --log-dir (default: cwd).
#
# All child processes are spawned with PR_SET_PDEATHSIG=SIGKILL so they die with this script. 
# On SIGTERM/SIGINT the histograms are dumped with oh_cp and the partitions are removed cleanly with ipc_rm.
#

import argparse
import os
import signal
import subprocess
import sys
import time
import socket

import logging
logging.basicConfig(stream=sys.stdout, level=logging.INFO,
                    format='%(asctime)s %(name)s %(levelname)-8s %(message)s')
log = logging.getLogger('athenaEF_infra')

# TDAQ release setup (hardcoded for now, see --tdaq-release)
CM_SETUP = '/cvmfs/atlas.cern.ch/repo/sw/tdaq/tools/cmake_tdaq/bin/cm_setup.sh'
DEFAULT_TDAQ_RELEASE = 'tdaq-14-00-00'

# Environment marker distinguishing stage 2 from stage 1
STAGE2_ENV = 'ATHENAEF_INFRA_STAGE2'

# Printed on stdout once the infrastructure is up
READY_MARKER = 'ATHENAEF_INFRA_READY'


def parse_args():
    parser = argparse.ArgumentParser(
        description='Start a private TDAQ infrastructure (IPC/IS/webproxy) for athenaEF online-monitoring tests')
    parser.add_argument('--partition', metavar='NAME', required=True,
                        help='partition name (TDAQ_PARTITION)')
    parser.add_argument('--webdaq-port', metavar='PORT', type=int, required=True,
                        help='port the webproxy listens on')
    parser.add_argument('--oh-server', metavar='NAME', required=True,
                        help='name of the OH IS server (TDAQ_OH_SERVER)')
    parser.add_argument('--run-number', metavar='N', type=int, required=True,
                        help='run number, used to name the output file')
    parser.add_argument('--schema', metavar='FILE', action='append', default=[],
                        help='IS schema file (absolute path) to load into the ISRepository, '
                             'repeatable. If none is given rdb_server is not started')
    parser.add_argument('--log-dir', metavar='DIR', default='.',
                        help='directory for infrastructure log files (default: cwd)')
    parser.add_argument('--tdaq-release', metavar='REL', default=DEFAULT_TDAQ_RELEASE,
                        help='TDAQ release to source via cm_setup.sh (default: %(default)s)')
    return parser.parse_args()


def setup_tdaq_and_reexec(args):
    """re-exec this script through bash after sourcing the TDAQ release."""
    if not os.path.exists(CM_SETUP):
        log.error('TDAQ release setup not found at %s', CM_SETUP)
        sys.exit(1)

    log.info('Sourcing TDAQ release %s via %s', args.tdaq_release, CM_SETUP)
    os.environ[STAGE2_ENV] = '1'
    script = os.path.abspath(__file__)
    cmd = (f'source {CM_SETUP} {args.tdaq_release} || exit 66; '
           f'exec python3 "{script}" "$@"')
    os.execv('/bin/bash', ['/bin/bash', '-c', cmd, script] + sys.argv[1:])


class Infrastructure:
    """Manage the private TDAQ infrastructure (adapted from HLTMPPy.runner.Infrastructure)"""

    sigs = [signal.SIGFPE, signal.SIGHUP, signal.SIGQUIT, signal.SIGSEGV,
            signal.SIGTERM, signal.SIGINT]

    def __init__(self, args):
        self.args = args
        self.processes = []      # (name, subprocess.Popen)
        self.pid = os.getpid()   # Distinguish mother from children after forking
        self.ready = False       # True once all servers are up
        self.register_handlers()

    def __del__(self):
        """Stop infrastructure in the mother process, in case program exits before stop()"""
        if os.getpid() == self.pid:
            self.stop()

    def register_handlers(self):
        self.prehandlers = {}
        for s in self.sigs:
            self.prehandlers[s] = signal.getsignal(s)
            signal.signal(s, self._handle_quit)

    def _handle_quit(self, signum, frame):
        log.info('Caught signal %d. Cleaning up the infrastructure and exiting', signum)
        self.stop()
        prehandler = self.prehandlers.pop(signum, signal.SIG_DFL)
        signal.signal(signum, prehandler)
        sys.exit(0)

    def _implant_bomb(self):
        """preexec_fn ensuring infrastructure processes exit when this process dies"""
        from ctypes import cdll
        PR_SET_PDEATHSIG = 1
        try:
            return lambda: cdll['libc.so.6'].prctl(PR_SET_PDEATHSIG, signal.SIGKILL)
        except Exception:
            log.error('Error setting PR_SET_PDEATHSIG for infrastructure processes. '
                      'Using a dummy function instead')
            return lambda: 1

    def _launch(self, name, cmd):
        logbase = os.path.join(self.args.log_dir, f'{name}_{self.args.partition}')
        proc = subprocess.Popen(cmd, preexec_fn=self._implant_bomb(),
                                stdout=open(logbase + '.out', 'w'),
                                stderr=open(logbase + '.err', 'w'),
                                close_fds=True)
        log.info('Started %s (pid %d): %s', name, proc.pid, ' '.join(cmd))
        self.processes.append((name, proc))
        return proc

    def start(self):
        from ispy import IPCPartition

        partition = self.args.partition

        # Private IPC domain: reference file local to this job's directory
        ipc_ref = 'file:' + os.path.join(os.getcwd(), 'ipc_init.ref')
        log.info('Setting TDAQ_IPC_INIT_REF: %s', ipc_ref)
        os.environ['TDAQ_IPC_INIT_REF'] = ipc_ref

        log.info('Initializing OH monitoring infrastructure for partition %s', partition)

        self._launch('ipc_initial', ['ipc_server'])
        while not IPCPartition('initial').isValid():
            log.info('Waiting until initial partition is available...')
            time.sleep(1)

        self._launch('ipc_partition', ['ipc_server', '-p', partition])
        while not IPCPartition(partition).isValid():
            log.info('Waiting until partition %s is available...', partition)
            time.sleep(1)

        for server in ['DF', 'RunParams', self.args.oh_server]:
            self._launch(f'is_{server}', ['is_server', '-p', partition, '-n', server])

        self.start_rdb()
        self.start_webproxy()
        self.ready = True

    def start_rdb(self):
        """Start rdb_server serving the IS type schema"""
        if not self.args.schema:
            log.info('No --schema given, not starting rdb_server '
                     '(IS publication will be rejected with HTTP 400)')
            return

        missing = [f for f in self.args.schema if not os.path.exists(f)]
        if missing:
            log.error('IS schema file(s) not found: %s', ', '.join(missing))
            self.stop()
            sys.exit(1)

        log.info('Starting rdb_server with IS schema: %s', ', '.join(self.args.schema))
        self._launch('rdb', ['rdb_server', '-p', self.args.partition,
                             '-d', 'ISRepository', '-s',
                             '-D'] + self.args.schema)

    def start_webproxy(self):
        """Start the webproxy REST server""" 
        port = self.args.webdaq_port
        webproxy = self._launch('webproxy', ['webproxy', '-p', str(port)])

        timeout = 60
        for _ in range(timeout):
            if webproxy.poll() is not None:
                log.error('webproxy exited early (code %s); see webproxy_%s.err',
                          webproxy.returncode, self.args.partition)
                self.stop()
                sys.exit(1)
            try:
                with socket.create_connection(('localhost', port), timeout=1):
                    log.info('webproxy listening on http://localhost:%d', port)
                    return
            except OSError:
                time.sleep(1)

        log.error('webproxy is not listening on localhost:%d after %d s', port, timeout)
        self.stop()
        sys.exit(1)

    def check_alive(self):
        """Return False if any infrastructure process has exited"""
        for name, proc in self.processes:
            ret = proc.poll()
            if ret is not None:
                log.error('Infrastructure process %s (pid %d) exited with code %s',
                          name, proc.pid, ret)
                return False
        return True

    def copy_histograms(self):
        fname = f'r{self.args.run_number:010d}_{self.args.partition}_{self.args.oh_server}.root'
        log.info('Copying histograms into %s (oh_cp)', fname)
        subprocess.call(['oh_cp', '-p', self.args.partition, '-s', self.args.oh_server,
                         '-n', '.*', '-o', '.*', '-O',
                         '-r', str(self.args.run_number), '-f', fname])

    def dump_is_content(self):
        fname = f'r{self.args.run_number:010d}_{self.args.partition}_DF.txt'
        log.info('Writing content of DF IS server to %s', fname)
        with open(fname, "w") as f:
            subprocess.call(['is_ls', '-p', self.args.partition, '-n', 'DF',
                             '-R', '.*', '-TNv'],
                            stdout=f, stderr=subprocess.STDOUT, text=True)

    def stop(self):
        if not self.processes:
            return

        if self.ready:   # Nothing was ever published if we did not fully start
            self.copy_histograms()
            self.dump_is_content()

        log.info('Finalizing OH monitoring infrastructure')
        for part in [self.args.partition, 'initial']:
            log.info('Destroying partition: %s', part)
            subprocess.call(['ipc_rm', '-f', '-p', part, '-i', '".*"', '-n', '".*"'],
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        for name, proc in self.processes:
            while proc.poll() is None:
                proc.kill()
                time.sleep(0.1)
        self.processes = []
        log.info('Terminated all infrastructure processes')


def main():
    args = parse_args()

    if STAGE2_ENV not in os.environ:
        setup_tdaq_and_reexec(args)   # does not return

    os.makedirs(args.log_dir, exist_ok=True)

    infra = Infrastructure(args)
    infra.start()
    print(READY_MARKER, flush=True)

    while infra.check_alive():
        time.sleep(2)

    infra.stop()
    return 1


if __name__ == '__main__':
    sys.exit(main())
