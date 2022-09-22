# BROKER MQTT v.0.1

## CONTENT OF THIS FILE

 * Introduction
 * Structure
 * Settings
 * Compilation and Execution
 * Operation
 * Tests and Performance Analysis
 * Author


## INTRODUCTION

The content of this repository is the implementation of a Broker MQTT Version 3.1.1, according to the
OASIS specifications. The structure, operation and instructions for compiling and running the
broker and performing the tests are described below.

In addition to the description of this README, the code is extensively documented in the header files,
include/* path. Also, all files contain their own header in the form of a comment.
explaining the contents of the file.


## STRUCTURE

In summary, the repository has the following structure:
```
ep1-henrique_carvalho
├── bin
│ └── broker - broker binary
├── font
│ ├── broker.c - broker main loop code
│ ├── handlers.c - code with handlers of incoming messages
│ ├── log.c - code with logging functions
│ ├── session.c - code with functions that manage a session between broker clients
│ ├── utils.c - code with project helper functions
│ └── wrappers.c - code with wrappers for some functions
│
├── include - headers with definitions from .c files
│ ├── config.h - configure broker settings
│ ├── rc.h - defines return code types that control the flow between functions
│ ├── handlers.h
│ ├── log.h
│ ├── internal.h
│ ├── utils.h
│ └── wrappers.h
│
├── README
├── Makefile
├── slides.pdf
└── tests
    ├── bin - unit test binaries
    ├── scripts
    │ ├── clients.sh - client generation script
    │ └── collect.sh - script to collect CPU and Network data
    └── unit_tests - unit test .c files
```

## SETTINGS

Some log settings can be modified before the broker runs. The settings that
can be edited directly in the include/config.h file:

LOGGING # 1 to enable log messages and 0 to disable (disables even
error messages)
TERMCOLORS #1 to enable colors in the terminal and 0 to disable

By default these settings are enabled.


## COMPILATION AND EXECUTION

To compile, just access the directory ep1-henrique_carvalho and type `make` in the shell. the binary
broker will be created in the bin/ directory. The broker is executed by typing the program and the port on which
the program will work:

```
./bin/broker <Port>
```

Execution example:

```user@host $ ./bin/broker 8888
[12:17:03.083944] Starting MQTT Broker v.3.1.1. Press Ctrl-C to finish.
[12:17:03.084098] Created topics directory at : /tmp/mqtt-topics-1650899823/.
[12:17:03.084142] Listening on port 8888.
```

When the above log messages appear, the broker is ready to receive connections.


## OPERATION

Every connection with the broker creates a process responsible for the client and this process enters the loop
of message handling. An incoming message is decoded into a packet and then that message is
managed in some specific way, according to the specifications of MQTT version 3.1.1. This one
broker supports CONNECT, DISCONNECT, PINGREQ, SUBSCRIBE and PUBLISH messages.

The SUBSCRIBE and PUBLISH management system basically works by writing and reading
a file. A topic created by a process that handles a PUBLISH is a file in a folder
temporary set in include/config.h, in the TMP_FILEPATH macro. A publication overwrites the
same topic file and a subscription reads from that file. For example, if a customer signs up
in the topic 'temperature', the broker creates a process to read in the topic
/tmp/mqtt-topics-XXXXXXXXXX/temperature and another to track new PING messages from the client
(the 'X' in the topic path is the time in seconds the file was created). if a customer
publishes at 'temperature', the broker creates a process to write to this topic.

By default, the broker adopts the same behavior as the mosquitto when a message is published to a
topic before a subscription, this message is not sent to the customer subscribed to the topic.

The broker does not implement some CONNECT flags and therefore does not accept many options normally
accepted by the mosquito. In short, the broker receives connections from the clients mosquitto_sub and mosquitto_pub
with the following options:

```
mosquitto_sub -h <ip address> -p <port> -t <topic> -V <version: 311>
mosquitto_pub -h <ip address> -p <port> -t <topic> -m <message> -V <version: 311>
```

Note that the supported <version> is only 3.1.1 and so if -V is passed it must be with
`-V 311`.

The broker does not implement wildcards and therefore the attempt to subscribe or publish to topics
using any wildcard contained in section 4.7 of the specification has undefined behavior.

## TESTS AND PERFORMANCE ANALYSIS

Some unit tests were created to test specific functions and specific behavior
of some parts of the code. These tests are in tests/testes_unitarios, but they are not
documented, their compilation is not guaranteed and their execution is not recommended.

Tests for performance analysis are concurrent client tests. These can be performed
with clients.sh and collect.sh scripts, in tests/scripts. The script named cpu.sh must be run
rather, with the command `./collect.sh <port> <trace file prefix>` and clients.sh should be run
on the computer that will run the clients with the command
`./clients.sh <number of clients> <ip address> <port>`. The data file returns a format
of `ps ux` "traces" by time, delimited by "+" characters and will be saved in parse/data.
Another data file is created in analyze/data with analysis of network consumption, generated by
from the nethogs program.

In analyze/notebooks are the codes, in jupyter notebook format, for cleaning the trace files
and the analyzes performed.


## LICENSE

Licensed under the GNU Public License version 3 (GPLv3).


## AUTHOR

Henrique de Carvalho <decarv.henrique@gmail.com>
