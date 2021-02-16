FROM gcc AS builder
# Ensure we are running as root so the compiled binary is owned by root.
# This is because we want it to be setuid-root, but using COPY --chown=0:0
# in the final image does not preserve the setuid bit.
USER 0:0
COPY . /tmp/src/
RUN mkdir /tmp/bin \
  && cd /tmp/src \
  && sources=$(find . -name '*.c' -and \! -name 'main.c') \  
  && gcc -static -o /tmp/bin/mountslister main.c ${sources} \
  && chmod 4555 /tmp/bin/mountslister \
  && ls -lh /tmp/bin/mountslister

FROM busybox AS env-builder
RUN truncate -s0 /etc/passwd /etc/group && \
    adduser -h / -g 'Host Mounts Lister' -s /bin/nologin -D -H -u 10034 host-mounts-lister

FROM scratch
COPY --from=env-builder /etc/passwd /etc/group /etc/
# The binary will run as setuid-root, then drop to this user after initial setup
USER host-mounts-lister:host-mounts-lister
COPY --from=builder /tmp/bin/mountslister /usr/bin/mountslister
ENTRYPOINT [ "/usr/bin/mountslister" ]