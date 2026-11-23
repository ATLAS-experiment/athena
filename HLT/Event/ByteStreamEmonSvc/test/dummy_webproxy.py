#!/usr/bin/env python3
"""
This is a dummy to replace the real webis_server or webproxy from
the TDAQ release. We assume TDAQ will no longer be available
going forward, only tdaq-common.

It implements only the REST API as needed for this package's tests.

POST   /info/current/PARTITION/event
GET    /info/current/PARTITION/event/SUBSCRIPTION
DELETE /info/current/PARTITION/event/SUBSCRIPTION
GET    /info/current/PARTITION/is/RunParams/RunParams.RunParams

where PARTITION is given with --partition (default: test).

Events are read from the raw data file given with -f, restarting at
the beginning when the end of the file is reached. RunParams are
derived from the file metadata and the first event in the file.
"""

import argparse
import datetime
import eformat
import libpyevent_storage
import json
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
import struct
import uuid

subscriptions = {}

file_name = None
# URL prefix for the served partition, set from --partition
base = '/info/current/test'
data_file = None

# IS RunParams.run_type strings for eformat run types
RUN_TYPES = {
    "PHYSICS": "Physics",
    "CALIBRATION": "Calibration",
    "COSMIC": "Cosmics",
    "TEST": "Test",
    "SIMULATION": "Simulation",
}

# Filled from the input file by make_runparams()
runparams = None


def is_time(seconds, nanoseconds=None):
    """Format a time stamp like IS does, e.g. '25/8/26 14:03:26[.734782]'"""
    t = datetime.datetime.fromtimestamp(seconds)
    result = f"{t.day}/{t.month}/{t:%y} {t:%H:%M:%S}"
    if nanoseconds is not None:
        result += f".{nanoseconds // 1000:06d}"
    return result


def make_runparams(file_name):
    """Derive RunParams from the file metadata and the first event.

    The run number is assumed to be the same for all events in the file.
    The detector mask, beam type/energy, recording flag and trigger type
    are only available in the file metadata, not in the event header.
    The start of run is approximated by the time stamp of the first event.
    The T0 project tag is taken from the file metadata, e.g. 'data22_13p6TeV'.
    """
    meta = libpyevent_storage.pickDataReader(file_name)
    first = next(iter(eformat.istream(file_name)))

    run_number = first.run_no()
    if meta.runNumber() != run_number:
        print(f"Warning: run number in file metadata ({meta.runNumber()}) "
              f"differs from first event ({run_number}), using the latter")

    sor_seconds = first.bc_time_seconds()
    sor_nanoseconds = first.bc_time_nanoseconds()

    return [
        "RunParams.RunParams",
        "RunParams",
        is_time(sor_seconds, sor_nanoseconds),
        {
            "run_number": run_number,
            "max_events": 0,
            "recording_enabled": int(meta.recEnable()),
            "trigger_type": int(meta.triggerType()),
            "run_type": RUN_TYPES.get(str(first.run_type()), "Physics"),
            # 128 bit mask as 32 hex characters, most significant word first,
            # as expected by eformat::helper::DetectorMask(const std::string&)
            "det_mask": f"{int(meta.detectorMask()):032x}",
            "beam_type": int(meta.beamType()),
            "beam_energy": int(meta.beamEnergy()),
            "filename_tag": "",
            "T0_project_tag": meta.projectTag(),
            "timeSOR": is_time(sor_seconds),
            "timeEOR": "1/1/70 01:00:00",
            "totalTime": 0
        }
    ]


def next_event():
    """Return the next event from the input file as read-only FullEventFragment,
    restarting from the beginning when the file is exhausted"""
    global data_file
    try:
        if data_file is None:
            data_file = iter(eformat.istream(file_name))
        return next(data_file)
    except StopIteration:
        data_file = iter(eformat.istream(file_name))
        return next(data_file)

class WebISHandler(BaseHTTPRequestHandler):

    # we want HTTP/1.1 for pipelining and keeping connections open
    version = "1.1"

    def do_GET(self):
        path = self.path.split('?')[0]
        if path == f'{base}/is/RunParams/RunParams.RunParams':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(runparams).encode('utf-8'))
        elif path.startswith(f'{base}/event/'):
            if not path.split('/')[-1] in subscriptions:
                self.send_response(404)
                self.end_headers()
            else:
                ev = next_event()
                self.send_response(200)
                self.send_header('Content-type', 'application/octetstream')

                # send data

                # list of 32 bit words of the complete event fragment
                raw_words = ev.__raw__()

                # Pack the 32-bit unsigned integers into a standard byte string
                raw_bytes = struct.pack(f'{len(raw_words)}I', *raw_words)

                self.send_header('Content-length', len(raw_bytes))
                self.end_headers()

                self.wfile.write(raw_bytes)
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path.split('?')[0].rstrip('/') != f'{base}/event':
            self.send_response(400);
            self.end_headers()
        else:
            sub = str(uuid.uuid4())
            content_length = int(self.headers.get("Content-Length", 0))
            subscriptions[sub] = json.loads(self.rfile.read(content_length).decode('utf-8'))
            self.send_response(201)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps({ "id": sub}).encode('utf-8'))

    def do_DELETE(self):
        path = self.path.split('?')[0]
        if path.startswith(f'{base}/event/'):
            sub = path.split('/')[-1]
            if sub in subscriptions:
                del subscriptions[sub]
                self.send_response(200)
                self.end_headers()
                return

        self.send_response(404)
        self.end_headers()

if __name__ == '__main__':
    parser = argparse.ArgumentParser();
    parser.add_argument('-f', '--file', required=True, help="Raw event data input file")
    parser.add_argument('-p', '--port', help="Port number", default=8080, type=int)
    parser.add_argument('--partition', default='test', help="Partition name (default: test)")

    args = parser.parse_args()

    file_name = args.file
    base = f'/info/current/{args.partition}'
    runparams = make_runparams(file_name)

    print(json.dumps(runparams, indent=2))
    print(args.port)
    server = ThreadingHTTPServer(('', args.port), WebISHandler)
    server.serve_forever()
