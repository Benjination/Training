Tuesday, last Tuesday in Norway. I'm so ready to go home. I still have a lot to do though.

1. Last night in the wee morning hours, My California counterparts scheduled a few meetings. The first looks like it came through at 6pm and was a meeting for 6:30pm. Had I seen it, I might have joined, but I don't think I can be blamed since it was 20 minutes before the interview. 
2. The second meeting is scheduled for tonight at 7p. This one I have known about for a few days. These are the questions they want answers to. I need to have the information prepared.
    a. How do you measure your switch latency? What is your latency?
        i. There are built in benchmark tests that measure latency using different methods.
            1. DMA_Bench: moves large amounts of data and prioritizes throughput. Best Latency: 36.04 us Worst Latency: 38.81 us
            2. SCIBench2: measures latency and throughput by ping-pong data transfers. Best Latency: 3.85 us Worst Latency: 3.98 us
            3. SCIPP: measures rtt latency and retry behaviour to measure effective throughput. Best Latency: 1.0359 us Worst Latency: 2.736 us
            4. SCIMemcopy: a series of different data transfers using memcopy accessing remote memory as local memory. Best Latency: 3.72 us Worst Latency: 3.72 us
            5. Interrupt Bench: measures rtt with interrupt process included. Best Latency: 4.92 us Worst Latency: 5.001 us
    b. What are your SISCI API features?
        i. SISCI features include:
            1. Shared/remote memory access: lets one node expose memory that another node can use like local memory.
            2. Reflective memory functionality: mirrors data between nodes for fast, synchronized state sharing.
            3. Direct peer-to-peer transfers: moves data directly between PCIe devices and nodes without going through the normal network stack.
            4. Remote device access: supports access to FPGAs, GPUs, and NVMes over PCIe.
            5. Local and networked scaling: works from a smaller multiprocessor setup up to a larger PCIe network.
            6. Driver/API examples and benchmarks: includes tools, demos, and benchmarks for testing performance and functionality.
        ii. Other Dolphin software features in the eXpressWare stack include:
            1. SmartIO: provides native access to PCIe devices in a remote server and is the base layer for Device Lending.
            2. Device Lending: lets GPUs, NVMe drives, and FPGAs be borrowed over the PCIe fabric without software overhead.
            3. SuperSockets: accelerates normal socket-based applications over PCIe networks and is meant for regular networked applications that want lower latency.
            4. IP driver for PCIe (IPoPCIe): supports regular TCP/IP-style networking over PCIe for cases that need standard routing and ARP behavior.
            5. Board Management Software: monitors PCIe adapter cards and active cables.
            6. PCI Express network management: provides install, configuration, and monitoring tools for the PCIe network.
            7. IRM (Interconnect Resource Manager): handles resources, link bring-up, heartbeats, and hot-plug/reboot events across the interconnect.
            8. eXpressWare platform support: the overall suite is available across Linux, Windows, RTX, QNX, and VxWorks depending on the component.
    c. We wish to have 16x GPU devices and they all can multicast to any device in the system
        i. Dolphin’s SISCI API supports direct peer-to-peer transfers and remote access to GPUs over PCIe, and SmartIO Device Lending is the feature that lets PCIe devices such as GPUs be borrowed over the fabric without software overhead. For a 16-GPU setup, the practical approach is to use SISCI/SmartIO to share and move data between devices and nodes, then use multicast or application-level fan-out to distribute the payload where needed.
    d. How do we develop our SW to use the multicast features?
        i. The public Dolphin stack describes SISCI as the API for shared/remote memory and device access, and the SISCI SmartIO extension as the way to access PCIe devices managed by SmartIO. So the software approach is to build on SISCI/SmartIO, use the provided driver/API stack and examples, and implement the send/receive flow around the multicast path so the same data can be delivered to multiple peers.
    e. How many multicast groups can we support? What if we need more groups than the limit?
        i. I did not find a public Dolphin page that states a fixed multicast-group limit.
        ii. If we hit a practical limit, the fallback is to partition devices into multiple groups or fan out the traffic at the application layer using SISCI/SmartIO-based data distribution.
        
3. I found out today I need to be very careful about what information I give Ai access to, and since I use copilot that means I need to make these notes more vague. I can't say who I am doing work for, and I can't give specifics about code and ssh keys and things. That's a totally reasonable and annoying hurdle that I need to adapt to. 
4. I just sent over the email with all of the answers to the questions above. I hope I was not wrong in any of my conclusions. 
5. Next I need to go through each of the NA companies in a list I was given to find if they have any open tickets. 
6.  