# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
"""How a boundary crosses, as something you can configure.

A codec descriptor names an encoding, knows which component implements it, and
says what that component needs to be told. It is the single place a boundary's
wire contract is stated, and it feeds two consumers that must never disagree:

- the **codec tool** on the gate, the pack algorithm, or the client's request
  algorithm -- the thing that actually converts;
- the **loop manager's menu string**, which is what ListSequences advertises and
  what a request is validated against.

What it deliberately does not feed is the scheduler. A codec declares whatever
it produces itself -- with ordinary ``SG::WriteHandleKey`` members where it has
a C++ type to name, and by resolving a CLID at initialize() where it has none --
so no dependency is assembled here, and no type is restated.

Note that a descriptor is *not* a registry lookup. Naming a codec in a fragment
names a component directly; nothing resolves an encoding string to an
implementation, anywhere, ever. There is no string-to-class mapping in this
module and no table to add to -- both ends of a boundary import the same
fragment declaration, so neither has to look anything up.

**Schemas are optional, per codec class.** A schema is worth stating exactly
when one codec component can carry more than one thing. A protobuf codec needs a
message name, because that is the one name no C++ type supplies. A codec that
streams whatever type it is pointed at takes the type name the configuration
already derived. A codec that carries exactly one payload type is named
completely by its encoding tag, and states nothing.
"""

__all__ = ["Codec"]


class Codec:
    """One boundary's wire contract, and the component that implements it.

    Subclasses set :attr:`encoding` and :attr:`tool_type`; both are properties
    of the class rather than of an instance, because a codec answers to exactly
    one encoding tag. That is what makes the tag a contract instead of a
    setting, and it is asserted in C++ too -- ``IPayloadCodec::encoding()`` is
    not configurable.
    """

    #: the tag this codec answers to, matching the C++ class's encoding()
    encoding = None
    #: the configurable that implements it
    tool_type = None
    #: where this codec's schema string comes from. ``"explicit"`` means the
    #: fragment states it; ``"derived"`` means it is the StoreGate type the
    #: configuration worked out from the payload's own handles; ``"none"``
    #: means this codec carries one payload type and needs no schema at all.
    schema_source = "explicit"

    @classmethod
    def available(cls):
        """Did this project build the component that implements it?"""
        return cls.encoding is not None and cls.tool_type is not None

    def __init__(self, schema=None, **properties):
        if self.encoding is None:
            raise TypeError(f"{type(self).__name__} does not name an encoding")
        if self.tool_type is None:
            raise TypeError(
                f"nothing in this project implements the '{self.encoding}' "
                f"encoding. {type(self).__name__} is guarded on an external "
                "the release may not have; see the package's CMakeLists")
        if self.schema_source == "explicit" and not schema:
            raise ValueError(
                f"{type(self).__name__} needs a schema: one component of this "
                "class can carry more than one thing, so the encoding tag does "
                "not name the payload by itself")
        if self.schema_source != "explicit" and schema:
            raise ValueError(
                f"{type(self).__name__} takes no schema; it is "
                + ("derived from the payload's own handles"
                   if self.schema_source == "derived"
                   else "named completely by its encoding tag"))
        self.schema = schema
        self.properties = properties

    def resolvedSchema(self, sg_type=""):
        """The schema string this boundary actually declares."""
        if self.schema_source == "explicit":
            return self.schema
        if self.schema_source == "derived":
            if not sg_type:
                raise ValueError(
                    f"{type(self).__name__} takes its schema from the "
                    "StoreGate type, which could not be derived for this key")
            return sg_type
        return ""

    def sgKeys(self, key):
        """The StoreGate keys this boundary maps to.

        For almost every codec the boundary key *is* the StoreGate key, because
        the payload is the object. A codec that converts between a wire schema
        and the fragment's own types is the exception, and says so by overriding
        this: its boundary is a name on the wire and the objects live under keys
        of the fragment's choosing.
        """
        return [key]

    def directionProperties(self, direction):
        """Properties that depend on which way this instance carries.

        Empty for a codec whose boundary is one object at one key, which needs
        nothing beyond ``Direction`` itself.
        """
        return {}

    def spec(self, key, sg_type=""):
        """The loop manager's description of this boundary.

        Three fields, all of them the wire contract; see RemoteExecBoundary.h for why
        only a service parses this.
        """
        return f"{key}#{self.encoding}#{self.resolvedSchema(sg_type)}"

    def tool(self, name, key, direction, sg_type=""):
        """The configured component that converts this boundary.

        :param direction: ``"Write"`` if this instance decodes into the store,
            ``"Read"`` if it encodes out of it. Not a property of the class: the
            same codec runs on both sides of a boundary and in both roles.
        """
        if direction not in ("Read", "Write"):
            raise ValueError(f"direction must be 'Read' or 'Write', not "
                             f"{direction!r}")
        return self.tool_type(name, Key=key,
                              Schema=self.resolvedSchema(sg_type),
                              Direction=direction,
                              **self.directionProperties(direction),
                              **self.properties)

    def __repr__(self):
        return f"{type(self).__name__}({self.schema!r})"
