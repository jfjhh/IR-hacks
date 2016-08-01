IR Hacks
========

All I wanted to do was blink a LED.
===================================

A Raspberry Pi (v2) IR controller, because I broke a cheap smart light IR remote
control. This is hacky because it pushes the hardware limits of what a Raspberry
Pi can do. With a Pi overclocked to 900 MHz and the highest possible priority
scheduling configuration, Raspbian just isn't a hard-RTOS.

Also the high frequency causes insane reactance on wires, so that can be fun to
deal with.

To check if it works, I have to let it bruteforce the light for 80 hours. If it
doesn't, then debug more and repeat. This may take a while.

