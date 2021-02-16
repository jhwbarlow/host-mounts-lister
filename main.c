

#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#include "log/log.h"

#include "printfile.h"
#include "watchmounts.h"

int main(int argc, char *argv[]) {
  log_debug("running as: [effective: %d/real: %d]", geteuid(), getuid());
  
  int ns_fd = open("/proc/1/ns/mnt", O_RDONLY);
  if (ns_fd == -1) {
    log_fatal("unable to open mount namespace: %s (%d)", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  if (setns(ns_fd, CLONE_NEWNS) == -1) {
    log_fatal("unable to set mount namespace: %s (%d)", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  if (setuid(getuid()) == -1) {
    log_fatal("unable to drop setuid privileges: %s (%d)", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  log_debug("running as: [effective: %d/real: %d]", geteuid(), getuid());

  if (close(ns_fd) == -1) {
    log_error("unable to close mount namespace: %s (%d)", strerror(errno), errno);
  }

  int mnt_fd = open("/proc/mounts", O_RDONLY);
  if (mnt_fd == -1) {
    log_fatal("unable to open mount file: %s (%d)", strerror(errno), errno);
    exit(EXIT_FAILURE);
  }

  if (printfile(mnt_fd) == -1) {
    log_fatal("unable to print initial mounts");
    exit(EXIT_FAILURE);
  }

  if (watchmounts(mnt_fd) == -1) {
    log_fatal("unable to watch mounts");
    exit(EXIT_FAILURE);
  }
}