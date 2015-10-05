/**
 * sysfslib.cpp
 *
 * @author <oroboto@oroboto.net>, www.oroboto.net, 2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "sysfslib.h"
#include "logger.h"

/**
 * Open, write, close a file.
 *
 * @param char * filename
 * @param char * str        the NULL terminated string to write
 *
 * @return int
 */
int sysfs_write(const char *filename, const char *str)
{
    int fd;

    fd = open(filename, O_WRONLY);
    if (fd < 0)
    {
    	Logger::getInstance()->error("sysfs::sysfs_write: failed to open %s for writing", filename);
        return fd;
    }

    write(fd, str, strlen(str));
    close(fd);

    return 0;
}

/**
 * Open, read, close a file.
 *
 * @param char * filename
 * @param char * buf        buffer to read into
 * @param int    maxlen     maximum number of bytes to read into buf
 *
 * @return int
 */
int sysfs_read(const char *filename, char *buf, int maxlen)
{
    int fd, bytes_read;

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
    	Logger::getInstance()->error("sysfs::sysfs_read: failed to open %s for reading", filename);
        return fd;
    }

    if ((bytes_read = read(fd, buf, maxlen)) <= 0)
    {
    	Logger::getInstance()->error("sysfs::sysfs_read: failed to read from %s", filename);
    }

    close(fd);

    return bytes_read;
}

/**
 * Open a file for reading and return the open filehandle.
 *
 * @param char * filename
 * @param int    flags
 *
 * @return int
 */
int sysfs_open_read(const char *filename, int flags = O_RDONLY)
{
    int fd;

    fd = open(filename, flags);
    if (fd < 0)
    {
    	Logger::getInstance()->error("sysfs::sysfs_open_read: failed to open %s for reading", filename);
        return fd;
    }

    return fd;
}

/**
 * Close a file that was opened by sysfs_open_read()
 *
 * @param int fd
 */
void sysfs_close(int fd)
{
    close(fd);
}
