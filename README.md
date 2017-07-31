usb-reset
=========

This is usb-reset, which allows you to easily perform a bus reset on a USB
device using its vendor and product ID.

If the device has got confused, this may sort it out.

usb-reset was inspired by the Ubuntu Podcast episode S10E20 where a piece of
code was described that did the same thing but based on usb bus and device
numbers, which sounded like a pain. Getting the snap working took longer than
writing the code.

Usage:

    sudo usb-reset 0328:6f51

https://github.com/ralight/usb-reset

