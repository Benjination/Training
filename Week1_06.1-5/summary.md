Week 1 Summary

I started the week with my first assignment from Roy: connect to two remote cluster nodes and upgrade SISCI/Dolphin software. I began with the standard remote workflow: SSH into each node, check OS and kernel details, transfer installers, run EEPROM update/reset steps, reboot, and verify node state with diagnostic tools.

Early on, I hit an execution gap. I had copied installer files to the nodes, but they were not actually being run. I corrected that by making the scripts executable and running them directly. After that, diagnostics improved, but I still saw lane-width warnings showing x8 instead of expected x16.

At that point, I shifted from software-only troubleshooting to connection health validation. I rechecked adapter status, firmware output, and configuration, and compared results between both nodes. Since software changes were not clearing the warning, the next step was hardware verification. With Roy's help, we checked the physical setup and found an unplugged cable. After reconnecting it, I repeated reset/reboot/config cycles and confirmed the system returned to active partitions and full x16 operation.

Once link health was restored, the next issue was version consistency. Driver, ExpressWare, EEPROM, and node OS versions still looked misaligned across systems. I tested service behavior, including node manager and host file generation, but automation did not produce expected results. Because repeated reinstall/config attempts were still blocked by compatibility differences, I decided to focus on OS and kernel alignment as the next logical step.

I then worked to standardize Linux versions across both nodes. I tested upgrade and rollback paths, reinstalled Dolphin each time, and evaluated RT vs non-RT combinations. This exposed additional incompatibilities and deprecated driver behavior, so I escalated to recovery-style work: local console access, USB image creation, and reinstall attempts from boot media. The installer path worked inconsistently, but this clarified the remaining blocker.

By the end of the week, I had completed remote upgrade and recovery work, fixed the x16 performance issue, and identified that full stability now depends on finishing clean OS/kernel alignment with a verified compatible Dolphin installer on both nodes.