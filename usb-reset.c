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
#include <string.h>

void print_usage(void)
{
	printf("usb-reset %s: perform a bus reset on a USB device.\n\n", VERSION);
	printf("Usage: usb-reset vendor_id:product_id\n");
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


int main(int argc, char *argv[])
{
	int vid, pid;
	struct libusb_device_handle *handle;
	int rc = 0;

	if(argc != 2){
		print_usage();
		return 1;
	}

	if(str_to_vid_pid(argv[1], &vid, &pid)){
		return 1;
	}

	if(libusb_init(NULL)){
		printf("Unable to initialise libusb, exiting.\n");
		return 1;
	}

	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if(!handle){
		printf("Unable to open device %04x:%04x, are you root?\n", vid, pid);
		libusb_exit(NULL);
		return 1;
	}

	if(libusb_reset_device(handle)){
		printf("Reset failed, you may need to replug your device.\n");
		rc = 1;
	}
	
	libusb_close(handle);
	libusb_exit(NULL);

	return rc;
}

