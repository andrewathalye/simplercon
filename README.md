# simplercon
A super-simple RCON client for Minecraft. Inspired by Tiiffi's mcrcon, and license-compatible (but without any shared code). Ideal for terminals which do not have colour support or termios availability.

Please let me know if you find any bugs - since this is mostly a test project I haven't created man pages or install scripts, but it should be fully useable on reliable connections.
Usage:
./simplercon SERVER PORT PASSWORD
It uses getaddrinfo for socket creation, so IPv6 and IPv4 servers are both supported (as well as servers by domain name if available).

This has so far been tested on Linux aarch64 (via Linux Deploy on Android) and x86_64. Help with testing on 32-bit architectures or different OSes (for example MinGW on Windows or BSDs
) would be greatly appreciated!
