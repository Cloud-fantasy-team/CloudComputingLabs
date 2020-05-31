<p align="center">
    <img src="./logo.png">
</p>

# Cdb

Cdb is simple distributed key-value data store that utilizes 2-phase commit and leveldb. It consists of one coordinator and multiple participants. The client interacts with the coordinator using a simplified RESP message format over a TCP connection, while the coordinator sends RPC to all its participants.

Cdb implements the requirements of the [Extreme version](https://github.com/1989chenguo/CloudComputingLabs/tree/master/Lab3#353-extreme-version) of the[CloudComputingLabs](https://github.com/1989chenguo/CloudComputingLabs/tree/master/Lab3). This means that the system can tolerate participant failures, be it transient or permanent. The coordinator can be restarted without risking of corrupted and lost data. The clients get replies only when the coordinator is up and running. For more details, please look into the internal documentation.

## Requirments

- macOS/Linux.
- GNU make.
- C++11 compliant compiler.
