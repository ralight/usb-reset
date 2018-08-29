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
#define RESET_CLASS 2
#define RESET_DEVICE 3

void print_usage(void)
{
	printf("usb-reset %s: perform a bus reset on a USB device.\n\n", VERSION);
	printf("Reset a single device:\n");
	printf("       usb-reset vendor_id:product_id\n\n");
	printf("Reset all devices of a particular type:\n");
	printf("       usb-reset -c <class>\n");
	printf("Classes: audio comm hid physical printer image\n");
	printf("         mass-storage hub data smart-card content-security\n");
	printf("         video personal-healthcare diagnostic-device\n");
	printf("         wireless application vendor-specific\n\n");
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


void print_open_warning(int vid, int pid)
{
	printf("Unable to open device %04x:%04x, are you root?\n", vid, pid);

	const char *snap = getenv("SNAP_NAME");
	if(snap){
		printf("usb-reset is installed as a snap. To allow it access to the usb bus you may need to run: \"sudo snap connect usb-reset:raw-usb core:raw-usb\"\n");
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


int str_to_class(const char *str, int *device_class)
{
	if(!strcasecmp(str, "audio")){
		*device_class = LIBUSB_CLASS_AUDIO;
	}else if(!strcasecmp(str, "comm")){
		*device_class = LIBUSB_CLASS_COMM;
	}else if(!strcasecmp(str, "hid")){
		*device_class = LIBUSB_CLASS_HID;
	}else if(!strcasecmp(str, "physical")){
		*device_class = LIBUSB_CLASS_PHYSICAL;
	}else if(!strcasecmp(str, "printer")){
		*device_class = LIBUSB_CLASS_PRINTER;
	}else if(!strcasecmp(str, "image")){
		*device_class = LIBUSB_CLASS_IMAGE;
	}else if(!strcasecmp(str, "mass-storage")){
		*device_class = LIBUSB_CLASS_MASS_STORAGE;
	}else if(!strcasecmp(str, "hub")){
		*device_class = LIBUSB_CLASS_HUB;
	}else if(!strcasecmp(str, "data")){
		*device_class = LIBUSB_CLASS_DATA;
	}else if(!strcasecmp(str, "smart-card")){
		*device_class = LIBUSB_CLASS_SMART_CARD;
	}else if(!strcasecmp(str, "content-security")){
		*device_class = LIBUSB_CLASS_CONTENT_SECURITY;
	}else if(!strcasecmp(str, "video")){
		*device_class = LIBUSB_CLASS_VIDEO;
	}else if(!strcasecmp(str, "personal-healthcare")){
		*device_class = LIBUSB_CLASS_PERSONAL_HEALTHCARE;
	}else if(!strcasecmp(str, "diagnostic-device")){
		*device_class = LIBUSB_CLASS_MASS_STORAGE;
	}else if(!strcasecmp(str, "wireless")){
		*device_class = LIBUSB_CLASS_WIRELESS;
	}else if(!strcasecmp(str, "application")){
		*device_class = LIBUSB_CLASS_APPLICATION;
	}else if(!strcasecmp(str, "vendor-specific")){
		*device_class = LIBUSB_CLASS_VENDOR_SPEC;
	}else{
		return 1;
	}

	return 0;
}


int reset_by_vid_pid(int vid, int pid)
{
	int rc = 0;
	struct libusb_device_handle *handle;

	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if(!handle){
		print_open_warning(vid, pid);
		return 1;
	}

	if(libusb_reset_device(handle)){
		printf("Reset failed, you may need to replug your device.\n");
		rc = 1;
	}

	libusb_close(handle);

	return rc;
}


int reset_single_device(struct libusb_device_descriptor *desc, libusb_device *device)
{
	struct libusb_device_handle *handle;
	int error;
	int rc = 0;
	unsigned char buf[100];

	error = libusb_open(device, &handle);
	if(error){
		print_open_warning(desc->idVendor, desc->idProduct);
		return 1;
	}

	if(libusb_get_string_descriptor_ascii(handle, desc->iProduct, buf, 100) > 0){
		printf("Trying to reset %s\n", buf);
	}

	if(libusb_reset_device(handle)){
		printf("Reset failed, you may need to replug the device %04x:%04x.\n", desc->idVendor, desc->idProduct);
		rc = 1;
	}
	libusb_close(handle);

	return rc;
}


int reset_per_interface_device(struct libusb_device_descriptor *desc, libusb_device *device, int device_class)
{
	struct libusb_config_descriptor *config;
	int rc;

	for(int c=0; c<desc->bNumConfigurations; c++){
		if(libusb_get_config_descriptor(device, c, &config)){

			for(int i=0; i<config->bNumInterfaces; i++){

				for(int a=0; a<config->interface[i].num_altsetting; a++){
					if(config->interface[i].altsetting[a].bInterfaceClass == device_class){
						rc = reset_single_device(desc, device);
						libusb_free_config_descriptor(config);
						return rc;
					}
				}
			}
			libusb_free_config_descriptor(config);
		}
	}

	return 1;
}


int reset_class(int device_class)
{
	libusb_device **devices;
	ssize_t device_count;
	int rc = 0;
	struct libusb_device_descriptor desc;

	device_count = libusb_get_device_list(NULL, &devices);

	if(device_count < 0){
		return 1;
	}

	// FIXME - per interface reset

	for(int i=0; i<device_count; i++){
		libusb_get_device_descriptor(devices[i], &desc);

		if(desc.bDeviceClass == device_class){
			reset_single_device(&desc, devices[i]);
		}else if(desc.bDeviceClass == LIBUSB_CLASS_PER_INTERFACE){
		}
	}

	libusb_free_device_list(devices, 1);

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
			print_open_warning(desc.idVendor, desc.idProduct);
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
	int vid, pid, device_class;
	int rc = 0;
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
	}else if(argc == 3){
		if(!strcmp(argv[1], "-c")){
			if(str_to_class(argv[2], &device_class)){
				fprintf(stderr, "Error: Unknown device class '%s'\n.", argv[2]);
				return 1;
			}
			op = RESET_CLASS;
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
		case RESET_CLASS:
			rc = reset_class(device_class);
			break;
		case RESET_DEVICE:
			rc = reset_by_vid_pid(vid, pid);
			break;
	}

	libusb_exit(NULL);

	return rc;
}

