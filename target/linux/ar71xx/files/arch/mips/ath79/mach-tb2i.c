/*
 * Creatcomm TB2I board support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "pci.h"
#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TB2I_GPIO_LED_STATS    13
#define TB2I_GPIO_LED_WLAN     12
#define TB2I_GPIO_LED_RED      16
#define TB2I_GPIO_LED_YELLOW   11
#define TB2I_GPIO_LED_GREEN    14
#define TB2I_GPIO_LED_WAN      4

#define TB2I_GPIO_BTN_RESET   17

#define TB2I_KEYS_POLL_INTERVAL	20	/* msecs */
#define TB2I_KEYS_DEBOUNCE_INTERVAL	(3 * TB2I_KEYS_POLL_INTERVAL)

#define TB2I_MAC0_OFFSET	0x1000

static struct gpio_led tb2i_leds_gpio[] __initdata = {
	{
		.name		= "tb2i:green:status",
		.gpio		= TB2I_GPIO_LED_STATS,
		.active_low	= 1,
	},
	{
		.name		= "tb2i:green:wlan",
		.gpio		= TB2I_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tb2i:red:weak",
		.gpio		= TB2I_GPIO_LED_RED,
		.active_low	= 1,
	},
	{
		.name		= "tb2i:green:medium",
		.gpio		= TB2I_GPIO_LED_YELLOW,
		.active_low	= 1,
	},
	{
		.name		= "tb2i:green:strong",
		.gpio		= TB2I_GPIO_LED_GREEN,
		.active_low	= 0,
	},
	{
		.name		= "tb2i:green:wan",
		.gpio		= TB2I_GPIO_LED_WAN,
		.active_low	= 0,
	}
};

static struct gpio_keys_button tb2i_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TB2I_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TB2I_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init tb2i_setup(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tb2i_leds_gpio),
				 tb2i_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TB2I_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tb2i_gpio_keys),
					tb2i_gpio_keys);

	ath79_register_pci();

	ath79_register_wmac(art + TB2I_MAC0_OFFSET, NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_eth(0);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_CREATCOMM_TB2I, "TB2I", "Creatcomm TB2I", tb2i_setup);
