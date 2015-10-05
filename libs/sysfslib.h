/**
 * sysfslib.h
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 *
 * Trivial abstraction to simplify reading/writing to files, primarily used for accessing SYSFS files on BeagleBone.
 */

#ifndef _SYSFSLIB_H_INCLUDED
#define _SYSFSLIB_H_INCLUDED

/**
 * This is the BeagleBone's CapeMgr cape loading virtual file. Writing to it causes CapeMgr to try and load a cape,
 * reading from it enumerates the loaded capes.
 *
 * The integer (9 in the example string below) may change depending on your BeagleBone. Have a look in the directory
 * and see what it needs to be for your setup.
 */
#define SLOTS_FILE          "/sys/devices/bone_capemgr.9/slots"

int  sysfs_write(const char *filename, const char *str);
int  sysfs_read(const char *filename, char *buf, int maxlen);
int  sysfs_open_read(const char *filename, int flags);
void sysfs_close(int fd);

#endif // _SYSFSLIB_H_INCLUDED
