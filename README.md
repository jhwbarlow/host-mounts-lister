host-mounts-lister
==================
List mounts on host from within a container.

The mounts (taken from `/proc/mounts`) are listed:
- At startup
- When a mount is added
- When a mount is unmounted

How does this work?
-------------------

The mount namespace of PID 1 is opened (this requires that the container is running in the host PID namespace, for example Docker `--pid=host` or Kubernetes `hostPID: true`).

Once this file descriptor is obtained, it is passed to the `setns()` system call which causes this process to join the mount namespace of the host (any host PID could be used here, but PID 1 will always be available).

Once in the host's mount namespace, a file descriptor to `/proc/mounts` is opened and polled continuously for changes. This "file" has the useful property that it will be marked as having a `POLLPRI` event when the mounts change, allowing its use as a trigger mechanism to list the mounts (see `man 5 proc`).

Capabilities Required
---------------------

- SYS_CHROOT - for the `setns()` system call
- SYS_ADMIN - for the `setns()` system call
- SYS_PTRACE - to be able to open another PID's namespace file descriptors

Docker Example
--------------

```
docker run --pid=host --cap-add=SYS_CHROOT --cap-add=SYS_ADMIN --cap-add=SYS_PTRACE jhwbarlow/host-mounts-lister:0.0.1
```

If using AppArmor (e.g. Ubuntu host), the `setns()` system call will be blocked - use the `--security-opt apparmor=unconfined` flag to disable AppArmor for this container.

Kubernetes Example
------------------

See `host-mounts-lister.yaml`.