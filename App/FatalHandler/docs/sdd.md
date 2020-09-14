App::FatalHandler Component
===

## 1. Introduction

`App::FatalHandler` is a passive F' component for handling Fatal events seen by the ActiveLogger component.  It saves the fatal events to OCM and then downlinks those 
Fatal events on the next power up. Upon the receipt of a Fatal event, the system is rebooted after a set number of ticks.

## 2. Requirements

Requirement | Description | Verification Method |
----------- | ----------- | -------------------
FPrime-FatalHandler-00 | This component shall store up to 10 FATAL events in a buffer on the on-chip GR712 memory | unit-test |
FPrime-FatalHandler-01 | This component shall provide a command interface to clear the FATAL event buffer | unit-test |
FPrime-FatalHandler-02 | This component shall generate WARNING event if the FATAL event buffer is 50% or more full at initialization | unit-test |
FPrime-FatalHandler-03 | This component shall delay a soft reset of FSW by [10 seconds - should be what's in the FatalHandler component by default] after storing the FATAL event to on-chip memory | fit test |
FPrime-FatalHandler-04 | This component shall send all FATAL event data in the buffer for real-time and recorded downlink streams at initialization | unit and fit test |

## 3. Design

#### 3.1 Component Diagram

The component diagram is as follows:

![`App::FatalHandler` Diagram](img/FatalHandlerBDD.jpg "App::FatalHandler")

#### 3.2 Ports

The `App::FatalHandler` component uses the following port types:

| Port Data Type                                         | Name              | Direction | Kind          | Usage                                                    |
|--------------------------------------------------------|-------------------|-----------|---------------|----------------------------------------------------------|
| [`Fw::Log`](../../../../Fw/Log/docs/sdd.md)            | fatalReceive      | input     | guarded_input | Port used to deliver a fatal event from the ActiveLogger |
| [`Fw::Log`](../../../../Fw/Log/docs/sdd.md)            | storedEvrOut      | output    | output        | Downlink the events at startup |

It also has the following role ports:

| Port Data Type                                        | Name           | Direction | Role            |
|-------------------------------------------------------|----------------|-----------|-----------------|
| [`Fw::Cmd`](../../../../Fw/Cmd/docs/sdd.md)           | cmdIn          | input     | Cmd             |
| [`Fw::CmdReg`](../../../../Fw/Cmd/docs/sdd.md)        | cmdRegOut      | output    | CmdRegistration |
| [`Fw::CmdResponse`](../../../../Fw/Cmd/docs/sdd.md)   | cmdResponseOut | output    | CmdResponse     |
| [`Fw::Log`](../../../../Fw/Log/docs/sdd.md)           | eventOut       | output    | Log             |
| [`Fw::Ping`](../../../../Svc/Ping/docs/sdd.md)        | PingRecv       | input     | Ping            |
| [`Fw::Ping`](../../../../Svc/Ping/docs/sdd.md)        | PingResponse   | output    | Ping            |
| [`Svc::Time`](../../../../Svc/SphinxTime/docs/sdd.md) | timeCaller     | output    | TimeGet         |
| [`Svc::TlmChan`](../../../../Svc/TlmChan/docs/sdd.md) | tlmOut         | output    | Telemetry       |

#### 3.3 Functional Description

The fatal handler responds to the events on the fatalReceive port and saves the log messages to the on chip memory. The format of the records saved should follow the following tables:

| Field     | Data Type |
|-----------------|-----|
| Size (bytes)    | U32 |
| ID              | U32 |
| Time (Seconds)  | U32 |
| Time (USeconds) | U32 |
| Argument bytes  | U8* |

*: The record is serialized into integers for use with the OCM. Thus some integers are only partially filled. Size reflects the true bytes, not the bytes for the number of integers used.

Ten of these records may be stored in the OCM, and after that new records are dropped. Cascading fatals, will stem from one event, thus the initial fatal event should be store, and hence any new
records should be dropped.  When the OCM has filled more that 50% of the OCM a warning event will be sent with the percentage filled. Once a single fatal is received, the system is set to restart 
after a set amount of time. Upon restart, the stored Fatals are downlinked but not removed.

There are two commands. The first emits a fatal event through normal channels and the other clears the memory.
