#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import test_bytestream_utils

if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = 10
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2
    flags.Concurrency.NumOffloadThreads = 8
    flags.PerfMon.doFullMonMT = True

    parser = flags.getArgumentParser()
    parser.add_argument(
        "--target",
        required=True,
        help="host:port, or a file the server writes its port into",
    )
    parser.add_argument(
        "--client-cfg",
        default="ByteStreamClientCfg",
        help="Name of the client configuration function in test_bytestream_utils",
    )
    parser.add_argument(
        "--ready-timeout",
        type=float,
        default=120.0,
        help="Seconds to wait for the server to answer",
    )
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    cfg = getattr(test_bytestream_utils, args.client_cfg)(
        flags, args.target, ReadyTimeout=args.ready_timeout
    )
    cfg.printConfig(withDetails=True, summariseProps=False)
    sys.exit(0 if cfg.run().isSuccess() else 1)
