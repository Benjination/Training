Picking up where I left off yesterday.

1. After I left last night, I realized that I had used `scp` to move the installer to the remote nodes, but I never ran the installer.
2. This morning I used `ssh` to access each node, then used `chmod +x` to make the shell scripts executable, and ran the installers.
3. After running the installers and setting the configuration, I ran `dis_diag`. There was a warning that one node was only running x8 instead of x16. They should both be running x16.
4. I used `dis_diag` and `dis_firmwaretool hexpert pci:00:03.1 pfx -i` to diagnose the connection health. I found that when I log into either location, `dis_diag` can only find one adapter card at address `08:00.1`. There was also a warning that the lane width was set to x8 instead of x16.
5. I tried using `dis_config` to set the width to x16. This was allowed with no warnings, but the warning persisted.
6. I discovered the problem, and it is now fixed. A cable was unplugged. I did not have access to the hardware, so I would not have seen that it was detached because I had only been accessing the system remotely.
7. Roy came by to check my progress. I told him I had reinstalled the Dolphin software, configured the nodes, tried setting the width using `dis_config`, and was in the process of upgrading the EEPROM, but the warning still persisted. He said the system might be missing a cable. We went together to check the hardware and found a cable that was not connected.
8. I connected the cable.
9. I returned to my laptop to access the nodes remotely again. I upgraded the EEPROM on both nodes and used `dis_firmwaretool hexpert pci:08:00.1 pfx -R` to reset the adapters.
10. I rebooted both computers.
11. I updated both computers with `dis_config`, then checked with `dis_diag` and made sure the systems were running in the active partitions instead of the inactive partitions.
12. The systems were running in inactive partitions, so I reset the cards with `dis_firmwaretool hexpert pci:00:03.1 pfx -R`, then rebooted again.
13. After reboot, the systems were running in active partitions at full x16. Everything seems to be working again.

Note: Success.

Questions:
1. I only found one address at `08:00.1`, and it seems like since there are two nodes, they should have two addresses, unless this address represents the connection between the two nodes and not the nodes themselves.