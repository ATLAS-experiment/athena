# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Example of configuring client"""

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthExRemoteExec.RemoteExecDemoHiveLoopConfig import RemoteExecTopCfg
from AthRemoteExecCore.RemoteExecFragments import RequestAlg
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg


def RemoteExecDemoClientCfg(flags, target, addends=(3, 4), **kwargs):
    """Configure entire client job"""

    cfg = MainServicesCfg(flags)
    cfg.merge(McEventSelectorCfg(flags))

    # Algorithm that provides numbers to be sent to server
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.Example.DemoNumbersAlg(
            "RemoteExecDemoNumbers", Values=["a", "b"], Offsets=list(addends)
        )
    )

    # Take remote server configuration, decleare that it's viewd by client and ask for a remote sequence
    fragment = RemoteExecTopCfg(flags).onClient().fragment("RemoteExecSeqSum")
    cfg.addEventAlgo(
        RequestAlg("RemoteExecDemoRequest", fragment, target=target, **kwargs)
    )

    # Algorithm thet checks if sent numbers were added correctly on the remote side
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.Example.DemoCheckAlg(
            "RemoteExecDemoCheck", Values=["a", "b"], Result="sum"
        )
    )

    return cfg


if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Input.Files = []
    flags.Exec.MaxEvents = 10
    flags.Concurrency.NumThreads = 2
    flags.Concurrency.NumConcurrentEvents = 2
    flags.Concurrency.NumOffloadThreads = 8

    parser = flags.getArgumentParser()
    parser.add_argument(
        "--target",
        required=True,
        help="host:port, or a file the server writes its port into",
    )
    parser.add_argument(
        "--ready-timeout",
        type=float,
        default=120.0,
        help="Seconds to wait for the server to answer",
    )
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    cfg = RemoteExecDemoClientCfg(flags, args.target, ReadyTimeout=args.ready_timeout)
    cfg.printConfig(withDetails=True, summariseProps=False)
    sys.exit(0 if cfg.run().isSuccess() else 1)
