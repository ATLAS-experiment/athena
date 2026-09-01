#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Does a codec that would misread its payload stop the job?

A boundary is declared once, by the codec: it says what it touches and the
scheduler takes its word for it. So the thing to get wrong is not a
disagreement between two declarations but the wire contract itself.

A protobuf codec is told which message it carries, and protobuf will
happily parse one message as another and hand back something empty rather than
failing -- so a codec pointed at the wrong schema does not crash, it quietly
decodes nothing. The component therefore checks at initialize() that the schema
it was configured with is the one it implements, and refuses to start otherwise.

This asserts that: take a working server, point exactly one boundary's codec at
the other demonstration message, and require the job to fail rather than run.
"""

import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags

from AthExRpcLoop.RpcDemoCodecs import DOUBLES
from AthExRpcLoop.RpcHiveLoopConfig import RpcHiveLoopCfg


def _flags():
    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = -1
    flags.Concurrency.NumThreads = 1
    flags.Concurrency.NumConcurrentEvents = 1
    flags.lock()
    return flags


def main():
    cfg = RpcHiveLoopCfg(_flags(), TestRequests=["RpcSeqPing"])

    # One boundary, pointed at a message its codec does not implement. Nothing
    # else about the job changes -- and note the job never even runs this
    # fragment, so a check that happened at request time rather than at
    # startup would let it through.
    codec = cfg.getEventAlgo("RpcSeqSumGate").Inputs[0]
    print(f"corrupting {codec.getName()}: Schema "
          f"{codec.Schema!r} -> {DOUBLES!r}")
    codec.Schema = DOUBLES

    if cfg.run().isSuccess():
        print("FAIL: the job started with a codec configured for a message it "
              "does not implement")
        return 1
    print("test_rpc_codec_mismatch: the job refused to start, as it should")
    return 0


if __name__ == "__main__":
    sys.exit(main())
