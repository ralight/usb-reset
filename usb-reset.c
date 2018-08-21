/*
MIT License

Copyright (c) 2017 Roger Light <roger@atchoo.org>.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RESET_ALL 1
#define RESET_DEVICE 3

void print_usage(void)
{
	printf("usb-reset %s: perform a bus reset on a USB device.\n\n", VERSION);
	printf("Reset a single device:\n");
	printf("       usb-reset vendor_id:product_id\n\n");
	printf("Reset all devices apart from hubs:\n");
	printf("       usb-reset -a\n\n");
	printf("Example: usb-reset 0245:7276\n\n");
	printf("See https://github.com/ralight/usb-reset\n");
}


int hex_char_to_int(char c)
{
	if(c >= '0' && c <= '9'){
		return c-'0';
	}

	switch(c){
		case 'A':
		case 'a':
			return 10;

		case 'B':
		case 'b':
			return 11;

		case 'C':
		case 'c':
			return 12;

		case 'D':
		case 'd':
			return 13;

		case 'E':
		case 'e':
			return 14;

		case 'F':
		case 'f':
			return 15;

		default:
			return -1;
	}
}


int str_to_vid_pid(char *vid_pid, int *vid, int *pid)
{
	int i;
	int val;

	if(strlen(vid_pid) != 9){
		printf("Invalid vendor ID/product ID length.\n");
		return 1;
	}

	if(vid_pid[4] != ':'){
		printf("Invalid vendor ID/product ID, no ':'.\n");
		return 1;
	}

	*vid = 0;
	for(i=0; i<4; i++){
		val = hex_char_to_int(vid_pid[i]);
		if(val == -1){
			printf("Invalid character in vendor id '%c'.\n", vid_pid[i]);
			return 1;
		}

		(*vid) = (*vid)*16 + val;
	}

	*pid = 0;
	for(i=5; i<9; i++){
		val = hex_char_to_int(vid_pid[i]);
		if(val == -1){
			printf("Invalid character in product id '%c'.\n", vid_pid[i]);
			return 1;
		}

		(*pid) = (*pid)*16 + val;
	}

	return 0;
}


int reset_by_vid_pid(int vid, int pid)
{
	int rc = 0;
	struct libusb_device_handle *handle;

	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if(!handle){
		printf("Unable to open device %04x:%04x, are you root?\n", vid, pid);

		const char *snap = getenv("SNAP_NAME");
		if(snap){
			printf("usb-reset is installed as a snap. To allow it access to the usb bus you may need to run: \"sudo snap connect usb-reset:raw-usb core:raw-usb\"\n");
		}

		return 1;
	}

	if(libusb_reset_device(handle)){
		printf("Reset failed, you may need to replug your device.\n");
		rc = 1;
	}

	libusb_close(handle);

	return rc;
}


int reset_all(void)
{
	struct libusb_device_handle *handle;
	libusb_device **devices;
	ssize_t device_count;
	int error;
	int rc = 0;
	struct libusb_device_descriptor desc;

	device_count = libusb_get_device_list(NULL, &devices);

	if(device_count < 0){
		return 1;
	}

	for(int i=0; i<device_count; i++){
		libusb_get_device_descriptor(devices[i], &desc);

		if(desc.bDeviceClass == LIBUSB_CLASS_HUB){
			continue;
		}

		error = libusb_open(devices[i], &handle);
		if(error){
			printf("Unable to open device %04x:%04x, are you root?\n", desc.idVendor, desc.idProduct);

			const char *snap = getenv("SNAP_NAME");
			if(snap){
				printf("usb-reset is installed as a snap. To allow it access to the usb bus you may need to run: \"sudo snap connect usb-reset:raw-usb core:raw-usb\"\n");
			}

			return 1;
		}

		if(libusb_reset_device(handle)){
			printf("Reset failed, you may need to replug the device %04x:%04x.\n", desc.idVendor, desc.idProduct);
			rc = 1;
		}
		libusb_close(handle);
	}

	libusb_free_device_list(devices, 1);

	return rc;
}


int main(int argc, char *argv[])
{
	int vid, pid;
	int rc;
	int op = -1;

	if(argc == 2){
		if(!strcmp(argv[1], "-a")){
			op = RESET_ALL;
		}else{
			if(str_to_vid_pid(argv[1], &vid, &pid)){
				return 1;
			}
			op = RESET_DEVICE;
		}
	}else{
		print_usage();
		return 1;
	}

	if(libusb_init(NULL)){
		printf("Unable to initialise libusb, exiting.\n");
		return 1;
	}

	switch(op){
		case RESET_ALL:
			rc = reset_all();
			break;
		case RESET_DEVICE:
			rc = reset_by_vid_pid(vid, pid);
			break;
	}

	libusb_exit(NULL);

	return rc;
}

