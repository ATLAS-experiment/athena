# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""Python side of test utilities"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthRemoteExecCore.RemoteExecCodecs import Codec, Direction
from AthRemoteExecCore.RemoteExecFragments import (
    REMOTE_EXEC_REQUEST_KEY,
    RemoteExecTop,
    RequestAlg,
)
from McEventSelector.McEventSelectorConfig import McEventSelectorCfg

INTS = "test_ints"


class test_IntsCodec(Codec):
    """Python helper for test_IntsCodec"""

    encoding = "protobuf"
    tool_type = CompFactory.AthRemoteExec.test_IntsCodec

    def __init__(self, values: list[str]):
        if not values:
            raise ValueError("test_IntsCodec carries at least one named integer")
        super().__init__(INTS)
        self.values = list(values)

    def sgKeys(self, key: str) -> list[str]:
        return list(self.values)

    def directionProperties(self, direction: Direction) -> dict[str, list[str]]:
        if direction is Direction.WriteToSG:
            return {"OutputInts": list(self.values)}
        return {"InputInts": list(self.values)}

    def __repr__(self):
        return f"test_IntsCodec({self.values!r})"


def RemoteExecSumPayloadCfg(flags) -> ComponentAccumulator:
    """Configuration for one algorithm that sums two numbers"""

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_SumAlg(
            "RemoteExecSeqSumAlg", IntA="a", IntB="b", IntSum="sum"
        )
    )

    return cfg


def RemoteExecChainPayloadCfg(flags, delay) -> ComponentAccumulator:
    """Configuration for two algorithms that sum two numbers each, with a dependency between them"""

    cfg = ComponentAccumulator()

    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_SumAlg(
            "RemoteExecSeqChainFirst", IntA="p", IntB="q", IntSum="pq"
        )
    )
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_DelayAlg(
            "RemoteExecSeqChainDelay", Milliseconds=delay
        )
    )
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_SumAlg(
            "RemoteExecSeqChainSecond", IntA="pq", IntB="r", IntSum="chain"
        )
    )

    return cfg


def RemoteExecFailPayloadCfg(flags) -> ComponentAccumulator:
    """Configuration for one algorithm that sums two numbers"""

    cfg = ComponentAccumulator()
    cfg.addEventAlgo(CompFactory.AthRemoteExec.test_FailAlg("RemoteExecSeqFailAlg"))

    return cfg


def RemoteExecTopCfg(flags, delay=0):
    """Top level configuration. Attach all sequences that will be available on the remote server."""
    remote = RemoteExecTop()

    remote.add(
        "RemoteExecSeqSum",
        RemoteExecSumPayloadCfg(flags),
        outputs=["total"],
        boundaries={
            "addends": test_IntsCodec(["a", "b"]),
            "total": test_IntsCodec(["sum"]),
        },
    )

    remote.add(
        "RemoteExecSeqChain",
        RemoteExecChainPayloadCfg(flags, delay),
        outputs=["chainTotal"],
        boundaries={
            "terms": test_IntsCodec(["p", "q", "r"]),
            "chainTotal": test_IntsCodec(["chain"]),
        },
    )

    # Always fails, to exercise the error path.
    remote.add("RemoteExecSeqFail", RemoteExecFailPayloadCfg(flags))

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


def RemoteExecHiveLoopCfg(flags, delay=0, **kwargs):
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


def ClientCfg(flags, target, addends=(3, 4), **kwargs):
    """Configure entire client job"""

    cfg = MainServicesCfg(flags)
    cfg.merge(McEventSelectorCfg(flags))

    # Algorithm that provides numbers to be sent to server
    cfg.addEventAlgo(
        CompFactory.AthRemoteExec.test_NumbersAlg(
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
        CompFactory.AthRemoteExec.test_CheckAlg(
            "RemoteExecDemoCheck", Values=["a", "b"], Result="sum"
        )
    )

    return cfg
