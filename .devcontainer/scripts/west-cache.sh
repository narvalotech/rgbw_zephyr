#!/usr/bin/env bash
# Copyright (c) 2024 Nordic Semiconductor ASA
# SPDX-License-Identifier: Apache-2.0

echo "Making the west cache"

# Can that have bad consequences if host UID != 1000?
sudo mkdir -p /workspace-cache
sudo chown user:user /workspace-cache

# Fetch the workspace into the cache
cd /workspace-cache
west init --mf /west-cache.yml
west config --global update.narrow true
west update

# Reset every project except the main zephyr repo.
west forall -c 'git reset --hard HEAD' || true

echo "West cache initialized successfully"
