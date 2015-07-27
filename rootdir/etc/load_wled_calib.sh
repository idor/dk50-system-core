#!/system/bin/sh
CALIB_PATH=/data/misc/display/wled_calib.conf
SYSFS_PATH=/sys/class/graphics/fb0/lumus_wled_calib
TAG="load_wled_calib"

if [ -e $CALIB_PATH ];
then # reading file and pushing to sysfs.
    echo "Using values from config file $CALIB_PATH"
    cat $CALIB_PATH | while read line
    do
        echo $line > $SYSFS_PATH
    done
else # else, push default values.
    echo "Using default values (linear)."
    echo "0 0 0 0" > $SYSFS_PATH
    echo "255 1023 1023 1023" > $SYSFS_PATH
    value=0
    while [ $(($value)) -le 254 ];
    do
       value=$(($value + 1));
       echo "$value $((value*4)) $((value*4)) $((value*4))" > $SYSFS_PATH
    done
fi
