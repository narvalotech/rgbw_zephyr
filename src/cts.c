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
#include "calendar.h"

#define LOG_LEVEL CONFIG_BT_CTS_LOG_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(cts);

#define CTS_NOTIFY_STACK_SIZE 500
#define CTS_NOTIFY_PRIORITY 5

static void cts_notify_entry_point(void *p1, void *p2, void *p3);
K_THREAD_DEFINE(cts_notify_tid, CTS_NOTIFY_STACK_SIZE,
                cts_notify_entry_point, NULL, NULL, NULL,
                CTS_NOTIFY_PRIORITY, 0, 0);

static struct current_time current_local_time;

static void apply_current_time(void);

static void cts_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	if (notif_enabled) {
		k_thread_resume(cts_notify_tid);
	} else {
		k_thread_suspend(cts_notify_tid);
	}

	LOG_INF("CTS Notifications %s", notif_enabled ? "enabled" : "disabled");
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
	apply_current_time();

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

static int cts_init(const struct device *dev)
{
	ARG_UNUSED(dev);

	memset(&current_local_time, 0, sizeof(current_local_time));

	return 0;
}

struct current_time *bt_cts_get_current_time(void)
{
	time_struct_t * p_time = clock_get_time_p();
	struct date_time* p_date = cal_get_date_ptr();
	day_of_week_t day = cal_get_weekday(p_date);

	/* Copy time section */
	memcpy(&current_local_time.exact_time_256.day_date_time.date_time.hours,
	       p_time, sizeof(time_struct_t));

	/* Copy date section */
	memcpy(&current_local_time.exact_time_256.day_date_time.date_time,
	       p_date, sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t));

	current_local_time.exact_time_256.day_date_time.
		day_of_week.day_of_week = day;

	return &current_local_time;
}

static void apply_current_time(void)
{
	time_struct_t new_time;
	struct date_time new_date;

	/* Write new time */
	memcpy(&new_time,
	       &current_local_time.exact_time_256.day_date_time.date_time.hours,
	       sizeof(new_time));
	clock_set_time(new_time);

	/* Write new date */
	memcpy(&new_date,
	       &current_local_time.exact_time_256.day_date_time.date_time,
	       sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint8_t));
	cal_set_date(&new_date);
}

SYS_INIT(cts_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

/* Notification thread pulls data from clock module */
static void cts_notify_entry_point(void *p1, void *p2, void *p3) {
	ARG_UNUSED(p1);
	ARG_UNUSED(p2);
	ARG_UNUSED(p3);

	/* Disable notifications on startup */
	k_thread_suspend(k_current_get());

	while(1)
	{
		/* Wait for next internal time update */
		clock_thread_sync();
		/* Copy into local structure */
		bt_cts_get_current_time();
		/* GATT notify */
		bt_gatt_notify(NULL, &cts.attrs[1],
			&current_local_time,
			sizeof(current_local_time));
	}
}
