### Low Signal Disconnect (C)

This utility provides information on how many AP clients connected to the Wi-Fi device have a dBm signal above a settable threshold.
The information is updated periodically after a configurable interval.

Usage:   
``` disconnect -i <interface> -f <interval> -t <threshold> -l <loops>```

The program will loop for `<loop>` number of times and print out devices under the `<threshold>` signal. For each iteration, the program will sleep for `<frequency>` seconds - which can be set to zero.