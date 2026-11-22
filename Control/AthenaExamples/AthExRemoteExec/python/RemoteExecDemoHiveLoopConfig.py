# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Example of configuring remote server"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthExRemoteExec.RemoteExecDemoCodecs import DemoInts, DemoWords
from AthRemoteExecCore.RemoteExecFragments import (
    REMOTE_EXEC_REQUEST_KEY,
    RemoteExecTop,
)


def RemoteExecCountWordsPayloadCfg(flags):
    """Configuration for an algorithm that takes vector of words and writes how many elements are there"""

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.Example.CountWordsAlg(
            "RemoteExecSeqCountWordsAlg",  # Name of the algorithm
            Words="myVector",  # Key of the vector of uint32_t that will be read on the remote side
            Count="myCount",  # Key of the number of vector elemtns that will be written on the remote side
        )
    )

    return cfg


def RemoteExecSumPayloadCfg(flags):
    """Configuration for one algorithm that sums two numbers"""

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.Example.SumAlg(
            "RemoteExecSeqSumAlg", A="a", B="b", Sum="sum"
        )
    )

    return cfg


def RemoteExecTopCfg(flags):
    """Top level configuration. Attach all sequences that will be available on the remote server."""
    remote = RemoteExecTop()

    remote.add(  # Declare new sequence that will be available on the server
        "RemoteExecSeqCountWords",  # Name of the requence
        RemoteExecCountWordsPayloadCfg(
            flags
        ),  # ComponentAccumulator with algorithms to add
        outputs=["count"],  # Key of message with encoded element count
        boundaries={  # Declaration of keys and codecs that handle them
            "words": DemoWords(  # Codec that de/encodes vector of uint32_t
                "myVector"  # Key where the vector will be stored on the remote server
            ),
            "count": DemoInts(  # Codec that de/encodes integers
                ["myCount"],  # Key where count will be stored on the remote server
            ),
        },
    )

    remote.add(
        "RemoteExecSeqSum",
        RemoteExecSumPayloadCfg(flags),
        outputs=["total"],
        boundaries={"addends": DemoInts(["a", "b"]), "total": DemoInts(["sum"])},
    )

    return remote


def RemoteExecHiveEventLoopMgrCfg(flags, remote: RemoteExecTop, **kwargs):
    """Configure RemoteExecHiveEventLoopMgr with sequences"""

    cfg = ComponentAccumulator()
    for key, value in remote.properties.items():
        kwargs.setdefault(key, value)
    kwargs.setdefault("SchedulerSvc", "AvalancheSchedulerSvc")

    cfg.addService(
        CompFactory.AthRemoteExec.RemoteExecHiveEventLoopMgr(
            "RemoteExecHiveEventLoopMgr", **kwargs
        )
    )

    return cfg


def RemoteExecHiveLoopCfg(flags, **kwargs):
    """Configure entire remote job"""

    if flags.Concurrency.NumThreads < 1:
        raise ValueError("RemoteExecHiveLoopCfg needs Concurrency.NumThreads >= 1")

    cfg = MainServicesCfg(flags)
    cfg.setAppProperty(
        "EventLoop",
        "AthRemoteExec::RemoteExecHiveEventLoopMgr/RemoteExecHiveEventLoopMgr",
        overwrite=True,
    )

    cfg.getEventAlgo("SGInputLoader").ExtraOutputs.add(
        (
            "AthRemoteExec::RequestDescriptor",
            f"StoreGateSvc+{REMOTE_EXEC_REQUEST_KEY}",
        )
    )

    remote = RemoteExecTopCfg(flags)
    cfg.merge(remote.build(flags))
    cfg.merge(RemoteExecHiveEventLoopMgrCfg(flags, remote, **kwargs))

    return cfg


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
    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    kwargs = {
        "Port": args.port,
    }

    cfg = RemoteExecHiveLoopCfg(flags, **kwargs)
    cfg.printConfig(withDetails=True, summariseProps=False)
    sys.exit(0 if cfg.run().isSuccess() else 1)
