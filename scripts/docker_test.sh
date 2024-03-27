#!/bin/bash

ALL_WORKSPACES_DIR=$(pwd)
MOUNT_POINT="/mnt/workspace"

CONTAINER_NAME="container_ubuntu_22"
IMAGE_NAME="ubuntu:22.04"

if ! docker ps -a --filter="name=${CONTAINER_NAME}" | grep -w "${CONTAINER_NAME}"; then
  echo "Creating docker container ${CONTAINER_NAME}"
  docker run --name "${CONTAINER_NAME}" -d \
    -v "${ALL_WORKSPACES_DIR}":${MOUNT_POINT} \
    -t "${IMAGE_NAME}" tee /tmp/runvm.log
elif ! docker ps --filter="name=${CONTAINER_NAME}" | grep -w "${CONTAINER_NAME}"; then
  echo "Starting docker container ${CONTAINER_NAME}"
  docker start "${CONTAINER_NAME}"
fi

docker exec "${CONTAINER_NAME}" "${MOUNT_POINT}/scripts/install_dep.sh"
docker exec "${CONTAINER_NAME}" "${MOUNT_POINT}/scripts/test.sh"
result=$?

echo "Stopping docker container ${CONTAINER_NAME}"
docker stop "${CONTAINER_NAME}"

if [ $result -eq 0 ]; then
  echo "Command completed successfully"
else
  echo "Executing command failed"
  exit 1
fi
