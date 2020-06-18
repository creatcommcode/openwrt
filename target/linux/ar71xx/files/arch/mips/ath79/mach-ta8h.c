/*
 * Creatcomm TA8H board support
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

#define TA8H_GPIO_LED_RED      13
#define TA8H_GPIO_LED_YELLOW   14
#define TA8H_GPIO_LED_GREEN    17

#define TA8H_GPIO_BTN_RESET   16

#define TA8H_KEYS_POLL_INTERVAL	20	/* msecs */
#define TA8H_KEYS_DEBOUNCE_INTERVAL	(3 * TA8H_KEYS_POLL_INTERVAL)

#define TA8H_MAC0_OFFSET		0x0
#define TA8H_MAC1_OFFSET		0x6
#define TA8H_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led ta8h_leds_gpio[] __initdata = {
	{
		.name		= "ta8h:red:weak",
		.gpio		= TA8H_GPIO_LED_RED,
		.active_low	= 1,
	},
	{
		.name		= "ta8h:green:medium",
		.gpio		= TA8H_GPIO_LED_YELLOW,
		.active_low	= 1,
	},
	{
		.name		= "ta8h:green:strong",
		.gpio		= TA8H_GPIO_LED_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ta8h_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TA8H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TA8H_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info ta8h_leds_qca8337[] = {
	AR8327_LED_INFO(PHY0_0, HW, "ta8h:green:lan"),
	AR8327_LED_INFO(PHY4_0, HW, "ta8h:green:wan"),
};

/* Blink rate: 1 Gbps -> 8 hz, 100 Mbs -> 4 Hz, 10 Mbps -> 2 Hz */
static struct ar8327_led_cfg ta8h_qca8337_led_cfg = {
	.led_ctrl0 = 0xcf37cf37,
	.led_ctrl1 = 0xcf37cf37,
	.led_ctrl2 = 0xcf37cf37,
	.led_ctrl3 = 0x0,
	.open_drain = true,
};

/* QCA8337 GMAC0 is connected with QCA9558 over RGMII */
static struct ar8327_pad_cfg ta8h_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.mac06_exchange_dis = true,
};

/* QCA8337 GMAC6 is connected with QCA9558 over SGMII */
static struct ar8327_pad_cfg ta8h_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_platform_data ta8h_qca8337_data = {
	.pad0_cfg = &ta8h_qca8337_pad0_cfg,
	.pad6_cfg = &ta8h_qca8337_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &ta8h_qca8337_led_cfg,
	.num_leds = ARRAY_SIZE(ta8h_leds_qca8337),
	.leds = ta8h_leds_qca8337,
};

static struct mdio_board_info ta8h_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &ta8h_qca8337_data,
	},
};

static void __init ta8h_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ta8h_leds_gpio),
				 ta8h_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TA8H_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ta8h_gpio_keys),
					ta8h_gpio_keys);

	ath79_register_pci();

	ath79_register_wmac(art + TA8H_WMAC_CALDATA_OFFSET, NULL);

	mdiobus_register_board_info(ta8h_mdio0_info,
				    ARRAY_SIZE(ta8h_mdio0_info));

	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* QCA9557 GMAC0 is connected to RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x86000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, art + TA8H_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	/* QCA9557 GMAC1 is connected to SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, art + TA8H_MAC1_OFFSET, 0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_CREATCOMM_TA8H, "TA8H", "Creatcomm TA8H", ta8h_setup);

