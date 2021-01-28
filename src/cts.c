/** @file
 *  @brief GATT Current Time Service
 */

/*
 * Copyright (c) 2021 Jonathan Rico
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <init.h>
#include <stdbool.h>
#include <zephyr/types.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>

#include "cts.h"
#include "clock.h"

#define LOG_LEVEL CONFIG_BT_CTS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(cts);

static struct current_time current_local_time;

static void apply_current_time(void);

static void cts_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	LOG_INF("BAS Notifications %s", notif_enabled ? "enabled" : "disabled");
}

static ssize_t read_current_time(struct bt_conn *conn,
				 const struct bt_gatt_attr *attr, void *buf,
				 uint16_t len, uint16_t offset)
{
	LOG_INF("Read attribute");
	/* Read values from clock */
	bt_cts_get_current_time();
	return bt_gatt_attr_read(conn, attr, buf, len, offset,
				 &current_local_time,
				 sizeof(current_local_time));
}

static ssize_t write_current_time(struct bt_conn *conn,
				  const struct bt_gatt_attr *attr,
				  const void *buf, uint16_t len,
				  uint16_t offset, uint8_t flags)
{
	LOG_INF("Write attribute");

	/* TODO: check data read */
	memcpy(&current_local_time, buf, len);

	return 0;
}

BT_GATT_SERVICE_DEFINE(
	cts, BT_GATT_PRIMARY_SERVICE(BT_UUID_CTS),
	BT_GATT_CHARACTERISTIC(BT_UUID_CTS_CURRENT_TIME,
			       BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY |
				       BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_READ | BT_GATT_CHRC_WRITE,
			       read_current_time, write_current_time,
			       &current_local_time),
	BT_GATT_CCC(cts_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

/* TODO: change below code and implement current time service */

static int cts_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	memset(&current_local_time, 0, sizeof(current_local_time));

	return 0;
}

struct current_time *bt_cts_get_current_time(void)
{
	time_struct_t * p_time = clock_get_time_p();
	current_local_time.exact_time_256.day_date_time.date_time.
		hours = p_time->hours;
	current_local_time.exact_time_256.day_date_time.date_time.
		minutes = p_time->minutes;
	current_local_time.exact_time_256.day_date_time.date_time.
		seconds = p_time->seconds;

	return &current_local_time;
}

static void apply_current_time(void)
{
	time_struct_t new_time;

	new_time.hours =
		current_local_time.exact_time_256.day_date_time.date_time
		.hours;
	new_time.minutes =
		current_local_time.exact_time_256.day_date_time.date_time
		.minutes;
	new_time.seconds =
		current_local_time.exact_time_256.day_date_time.date_time
		.seconds;

	clock_set_time(new_time);
}

int bt_cts_set_current_time(struct current_time *p_current_time)
{
	int rc;

	/* TODO: check the passed values before blind write */
	memcpy(&current_local_time, p_current_time, sizeof(current_local_time));

	rc = bt_gatt_notify(NULL, &cts.attrs[1], &current_local_time,
			    sizeof(current_local_time));

	return rc == -ENOTCONN ? 0 : rc;
}

SYS_INIT(cts_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
