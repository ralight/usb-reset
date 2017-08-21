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

It can also be used to reset all USB devices in one go, for even easier
operation:

	sudo usb-reset -a

This option does not reset USB hubs (to avoid interfering with internal hubs),
if you need to reset a hub you should specify the vendor and product ID as
above.

https://github.com/ralight/usb-reset

