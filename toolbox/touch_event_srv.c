#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <linux/input.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "getevent.h"

#ifdef HAVE_SELINUX
#include <selinux/selinux.h>
#endif

#define CMD_REPLY_ANYWAY		0
#define RUN_PD40_BACKGROUND_SERVICES 0

#define EXIT_CODE_OKAY			0
#define EXIT_CODE_DAEMONIZE		1
#define EXIT_CODE_NO_INPUT_DEVICE	2
#define EXIT_CODE_SOCKET_ERROR		3
#define EXIT_CODE_CONNECTION_FAILURE	4

#define TOUCH_SRV_PORTNO	2301
#define TOUCH_SRV_MAX_BACKLOG_CONNECTIONS 5
#define TOUCH_SRV_SOCKET_BUFF_SIZE	512

#define TOUCHSCREEN_DEV_NAME "TSC2004 Touchscreen"
#define TOUCHSCREEN_RESOLUTION_X (800)
#define TOUCHSCREEN_RESOLUTION_Y (480)
#define TOUCHSCREEN_RESOLUTION_PRESSURE (1)

#define GPIO_KEYS_NAME "gpio-keys"

#define MOUSE_DEV_NAME "Android Virtual Mouse"
#define MOUSE_DEV_PATH "/dev/avms"

#define PWR_SUPPLY_PATH "/sys/class/power_supply/bq27500-0"
#define PWR_SUPPLY_CAPACITY "capacity"
#define PWR_SUPPLY_STATUS "status"

#define LCD_BACKLIGHT_PATH "/sys/class/leds/lcd-backlight"
#define LCD_BRIGHTNESS_LEVEL "brightness"

#define KEYBOARD_DEV_NAME "Android Virtual Keyboard"
#define KEYBOARD_DEV_PATH "/dev/avkbd"

#define LOG(...) do { fprintf(stderr, __VA_ARGS__); } while(0)
#define LOG_V(string, ...)
#define LOG_D(...) LOG("D: " __VA_ARGS__)
#define LOG_I(...) LOG("I: " __VA_ARGS__)
#define LOG_W(...) LOG("W: " __VA_ARGS__)
#define LOG_E(...) LOG("E: " __VA_ARGS__)


//Event types definitions:
#define AVKBD_KEY_GRAVE 0
#define AVKBD_KEY_ESC 1
#define AVKBD_KEY_1 2
#define AVKBD_KEY_2 3
#define AVKBD_KEY_3 4
#define AVKBD_KEY_4 5
#define AVKBD_KEY_5 6
#define AVKBD_KEY_6 7
#define AVKBD_KEY_7 8
#define AVKBD_KEY_8 9
#define AVKBD_KEY_9 10
#define AVKBD_KEY_0 11
#define AVKBD_KEY_MINUS 12
#define AVKBD_KEY_EQUAL 13
#define AVKBD_KEY_BACKSPACE 14
#define AVKBD_KEY_TAB 15
#define AVKBD_KEY_Q 16
#define AVKBD_KEY_W 17
#define AVKBD_KEY_E 18
#define AVKBD_KEY_R 19
#define AVKBD_KEY_T 20
#define AVKBD_KEY_Y 21
#define AVKBD_KEY_U 22
#define AVKBD_KEY_I 23
#define AVKBD_KEY_O 24
#define AVKBD_KEY_P 25
#define AVKBD_KEY_LEFTBRACE 26
#define AVKBD_KEY_RIGHTBRACE 27
#define AVKBD_KEY_BACKSLASH 28
#define AVKBD_KEY_CAPSLOCK 29
#define AVKBD_KEY_A 30
#define AVKBD_KEY_S 31
#define AVKBD_KEY_D 32
#define AVKBD_KEY_F 33
#define AVKBD_KEY_G 34
#define AVKBD_KEY_H 35
#define AVKBD_KEY_J 36
#define AVKBD_KEY_K 37
#define AVKBD_KEY_L 38
#define AVKBD_KEY_SEMICOLON 39
#define AVKBD_KEY_APOSTROPHE 40
#define AVKBD_KEY_ENTER 41
#define AVKBD_KEY_LEFTSHIFT 42
#define AVKBD_KEY_Z 43
#define AVKBD_KEY_X 44
#define AVKBD_KEY_C 45
#define AVKBD_KEY_V 46
#define AVKBD_KEY_B 47
#define AVKBD_KEY_N 48
#define AVKBD_KEY_M 49
#define AVKBD_KEY_COMMA 50
#define AVKBD_KEY_DOT 51
#define AVKBD_KEY_SLASH 52
#define AVKBD_KEY_RIGHTSHIFT 53
#define AVKBD_KEY_LEFTCTRL 54
#define AVKBD_KEY_LEFTMETA 55
#define AVKBD_KEY_LEFTALT 56
#define AVKBD_KEY_SPACE 57
#define AVKBD_KEY_RIGHTALT 58
#define AVKBD_KEY_RIGHTCTRL 59
#define AVKBD_KEY_UP 60
#define AVKBD_KEY_DOWN 61
#define AVKBD_KEY_LEFT 62
#define AVKBD_KEY_RIGHT 63
#define AVKBD_KEY_PAGEUP 64
#define AVKBD_KEY_PAGEDOWN 65
#define AVKBD_KEY_F1 66
#define AVKBD_KEY_F2 67
#define AVKBD_KEY_F3 68
#define AVKBD_KEY_F4 69
#define AVKBD_KEY_F5 70
#define AVKBD_KEY_F6 71
#define AVKBD_KEY_F7 72
#define AVKBD_KEY_F8 73
#define AVKBD_KEY_F9 74
#define AVKBD_KEY_F10 75
#define AVKBD_KEY_F11 76
#define AVKBD_KEY_F12 77
#define AVKBD_KEY_HOME 78
#define AVKBD_KEY_END 79
#define AVKBD_KEY_INSERT 80
#define AVKBD_KEY_DELETE 81
#define AVKBD_KEY_SYSRQ 82
#define AVKBD_KEY_PRINTSCRN AVKBD_KEY_SYSRQ
#define AVKBD_KEY_KP1 83
#define AVKBD_KEY_KP2 84
#define AVKBD_KEY_KP3 85
#define AVKBD_KEY_KP4 86
#define AVKBD_KEY_KP5 87
#define AVKBD_KEY_KP6 88
#define AVKBD_KEY_KP7 89
#define AVKBD_KEY_KP8 90
#define AVKBD_KEY_KP9 91
#define AVKBD_KEY_KP0 92
#define AVKBD_KEY_VOLUMEUP 93
#define AVKBD_KEY_VOLUMEDOWN 94
#define AVKBD_KEY_MUTE 95
#define AVKBD_KEY_PLAYPAUSE 96
#define AVKBD_KEY_PREVIOUSSONG 97
#define AVKBD_KEY_NEXTSONG 98
#define AVKBD_KEY_HOMEPAGE 99

#define AVKBD_KEYMAP_SIZE 0x64


struct dummy_dev {
	char* dev_name;
	int fd;
};

struct gpiokeys_device {
	char* dev_name;
	int fd;
};

struct mouse_device {
	char* dev_name;
	int fd;
};

struct keyboard_device {
	char* dev_name;
	int fd;
};

struct touchscreen_device_events_prop {
	uint16_t key_desc;
	struct input_absinfo abs_x;
	struct input_absinfo abs_y;
	struct input_absinfo abs_pressure;
};

struct touchscreen_device {
	char* dev_name;
	int fd;
	int events;
	int version;
	char location[80];
	char idstr[80];
	struct input_id id;

	struct touchscreen_device_events_prop prop;
};

struct system_devices {
	struct touchscreen_device* touch_dev;
	struct gpiokeys_device* gpio_dev;
	struct mouse_device* mouse_dev;
	struct keyboard_device* keyboard_dev;
};

enum {
	PRINT_DEVICE_ERRORS = 1U << 0,
	PRINT_DEVICE = 1U << 1,
	PRINT_DEVICE_NAME = 1U << 2,
	PRINT_DEVICE_INFO = 1U << 3,
	PRINT_VERSION = 1U << 4,
	PRINT_POSSIBLE_EVENTS = 1U << 5,
	PRINT_INPUT_PROPS = 1U << 6,
	PRINT_HID_DESCRIPTOR = 1U << 7,

	PRINT_ALL_INFO = (1U << 8) - 1,

	PRINT_LABELS = 1U << 16,
};

typedef enum {
START,
BROADCAST,
START_SERVICE
} am_subcommand;

int start_activity(am_subcommand subCmd ,const char* action,
						  const char* package,
						  const char* activity , const char * extras,
						  int extrasLength);
static uint32_t client_pad_height = TOUCHSCREEN_RESOLUTION_Y;
static uint32_t client_pad_width = TOUCHSCREEN_RESOLUTION_X;
static struct pollfd *ufds;
static char **device_names;
static int nfds;
const char* input_device;

static float resolution_factor_x = 1;
static float resolution_factor_y = 1;

int running = 0;
int daemonize = 0;

socklen_t clilen;
struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;

static const char *get_label(const struct label *labels, int value) {
	while (labels->name && value != labels->value) {
		labels++;
	}
	return labels->name;
}

static int print_input_props(int fd) {
	uint8_t bits[INPUT_PROP_CNT / 8];
	int i, j;
	int res;
	int count;
	const char *bit_label;

	printf("  input props:\n");
	res = ioctl(fd, EVIOCGPROP(sizeof(bits)), bits);
	if (res < 0) {
		printf("    <not available\n");
		return 1;
	}
	count = 0;
	for (i = 0; i < res; i++) {
		for (j = 0; j < 8; j++) {
			if (bits[i] & 1 << j) {
				bit_label = get_label(input_prop_labels, i * 8 + j);
				if (bit_label)
					printf("    %s\n", bit_label);
				else
					printf("    %04x\n", i * 8 + j);
				count++;
			}
		}
	}
	if (!count)
		printf("    <none>\n");
	return 0;
}

static int print_possible_events(int fd, int print_flags) {
	uint8_t *bits = NULL;
	ssize_t bits_size = 0;
	const char* label;
	int i, j, k;
	int res, res2;
	struct label* bit_labels;
	const char *bit_label;

	printf("  events:\n");
	for (i = EV_KEY; i <= EV_MAX; i++) { // skip EV_SYN since we cannot query its available codes
		int count = 0;
		while (1) {
			res = ioctl(fd, EVIOCGBIT(i, bits_size), bits);
			if (res < bits_size)
				break;
			bits_size = res + 16;
			bits = realloc(bits, bits_size * 2);
			if (bits == NULL) {
				fprintf(stderr, "failed to allocate buffer of size %d\n",
						(int) bits_size);
				return 1;
			}
		}
		res2 = 0;
		switch (i) {
		case EV_KEY:
			res2 = ioctl(fd, EVIOCGKEY(res), bits + bits_size);
			label = "KEY";
			bit_labels = key_labels;
			break;
		case EV_REL:
			label = "REL";
			bit_labels = rel_labels;
			break;
		case EV_ABS:
			label = "ABS";
			bit_labels = abs_labels;
			break;
		case EV_MSC:
			label = "MSC";
			bit_labels = msc_labels;
			break;
		case EV_LED:
			res2 = ioctl(fd, EVIOCGLED(res), bits + bits_size);
			label = "LED";
			bit_labels = led_labels;
			break;
		case EV_SND:
			res2 = ioctl(fd, EVIOCGSND(res), bits + bits_size);
			label = "SND";
			bit_labels = snd_labels;
			break;
		case EV_SW:
			res2 = ioctl(fd, EVIOCGSW(bits_size), bits + bits_size);
			label = "SW ";
			bit_labels = sw_labels;
			break;
		case EV_REP:
			label = "REP";
			bit_labels = rep_labels;
			break;
		case EV_FF:
			label = "FF ";
			bit_labels = ff_labels;
			break;
		case EV_PWR:
			label = "PWR";
			bit_labels = NULL;
			break;
		case EV_FF_STATUS:
			label = "FFS";
			bit_labels = ff_status_labels;
			break;
		default:
			res2 = 0;
			label = "???";
			bit_labels = NULL;
		}
		for (j = 0; j < res; j++) {
			for (k = 0; k < 8; k++)
				if (bits[j] & 1 << k) {
					char down;
					if (j < res2 && (bits[j + bits_size] & 1 << k))
						down = '*';
					else
						down = ' ';
					if (count == 0)
						printf("    %s (%04x):", label, i);
					else if ((count & (print_flags & PRINT_LABELS ? 0x3 : 0x7))
							== 0 || i == EV_ABS)
						printf("\n               ");
					if (bit_labels && (print_flags & PRINT_LABELS)) {
						bit_label = get_label(bit_labels, j * 8 + k);
						if (bit_label)
							printf(" %.20s%c%*s", bit_label, down,
									20 - strlen(bit_label), "");
						else
							printf(" %04x%c                ", j * 8 + k, down);
					} else {
						printf(" %04x%c", j * 8 + k, down);
					}
					if (i == EV_ABS) {
						struct input_absinfo abs;
						if (ioctl(fd, EVIOCGABS(j * 8 + k), &abs) == 0) {
							printf(
									" : value %d, min %d, max %d, fuzz %d, flat %d, resolution %d",
									abs.value, abs.minimum, abs.maximum,
									abs.fuzz, abs.flat, abs.resolution);
						}
					}
					count++;
				}
		}
		if (count)
			printf("\n");
	}
	free(bits);
	return 0;
}

static void print_event(int type, int code, int value, int print_flags) {
	const char *type_label, *code_label, *value_label;

	if (print_flags & PRINT_LABELS) {
		type_label = get_label(ev_labels, type);
		code_label = NULL;
		value_label = NULL;

		switch (type) {
		case EV_SYN:
			code_label = get_label(syn_labels, code);
			break;
		case EV_KEY:
			code_label = get_label(key_labels, code);
			value_label = get_label(key_value_labels, value);
			break;
		case EV_REL:
			code_label = get_label(rel_labels, code);
			break;
		case EV_ABS:
			code_label = get_label(abs_labels, code);
			switch (code) {
			case ABS_MT_TOOL_TYPE:
				value_label = get_label(mt_tool_labels, value);
			}
			break;
		case EV_MSC:
			code_label = get_label(msc_labels, code);
			break;
		case EV_LED:
			code_label = get_label(led_labels, code);
			break;
		case EV_SND:
			code_label = get_label(snd_labels, code);
			break;
		case EV_SW:
			code_label = get_label(sw_labels, code);
			break;
		case EV_REP:
			code_label = get_label(rep_labels, code);
			break;
		case EV_FF:
			code_label = get_label(ff_labels, code);
			break;
		case EV_FF_STATUS:
			code_label = get_label(ff_status_labels, code);
			break;
		}

		if (type_label)
			printf("%-12.12s", type_label);
		else
			printf("%04x        ", type);
		if (code_label)
			printf(" %-20.20s", code_label);
		else
			printf(" %04x                ", code);
		if (value_label)
			printf(" %-20.20s", value_label);
		else
			printf(" %08x            ", value);
	} else {
		printf("%04x %04x %08x", type, code, value);
	}
}

static void print_hid_descriptor(int bus, int vendor, int product) {
	const char *dirname = "/sys/kernel/debug/hid";
	char prefix[16];
	DIR *dir;
	struct dirent *de;
	char filename[PATH_MAX];
	FILE *file;
	char line[2048];

	snprintf(prefix, sizeof(prefix), "%04X:%04X:%04X.", bus, vendor, product);

	dir = opendir(dirname);
	if (dir == NULL)
		return;
	while ((de = readdir(dir))) {
		if (strstr(de->d_name, prefix) == de->d_name) {
			snprintf(filename, sizeof(filename), "%s/%s/rdesc", dirname,
					de->d_name);

			file = fopen(filename, "r");
			if (file) {
				printf("  HID descriptor: %s\n\n", de->d_name);
				while (fgets(line, sizeof(line), file)) {
					fputs("    ", stdout);
					fputs(line, stdout);
				}
				fclose(file);
				puts("");
			}
		}
	}
	closedir(dir);
}

static int open_device(const char *device, int print_flags) {
	int version;
	int fd;
	struct pollfd *new_ufds;
	char **new_device_names;
	char name[80];
	char location[80];
	char idstr[80];
	struct input_id id;

	fd = open(device, O_RDWR);
	if (fd < 0) {
		if (print_flags & PRINT_DEVICE_ERRORS)
			fprintf(stderr, "could not open %s, %s\n", device, strerror(errno));
		return -1;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		if (print_flags & PRINT_DEVICE_ERRORS)
			fprintf(stderr, "could not get driver version for %s, %s\n", device,
					strerror(errno));
		return -1;
	}
	if (ioctl(fd, EVIOCGID, &id)) {
		if (print_flags & PRINT_DEVICE_ERRORS)
			fprintf(stderr, "could not get driver id for %s, %s\n", device,
					strerror(errno));
		return -1;
	}
	name[sizeof(name) - 1] = '\0';
	location[sizeof(location) - 1] = '\0';
	idstr[sizeof(idstr) - 1] = '\0';
	if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
		//fprintf(stderr, "could not get device name for %s, %s\n", device, strerror(errno));
		name[0] = '\0';
	}
	if (ioctl(fd, EVIOCGPHYS(sizeof(location) - 1), &location) < 1) {
		//fprintf(stderr, "could not get location for %s, %s\n", device, strerror(errno));
		location[0] = '\0';
	}
	if (ioctl(fd, EVIOCGUNIQ(sizeof(idstr) - 1), &idstr) < 1) {
		//fprintf(stderr, "could not get idstring for %s, %s\n", device, strerror(errno));
		idstr[0] = '\0';
	}

	new_ufds = realloc(ufds, sizeof(ufds[0]) * (nfds + 1));
	if (new_ufds == NULL) {
		fprintf(stderr, "out of memory\n");
		return -1;
	}
	ufds = new_ufds;
	new_device_names = realloc(device_names,
			sizeof(device_names[0]) * (nfds + 1));
	if (new_device_names == NULL) {
		fprintf(stderr, "out of memory\n");
		return -1;
	}
	device_names = new_device_names;

	if (print_flags & PRINT_DEVICE)
		printf("add device %d: %s\n", nfds, device);
	if (print_flags & PRINT_DEVICE_INFO)
		printf("  bus:      %04x\n"
				"  vendor    %04x\n"
				"  product   %04x\n"
				"  version   %04x\n", id.bustype, id.vendor, id.product,
				id.version);
	if (print_flags & PRINT_DEVICE_NAME)
		printf("  name:     \"%s\"\n", name);
	if (print_flags & PRINT_DEVICE_INFO)
		printf("  location: \"%s\"\n"
				"  id:       \"%s\"\n", location, idstr);
	if (print_flags & PRINT_VERSION)
		printf("  version:  %d.%d.%d\n", version >> 16, (version >> 8) & 0xff,
				version & 0xff);

	if (print_flags & PRINT_POSSIBLE_EVENTS) {
		print_possible_events(fd, print_flags);
	}

	if (print_flags & PRINT_INPUT_PROPS) {
		print_input_props(fd);
	}
	if (print_flags & PRINT_HID_DESCRIPTOR) {
		print_hid_descriptor(id.bustype, id.vendor, id.product);
	}

	ufds[nfds].fd = fd;
	ufds[nfds].events = POLLIN;
	device_names[nfds] = strdup(device);
	nfds++;

	return 0;
}

static int close_device(const char *device, int print_flags) {
	int i;
	for (i = 1; i < nfds; i++) {
		if (strcmp(device_names[i], device) == 0) {
			int count = nfds - i - 1;
			if (print_flags & PRINT_DEVICE)
				printf("remove device %d: %s\n", i, device);
			free(device_names[i]);
			memmove(device_names + i, device_names + i + 1,
					sizeof(device_names[0]) * count);
			memmove(ufds + i, ufds + i + 1, sizeof(ufds[0]) * count);
			nfds--;
			return 0;
		}
	}
	if (print_flags & PRINT_DEVICE_ERRORS)
		fprintf(stderr, "remote device: %s not found\n", device);
	return -1;
}

static int read_notify(const char *dirname, int nfd, int print_flags) {
	int res;
	char devname[PATH_MAX];
	char *filename;
	char event_buf[512];
	int event_size;
	int event_pos = 0;
	struct inotify_event *event;

	res = read(nfd, event_buf, sizeof(event_buf));
	if (res < (int) sizeof(*event)) {
		if (errno == EINTR)
			return 0;
		fprintf(stderr, "could not get event, %s\n", strerror(errno));
		return 1;
	}
	//printf("got %d bytes of event information\n", res);

	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';

	while (res >= (int) sizeof(*event)) {
		event = (struct inotify_event *) (event_buf + event_pos);
		//printf("%d: %08x \"%s\"\n", event->wd, event->mask, event->len ? event->name : "");
		if (event->len) {
			strcpy(filename, event->name);
			if (event->mask & IN_CREATE) {
				open_device(devname, print_flags);
			} else {
				close_device(devname, print_flags);
			}
		}
		event_size = sizeof(*event) + event->len;
		res -= event_size;
		event_pos += event_size;
	}
	return 0;
}

static int scan_dir(const char *dirname, int print_flags) {
	char devname[PATH_MAX];
	char *filename;
	DIR *dir;
	struct dirent *de;
	dir = opendir(dirname);
	if (dir == NULL)
		return -1;
	strcpy(devname, dirname);
	filename = devname + strlen(devname);
	*filename++ = '/';
	while ((de = readdir(dir))) {
		if (de->d_name[0] == '.'
				&& (de->d_name[1] == '\0'
						|| (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;
		strcpy(filename, de->d_name);
		open_device(devname, print_flags);
	}
	closedir(dir);
	return 0;
}

static void usage(int argc, char *argv[]) {
	fprintf(stderr,
			"Usage: %s [-t] [-n] [-s switchmask] [-S] [-v [mask]] [-d] [-p] [-i] [-l] [-q] [-c count] [-r] [device]\n",
			argv[0]);
	fprintf(stderr, "    -t: show time stamps\n");
	fprintf(stderr, "    -n: don't print newlines\n");
	fprintf(stderr, "    -s: print switch states for given bits\n");
	fprintf(stderr, "    -S: print all switch states\n");
	fprintf(stderr,
			"    -v: verbosity mask (errs=1, dev=2, name=4, info=8, vers=16, pos. events=32, props=64)\n");
	fprintf(stderr, "    -d: show HID descriptor, if available\n");
	fprintf(stderr,
			"    -p: show possible events (errs, dev, name, pos. events)\n");
	fprintf(stderr, "    -i: show all device info and possible events\n");
	fprintf(stderr, "    -l: label event types and names in plain text\n");
	fprintf(stderr, "    -q: quiet (clear verbosity mask)\n");
	fprintf(stderr, "    -c: print given number of events then exit\n");
	fprintf(stderr, "    -r: print rate events are received\n");
}

static int getevent(int argc, char *argv[]) {
	int c;
	int i;
	int res;
	int pollres;
	int get_time = 0;
	int print_device = 0;
	char *newline = "\n";
	uint16_t get_switch = 0;
	struct input_event event;
	int version;
	int print_flags = 0;
	int print_flags_set = 0;
	int dont_block = -1;
	int event_count = 0;
	int sync_rate = 0;
	int64_t last_sync_time = 0;
	const char *device = NULL;
	const char *device_path = "/dev/input";

	opterr = 0;
	do {
		c = getopt(argc, argv, "tns:Sv::dpilqc:rh");
		if (c == EOF)
			break;
		switch (c) {
		case 't':
			get_time = 1;
			break;
		case 'n':
			newline = "";
			break;
		case 's':
			get_switch = strtoul(optarg, NULL, 0);
			if (dont_block == -1)
				dont_block = 1;
			break;
		case 'S':
			get_switch = ~0;
			if (dont_block == -1)
				dont_block = 1;
			break;
		case 'v':
			if (optarg)
				print_flags |= strtoul(optarg, NULL, 0);
			else
				print_flags |= PRINT_DEVICE | PRINT_DEVICE_NAME
						| PRINT_DEVICE_INFO | PRINT_VERSION;
			print_flags_set = 1;
			break;
		case 'd':
			print_flags |= PRINT_HID_DESCRIPTOR;
			break;
		case 'p':
			print_flags |= PRINT_DEVICE_ERRORS | PRINT_DEVICE
					| PRINT_DEVICE_NAME | PRINT_POSSIBLE_EVENTS
					| PRINT_INPUT_PROPS;
			print_flags_set = 1;
			if (dont_block == -1)
				dont_block = 1;
			break;
		case 'i':
			print_flags |= PRINT_ALL_INFO;
			print_flags_set = 1;
			if (dont_block == -1)
				dont_block = 1;
			break;
		case 'l':
			print_flags |= PRINT_LABELS;
			break;
		case 'q':
			print_flags_set = 1;
			break;
		case 'c':
			event_count = atoi(optarg);
			dont_block = 0;
			break;
		case 'r':
			sync_rate = 1;
			break;
		case '?':
			fprintf(stderr, "%s: invalid option -%c\n", argv[0], optopt);
		case 'h':
			usage(argc, argv);
			exit(1);
		}
	} while (1);
	if (dont_block == -1)
		dont_block = 0;

	if (optind + 1 == argc) {
		device = argv[optind];
		optind++;
	}
	if (optind != argc) {
		usage(argc, argv);
		exit(1);
	}
	nfds = 1;
	ufds = calloc(1, sizeof(ufds[0]));
	ufds[0].fd = inotify_init();
	ufds[0].events = POLLIN;
	if (device) {
		if (!print_flags_set)
			print_flags |= PRINT_DEVICE_ERRORS;
		res = open_device(device, print_flags);
		if (res < 0) {
			return 1;
		}
	} else {
		if (!print_flags_set)
			print_flags |= PRINT_DEVICE_ERRORS | PRINT_DEVICE
					| PRINT_DEVICE_NAME;
		print_device = 1;
		res = inotify_add_watch(ufds[0].fd, device_path, IN_DELETE | IN_CREATE);
		if (res < 0) {
			fprintf(stderr, "could not add watch for %s, %s\n", device_path,
					strerror(errno));
			return 1;
		}
		res = scan_dir(device_path, print_flags);
		if (res < 0) {
			fprintf(stderr, "scan dir failed for %s\n", device_path);
			return 1;
		}
	}

	if (get_switch) {
		for (i = 1; i < nfds; i++) {
			uint16_t sw;
			res = ioctl(ufds[i].fd, EVIOCGSW(1), &sw);
			if (res < 0) {
				fprintf(stderr, "could not get switch state, %s\n",
						strerror(errno));
				return 1;
			}
			sw &= get_switch;
			printf("%04x%s", sw, newline);
		}
	}

	if (dont_block)
		return 0;

	while (1) {
		pollres = poll(ufds, nfds, -1);
		//printf("poll %d, returned %d\n", nfds, pollres);
		if (ufds[0].revents & POLLIN) {
			read_notify(device_path, ufds[0].fd, print_flags);
		}
		for (i = 1; i < nfds; i++) {
			if (ufds[i].revents) {
				if (ufds[i].revents & POLLIN) {
					res = read(ufds[i].fd, &event, sizeof(event));
					if (res < (int) sizeof(event)) {
						fprintf(stderr, "could not get event\n");
						return 1;
					}
					if (get_time) {
						printf("[%8ld.%06ld] ", event.time.tv_sec,
								event.time.tv_usec);
					}
					if (print_device)
						printf("%s: ", device_names[i]);
					print_event(event.type, event.code, event.value,
							print_flags);
					if (sync_rate && event.type == 0 && event.code == 0) {
						int64_t now = event.time.tv_sec * 1000000LL
								+ event.time.tv_usec;
						if (last_sync_time)
							printf(" rate %lld",
									1000000LL / (now - last_sync_time));
						last_sync_time = now;
					}
					printf("%s", newline);
					if (event_count && --event_count == 0)
						return 0;
				}
			}
		}
	}

	return 0;
}

static int touchscreen_get_events_prop(struct touchscreen_device* touch_dev) {
	uint8_t *bits = NULL;
	ssize_t bits_size = 0;
	const char* label;
	int i, j, k;
	int res, res2;
	struct label* bit_labels;
	const char *bit_label;

	for (i = EV_KEY; i <= EV_MAX; i++) { // skip EV_SYN since we cannot query its available codes
		int count = 0;
		while (1) {
			res = ioctl(touch_dev->fd, EVIOCGBIT(i, bits_size), bits);
			if (res < bits_size)
				break;
			bits_size = res + 16;
			bits = realloc(bits, bits_size * 2);
			if (bits == NULL) {
				fprintf(stderr, "failed to allocate buffer of size %d\n",
						(int) bits_size);
				return 1;
			}
		}
		for (j = 0; j < res; j++) {
			for (k = 0; k < 8; k++)
				if (bits[j] & 1 << k) {
					if (i == EV_KEY) {
						touch_dev->prop.key_desc = j * 8 + k;
					}
					if (i == EV_ABS) {
						struct input_absinfo abs;
						if (ioctl(touch_dev->fd, EVIOCGABS(j * 8 + k), &abs)
								!= 0) {
							fprintf(stderr, "failed to get absinfo\n");
							return 2;
						}
						switch (j * 8 + k) {
						case ABS_X:
							memcpy(&touch_dev->prop.abs_x, &abs,
									sizeof(struct input_absinfo));
							break;
						case ABS_Y:
							memcpy(&touch_dev->prop.abs_y, &abs,
									sizeof(struct input_absinfo));
							break;
						case ABS_PRESSURE:
							memcpy(&touch_dev->prop.abs_pressure, &abs,
									sizeof(struct input_absinfo));
							break;
						default:
							fprintf(stderr, "unknown case!\n");
							return 3;
						}
					}
					count++;
				}
		}
		if (count)
			printf("\n");
	}
	free(bits);
	return 0;
}

static int find_input_device(struct system_devices* devices) {
	struct touchscreen_device* touch_dev = devices->touch_dev;
	struct gpiokeys_device* gpio_dev = devices->gpio_dev;
	struct mouse_device* mouse_dev = devices->mouse_dev;
	struct keyboard_device* keyboard_dev = devices->keyboard_dev;

	int c;
	int i;
	int res;
	int pollres;
	int get_time = 0;
	int print_device = 0;
	char *newline = "\n";
	uint16_t get_switch = 0;
	struct input_event event;
	int print_flags = 0;
	int print_flags_set = 0;
	int dont_block = -1;
	int event_count = 0;
	int sync_rate = 0;
	int64_t last_sync_time = 0;
	const char *device = NULL;
	const char *device_path = "/dev/input";
	int ret = 3;

	if (!touch_dev || !gpio_dev || !mouse_dev) {
		fprintf(stderr, "%s :: null argument\n", __FUNCTION__);
		return -1;
	}

	nfds = 1;
	ufds = calloc(1, sizeof(ufds[0]));
	ufds[0].fd = inotify_init();
	ufds[0].events = POLLIN;
	print_device = 1;

	res = inotify_add_watch(ufds[0].fd, device_path, IN_DELETE | IN_CREATE);
	if (res < 0) {
		fprintf(stderr, "could not add watch for %s, %s\n", device_path,
				strerror(errno));
		return 1;
	}
	res = scan_dir(device_path, print_flags);
	if (res < 0) {
		fprintf(stderr, "scan dir failed for %s\n", device_path);
		return 2;
	}

	printf("device scan results:\n");
	for (i = 1; i < nfds; i++) {
		int fd = ufds[i].fd;
		char name[80];

		name[sizeof(name) - 1] = '\0';
		ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name);

		printf("dev name #%d: %s\n", i, device_names[i]);
		if (strstr(name, MOUSE_DEV_NAME)) {
			struct stat st;
			dev_t dev;
			mouse_dev->fd = fd;
			mouse_dev->dev_name = device_names[i];
			printf("device %s found: %s\n", name, mouse_dev->dev_name);
			do {
				if (stat(MOUSE_DEV_PATH, &st) != 0) {
					dev = makedev(60, 0);
					if (mknod(MOUSE_DEV_PATH, S_IFCHR | S_IWUSR, dev) == -1) {
						LOG_E(
								"%s: Could not create a mouse device file (errno=%d)\n",
								__FUNCTION__, errno);
						break;
					}
				}
				if ((mouse_dev->fd = open(MOUSE_DEV_PATH, O_WRONLY)) == -1) {
					LOG_E("%s: could not open mouse file (errno=%d)\n",
							__FUNCTION__, errno);
					break;
				}
				ret--;
			} while (0);
			continue;
		}
		if (strstr(name, KEYBOARD_DEV_NAME)) {
			struct stat st;
			dev_t dev;
			keyboard_dev->fd = fd;
			keyboard_dev->dev_name = device_names[i];
			printf("device %s found: %s\n", name, keyboard_dev->dev_name);
			do {
				if (stat(KEYBOARD_DEV_PATH, &st) != 0) {
					dev = makedev(61, 0);
					if (mknod(KEYBOARD_DEV_PATH, S_IFCHR | S_IWUSR, dev)
							== -1) {
						LOG_E(
								"%s: Could not create a keyboard device file (errno=%d)\n",
								__FUNCTION__, errno);
						break;
					}
				}
				if ((keyboard_dev->fd = open(KEYBOARD_DEV_PATH, O_WRONLY))
						== -1) {
					LOG_E("%s: could not open keyboard file (errno=%d)\n",
							__FUNCTION__, errno);
					break;
				}
				ret--;
			} while (0);
			continue;
		}
		if (strstr(name, GPIO_KEYS_NAME)) {
			//gpiokeys_device // dev_name // fd
			gpio_dev->fd = fd;
			gpio_dev->dev_name = device_names[i];
			printf("device %s found: %s\n", name, gpio_dev->dev_name);
			ret--;
			continue;
		}
		if (strstr(name, TOUCHSCREEN_DEV_NAME)) {
			touch_dev->fd = fd;
			touch_dev->dev_name = device_names[i];
			printf("device %s found: %s\n", name, touch_dev->dev_name);

			touch_dev->location[sizeof(touch_dev->location) - 1] = '\0';
			ioctl(fd, EVIOCGPHYS(sizeof(touch_dev->location) - 1),
					&touch_dev->location);
			touch_dev->idstr[sizeof(touch_dev->idstr) - 1] = '\0';
			ioctl(fd, EVIOCGUNIQ(sizeof(touch_dev->idstr) - 1),
					&touch_dev->idstr);
			ioctl(fd, EVIOCGVERSION, &touch_dev->version);
			ioctl(fd, EVIOCGID, &touch_dev->id);

			if (touchscreen_get_events_prop(touch_dev)) {
				fprintf(stderr, "could not get touchscreen props\n");
				return 1;
			}

			if (!touch_dev->prop.abs_x.resolution)
				touch_dev->prop.abs_x.resolution = TOUCHSCREEN_RESOLUTION_X;
			if (!touch_dev->prop.abs_y.resolution)
				touch_dev->prop.abs_y.resolution = TOUCHSCREEN_RESOLUTION_Y;

			printf("touchscreen key value: %04x\n", touch_dev->prop.key_desc);
			printf(
					"touchscreen abs_x prop: value %d, min %d, max %d, fuzz %d, flat %d, resolution %d\n",
					touch_dev->prop.abs_x.value, touch_dev->prop.abs_x.minimum,
					touch_dev->prop.abs_x.maximum, touch_dev->prop.abs_x.fuzz,
					touch_dev->prop.abs_x.flat,
					touch_dev->prop.abs_x.resolution);
			printf(
					"touchscreen abs_y prop: value %d, min %d, max %d, fuzz %d, flat %d, resolution %d\n",
					touch_dev->prop.abs_y.value, touch_dev->prop.abs_y.minimum,
					touch_dev->prop.abs_y.maximum, touch_dev->prop.abs_y.fuzz,
					touch_dev->prop.abs_y.flat,
					touch_dev->prop.abs_y.resolution);
			printf(
					"touchscreen abs_pressure prop: value %d, min %d, max %d, fuzz %d, flat %d, resolution %d\n",
					touch_dev->prop.abs_pressure.value,
					touch_dev->prop.abs_pressure.minimum,
					touch_dev->prop.abs_pressure.maximum,
					touch_dev->prop.abs_pressure.fuzz,
					touch_dev->prop.abs_pressure.flat,
					touch_dev->prop.abs_pressure.resolution);

			print_flags |= PRINT_ALL_INFO;
			printf("  bus:      %04x\n"
					"  vendor    %04x\n"
					"  product   %04x\n"
					"  version   %04x\n", touch_dev->id.bustype,
					touch_dev->id.vendor, touch_dev->id.product,
					touch_dev->id.version);

			printf("  name:     \"%s\"\n", name);
			printf("  location: \"%s\"\n"
					"  id:       \"%s\"\n", touch_dev->location,
					touch_dev->idstr);
			printf("  version:  %d.%d.%d\n", touch_dev->version >> 16,
					(touch_dev->version >> 8) & 0xff,
					touch_dev->version & 0xff);
			print_possible_events(fd, print_flags);
			print_input_props(fd);
			print_hid_descriptor(touch_dev->id.bustype, touch_dev->id.vendor,
					touch_dev->id.product);
			ret--;
			continue;
		}
	}
	return ret; // non zero if not found
}

static int sendevent(struct dummy_dev* pdev, struct input_event* events,
		int count) {
	int i;
	int fd = pdev->fd;
	int version;

	if (fd < 0) {
		fprintf(stderr, "could not use %s, %s\n", pdev->dev_name,
				strerror(errno));
		return 1;
	}
	for (i = 0; i < count; i++) {
		struct input_event* event = &events[i];
		int ret = write(fd, event, sizeof(struct input_event));
		if (ret < (int) sizeof(struct input_event)) {
			fprintf(stderr, "write event failed, %s\n", strerror(errno));
			LOG_E("write event failed, %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

static int send_mouse_event(int fd, const unsigned char type, const int Xvalue,
		const int Yvalue) {
	char write_buffer[sizeof(char) + 2 * sizeof(int)];
	memcpy(write_buffer, &type, sizeof(char));
	memcpy(write_buffer + sizeof(char), &Xvalue, sizeof(int));
	memcpy(write_buffer + sizeof(char) + sizeof(int), &Yvalue, sizeof(int));
	if (write(fd, write_buffer, sizeof(write_buffer)) != sizeof(write_buffer)) {
		LOG_E("%s write() error\n", __FUNCTION__);
		return -1;
	}
	return 0;
}

static int send_keyboard_event(int fd, const unsigned char key,
		const unsigned char value) {
	unsigned char write_buffer[2];
	write_buffer[0] = key;
	write_buffer[1] = value;
	if (write(fd, write_buffer, sizeof(write_buffer)) != sizeof(write_buffer)) {
		LOG_E("%s write() error\n", __FUNCTION__);
		return -1;
	}
	return 0;
}
static void shiftMod(int fd, int press)
{
	if(!send_keyboard_event(fd, AVKBD_KEY_LEFTSHIFT, press)) {
		LOG_E("%c write() error\n",AVKBD_KEY_LEFTSHIFT);
	}
}

static void pressAndRelease(int fd, const unsigned char key){
	if(!send_keyboard_event(fd,key,1)) {
		LOG_E("%c write() error\n",key);
	}
	if(!send_keyboard_event(fd,key,0)) {
		LOG_E("%c write() error\n",key);
	}
}

static void sendKey(int fd, const char key){
	switch (key) {
		case ' ':
			pressAndRelease(fd,AVKBD_KEY_SPACE);
		break;

		case '1':
			pressAndRelease(fd,AVKBD_KEY_1);
		break;

		case '2':
			pressAndRelease(fd,AVKBD_KEY_2);
		break;

		case '3':
			pressAndRelease(fd,AVKBD_KEY_3);
		break;

		case '4':
			pressAndRelease(fd,AVKBD_KEY_4);
		break;

		case '5':
			pressAndRelease(fd,AVKBD_KEY_5);
		break;

		case '6':
			pressAndRelease(fd,AVKBD_KEY_6);
		break;

		case '7':
			pressAndRelease(fd,AVKBD_KEY_7);
		break;

		case '8':
			pressAndRelease(fd,AVKBD_KEY_8);
		break;

		case '9':
			pressAndRelease(fd,AVKBD_KEY_9);
		break;

		case '0':
			pressAndRelease(fd,AVKBD_KEY_0);
		break;

		case 'q':
			pressAndRelease(fd,AVKBD_KEY_Q);
		break;

		case 'w':
			pressAndRelease(fd,AVKBD_KEY_W);
		break;

		case 'e':
			pressAndRelease(fd,AVKBD_KEY_E);
		break;

		case 'r':
			pressAndRelease(fd,AVKBD_KEY_R);
		break;

		case 't':
			pressAndRelease(fd,AVKBD_KEY_T);
		break;

		case 'y':
			pressAndRelease(fd,AVKBD_KEY_Y);
		break;

		case 'u':
			pressAndRelease(fd,AVKBD_KEY_U);
		break;

		case 'i':
			pressAndRelease(fd,AVKBD_KEY_I);
		break;

		case 'o':
			pressAndRelease(fd,AVKBD_KEY_O);
		break;

		case 'p':
			pressAndRelease(fd,AVKBD_KEY_P);
		break;

		case 'a':
			pressAndRelease(fd,AVKBD_KEY_A);
		break;

		case 's':
			pressAndRelease(fd,AVKBD_KEY_S);
		break;

		case 'd':
			pressAndRelease(fd,AVKBD_KEY_D);
		break;

		case 'f':
			pressAndRelease(fd,AVKBD_KEY_F);
		break;

		case 'g':
			pressAndRelease(fd,AVKBD_KEY_G);
		break;

		case 'h':
			pressAndRelease(fd,AVKBD_KEY_H);
		break;

		case 'j':
			pressAndRelease(fd,AVKBD_KEY_J);
		break;

		case 'k':
			pressAndRelease(fd,AVKBD_KEY_K);
		break;

		case 'l':
			pressAndRelease(fd,AVKBD_KEY_L);
		break;

		case 'z':
			pressAndRelease(fd,AVKBD_KEY_Z);
		break;

		case 'x':
			pressAndRelease(fd,AVKBD_KEY_X);
		break;

		case 'c':
			pressAndRelease(fd,AVKBD_KEY_C);
		break;

		case 'v':
			pressAndRelease(fd,AVKBD_KEY_V);
		break;

		case 'b':
			pressAndRelease(fd,AVKBD_KEY_B);
		break;

		case 'n':
			pressAndRelease(fd,AVKBD_KEY_N);
		break;

		case 'm':
			pressAndRelease(fd,AVKBD_KEY_M);
		break;

		case '-':
			pressAndRelease(fd,AVKBD_KEY_MINUS);
		break;

		case '=':
			pressAndRelease(fd,AVKBD_KEY_EQUAL);
		break;

		case '[':
			pressAndRelease(fd,AVKBD_KEY_LEFTBRACE);
		break;

		case ']':
			pressAndRelease(fd,AVKBD_KEY_RIGHTBRACE);
		break;

		case '\\':
			pressAndRelease(fd,AVKBD_KEY_BACKSLASH);
		break;

		case ';':
			pressAndRelease(fd,AVKBD_KEY_SEMICOLON);
		break;

		case '\'':
			pressAndRelease(fd,AVKBD_KEY_APOSTROPHE);
		break;

		case ',':
			pressAndRelease(fd,AVKBD_KEY_COMMA);
		break;

		case '.':
			pressAndRelease(fd,AVKBD_KEY_DOT);
		break;

		case '/':
			pressAndRelease(fd,AVKBD_KEY_SLASH);
		break;

		case '`':
			pressAndRelease(fd,AVKBD_KEY_GRAVE);
		break;

		case 'A':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_A);
			shiftMod(fd, 0);
		break;

		case 'B':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_B);
			shiftMod(fd, 0);
		break;

		case 'C':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_C);
			shiftMod(fd, 0);
		break;

		case 'D':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_D);
			shiftMod(fd, 0);
		break;

		case 'E':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_E);
			shiftMod(fd, 0);
		break;

		case 'F':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_F);
			shiftMod(fd, 0);
		break;

		case 'G':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_G);
			shiftMod(fd, 0);
		break;

		case 'H':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_H);
			shiftMod(fd, 0);
		break;

		case 'I':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_I);
			shiftMod(fd, 0);
		break;

		case 'J':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_J);
			shiftMod(fd, 0);
		break;

		case 'K':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_K);
			shiftMod(fd, 0);
		break;

		case 'L':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_L);
			shiftMod(fd, 0);
		break;

		case 'M':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_M);
			shiftMod(fd, 0);
		break;

		case 'N':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_N);
			shiftMod(fd, 0);
		break;

		case 'O':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_O);
			shiftMod(fd, 0);
		break;

		case 'P':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_P);
			shiftMod(fd, 0);
		break;

		case 'Q':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_Q);
			shiftMod(fd, 0);
		break;

		case 'R':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_R);
			shiftMod(fd, 0);
		break;

		case 'S':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_S);
			shiftMod(fd, 0);
		break;

		case 'T':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_T);
			shiftMod(fd, 0);
		break;

		case 'U':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_U);
			shiftMod(fd, 0);
		break;

		case 'V':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_V);
			shiftMod(fd, 0);
		break;

		case 'W':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_W);
			shiftMod(fd, 0);
		break;

		case 'X':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_X);
			shiftMod(fd, 0);
		break;

		case 'Y':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_Y);
			shiftMod(fd, 0);
		break;

		case 'Z':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_Z);
			shiftMod(fd, 0);
		break;

		case '!':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_1);
			shiftMod(fd, 0);
		break;

		case '@':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_2);
			shiftMod(fd, 0);
		break;

		case '#':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_3);
			shiftMod(fd, 0);
		break;

		case '$':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_4);
			shiftMod(fd, 0);
		break;

		case '%':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_5);
			shiftMod(fd, 0);
		break;

		case '^':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_6);
			shiftMod(fd, 0);
		break;

		case '&':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_7);
			shiftMod(fd, 0);
		break;

		case '*':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_8);
			shiftMod(fd, 0);
		break;

		case '(':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_9);
			shiftMod(fd, 0);
		break;

		case ')':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_0);
			shiftMod(fd, 0);
		break;

		case '_':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_MINUS);
			shiftMod(fd, 0);
		break;

		case '+':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_EQUAL);
			shiftMod(fd, 0);
		break;

		case '{':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_LEFTBRACE);
			shiftMod(fd, 0);
		break;

		case '}':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_RIGHTBRACE);
			shiftMod(fd, 0);
		break;

		case '|':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_BACKSLASH);
			shiftMod(fd, 0);
		break;

		case ':':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_SEMICOLON);
			shiftMod(fd, 0);
		break;

		case '"':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_APOSTROPHE);
			shiftMod(fd, 0);
		break;

		case '<':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_COMMA);
			shiftMod(fd, 0);
		break;

		case '>':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_DOT);
			shiftMod(fd, 0);
		break;

		case '?':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_SLASH);
			shiftMod(fd, 0);
		break;

		case '~':
			shiftMod(fd, 1);
			pressAndRelease(fd,AVKBD_KEY_GRAVE);
			shiftMod(fd, 0);
		break;

		case '\n':
			pressAndRelease(fd,AVKBD_KEY_ENTER);
		break;

		default:
			LOG_E("Error - chracter is not supported\n");
		break;
	}
}

static uint32_t convert_touch_value(float val, struct input_absinfo info,
		uint16_t resolution, float resolution_factor) {
	uint32_t ret = val * (info.maximum - info.minimum + 1) * resolution_factor
			/ resolution + info.minimum;
	LOG_V("convertion result: %d, val: %f min: %d max: %d res: %d res_factor: %f\n",
			ret, val, info.minimum, info.maximum, resolution, resolution_factor);
	return ret;
}

static void adjust_resolution_factor(struct touchscreen_device* touch_dev) {
	resolution_factor_x = ((float) ((float) touch_dev->prop.abs_x.resolution
			/ (float) client_pad_width));
	resolution_factor_y = ((float) ((float) touch_dev->prop.abs_y.resolution
			/ (float) client_pad_height));
	LOG_V("resolution factor calculated: %f, %f (from %d/%d, %d/%d)\n",
			resolution_factor_x, resolution_factor_y,
			touch_dev->prop.abs_x.resolution, client_pad_width,
			touch_dev->prop.abs_y.resolution, client_pad_height);
}

#if RUN_PD40_BACKGROUND_SERVICES
/*
 * service to be started using command "am startservice":
 * com.tandemg.pd40_background_service/
 * com.tandemg.pd40_background_service.PD40LocationService
 */

static const char* bgs_pkg = "com.tandemg.pd40_background_service";
static const char* bgs_name = "PD40LocationService";

static void start_pd40background_services() {
	char cmd[256];
	LOG_I("starting pd40 background services\n");

	sprintf(cmd, "am startservice %s/%s.%s", bgs_pkg, bgs_pkg, bgs_name);
	system(cmd);
}

static void ping_pd40background_services() {
	start_pd40background_services();
}

static void send_intent_pd40background_services(const char* args) {
	char cmd[1024];

	sprintf(cmd, "am startservice %s %s/%s.%s", args, bgs_pkg, bgs_pkg,
			bgs_name);
	system(cmd);
}

#else

static void start_pd40background_services() {}
static void ping_pd40background_services() {}
static void send_intent_pd40background_services(const char *args) {}
#endif

static int send_location_event(const char* buf) {
    char provider[32];
    long time;
    int scanfedValues = 0;
    double latitude, longitude, altitude;
    float accuracy, bearing, speed;
    char cmd[512];
    int l = 0;

	LOG_D("location details: %s\n", buf);
   scanfedValues = sscanf(buf, "%s %ld %lf %lf %lf %f %f %f", provider, &time,
            &latitude, &longitude, &altitude, &accuracy, &bearing, &speed);
    // "adb shell am startservice -a MockLocationReady -n com.tandemg.pd40locationmocker/.LocationMocker --es Provider gps --el Time 1685211000 --ef Latitude 26.9289183 --ef Longitude 147.7470706 --ef Altitude 46.5797731 --ef Accuracy 402.000000 --ef Bearing 0.000000 --ef Speed 0.000000"
    if(scanfedValues != 8){
        LOG_E("scanned: %d != 8 \n Failing to inject GPS Location",
            scanfedValues);
        return -1;
    }
    l += sprintf(cmd + l, "--es Provider %s ", provider);
    l += sprintf(cmd + l, "--el Time %ld ", time);
    l += sprintf(cmd + l, "--ef Latitude %lf ", latitude);
    l += sprintf(cmd + l, "--ef Longitude %lf ", longitude);
    l += sprintf(cmd + l, "--ef Altitude %lf ", altitude);
    l += sprintf(cmd + l, "--ef Accuracy %f ", accuracy);
    l += sprintf(cmd + l, "--ef Bearing %f ", bearing);
    l += sprintf(cmd + l, "--ef Speed %f ", speed);

    start_activity(START_SERVICE, "MockLocationReady", "com.tandemg.pd40locationmocker", "LocationMocker" , cmd , l);
    return 0;
}

static int update_brightness_level(int value) { //returns the nmber of chars writen(should be 1-3)
	int ret = 0;
	char cmd[1024];
	LOG_D("in update_brightness_level value=%d\n", value);
	const char * brightnessFile = LCD_BACKLIGHT_PATH "/" LCD_BRIGHTNESS_LEVEL;
	FILE * file = fopen(brightnessFile, "w");
	if (!file) {
		LOG_E("Failed opening file: %s \n", brightnessFile);
		return -1;
	}
	ret = fprintf(file, "%d\n", value);
	fflush(file);
	fclose(file);
	sprintf(cmd,
			"sqlite3 /data/data/com.android.providers.settings/databases/settings.db \"update system set value=%d where name=\'screen_brightness\';\"",
			value);
	system(cmd);
	return ret;
}

static int get_brightness_level() { //returns 0-255 brightness value
	int ret;
	const char * brightnessFile = LCD_BACKLIGHT_PATH "/" LCD_BRIGHTNESS_LEVEL;
	FILE * file = fopen(brightnessFile, "r");
	if (!file) {
		LOG_E("Failed opening file: %s \n", brightnessFile);
		return -1;
	}
	fscanf(file, "%d", &ret);
	fclose(file);
	return ret;
}

static int get_battery_level() { //returns 0-100 battery capacity value
	int ret;
	const char * batteryFile = PWR_SUPPLY_PATH "/" PWR_SUPPLY_CAPACITY;
	FILE * file = fopen(batteryFile, "r");
	if (!file) {
		LOG_E("Failed opening battery capacity file: %s\n", batteryFile);
		return -1;
	}
	LOG_I("Battery capacity: %d \n", ret);
	fscanf(file, "%d", &ret);
	fclose(file);
	return ret;
}

static int is_battery_charging() { //returns 1 while charger is connected
	int ret = 0;
	char output[15];
	const char * batteryStatusFilePath = PWR_SUPPLY_PATH "/" PWR_SUPPLY_STATUS;
	FILE * statusFile = fopen(batteryStatusFilePath, "r");
	if (!statusFile) {
		LOG_E("Failed opening battery status file: %s\n", batteryStatusFilePath);
		return -1;
	}
	fscanf(statusFile, "%s", output);
	fclose(statusFile);
	/*file contains either "Discharging" or "Charging"*/
	LOG_I("Charger status: %s \n", output);
	if(output[0] == 'C') { // 'C' denotes "Charging"
		ret = 1;
	}
	return ret;
}

int start_activity(am_subcommand subCmd ,const char* action,
						  const char* package,
						  const char* activity , const char * extras, int extrasLength) {
	char cmd[512];
	char *p = cmd;

	if (!package || !activity) {
		LOG_E("%s :: invalid arguments, pkg: %s, app: %s\n",
				__FUNCTION__, package, activity);
		return -1;
	}

	p += sprintf(p, "am ");
	switch (subCmd)
	{
		case START:
			p += sprintf(p, "start -S ");
			break;
		case BROADCAST:
			p += sprintf(p, "broadcast ");
			break;
		case START_SERVICE:
			p += sprintf(p, "startservice ");
			break;
	}
	if(action) {
		p += sprintf(p, "-a %s ", action);
	}
	if( *package!=0 && *activity != 0 ) {
		p += sprintf(p, "-n %s/.%s -S ", package, activity);
	}
	if( *extras != 0) {
        p += sprintf(p, "%.*s", extrasLength, extras);
	}
	LOG_D("%s :: command to execute: %s\n", __FUNCTION__, cmd);
	system(cmd);
	return 0;
}

static int handle_request(struct system_devices* devices, const char* req,
		int req_len, char * out_args) {
	struct touchscreen_device* touch_dev = devices->touch_dev;
	struct gpiokeys_device* gpio_dev = devices->gpio_dev;
	struct mouse_device* mouse_dev = devices->mouse_dev;
	struct keyboard_device* keyboard_dev = devices->keyboard_dev;
	struct input_event events[5];
	float x, y, pressure;
	int mtype, mx, my;
	int brightness;
	int kkey, kvalue;
	const char * charIndex;
	int count;
	const char* p = req;
	int ret = 0;
    int optionsAsciiValue = 0;
    char cmd[30];
    int asciiValue = 0;

	while (1) {
		const char* eol = strstr(p, "\n");
		int len;

		if (!eol) {
			LOG_E("can't find EOL in request");
			return -1;
		}
		len = (eol - p);
		if (len > req_len) {
			LOG_E("EOL is found beyond req_len");
			return -2;
		} LOG_V("handling message (length: %d)\n", len);

		memset(events, 0, sizeof(events));
		if (!len) {
			LOG_E("args error (empty request)\n");
			return -6;
		}
		if (len < 1) {
			LOG_E("args error, len: %d\n", len);
			return -3;
		}
		switch (*p) {
		case 'd':
			sscanf(p + 2, "%d %d", &client_pad_height, &client_pad_width);
			adjust_resolution_factor(touch_dev);
			LOG_D("Dimensions received, height: %d, width: %d\n",
					client_pad_height, client_pad_width);
			break;
		case 'D':
			sscanf(p + 2, "%f %f %f", &x, &y, &pressure);
			LOG_D("DOWN: %f %f %f\n", x, y, pressure);

			events[0].type = EV_KEY;
			events[0].code = BTN_TOUCH;
			events[0].value = 1; // DOWN
			events[1].type = EV_ABS;
			events[1].code = ABS_X;
			events[1].value = convert_touch_value(x, touch_dev->prop.abs_x,
					touch_dev->prop.abs_x.resolution, resolution_factor_x); //TOUCHSCREEN_RESOLUTION_X
			events[2].type = EV_ABS;
			events[2].code = ABS_Y;
			events[2].value = convert_touch_value(y, touch_dev->prop.abs_y,
					touch_dev->prop.abs_y.resolution, resolution_factor_y); //TOUCHSCREEN_RESOLUTION_Y
			events[3].type = EV_ABS;
			events[3].code = ABS_PRESSURE;
			events[3].value = convert_touch_value(pressure,
					touch_dev->prop.abs_pressure,
					TOUCHSCREEN_RESOLUTION_PRESSURE, 1);
			events[4].type = EV_SYN;
			events[4].code = SYN_REPORT;
			events[4].value = 0;
			count = 5;
			ret = sendevent((struct dummy_dev*) touch_dev, events, count);
			break;
		case 'M':
			sscanf(p + 2, "%f %f %f", &x, &y, &pressure);
			LOG_D("MOVE: %f %f %f\n", x, y, pressure);

			events[0].type = EV_ABS;
			events[0].code = ABS_X;
			events[0].value = convert_touch_value(x, touch_dev->prop.abs_x,
					touch_dev->prop.abs_x.resolution, resolution_factor_x); //TOUCHSCREEN_RESOLUTION_X
			events[1].type = EV_ABS;
			events[1].code = ABS_Y;
			events[1].value = convert_touch_value(y, touch_dev->prop.abs_y,
					touch_dev->prop.abs_y.resolution, resolution_factor_y); //TOUCHSCREEN_RESOLUTION_Y
			events[2].type = EV_ABS;
			events[2].code = ABS_PRESSURE;
			events[2].value = convert_touch_value(pressure,
					touch_dev->prop.abs_pressure,
					TOUCHSCREEN_RESOLUTION_PRESSURE, 1);
			events[3].type = EV_SYN;
			events[3].code = SYN_REPORT;
			events[3].value = 0;
			count = 4;
			ret = sendevent((struct dummy_dev*) touch_dev, events, count);
			break;
		case 'U':
			sscanf(p + 2, "%f %f %f", &x, &y, &pressure);
			LOG_D("UP: %f\n", pressure);

			events[0].type = EV_KEY;
			events[0].code = BTN_TOUCH;
			events[0].value = 0; // UP
			events[1].type = EV_ABS;
			events[1].code = ABS_PRESSURE;
			events[1].value = convert_touch_value(pressure,
					touch_dev->prop.abs_pressure,
					TOUCHSCREEN_RESOLUTION_PRESSURE, 1);
			events[2].type = EV_SYN;
			events[2].code = SYN_REPORT;
			events[2].value = 0;
			count = 3;
			ret = sendevent((struct dummy_dev*) touch_dev, events, count);
			break;
		case 'B':
			LOG_D("BACK\n");

			events[0].type = EV_KEY;
			events[0].code = KEY_BACK;
			events[0].value = 1; // DOWN
			events[1].type = EV_SYN;
			events[1].code = SYN_REPORT;
			events[1].value = 0;
			count = 2;
			ret = sendevent((struct dummy_dev*) gpio_dev, events, count);
			events[0].value = 0; // UP
			ret |= sendevent((struct dummy_dev*) gpio_dev, events, count);
			break;
		case 'w': //press power button
			LOG_D("PWR_PRESS, key_code: %d\n ",KEY_POWER);
			events[0].type = EV_KEY;
			events[0].code = KEY_POWER;
			events[0].value = 1; // Press
			events[1].type = EV_SYN;
			events[1].code = SYN_REPORT;
			events[1].value = 0;
			count = 2;
			ret = sendevent((struct dummy_dev*) gpio_dev, events, count);
			break;
		case 'W': //Release power button
			LOG_D("PWR_RELEASE, key_code: %d\n ",KEY_POWER);
			events[0].type = EV_KEY;
			events[0].code = KEY_POWER;
			events[0].value = 0; // Release
			events[1].type = EV_SYN;
			events[1].code = SYN_REPORT;
			events[1].value = 0;
			count = 2;
			ret = sendevent((struct dummy_dev*) gpio_dev, events, count);
			break;
		case 'H':
			LOG_D("HOME\n");

			send_keyboard_event(keyboard_dev->fd, 99, 1);
			send_keyboard_event(keyboard_dev->fd, 99, 0);
			break;
		case 'O':
            if(sscanf(p + 2,"%d" ,&optionsAsciiValue) > 0)
            {
                LOG_D("Received a keyevent command, with value: %d\n", optionsAsciiValue);
                sprintf(cmd, "input keyevent %d\n", optionsAsciiValue);
            }
            else
            { //backward compatability to the old 'O' event;
                LOG_D("Received a keyevent command failing to Options-Btn\n");
                system("input keyevent 82");
            }
            break;
		case 'k':
			LOG_D("KEYBOARD event looper\n");
			for(charIndex = p+2; *charIndex != '\n' ; charIndex++)
			{
				LOG_D("keyboard event, key=%c \n", *(charIndex));
				sendKey(keyboard_dev->fd , *(charIndex));
			}
			break;

        case 'v':
            // AVKBD_KEY_VOLUMEUP 93
            // AVKBD_KEY_VOLUMEDOWN 94
            // AVKBD_KEY_MUTE 95
            if(sscanf(p + 2,"%d" ,&asciiValue) > 0)
            {
                LOG_D("Received a Generic keyboard event with value: %d\n", asciiValue);
                pressAndRelease(keyboard_dev->fd, asciiValue);
            }
            else
            {
                LOG_E("Error Parsing Generic Keyboard event value.");
            }
            break;
		case 'm':
			sscanf(p + 2, "%d %d %d", &mtype, &mx, &my);
			send_mouse_event(mouse_dev->fd, (unsigned char) mtype, mx, my);
			break;
		case 'L':
			LOG_D("LOCATION event\n");

			send_location_event(p + 2);
			break;
		case 'P':
			/* Syntax:
			 * req: "P [requested value 0-255]"
			 * res: "P [1|0 - 1 for success ] [current Value]"
			 */
			sscanf(p + 2, "%d", &brightness);
			LOG_D("BRIGHTNESS_SET event, brightness=%d\n", brightness);
			if(0 > update_brightness_level(brightness))
			{
				LOG_E("Failed to set brightness level");
				ret = sprintf(out_args, "P 0 %d", (char) get_brightness_level());
				return -1;
			}
			ret = sprintf(out_args, "P 1 %d", (char) get_brightness_level());
			return ret;
			break;
		case 'G':
			LOG_D("BRIGHTNESS_GET event\n");
			if (!out_args) {
				LOG_D("missing output buffer\n");
				return -5;
			}
			ret = sprintf(out_args, "G %d", (char) get_brightness_level());
			return ret;
		case 'S':
			LOG_D("BATTERY_SET event\n");
			if (!out_args) {
				LOG_D("missing output buffer\n");
				return -5;
			}
			ret = sprintf(out_args, "S %d %d", (char) get_battery_level(),(char) is_battery_charging());
			return ret;
		case 'I': {
				char action[64] = {0};
				char package[64] = {0};
				char activity[64] = {0};
				sscanf(p+2, "%s %s %s", action, package, activity);
				LOG_D("execute intent, package:%s, activity:%s, action:%s\n",
						package, activity, action);
				start_activity(START,action, package, activity, NULL,0);
			}
			break;
		case 'b': {
				char action[64] = {0};
				char package[64] = {0};
				char activity[64] = {0};
				sscanf(p+2, "%s %s %s", action, package, activity);
				LOG_D("execute broadcast with intent, package:%s, activity:%s, action:%s\n",
						package, activity, action);
				start_activity(BROADCAST,action, package, activity, NULL,0);
			}
			break;
		case 's': {
				char action[64] = {0};
				char package[64] = {0};
				char activity[64] = {0};
				sscanf(p+2, "%s %s %s", action, package, activity);
				LOG_D("execute broadcast with intent, package:%s, activity:%s, action:%s\n",
						package, activity, action);
				start_activity(START_SERVICE,action, package, activity, NULL,0);
			}
			break;

		default:
			LOG_E("args error\n");
			return -4;
		}

		p = eol + 1;
		if (*p == '\0')
			break;
	}
	return ret;
}

static void nonblock(int sockfd) {
	int opts;
	opts = fcntl(sockfd, F_GETFL);
	if (opts < 0) {
		LOG_E("fcntl(F_GETFL) failed\n");
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(sockfd, F_SETFL, opts) < 0) {
		LOG_E("fcntl(F_SETFL) failed\n");
	}
}

static int start_server(int* psockfd, int portno) {
	int sockfd;
	int n, i;
	int one = 1;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		LOG_E("ERROR create socket");
		return 1;
	}
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one); //allow reuse of port
	//bind to a local address
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		LOG_E("ERROR on bind\n");
		return 2;
	}
	*psockfd = sockfd;
	LOG_I("server started\n");
	return 0;
}

static int wait_for_connection(int sockfd, int* pnewsockfd) {
	int newsockfd;

	int n, i;
	int one = 1;
	//listen marks the socket as passive socket listening to incoming connections, 
	//it allows max 5 backlog connections: backlog connections are pending in queue
	//if pending connections are more than 5, later request may be ignored
	listen(sockfd, TOUCH_SRV_MAX_BACKLOG_CONNECTIONS);
	//accept incoming connections
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	//nonblock(newsockfd);        //if we want to set the socket as nonblock, we can uncomment this
	if (newsockfd < 0) {
		LOG_E("ERROR on accept\n");
		return -1;
	}
	*pnewsockfd = newsockfd;
	LOG_I("connection accepted\n");
	return 0;
}

static int close_server(int sockfd, int newsockfd) {
	close(newsockfd);
	close(sockfd);
	LOG_I("Connection closed\n");
	return 0;
}

int daemonize_process() {
	pid_t process_id = 0;
	pid_t sid = 0;
	// Create child process
	process_id = fork();
	// Indication of fork() failure
	if (process_id < 0) {
		printf("fork failed!\n");
		// Return failure in exit status
		return 1;
	}
	// PARENT PROCESS. Need to kill it.
	if (process_id > 0) {
		printf("process_id of child process %d \n", process_id);
		// return success in exit status
		exit(EXIT_CODE_OKAY);
	}
	//unmask the file mode
	umask(0);
	//set new session
	sid = setsid();
	if (sid < 0) {
		// Return failure
		return 2;
	}
	return 0;
}

int touch_event_srv_main(int argc, char *argv[]) {
	char req[TOUCH_SRV_SOCKET_BUFF_SIZE];
	char res[TOUCH_SRV_SOCKET_BUFF_SIZE];
	struct touchscreen_device touchscreen;
	struct gpiokeys_device gpio;
	struct mouse_device mouse;
	struct keyboard_device keyboard;
	int sockfd, newsockfd;
	int i, n;
	struct system_devices devices;
	int ret = EXIT_CODE_OKAY;
	devices.touch_dev = &touchscreen;
	devices.gpio_dev = &gpio;
	devices.mouse_dev = &mouse;
	devices.keyboard_dev = &keyboard;

	if (daemonize && daemonize_process() != 0) {
		printf("unable to daemonize process, exiting");
		exit(EXIT_CODE_DAEMONIZE);
	}

	running = 1;

	if (find_input_device(&devices)) {
		LOG_E("could not find %s device, exiting...\n", TOUCHSCREEN_DEV_NAME);
		exit(EXIT_CODE_NO_INPUT_DEVICE);
	}
	adjust_resolution_factor(&touchscreen);
	start_pd40background_services();

	do {
		if (start_server(&sockfd, TOUCH_SRV_PORTNO)) {
			LOG_E("could not start server on port %d, exiting...\n",
					TOUCH_SRV_PORTNO);
			ret = EXIT_CODE_SOCKET_ERROR;
			break;
		}
		ping_pd40background_services();
		if (wait_for_connection(sockfd, &newsockfd)) {
			LOG_E("wait for connection failed, exiting...\n");
			ret = EXIT_CODE_CONNECTION_FAILURE;
			break;
		}
		ping_pd40background_services();
		while (1) {
			bzero(req, TOUCH_SRV_SOCKET_BUFF_SIZE);
			n = read(newsockfd, req, TOUCH_SRV_SOCKET_BUFF_SIZE);
			if (n < 0) {
				LOG_E("ERROR read from socket, restarting server\n");
				running = 0;
				break;
			}
			if (!n) {
				LOG_E("socket closed, restarting server\n");
				break;
			} LOG_V("received %d bytes:\n%s\n\n", n, req);

			n = handle_request(&devices, req, n, res);
			if (n < 0) {
				LOG_E(
						"handle request returned with error: %d, request ignored: %s\n",
						n,req);
				continue;
			} else {
				n = write(newsockfd, res, n);
				if (n < 0) {
					LOG_E("ERROR write to socket, restarting server\n");
					break;
				} LOG_V("sent %d bytes back: %s\n", n, res);
			}
		}
		close_server(sockfd, newsockfd);
	} while (running);

	close(devices.mouse_dev->fd);
	close(devices.gpio_dev->fd);
	close(devices.touch_dev->fd);
	close(devices.keyboard_dev->fd);

	return ret;
}
