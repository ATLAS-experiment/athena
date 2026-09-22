#!/usr/bin/env python3
"""
This is a dummy to replace the real webis_server or webproxy from
the TDAQ release. We assume the latter will no longer be available
going forward.

It implements only the REST API as needed for this package's tests.

POST   /info/current/test/event
GET    /info/current/test/event/SUBSCRIPTION
DELETE /info/current/test/event/SUBSCRIPTION
GET    /info/current/test/is/RunParams/RunParams.RunParams
"""

import argparse
import eformat, eformat.dummy, eformat.stream
import json
from http.server import ThreadingHTTPServer, BaseHTTPRequestHandler
import struct
import uuid

subscriptions = {}

file_name = None
data_file = None

runparams = [
    "RunParams.RunParams",
    "RunParams",
    "25/8/26 14:03:26.734782",
    {
        "run_number": 524965,
        "max_events": 0,
        "recording_enabled": 0,
        "trigger_type": 0,
        "run_type": "Physics",
        "det_mask": "00000000000000000000000000000000",
        "beam_type": 0,
        "beam_energy": 0,
        "filename_tag": "",
        "T0_project_tag": "",
        "timeSOR": "25/8/26 14:03:26",
        "timeEOR": "1/1/70 01:00:00",
        "totalTime": 0
    }
]

lvl1_id = 1
global_id = 1

def next_event() -> [ bytes ]:
    global lvl1_id, global_id, data_file
    if not file_name:
        event = eformat.dummy.make_fe(lvl1_id = lvl1_id)
        event.global_id(global_id)
        lvl1_id += 1
        global_id += 1
        return event
    else:
        try:
            if data_file is None:
                data_file = eformat.streams.istream(data_file)
            return next(data_file)
        except Exception as ex:
            data_file = None
            return next_event()

class WebISHandler(BaseHTTPRequestHandler):

    # we want HTTP/1.1 for pipelining and keeping connections open
    version = "1.1"

    def do_GET(self):
        if self.path == '/info/current/test/is/RunParams/RunParams.RunParams':
            self.send_response(200)
            self.send_header('Content-type', 'application/json')
            self.end_headers()
            self.wfile.write(json.dumps(runparams).encode('utf-8'))
        elif self.path.startswith('/info/current/test/event'):
            if not self.path.split('/')[-1] in subscriptions:
                self.send_response(404)
                self.end_headers()
            else:
                ev = next_event().readonly()
                self.send_response(200)
                self.send_header('Content-type', 'application/octetstream')

                # send data

                # list of 32 byte words
                raw_words = ev.payload()

                # Pack the 32-bit unsigned integers into a standard byte string
                raw_bytes = struct.pack(f'{len(raw_words)}I', *raw_words)

                self.send_header('Content-length', len(raw_bytes))
                self.end_headers()

                self.wfile.write(raw_bytes)
        else:
            self.send_response(404)
            self.end_headers()

    def do_POST(self):
        if self.path != '/info/current/test/event':
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
        if self.path.startswith('/info/current/test/event/'):
            sub = self.path.split('/')[-1]
            if sub in subscriptions:
                del subscriptions[sub]
                self.send_response(200)
                self.end_headers()

        self.send_response(404)
        self.end_headers()

if __name__ == '__main__':
    parser = argparse.ArgumentParser();
    parser.add_argument('-f', '--file', help="Raw event data input file")
    parser.add_argument('-p', '--port', help="Port number", default=8080, type=int)

    args = parser.parse_args()

    if not args.file:
        print("Emulating event data")
    else:
        data_file = eformat.stream.istream(args.file)

    print(args.port)
    server = ThreadingHTTPServer(('', args.port), WebISHandler)
    server.serve_forever()
