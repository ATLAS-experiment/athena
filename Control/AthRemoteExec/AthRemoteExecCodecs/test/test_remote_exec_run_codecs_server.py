#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from test_bytestream_utils import CodecsLoopCfg

if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = -1  # serve until stopped
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2
    flags.PerfMon.doFullMonMT = True

    parser = flags.getArgumentParser()
    parser.add_argument(
        "--port", type=int, default=0, help="TCP port to bind; 0 lets the OS choose"
    )
    parser.add_argument(
        "--port-file", default="", help="File to write the bound port to"
    )
    parser.add_argument(
        "--queue-limit",
        type=int,
        default=128,
        help="Queued requests before clients are refused with RESOURCE_EXHAUSTED",
    )
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    cfg = CodecsLoopCfg(
        flags, Port=args.port, PortFile=args.port_file, QueueLimit=args.queue_limit
    )
    cfg.printConfig(withDetails=True, summariseProps=False)
    sys.exit(0 if cfg.run().isSuccess() else 1)
