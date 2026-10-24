# CS378 Socket Programming: Link State Emulator

- Oracle Node (ON): `on/on.py` (Python 3)
- Virtual Node (VN): `vn/vn.c` (C)

## Build

```
make
```

## Run

ON (TCP port 5000). The IP is optional, if not given it finds its own IP:

```
python3 on/on.py config.txt <on_ip>
```

VNs, start them one by one (first one to connect is A, next is B, ...):

```
./vn/vn <my_ip> <my_udp_port> <on_ip>
```

Example on one machine with IP 10.0.0.1:

```
python3 on/on.py config.txt 10.0.0.1
./vn/vn 10.0.0.1 6000 10.0.0.1
./vn/vn 10.0.0.1 6001 10.0.0.1
...
```

To change the topology during the run, just edit and save `config.txt`. The ON checks it every second.

## Messages (all binary, network byte order)

- CONNECT (VN -> ON, TCP): IP (4 bytes), UDP port (2)
- LINK-STATE (ON -> VN, TCP): count (1), then count times: letter (1), IP (4), UDP port (2), cost (4). The tuple with cost 0 is the VN itself.
- LSP (VN -> VN, UDP): origin letter (1), seq no (4), count (1), then count times: letter (1), cost (4)

## What the VN does

- prints its link state when it gets LINK-STATE, e.g. `E=0,C=1,D=7`
- sends its LSP on every LINK-STATE and every 10 sec, each LSP is sent twice
- a new LSP (higher seq no than seen before) is saved and sent to all neighbours, old ones are dropped
- forgets a node if no LSP came from it for 30 sec
- prints the graph it knows every 10 sec in the same format as the config file

## Tests

```
bash tests/run_all.sh
```

Configs used by the tests are in `tests/configs/`. To run VNs on a second machine: `bash tests/run_vns.sh <on_ip> <first_port> <how_many>`.
