Event Data Model for GlobalSimulation
===

This holds source code for the event data model (EDM) of the GlobalSimulation

CommonTOBs
---

The Global system will internally exchange Trigger Object information (mainly) via Global Common Trigger Objects (CommonTOBs). These are 64bit words where;
- The initial 32b contains bitfields describing the ET, eta, phi of the candidate object
- The second 32b word contains bitfields providing additional object specific information

In GlobalSim, this data model is represented by an inherited class structure where;
- CommonTOB: represents the initial 32b, and is the parent TOB class
- OtherTOB: define the second 32b fields for inherited classes
