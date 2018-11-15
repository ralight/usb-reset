usb-reset
=========

This is usb-reset, which allows you to easily perform a bus reset on a USB
device using its vendor and product ID.

If the device has got confused, this may sort it out.


Usage
=====

Reset a single device:

    sudo usb-reset 0328:6f51

Reset all USB devices in one go, for even easier operation:

	sudo usb-reset -a

This option does not reset USB hubs (to avoid interfering with internal hubs),
if you need to reset a hub you should specify the vendor and product ID as
above.


Installing
==========

The latest source can be found at:

https://github.com/ralight/usb-reset

It only depends on libusb 1.0 and should compile anywhere libusb is supported.


On systems where snaps are supported, usb-reset can be installed from the
Ubuntu Store. On Ubuntu 15.10 and earlier, or on other systems, you may need to
install snapd first.

[![Get it from the Snap Store](https://snapcraft.io/static/images/badges/en/snap-store-white.svg)](https://snapcraft.io/usb-reset)

History
=======

usb-reset was inspired by the Ubuntu Podcast episode S10E20 where a piece of
code was described that did the same thing but based on usb bus and device
numbers, which sounded like a pain. Getting the snap working took longer than
writing the code.

