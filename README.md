# EDCA-Simulator

This repository contains a simple EDCA simulator for didacting purposes (targetting undergrad and masters' students).

**Authors:**
Francesc Wilhelmi [francisco.wilhelmi@upf.edu]
Costas Michaelides [costas.michaelides@upf.ed]

## Introduction

EDCA (Enhanced Distributed Channel Access) is a channel access mechanism used in IEEE 802.11e (Wi-Fi QoS) to provide quality of service (QoS) for wireless networks. It enhances the original Distributed Coordination Function (DCF) by introducing differentiated access to the wireless medium based on traffic priority.

### How EDCA Works
EDCA divides network traffic into four Access Categories (ACs), each with different priority levels:

* AC_VO (Voice) – Highest priority, for real-time voice applications.
* AC_VI (Video) – Second highest, for video streaming.
* AC_BE (Best Effort) – Default priority, for normal data traffic.
* AC_BK (Background) – Lowest priority, for non-urgent data.

Each category has its own Contention Window (CW), Arbitration Interframe Space (AIFS), and Transmission Opportunity (TXOP) to control how often it can access the channel.

### Running the simulator

**Compile** the code using the following command: 

``chmod +x build_local COST/cxx; LD_PRELOAD= ./build_local``

You only need to compile the code when you add changes to source files (e.g., .cc, .h). You don't need to re-compile after tuning configuration parameters from `config_edca` or using different console arguments.

**Run** the code using the following command:

``./EdcaSim 77 1``

The arguments of the simulator are:
* arg[1]: Random seed (e.g., 77)
* arg[2]: Simulation time in seconds (e.g., 1)
