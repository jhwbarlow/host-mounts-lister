#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include "log/log.h"

int printfile(int fd) {
  size_t buf_len = BUFSIZ;  
  int buf_pos = 0;

  // Allocate initial buffer
  char *buf = malloc(buf_len);
  if (buf == NULL) {
    log_error("unable to create buffer:  %s (%d)", strerror(errno), errno);
    return -1;
  }
  
  // Loop until EOF
  for (;;) {
    // Write into the buffer at the current buf_pos
    char *write_ptr = buf + buf_pos;
    
    // The next read should be limited to the number of bytes left in the buffer,
    // reserving 1 for the null terminator
    size_t max_to_read = buf_len - buf_pos - 1;
    
    int no_read = read(fd, write_ptr, max_to_read);
    if (no_read == -1) {
      log_error("unable to read from file descriptor:  %s (%d)", strerror(errno), errno);
      free(buf);
      return -1;
    }

    log_debug("buf_len: %d, buf_pos: %d, no read: %d", buf_len, buf_pos, no_read);

    // EOF reached
    if (no_read == 0) { 
      // Reset FD for next read to this same FD
      if (lseek(fd, 0, SEEK_SET) == -1) {
        log_error("unable to reset file descriptor:  %s (%d)", strerror(errno), errno);
        free(buf);
        return -1;
      }

      // Null-terminate the buffer to make it a valid string
      buf[buf_pos] = 0;
      
      log_info("mounts:\n%s", buf);
      free(buf);
      return 0;
    }

    // More to read - move the buf_pos to the new write position, ready for the next read 
    buf_pos = buf_pos + no_read;

    // Enlarge buffer:
    // If the new buf_pos is equal to the current buf_len, then the last read
    // filled the buf completely, so we double its size ready for the next read.
    // We add 1 to the no_read to account for the spare byte that
    // we reserved for the null terminator.
    // Otherwise, the read was less than the buffer had available. No need to enlarge!
    if (buf_pos + 1 == buf_len) {
      // Double the size of the buffer, this could be more intelligent
      buf_len = buf_len * 2;
      char *old_buf = buf;
      buf = realloc(buf, buf_len);
      if (buf == NULL) {
        log_error("unable to reallocate buffer:  %s (%d)", strerror(errno), errno);
        free(old_buf);
        return -1;
      }
    }
  }

  return -1;
}