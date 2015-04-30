
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/properties.h>

#define FB_DEV_PATH "/sys/devices/virtual/graphics/fb0"
#define FB_3D_NAME "lumus_resolution_double"

static int getBooleanFromSysfs(const char * threeDeeSysfsFile) {
	int ret = 0;

	FILE* file = fopen(threeDeeSysfsFile, "r+");
	if (file == NULL) {
		fprintf(stderr,"could not open %s, %s\n", threeDeeSysfsFile, strerror(errno));
		return -1;
	}
	ret = fgetc(file);
	fclose(file);
	if (ret == '1') {
		return 1;
	}
	return 0;
}

static int toggle_3d_mode() {
 	char cmd[260];
	int nextState;
	const char * threeDeeSysfsFile = FB_DEV_PATH "/" FB_3D_NAME;
	const char * command_format  = "echo %d > \
        /sys/class/graphics/fb0/lumus_resolution_double && echo 1 > \
        /sys/devices/virtual/graphics/fb0/blank && stop surfaceflinger \
        && start surfaceflinger  && sleep 0.3 && echo 0 > \
        /sys/devices/virtual/graphics/fb0/blank";

	nextState = 1 - getBooleanFromSysfs(threeDeeSysfsFile);
	sprintf(cmd, command_format, nextState);
	system(cmd);
	return nextState;
}

int toggle_3d_mode_main(int argc, char *argv[]) {
    return toggle_3d_mode();
}


