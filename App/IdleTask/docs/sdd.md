App::IdleTask Component
===

## 1. Introduction

`App::IdleTask` is a passive F' component for handling SDRAM and OCM memory setup, error counting, and memory scrubbing to be run as the lowest-priority task. It is passive
but does create and run its own task used to scrub memory.

## 2. Requirements

Requirement | Description | Verification Method |
----------- | ----------- | -------------------
App-IdleTask-00 | This component shall periodically read data from all 4byte aligned memory addresses from OCM and SDRAM to trigger EDAC | code-review |
App-IdleTask-01 | This component shall telemeter the last 5 SBE and DBE locations encountered while scrubbing OCM and SDRAM after a complete cycle. | unit-test |
App-IdleTask-02 | This component shall telemeter SBE and DBE locations reported by bootloader as read from OCM once after initialization. | unit-test |

## 3. Design

#### 3.1 Component Diagram

The component diagram is as follows:

![`App::IdleTask` Diagram](img/IdleTaskBDD.jpg "App::IdleTask")

#### 3.2 Ports

The `App::IdleTask` component uses the following port types:

| Port Data Type              | Name                  | Direction | Kind          | Usage                                                    |
|-----------------------------|-----------------------|-----------|---------------|----------------------------------------------------------|
| `Fw::Com`                   | getKeyTlm             | input     | guarded_input | Return last updated set of key telemetry values        |

It also has the following role ports:

| Port Data Type                                        | Name           | Direction | Role            |
|-------------------------------------------------------|----------------|-----------|-----------------|
| [`Fw::Log`](../../../../Fw/Log/docs/sdd.md)           | eventOut       | output    | Log             |
| [`Svc::Sched`](../../../../Svc/Sched/docs/sdd.md)     | schedIn        | input     | Sched           |
| [`Svc::Time`](../../../../Svc/SphinxTime/docs/sdd.md) | timeCaller     | output    | TimeGet         |
| [`Svc::TlmChan`](../../../../Svc/TlmChan/docs/sdd.md) | tlmOut         | output    | Telemetry       |

#### 3.3 Functional Description

The IdleTask checks counts of memory errors locations on startup, handles the hard/soft reboot handshake, and then continually reads all memory addresses to trigger
the EDAC protection of SDRAM and OCM. It then telemeters down the addresses of any errors encountered as well as the latest error counts. The AHB failing address register may indicate a failing address outside SDRAM and OCM (such as in PROM/NOR or SRAM address space). In this event, IdleTask does not write back corrected value to memory but telemeters the failing address location in telemetry.

**Note:** since there is no hardware count for non-OCM errors, it is impossible to prevent other tasks from triggering multiple errors before the idle task can
detect that an error has occurred. Thus all telemetered counts are a best-effort lower bound of the actual number of errors the system has experienced.

Also, IdleTask telemeters the power on reset count value at boot-up. If a hard reset is detected at boot-up, then IdleTask increments the value of power on reset count read from prmDb before telemetering it. The power on reset count is a non-volatile parameter managed through the prmDb interface. If the power on reset count is found to be at the max allowable value, defined by a macro (IDLE_TASK_MAX_POR_CNT_VAL), upon a hard reset, then the power on reset count value wraps around to 0 before the increment. In this case, the value of power on reset count is 1 after the increment similar to the value upon first hard power cycle.
