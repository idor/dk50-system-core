#!/system/bin/sh
FACTORY_CONF_PATH=/system/etc/display/offset.conf
USER_CONF_PATH=/data/misc/display/offset.conf
OFFSET_SYSFS_ENTRY=/sys/class/graphics/fb0/lumus_offset
DISPLAY_ENABLE_SYSFS_ENTRY=/sys/class/graphics/fb0/lumus_conf_loaded
THREE_DEE_ENABLE_SYSFS_ENTRY=/sys/class/graphics/fb0/lumus_resolution_double
TAG="load_display_conf"

echo "trying user_conf"
cat $USER_CONF_PATH > $OFFSET_SYSFS_ENTRY # push
if [ "$(cat $OFFSET_SYSFS_ENTRY)" == "$(cat $USER_CONF_PATH)" ];
#    if [ $new_offset = $current_offset ]; # make sure pushed and probe agree
then # start display system
    echo "$TAG: offset loaded from USER: $USER_CONF_PATH"
    echo 1 > $DISPLAY_ENABLE_SYSFS_ENTRY
    exit
else # else, do the same with factory file.
    cat $FACTORY_CONF_PATH > $OFFSET_SYSFS_ENTRY
    if [ "$(cat $OFFSET_SYSFS_ENTRY)" == "$(cat $FACTORY_CONF_PATH)" ];
    then
        echo "$TAG: offset loaded from FACTORY: $FACTORY_CONF_PATH"
        echo 1 > $DISPLAY_ENABLE_SYSFS_ENTRY
        exit
    fi
fi
echo "$TAG: offset loaded from DEFAULTS"
