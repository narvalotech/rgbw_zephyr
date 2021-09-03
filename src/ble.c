#include <zephyr.h>
#include "disp.h"
#include "board.h"
#include "state.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <bluetooth/services/lbs.h>

#include "img_mgmt/img_mgmt.h"
#include "os_mgmt/os_mgmt.h"
#include <mgmt/mcumgr/smp_bt.h>

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

extern struct g_state state;
struct bt_conn * master_conn = NULL;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_LBS_VAL),
};

void gatt_mtu_cb(struct bt_conn *conn, uint8_t err,
		 struct bt_gatt_exchange_params *params)
{
	/* Do nothing */
}

static void connected(struct bt_conn *conn, uint8_t err)
{
	static struct bt_gatt_exchange_params gatt_mtu_params;

	if (err) {
		printk("Connection failed (err %u)\n", err);
		return;
	}

	master_conn = conn;
	printk("Connected\n");

	board_enable_5v(1);
	display_clear();
	display_string("conn", 0, 50);
	board_enable_5v(0);

	/* Exchange MTU to be able to transfer CTS characteristic
	 * in one go. */
	gatt_mtu_params.func = gatt_mtu_cb;
	bt_gatt_exchange_mtu(conn, &gatt_mtu_params);

	/* TODO: re-enable whe it works reliably */
	/* if (bt_conn_set_security(conn, BT_SECURITY_L4)) { */
	/* 	printk("Failed to set security\n"); */
	/* } */
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	master_conn = NULL;

	board_enable_5v(1);
	display_clear();
	display_string("disc", 0, 50);
	board_enable_5v(0);
}

static void identity_resolved(struct bt_conn *conn, const bt_addr_le_t *rpa,
			      const bt_addr_le_t *identity)
{
	char addr_identity[BT_ADDR_LE_STR_LEN];
	char addr_rpa[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(identity, addr_identity, sizeof(addr_identity));
	bt_addr_le_to_str(rpa, addr_rpa, sizeof(addr_rpa));

	printk("Identity resolved %s -> %s\n", addr_rpa, addr_identity);
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		printk("Security changed: %s level %u\n", addr, level);
		board_enable_5v(1);
		display_clear();
		display_string("encrypted", 0, 50);
		board_enable_5v(0);
	} else {
		printk("Security failed: %s level %u err %d\n", addr, level,
		       err);
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected        = connected,
	.disconnected     = disconnected,
	.identity_resolved = identity_resolved,
	.security_changed = security_changed,
};

static struct bt_conn_auth_cb conn_auth_callbacks;

/* TODO: remove LBS */
static void app_led_cb(bool led_state)
{
	board_enable_5v(1);
	display_clear();
	if(led_state)
		display_string("on", 0, 50);
	else
		display_string("off", 0, 50);
	board_enable_5v(0);
}

static bool app_button_cb(void)
{
	uint8_t ret = state.but_ur;
	state.but_ur = 0;
	return (bool)ret;
}

static struct bt_lbs_cb lbs_callbacs = {
	.led_cb    = app_led_cb,
	.button_cb = app_button_cb,
};

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Passkey for %s: %06u\n", addr, passkey);

	board_enable_5v(1);
	display_clear();
	display_string("passkey", 0, 50);
	uint32_t high = passkey / 10000;
	uint32_t mid = (passkey / 100) - (high * 100);
	uint32_t low = passkey - ((passkey / 100) * 100);
	display_bcd(high, mid, low, 0);
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);

	board_enable_5v(1);
	display_string("auth cancel", 0, 50);
	board_enable_5v(0);
}

static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	printk("Pairing Complete\n");
	board_enable_5v(1);
	display_string("auth complete", 0, 50);
	board_enable_5v(0);
}

static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	printk("Pairing Failed (%d). Disconnecting.\n", reason);
	/* TODO: re-enable when it works reliably */
	/* bt_conn_disconnect(conn, BT_HCI_ERR_AUTH_FAIL); */
	board_enable_5v(1);
	display_string("auth failed", 0, 50);
	board_enable_5v(0);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.passkey_display = auth_passkey_display,
	.passkey_entry = NULL,
	.cancel = auth_cancel,
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed,
};

void dfu_upload_complete_cb(void)
{
	state.pgm_state = PGM_STATE_DFU_END;
	state.next = 1;
	state.abort = 1;	/* Maybe this one is not necessary ? */
	k_wakeup(state.main_tid); /* Wake from sleep */
}

const img_mgmt_dfu_callbacks_t dfu_callbacks = {
	.dfu_started_cb = NULL,
	.dfu_stopped_cb = NULL,
	.dfu_pending_cb = dfu_upload_complete_cb,
	.dfu_confirmed_cb = NULL
};

static int dfu_init(void)
{
	/* Define a callback at the end of the image upload */
	img_mgmt_register_callbacks(&dfu_callbacks);
	img_mgmt_register_group();

	os_mgmt_register_group();
	int err = smp_bt_register();
	return err;
}

int ble_init(void)
{
	int err;

	bt_conn_auth_cb_register(&auth_cb_display);
	bt_conn_cb_register(&conn_callbacks);
	if (IS_ENABLED(CONFIG_BT_LBS_SECURITY_ENABLED)) {
		bt_conn_auth_cb_register(&conn_auth_callbacks);
	}

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	err = bt_lbs_init(&lbs_callbacs);
	if (err) {
		printk("Failed to init LBS (err:%d)\n", err);
		return err;
	}

	err = dfu_init();
	if (err) {
		printk("Failed to init DFU (err:%d)\n", err);
		return err;
	}
	printk("DFU initialized\n");

	return err;
}

#define BT_LE_ADV_CUSTOM                                                       \
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, \
			BT_GAP_ADV_SLOW_INT_MIN, BT_GAP_ADV_SLOW_INT_MAX,      \
			NULL)

int ble_adv(bool enable)
{
	int err = 0;

	if(enable)
	{
		err = bt_le_adv_start(BT_LE_ADV_CUSTOM, ad, ARRAY_SIZE(ad),
				      sd, ARRAY_SIZE(sd));
		if (err) {
			printk("Advertising failed to start (err %d)\n", err);
			return err;
		}

		printk("Advertising successfully started\n");
	}
	else {
		err = bt_le_adv_stop();
		if (err) {
			printk("Advertising failed to stop (err %d)\n", err);
			return err;
		}
	}

	return err;
}

int ble_update_param(void)
{
	if (!master_conn) {
		return -1;
	}

	const struct bt_le_conn_param param = { /* 500ms */
						.interval_min = 400,
						/* 2s */
						.interval_max = 1600,
						.latency = 0,
						/* 8s */
						.timeout = 800
	};
	return bt_conn_le_param_update(master_conn, &param);
}
