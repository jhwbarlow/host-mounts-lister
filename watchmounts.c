#include <errno.h>
#include <poll.h>
#include <string.h>
#include "printfile.h"
#include "log/log.h"

int watchmounts(int mnt_fd) {
  struct pollfd poll_fd = {
    .fd = mnt_fd,
    .events = POLLPRI,  
  };

  for (;;) {
    poll_fd.revents = 0;
    int changed = poll(&poll_fd, 1, -1);
    if (changed == -1) {
      log_error("unable to poll mounts: %s (%d)", strerror(errno), errno);
      return -1;
    } else if (changed == 0) {
      // Should not happen
      continue;
    }
  
    if (poll_fd.revents & POLLPRI) {
      log_info("mounts changed!");

      if (printfile(mnt_fd) == -1) {
        log_error("unable to print mounts");
        return -1;
      }
    }
  }
}