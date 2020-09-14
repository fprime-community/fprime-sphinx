App::FSWInfo Component
===

## 1. Introduction

`App::FSWInfo` is an active F prime component that provides the ground with
various information about the flight system, such as the flight software build
version.

## 2. Design

#### 2.1 Component Diagram

The `App::FSWInfo` component has the following component block description
diagram (BDD) diagram:

![`App::FSWInfo` Diagram](img/FSWInfoBDD.jpg "App::FSWInfo")

#### 2.2 Ports

The `App::FSWInfo` component uses the following port types:

| Port Data Type                    | Name          | Direction | Kind        | Usage                                 |
|-----------------------------------|---------------|-----------|-------------|---------------------------------------|
| `App::Port::FSWInfo_Generate` | reportFSWInfo | input     | async_input | Tells FSWInfo report various FSW info |

It also has the following role ports:

| Port Data Type                                      | Name           | Direction | Role            |
|-----------------------------------------------------|----------------|-----------|-----------------|
| [`Fw::Cmd`](../../../../Fw/Cmd/docs/sdd.md)         | cmdIn          | input     | Cmd             |
| [`Fw::CmdReg`](../../../../Fw/Cmd/docs/sdd.md)      | cmdReg         | output    | CmdRegistration |
| [`Fw::CmdResponse`](..././../../Fw/Cmd/docs/sdd.md) | cmdResponseOut | output    | CmdResponse     |
| [`Svc::Sched`](../../../Svc/Sched/docs/sdd.md)      | schedIn        | input     | Sched           |
| [`Fw::Log`](../../../../Fw/Log/docs/sdd.md)         | eventOut       | output    | Log             |

### 2.3 Functional Description

#### 2.3.1 reportFSWInfo Port

Generates events for the flight software and Sphinx firmware versions. The FSW version string is sourced from
`FswVersionAc.hpp`, which should be generated during the FSW build process.
