#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from test_utils import RemoteExecHiveLoopCfg

if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = -1  # serve until stopped
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2

    parser = flags.getArgumentParser()
    parser.add_argument(
        "--port", type=int, default=0, help="TCP port to bind; 0 lets the OS choose"
    )
    parser.add_argument(
        "--port-file", default="", help="File to write the bound port to"
    )
    parser.add_argument(
        "--delay",
        type=int,
        default=0,
        help="Milliseconds RemoteExecSeqChain sleeps, to make "
        "concurrent requests overlap",
    )
    parser.add_argument(
        "--queue-limit",
        type=int,
        default=128,
        help="Queued requests before clients are refused with RESOURCE_EXHAUSTED",
    )
    parser.add_argument(
        "--request-timeout",
        type=float,
        default=0.0,
        help="Seconds before a request is answered TIMEOUT; 0 disables",
    )
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    kwargs = {
        "Port": args.port,
        "PortFile": args.port_file,
        "QueueLimit": args.queue_limit,
        "RequestTimeout": args.request_timeout,
    }

    cfg = RemoteExecHiveLoopCfg(flags, delay=args.delay, **kwargs)
    cfg.printConfig(withDetails=True, summariseProps=False)
    sys.exit(0 if cfg.run().isSuccess() else 1)
