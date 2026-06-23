Picking up where I left off yesterday:

1. I have not been updating my progress today because until just now there was nothing to report. I started the day by trying to update the linux kernals. 
2. Genfi-5 was on Redhat Linux 8, and Genfi-6 was on Linux Redhat 9. First I brought G5 up to Linux 9, but they were still on different sub-versions. g5 was on 5.14, and g6 was on 6.1. 
3. Each time I changed the Linux version, I cleared any previous Dolphin software and reinstalled. I continued to have compatibility issues. 
4. Even though both kernals were now on version 9, and the dolphin software installer was for version 9, (I tried both RT and non-RT) they were still incompatible.
5. I tried to bring g5 from 5.14 to 6.1, but then found that 6.1 is an experimental version of Linux that allows for newer versions.
6. Instead of bring g5 up, I brought g6 back down to the default version of 9 on 5.14. 
7. dmesg is giving all sorts of incompatibilities including a depricated driver
8. It looks like I will have to revert the Linux versions back to 8 and then install that driver. 
9. 