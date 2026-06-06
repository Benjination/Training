Picking up where I left off yesterday:

1. Today was mostly troubleshooting and reinstall attempts on the two remote nodes.
2. I confirmed the kernel versions were mismatched again and kept trying to bring both nodes to the same tested version.
3. I had SSH issues on one node where it accepted password entry and then immediately closed the connection.
4. I checked install/tooling dependencies and found one machine did not have gcc/build tools installed correctly.
5. I worked through package manager command differences and got the install commands sorted for this distro.
6. I moved to local console with monitor and keyboard so I could reinstall Linux directly from USB.
7. I had to figure out the boot menu entries because it did not say "USB" directly, it showed UEFI/General UDISK options.
8. I wrote the ISO to USB from terminal. At first dd was not recognized, then I got it working.
9. The write completed successfully with records in/out and bytes copied, so the USB image was created.
10. Booting from USB worked, but the installer behavior was inconsistent.
11. The Red Hat onboarding installer opened, but after entering user/password steps it dropped me into an anaconda root shell instead of staying in the GUI.
12. I saw it hanging on "Attempting to start vcconfig" and checked whether that was a real service.
13. It turns out vcconfig.service was not found, so the issue looked more like installer/console mode behavior than a normal system service failure.
14. I rebooted and planned to retry with safer installer boot options (basic graphics/nomodeset) to keep it from switching to text console.
15. I also checked Red Hat account access options in case registration/subscription becomes a blocker during setup.
16. Current status: USB boot/install path works, but installer stability is still the main issue before final user/password and complete node alignment.
17. 