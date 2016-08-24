/*
 * Copyright (C) 2011-2014 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/imx6q-iomuxc-gpr.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/fsl_devices.h>
#include <linux/mipi_csi2.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-int-device.h>
#include "mxc_v4l2_capture.h"
#include "s5k4ecgx_regs_odroid.h"
//
//by crazyboy 20151013 by treego  For Test MOVE To Header File
#define FXL6408_SLAVE_ADDRESS 0x43

#define FXL6408_REG_ID 0x01
#define FXL6408_REG_GPIO_DIRECTION 0x03
#define FXL6408_REG_OUT_STATE  	0x05
#define FXL6408_REG_OUT_HIGH_Z  0x07
#define FXL6408_REG_GPIO_PULLEN 0x0B

#define S5K4ECGX_VOLTAGE_ANALOG               2800000
#define S5K4ECGX_VOLTAGE_DIGITAL_CORE         1500000
#define S5K4ECGX_VOLTAGE_DIGITAL_IO           1800000

#define MIN_FPS 15
#define MAX_FPS 30
#define DEFAULT_FPS 30

#define S5K4ECGX_XCLK_MIN 6000000
#define S5K4ECGX_XCLK_MAX 24000000

#define S5K4ECGX_CHIP_ID	0x4EC0
#define S5K4ECGX_CHIP_REV	0x0011

#define S5K4ECGX_DELAY      0xFFFF0000

enum s5k4ecgx_mode {
	s5k4ecgx_mode_MIN = 0,
	s5k4ecgx_mode_VGA_640_480 = 0,
	s5k4ecgx_mode_QVGA_320_240 = 1,
	s5k4ecgx_mode_NTSC_720_480 = 2,
	s5k4ecgx_mode_PAL_720_576 = 3,
	s5k4ecgx_mode_720P_1280_720 = 4,
	s5k4ecgx_mode_1080P_1920_1080 = 5,
	s5k4ecgx_mode_QSXGA_2592_1944 = 6,
	s5k4ecgx_mode_QCIF_176_144 = 7,
	s5k4ecgx_mode_XGA_1024_768 = 8,
	s5k4ecgx_mode_MAX = 8,
	s5k4ecgx_mode_INIT = 0xff, /*only for sensor init*/
};

enum s5k4ecgx_frame_rate {
	s5k4ecgx_15_fps,
	s5k4ecgx_30_fps
};

static int s5k4ecgx_framerates[] = {
	[s5k4ecgx_15_fps] = 15,
	[s5k4ecgx_30_fps] = 30,
};

/* image size under 1280 * 960 are SUBSAMPLING
 * image size upper 1280 * 960 are SCALING
 */
enum s5k4ecgx_downsize_mode {
	SUBSAMPLING,
	SCALING,
};

struct reg_value {
	u16 u16RegAddr;
	u16 u16Val;
	u8 u8Mask;
	u32 u32Delay_ms;
};

struct s5k4ecgx_mode_info {
	enum s5k4ecgx_mode mode;
	enum s5k4ecgx_downsize_mode dn_mode;
	u32 width;
	u32 height;
	struct reg_value *init_data_ptr;
	u32 init_data_size;
};

/*!
 * Maintains the information on the current state of the sesor.
 */
static struct sensor_data s5k4ecgx_data;
static int pwn_gpio, rst_gpio;

static struct regulator *io_regulator;
static struct regulator *core_regulator;
static struct regulator *analog_regulator;
static struct regulator *gpo_regulator;

static int s5k4ecgx_probe(struct i2c_client *adapter,
				const struct i2c_device_id *device_id);
static int s5k4ecgx_remove(struct i2c_client *client);

static s32 s5k4ecgx_read_reg(u16 reg, u8 *val);
static s32 s5k4ecgx_write_reg(u16 reg, u8 val);

static const struct i2c_device_id s5k4ecgx_id[] = {
	{"s5k4ecgx_mipi", 0},
	{},
};

MODULE_DEVICE_TABLE(i2c, s5k4ecgx_id);

static struct i2c_driver s5k4ecgx_i2c_driver = {
	.driver = {
		  .owner = THIS_MODULE,
		  .name  = "s5k4ecgx_mipi",
		  },
	.probe  = s5k4ecgx_probe,
	.remove = s5k4ecgx_remove,
	.id_table = s5k4ecgx_id,
};


static int s5k4ecgx_power_on(struct device *dev)
{
	int ret = 0;

	return ret;
}

static int s5k4ecgx_i2c_read_twobyte(struct i2c_client *client,
				  u16 subaddr, u16 *data)
{
	int err;
	u8 buf[2];
	struct i2c_msg msg[2];

	cpu_to_be16s(&subaddr);

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].len = 2;
	msg[0].buf = (u8 *)&subaddr;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 2;
	msg[1].buf = buf;

	err = i2c_transfer(client->adapter, msg, 2);

	*data = ((buf[0] << 8) | buf[1]);

	return 0;
}


static int s5k4ecgx_i2c_write_twobyte(struct i2c_client *client,
					 u16 addr, u16 w_data)
{
	int retry_count = 5;
	int ret = 0;
	u8 buf[4] = {0,};
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 4,
		.buf	= buf,
	};

	buf[0] = addr >> 8;
	buf[1] = addr;
	buf[2] = w_data >> 8;
	buf[3] = w_data & 0xff;

	/* cam_dbg("I2C writing: 0x%02X%02X%02X%02X\n",
			buf[0], buf[1], buf[2], buf[3]); */

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(5);
		printk("%s: ERROR(%d), write (%04X, %04X), retry %d.\n",
				__func__, ret, addr, w_data, retry_count);
	} while (retry_count-- > 0);


	return 0;
}

static int s5k4ecgx_write_regs(const u32 regs[], int size)
{
	u16 delay = 0;
	int i, err = 0;

	for (i = 0; i < size; i++) {
		if ((regs[i] & S5K4ECGX_DELAY) == S5K4ECGX_DELAY) {
			delay = regs[i] & 0xFFFF;
			msleep(delay);
			continue;
		}

		err = s5k4ecgx_i2c_write_twobyte(s5k4ecgx_data.i2c_client,
			(regs[i] >> 16), regs[i]);
	}

	return 0;
}
#define BURST_MODE_BUFFER_MAX_SIZE 2700
u8 s5k4ecgx_burstmode_buf[BURST_MODE_BUFFER_MAX_SIZE];

static int s5k4ecgx_burst_write_regs(const u32 list[], u32 size, char *name)
{
	struct i2c_client *client = s5k4ecgx_data.i2c_client;
	int err = -EINVAL;
	int i = 0, idx = 0;
	u16 subaddr = 0, next_subaddr = 0, value = 0;
	struct i2c_msg msg = {
		.addr	= client->addr,
		.flags	= 0,
		.len	= 0,
		.buf	= s5k4ecgx_burstmode_buf,
	};


	for (i = 0; i < size; i++) {

		subaddr = (list[i] & 0xFFFF0000) >> 16;
		if (subaddr == 0x0F12)
			next_subaddr = (list[i+1] & 0xFFFF0000) >> 16;

		value = list[i] & 0x0000FFFF;

		switch (subaddr) {
		case 0x0F12:
			/* make and fill buffer for burst mode write. */
			if (idx == 0) {
				s5k4ecgx_burstmode_buf[idx++] = 0x0F;
				s5k4ecgx_burstmode_buf[idx++] = 0x12;
			}
			s5k4ecgx_burstmode_buf[idx++] = value >> 8;
			s5k4ecgx_burstmode_buf[idx++] = value & 0xFF;

			/* write in burstmode*/
			if (next_subaddr != 0x0F12) {
				msg.len = idx;
				err = i2c_transfer(client->adapter,
					&msg, 1) == 1 ? 0 : -EIO;
				// crazyboy to trace 20150604
				printk("s5k4ecgx_sensor_burst_write, idx = %d\n", idx); 
				idx = 0;
			}
			break;

		case 0xFFFF:
			msleep(value);
			break;

		default:
			idx = 0;
			err = s5k4ecgx_i2c_write_twobyte(client,
						subaddr, value);
			break;
		}
	}

	return 0;
}
static int fxl6408_i2c_write_byte(struct i2c_client *client,
					 u8 reg, u8 w_data)
{
	int retry_count = 5;
	int ret = 0;
	u8 buf[2] = {0,};
	struct i2c_msg msg = {
		.addr	= FXL6408_SLAVE_ADDRESS,
		.flags	= 0,
		.len	= 2,
		.buf	= buf,
	};
	buf[0] = reg & 0xFF;
	buf[1] = w_data & 0xFF;


	/* cam_dbg("I2C writing: 0x%02X%02X%02X%02X\n",
			buf[0], buf[1], buf[2], buf[3]); */

	do {
		ret = i2c_transfer(client->adapter, &msg, 1);
		if (likely(ret == 1))
			break;
		msleep(5);
		printk("%s: ERROR(%d), write (%04X, %04X), retry %d.\n",
				__func__, ret, reg, w_data, retry_count);
	} while (retry_count-- > 0);

	return 0;
}


static int fxl6408_i2c_read_byte(struct i2c_client *client,
					 u8 reg, u8 *r_data)
{
	int err;
	u8 buf[2];
	struct i2c_msg msg[2];

	msg[0].addr = FXL6408_SLAVE_ADDRESS;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = FXL6408_SLAVE_ADDRESS;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = r_data;

	err = i2c_transfer(client->adapter, msg, 2);

	return 0;
}

static void fxl6408_CamPower_on(unsigned int on)
{
	struct i2c_client *client = s5k4ecgx_data.i2c_client;
	char r_data,w_data;

	if(on)
	{
		//FXL6408 GPIO 1  - High
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		w_data = (r_data | 0x2);
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);

	}
	else 
	{
		//FXL6408 GPIO 0,1 - Low
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		w_data = ~(r_data & 0x3);
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);

		msleep(1);

		//FXL6408 GPIO 0(RST) - High
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		w_data = (r_data | 0x1);
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);
	}

	fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
	MANGO_DBG("fxl6408 State = 0x%x\n",r_data);
}
static void fxl6408_CamPower_init(struct i2c_client *client)
{
		char r_data,w_data;
		fxl6408_i2c_read_byte(client,FXL6408_REG_ID,&r_data);
		MANGO_DBG("fxl6408 ID = 0x%x\n",r_data);
		
		//IO Direction
		w_data = 0x3;
		fxl6408_i2c_write_byte(client,FXL6408_REG_GPIO_DIRECTION,w_data);

		//Output State
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);

		//HIGH-Z
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_HIGH_Z,&r_data);
		MANGO_DBG("fxl6408 HIGH-Z = 0x%x\n",r_data);
		w_data = ~(r_data & 0x03);
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_HIGH_Z,w_data);

		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_HIGH_Z,&r_data);
		MANGO_DBG("fxl6408 HIGH-Z = 0x%x\n",r_data);

		//PULL Enable 
		fxl6408_i2c_read_byte(client,FXL6408_REG_GPIO_PULLEN,&r_data);
		MANGO_DBG("fxl6408 PULL_UP = 0x%x\n",r_data);

		w_data = ~(r_data & 0x03);
		fxl6408_i2c_write_byte(client,FXL6408_REG_GPIO_PULLEN,w_data);
		fxl6408_i2c_read_byte(client,FXL6408_REG_GPIO_PULLEN,&r_data);
		MANGO_DBG("fxl6408 PULL_UP = 0x%x\n",r_data);

		fxl6408_i2c_read_byte(client,FXL6408_REG_GPIO_DIRECTION,&r_data);
		MANGO_DBG("fxl6408 Direction = 0x%x\n",r_data);
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		MANGO_DBG("fxl6408 State = 0x%x\n",r_data);

		//Reset-FXL6408 GPIO 0
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		w_data = ~(r_data & 0x1);
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);

		//For Check
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		MANGO_DBG("fxl6408 State = 0x%x\n",r_data);

		mdelay(10);
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		w_data = r_data | 0x1;
		fxl6408_i2c_write_byte(client,FXL6408_REG_OUT_STATE,w_data);

		//For Check
		fxl6408_i2c_read_byte(client,FXL6408_REG_OUT_STATE,&r_data);
		MANGO_DBG("fxl6408 State = 0x%x\n",r_data);
		
}

static int s5k4ecgx_init_regs(void)
{
	struct i2c_client *client = s5k4ecgx_data.i2c_client;
	u16 read_value = 0;
	int err = -ENODEV;

	/* we'd prefer to do this in probe, but the framework hasn't
	 * turned on the camera yet so our i2c operations would fail
	 * if we tried to do it in probe, so we have to do it here
	 * and keep track if we succeeded or not.
	 */

	/* enter read mode */
	err = s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	if (unlikely(err < 0))
		return -ENODEV;

	s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x01A4);
	s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
	if (likely(read_value == S5K4ECGX_CHIP_ID))
		printk("Sensor ChipID: 0x%04X\n", S5K4ECGX_CHIP_ID);
	else
		printk("Sensor ChipID: 0x%04X, unknown ChipID\n", read_value);

	printk("[CRZ] Sensor ChipID: 0x%04X\n", read_value);
	s5k4ecgx_i2c_write_twobyte(client, 0x002C, 0x7000);
	s5k4ecgx_i2c_write_twobyte(client, 0x002E, 0x01A6);
	s5k4ecgx_i2c_read_twobyte(client, 0x0F12, &read_value);
	if (likely(read_value == S5K4ECGX_CHIP_REV))
		printk("Sensor revision: 0x%04X\n", S5K4ECGX_CHIP_REV);
	else
		printk("Sensor revision: 0x%04X, unknown revision\n",
				read_value);

	printk("[CRZ]Sensor revision: 0x%04X\n", read_value);
	/* restore write mode */
	err = s5k4ecgx_i2c_write_twobyte(client, 0x0028, 0x7000);

	return 0;
}
static int __init v4l2_i2c_drv_init(void);

static int s5k4ecgx_init_mode(enum s5k4ecgx_frame_rate frame_rate,
			    enum s5k4ecgx_mode mode, enum s5k4ecgx_mode orig_mode)
{
	struct reg_value *pModeSetting = NULL;
	s32 ArySize = 0;
	int retval = 0;
	void *mipi_csi2_info;
	u32 mipi_reg, msec_wait4stable = 0;
	enum s5k4ecgx_downsize_mode dn_mode, orig_dn_mode;

	MANGO_DBG_DEFAULT;
	if ((mode > s5k4ecgx_mode_MAX || mode < s5k4ecgx_mode_MIN)
		&& (mode != s5k4ecgx_mode_INIT)) {
		pr_err("Wrong s5k4ecgx mode detected!\n");
		return -1;
	}

	mipi_csi2_info = mipi_csi2_get_info();

	/* initial mipi dphy */
	if (!mipi_csi2_info) {
		printk(KERN_ERR "%s() in %s: Fail to get mipi_csi2_info!\n",
		       __func__, __FILE__);
		return -1;
	}

	if (!mipi_csi2_get_status(mipi_csi2_info))
		mipi_csi2_enable(mipi_csi2_info);

	if (!mipi_csi2_get_status(mipi_csi2_info)) {
		pr_err("Can not enable mipi csi2 driver!\n");
		return -1;
	}

	mipi_csi2_set_lanes(mipi_csi2_info);

	/*Only reset MIPI CSI2 HW at sensor initialize*/
	if (mode == s5k4ecgx_mode_INIT)
		mipi_csi2_reset(mipi_csi2_info);

	if (s5k4ecgx_data.pix.pixelformat == V4L2_PIX_FMT_UYVY)
		mipi_csi2_set_datatype(mipi_csi2_info, MIPI_DT_YUV422);
	else if (s5k4ecgx_data.pix.pixelformat == V4L2_PIX_FMT_RGB565)
		mipi_csi2_set_datatype(mipi_csi2_info, MIPI_DT_RGB565);
	else
		pr_err("currently this sensor format can not be supported!\n");

	if (mode == s5k4ecgx_mode_INIT) {
		s5k4ecgx_data.pix.width = 640;
		s5k4ecgx_data.pix.height = 480;
	{
	s5k4ecgx_init_regs();

	s5k4ecgx_write_regs(s5k4ecgx_init_arm, sizeof(s5k4ecgx_init_arm)/sizeof(s5k4ecgx_init_arm[0]));

	s5k4ecgx_burst_write_regs(s5k4ecgx_init_reg, sizeof(s5k4ecgx_init_reg)/sizeof(s5k4ecgx_init_reg[0]),NULL);

#if 1
	s5k4ecgx_write_regs(s5k4ecgx_Jpeg_Quality_Normal, sizeof(s5k4ecgx_Jpeg_Quality_Normal)/sizeof(s5k4ecgx_Jpeg_Quality_Normal[0]));

	//s5k4ecgx_write_regs(s5k4ecgx_init_arm, sizeof(s5k4ecgx_init_arm)/sizeof(s5k4ecgx_init_arm[0]));

	s5k4ecgx_write_regs(s5k4ecgx_640_Preview, sizeof(s5k4ecgx_640_Preview)/sizeof(s5k4ecgx_640_Preview[0]));

	s5k4ecgx_write_regs(s5k4ecgx_AE_AWB_Lock_Off, sizeof(s5k4ecgx_AE_AWB_Lock_Off)/sizeof(s5k4ecgx_AE_AWB_Lock_Off[0]));

	s5k4ecgx_write_regs(s5k4ecgx_Single_AF_Start, sizeof(s5k4ecgx_Single_AF_Start)/sizeof(s5k4ecgx_Single_AF_Start[0]));

#endif
	}
#if 0
		retval = s5k4ecgx_download_firmware(pModeSetting, ArySize);
		if (retval < 0)
			goto err;

		pModeSetting = s5k4ecgx_init_setting_30fps_VGA;
		ArySize = ARRAY_SIZE(s5k4ecgx_init_setting_30fps_VGA);
		retval = s5k4ecgx_download_firmware(pModeSetting, ArySize);
#endif
	}

	if (retval < 0)
		goto err;

	/* add delay to wait for sensor stable */
	if (mode == s5k4ecgx_mode_QSXGA_2592_1944) {
		/* dump the first two frames: 1/7.5*2
		 * the frame rate of QSXGA is 7.5fps */
		msec_wait4stable = 267;
	} else if (frame_rate == s5k4ecgx_15_fps) {
		/* dump the first nine frames: 1/15*9 */
		msec_wait4stable = 600;
	} else if (frame_rate == s5k4ecgx_30_fps) {
		/* dump the first nine frames: 1/30*9 */
		msec_wait4stable = 300;
	}
	msleep(msec_wait4stable);

	if (mipi_csi2_info) {
		unsigned int i;

		i = 0;

		/* wait for mipi sensor ready */
		mipi_reg = mipi_csi2_dphy_status(mipi_csi2_info);
		while ((mipi_reg == 0x200) && (i < 10)) {
			mipi_reg = mipi_csi2_dphy_status(mipi_csi2_info);
			i++;
			msleep(10);
		}

		if (i >= 10) {
			pr_err("mipi csi2 can not receive sensor clk!\n");
			return -1;
		}

		i = 0;

		/* wait for mipi stable */
		mipi_reg = mipi_csi2_get_error1(mipi_csi2_info);
		while ((mipi_reg != 0x0) && (i < 10)) {
			mipi_reg = mipi_csi2_get_error1(mipi_csi2_info);
			i++;
			msleep(10);
		}

		if (i >= 10) {
			pr_err("mipi csi2 can not reveive data correctly!\n");
			return -1;
		}
	}
err:
	return retval;
}

static int ioctl_s_power(struct v4l2_int_device *s, int on)
{
	struct sensor_data *sensor = s->priv;

	MANGO_DBG_DEFAULT;
	if (on && !sensor->on) {
    	MANGO_DBG(" %s() Power_ON.....\n", __func__);
		fxl6408_CamPower_on(1);
	} else if (!on && sensor->on) {
		fxl6408_CamPower_on(0);
    	MANGO_DBG(" %s() Reset.....\n", __func__);
	}

	sensor->on = on;

	return 0;
}

static int ioctl_g_ifparm(struct v4l2_int_device *s, struct v4l2_ifparm *p)
{
	if (s == NULL) {
		pr_err("   ERROR!! no slave device set!\n");
		return -1;
	}

	MANGO_DBG_DEFAULT;
	memset(p, 0, sizeof(*p));
	p->u.bt656.clock_curr = s5k4ecgx_data.mclk;
	pr_debug("   clock_curr=mclk=%d\n", s5k4ecgx_data.mclk);
	p->if_type = V4L2_IF_TYPE_BT656;
	p->u.bt656.mode = V4L2_IF_TYPE_BT656_MODE_NOBT_8BIT;
	p->u.bt656.clock_min = S5K4ECGX_XCLK_MIN;
	p->u.bt656.clock_max = S5K4ECGX_XCLK_MAX;
	p->u.bt656.bt_sync_correct = 1;  /* Indicate external vsync */

	return 0;
}

static int ioctl_g_fmt_cap(struct v4l2_int_device *s, struct v4l2_format *f)
{
	struct sensor_data *sensor = s->priv;
	MANGO_DBG_DEFAULT;

	f->fmt.pix = sensor->pix;

	return 0;
}

static int ioctl_g_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor_data *sensor = s->priv;
	struct v4l2_captureparm *cparm = &a->parm.capture;
	int ret = 0;

	MANGO_DBG_DEFAULT;
	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		memset(a, 0, sizeof(*a));
		a->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		cparm->capability = sensor->streamcap.capability;
		cparm->timeperframe = sensor->streamcap.timeperframe;
		cparm->capturemode = sensor->streamcap.capturemode;
		ret = 0;
		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}


static int ioctl_s_parm(struct v4l2_int_device *s, struct v4l2_streamparm *a)
{
	struct sensor_data *sensor = s->priv;
	struct v4l2_fract *timeperframe = &a->parm.capture.timeperframe;
	u32 tgt_fps;	/* target frames per secound */
	int ret = 0;

	/* Make sure power on */
	MANGO_DBG_DEFAULT;

	switch (a->type) {
	/* This is the only case currently handled. */
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		/* Check that the new frame rate is allowed. */
		if ((timeperframe->numerator == 0) ||
		    (timeperframe->denominator == 0)) {
			timeperframe->denominator = DEFAULT_FPS;
			timeperframe->numerator = 1;
		}

		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

		if (tgt_fps > MAX_FPS) {
			timeperframe->denominator = MAX_FPS;
			timeperframe->numerator = 1;
		} else if (tgt_fps < MIN_FPS) {
			timeperframe->denominator = MIN_FPS;
			timeperframe->numerator = 1;
		}

		/* Actual frame rate we use */
		tgt_fps = timeperframe->denominator /
			  timeperframe->numerator;

#if 0
		if (tgt_fps == 15)
			frame_rate = ov5640_15_fps;
		else if (tgt_fps == 30)
			frame_rate = ov5640_30_fps;
		else {
			pr_err(" The camera frame rate is not supported!\n");
			return -EINVAL;
		}

		orig_mode = sensor->streamcap.capturemode;

		ret = ov5640_init_mode(frame_rate,
				(u32)a->parm.capture.capturemode, orig_mode);
		if (ret < 0)
			return ret;
#endif

		sensor->streamcap.timeperframe = *timeperframe;
		sensor->streamcap.capturemode =
				(u32)a->parm.capture.capturemode;

		break;

	/* These are all the possible cases. */
	case V4L2_BUF_TYPE_VIDEO_OUTPUT:
	case V4L2_BUF_TYPE_VIDEO_OVERLAY:
	case V4L2_BUF_TYPE_VBI_CAPTURE:
	case V4L2_BUF_TYPE_VBI_OUTPUT:
	case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
	case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
		pr_debug("   type is not " \
			"V4L2_BUF_TYPE_VIDEO_CAPTURE but %d\n",
			a->type);
		ret = -EINVAL;
		break;

	default:
		pr_debug("   type is unknown - %d\n", a->type);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*!
 * ioctl_s_ctrl - V4L2 sensor interface handler for VIDIOC_S_CTRL ioctl
 * @s: pointer to standard V4L2 device structure
 * @vc: standard V4L2 VIDIOC_S_CTRL ioctl structure
 *
 * If the requested control is supported, sets the control's current
 * value in HW (and updates the video_control[] array).  Otherwise,
 * returns -EINVAL if the control is not supported.
 */
static int ioctl_s_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int retval = 0;

	pr_debug("In s5k4ecgx:ioctl_s_ctrl %d\n",
		 vc->id);

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
		break;
	case V4L2_CID_CONTRAST:
		break;
	case V4L2_CID_SATURATION:
		break;
	case V4L2_CID_HUE:
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		break;
	case V4L2_CID_DO_WHITE_BALANCE:
		break;
	case V4L2_CID_RED_BALANCE:
		break;
	case V4L2_CID_BLUE_BALANCE:
		break;
	case V4L2_CID_GAMMA:
		break;
	case V4L2_CID_EXPOSURE:
		break;
	case V4L2_CID_AUTOGAIN:
		break;
	case V4L2_CID_GAIN:
		break;
	case V4L2_CID_HFLIP:
		break;
	case V4L2_CID_VFLIP:
		break;
	default:
		retval = -EPERM;
		break;
	}

	return retval;
}

/*!
 * ioctl_enum_framesizes - V4L2 sensor interface handler for
 *			   VIDIOC_ENUM_FRAMESIZES ioctl
 * @s: pointer to standard V4L2 device structure
 * @fsize: standard V4L2 VIDIOC_ENUM_FRAMESIZES ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ioctl_enum_framesizes(struct v4l2_int_device *s,
				 struct v4l2_frmsizeenum *fsize)
{
	if (fsize->index > s5k4ecgx_mode_MAX)
		return -EINVAL;

	fsize->pixel_format = s5k4ecgx_data.pix.pixelformat;
#if 0
	fsize->discrete.width =
			max(s5k4ecgx_mode_info_data[0][fsize->index].width,
			    s5k4ecgx_mode_info_data[1][fsize->index].width);
	fsize->discrete.height =
			max(s5k4ecgx_mode_info_data[0][fsize->index].height,
			    s5k4ecgx_mode_info_data[1][fsize->index].height);
#endif
	return 0;
}

/*!
 * ioctl_enum_frameintervals - V4L2 sensor interface handler for
 *			       VIDIOC_ENUM_FRAMEINTERVALS ioctl
 * @s: pointer to standard V4L2 device structure
 * @fival: standard V4L2 VIDIOC_ENUM_FRAMEINTERVALS ioctl structure
 *
 * Return 0 if successful, otherwise -EINVAL.
 */
static int ioctl_enum_frameintervals(struct v4l2_int_device *s,
					 struct v4l2_frmivalenum *fival)
{
	int i, j, count = 0;

	fival->type = V4L2_FRMIVAL_TYPE_DISCRETE;
	fival->discrete.numerator = 1;

#if 0
	for (i = 0; i < ARRAY_SIZE(s5k4ecgx_mode_info_data); i++)
		for (j = 0; j < (s5k4ecgx_mode_MAX + 1); j++)
			if (fival->pixel_format == s5k4ecgx_data.pix.pixelformat
			 && fival->width == s5k4ecgx_mode_info_data[i][j].width
			 && fival->height == s5k4ecgx_mode_info_data[i][j].height
			 && s5k4ecgx_mode_info_data[i][j].init_data_ptr != NULL
			 && fival->index == count++) {
				fival->discrete.denominator =
						s5k4ecgx_framerates[i];
				return 0;
			}
#endif

	return -EINVAL;
}
static int ioctl_g_ctrl(struct v4l2_int_device *s, struct v4l2_control *vc)
{
	int ret = 0;

	MANGO_DBG_DEFAULT;

	switch (vc->id) {
	case V4L2_CID_BRIGHTNESS:
//		vc->value = ov5640_data.brightness;
		break;
	case V4L2_CID_HUE:
//		vc->value = ov5640_data.hue;
		break;
	case V4L2_CID_CONTRAST:
//		vc->value = ov5640_data.contrast;
		break;
	case V4L2_CID_SATURATION:
//		vc->value = ov5640_data.saturation;
		break;
	case V4L2_CID_RED_BALANCE:
//		vc->value = ov5640_data.red;
		break;
	case V4L2_CID_BLUE_BALANCE:
//		vc->value = ov5640_data.blue;
		break;
	case V4L2_CID_EXPOSURE:
//		vc->value = ov5640_data.ae_mode;
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

/*!
 * ioctl_g_chip_ident - V4L2 sensor interface handler for
 *			VIDIOC_DBG_G_CHIP_IDENT ioctl
 * @s: pointer to standard V4L2 device structure
 * @id: pointer to int
 *
 * Return 0.
 */
static int ioctl_g_chip_ident(struct v4l2_int_device *s, int *id)
{
	((struct v4l2_dbg_chip_ident *)id)->match.type =
					V4L2_CHIP_MATCH_I2C_DRIVER;
	strcpy(((struct v4l2_dbg_chip_ident *)id)->match.name,
		"s5k4ecgx_mipi_camera");

	return 0;
}

/*!
 * ioctl_init - V4L2 sensor interface handler for VIDIOC_INT_INIT
 * @s: pointer to standard V4L2 device structure
 */
static int ioctl_init(struct v4l2_int_device *s)
{

	return 0;
}

/*!
 * ioctl_enum_fmt_cap - V4L2 sensor interface handler for VIDIOC_ENUM_FMT
 * @s: pointer to standard V4L2 device structure
 * @fmt: pointer to standard V4L2 fmt description structure
 *
 * Return 0.
 */
static int ioctl_enum_fmt_cap(struct v4l2_int_device *s,
			      struct v4l2_fmtdesc *fmt)
{
	if (fmt->index > s5k4ecgx_mode_MAX)
		return -EINVAL;

	fmt->pixelformat = s5k4ecgx_data.pix.pixelformat;

	return 0;
}

/*!
 * ioctl_dev_init - V4L2 sensor interface handler for vidioc_int_dev_init_num
 * @s: pointer to standard V4L2 device structure
 *
 * Initialise the device when slave attaches to the master.
 */
static int ioctl_dev_init(struct v4l2_int_device *s)
{
	struct sensor_data *sensor = s->priv;
	u32 tgt_xclk;	/* target xclk */
	u32 tgt_fps;	/* target frames per secound */
	int ret;
	enum s5k4ecgx_frame_rate frame_rate;
	void *mipi_csi2_info;

	MANGO_DBG_DEFAULT;
	s5k4ecgx_data.on = true;

	/* mclk */
	tgt_xclk = s5k4ecgx_data.mclk;
	tgt_xclk = min(tgt_xclk, (u32)S5K4ECGX_XCLK_MAX);
	tgt_xclk = max(tgt_xclk, (u32)S5K4ECGX_XCLK_MIN);
	s5k4ecgx_data.mclk = tgt_xclk;

	pr_debug("   Setting mclk to %d MHz\n", tgt_xclk / 1000000);

	/* Default camera frame rate is set in probe */
	tgt_fps = sensor->streamcap.timeperframe.denominator /
		  sensor->streamcap.timeperframe.numerator;

	if (tgt_fps == 15)
		frame_rate = s5k4ecgx_15_fps;
	else if (tgt_fps == 30)
		frame_rate = s5k4ecgx_30_fps;
	else
		return -EINVAL; /* Only support 15fps or 30fps now. */

	mipi_csi2_info = mipi_csi2_get_info();

	/* enable mipi csi2 */
	if (mipi_csi2_info)
		mipi_csi2_enable(mipi_csi2_info);
	else {
		printk(KERN_ERR "%s() in %s: Fail to get mipi_csi2_info!\n",
		       __func__, __FILE__);
		return -EPERM;
	}

#if 0
	{
	s5k4ecgx_burst_write_regs(s5k4ecgx_init_reg, sizeof(s5k4ecgx_init_reg)/sizeof(s5k4ecgx_init_reg[0]),NULL);

//by crazyboy 20160104 by treego For Preview Test
	s5k4ecgx_write_regs(s5k4ecgx_1MP_Preview, sizeof(s5k4ecgx_1MP_Preview)/sizeof(s5k4ecgx_1MP_Preview[0]));
	s5k4ecgx_write_regs(s5k4ecgx_FPS_30, sizeof(s5k4ecgx_FPS_30)/sizeof(s5k4ecgx_FPS_30[0]));
	}
#endif
	ret = s5k4ecgx_init_mode(frame_rate, s5k4ecgx_mode_INIT, s5k4ecgx_mode_INIT);

	return ret;
}

/*!
 * ioctl_dev_exit - V4L2 sensor interface handler for vidioc_int_dev_exit_num
 * @s: pointer to standard V4L2 device structure
 *
 * Delinitialise the device when slave detaches to the master.
 */
static int ioctl_dev_exit(struct v4l2_int_device *s)
{
	void *mipi_csi2_info;

	mipi_csi2_info = mipi_csi2_get_info();

	/* disable mipi csi2 */
	if (mipi_csi2_info)
		if (mipi_csi2_get_status(mipi_csi2_info))
			mipi_csi2_disable(mipi_csi2_info);

	return 0;
}

/*!
 * This structure defines all the ioctls for this module and links them to the
 * enumeration.
 */
static struct v4l2_int_ioctl_desc s5k4ecgx_ioctl_desc[] = {
	{vidioc_int_dev_init_num, (v4l2_int_ioctl_func *) ioctl_dev_init},
	{vidioc_int_dev_exit_num, ioctl_dev_exit},
	{vidioc_int_s_power_num, (v4l2_int_ioctl_func *) ioctl_s_power},
	{vidioc_int_g_ifparm_num, (v4l2_int_ioctl_func *) ioctl_g_ifparm},
/*	{vidioc_int_g_needs_reset_num,
				(v4l2_int_ioctl_func *)ioctl_g_needs_reset}, */
/*	{vidioc_int_reset_num, (v4l2_int_ioctl_func *)ioctl_reset}, */
	{vidioc_int_init_num, (v4l2_int_ioctl_func *) ioctl_init},
	{vidioc_int_enum_fmt_cap_num,
				(v4l2_int_ioctl_func *) ioctl_enum_fmt_cap},
/*	{vidioc_int_try_fmt_cap_num,
				(v4l2_int_ioctl_func *)ioctl_try_fmt_cap}, */
	{vidioc_int_g_fmt_cap_num, (v4l2_int_ioctl_func *) ioctl_g_fmt_cap},
/*	{vidioc_int_s_fmt_cap_num, (v4l2_int_ioctl_func *) ioctl_s_fmt_cap}, */
	{vidioc_int_g_parm_num, (v4l2_int_ioctl_func *) ioctl_g_parm},
	{vidioc_int_s_parm_num, (v4l2_int_ioctl_func *) ioctl_s_parm},
/*	{vidioc_int_queryctrl_num, (v4l2_int_ioctl_func *)ioctl_queryctrl}, */
	{vidioc_int_g_ctrl_num, (v4l2_int_ioctl_func *) ioctl_g_ctrl},
	{vidioc_int_s_ctrl_num, (v4l2_int_ioctl_func *) ioctl_s_ctrl},
	{vidioc_int_enum_framesizes_num,
				(v4l2_int_ioctl_func *) ioctl_enum_framesizes},
	{vidioc_int_enum_frameintervals_num,
			(v4l2_int_ioctl_func *) ioctl_enum_frameintervals},
	{vidioc_int_g_chip_ident_num,
				(v4l2_int_ioctl_func *) ioctl_g_chip_ident},
};

static struct v4l2_int_slave s5k4ecgx_slave = {
	.ioctls = s5k4ecgx_ioctl_desc,
	.num_ioctls = ARRAY_SIZE(s5k4ecgx_ioctl_desc),
};

static struct v4l2_int_device s5k4ecgx_int_device = {
	.module = THIS_MODULE,
	.name = "s5k4ecgx",
	.type = v4l2_int_type_slave,
	.u = {
		.slave = &s5k4ecgx_slave,
	},
};

/*!
 * s5k4ecgx I2C probe function
 *
 * @param adapter            struct i2c_adapter *
 * @return  Error code indicating success or failure
 */
static int s5k4ecgx_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	int retval;
	struct regmap *gpr;
	u8 chip_id_high[2] = {0}, chip_id_low;

	 //by crazyboy 20160115 by treego  IPU0-CSI0 MIPI
 	gpr = syscon_regmap_lookup_by_compatible("fsl,imx6q-iomuxc-gpr");
 	if(!IS_ERR(gpr)){
 	    printk("[CRZ] find iomuxc registers\n");//by crazyboy 20160115 by treego  
 	    regmap_update_bits(gpr, IOMUXC_GPR1, 1 << 19, 0 << 19); //mipi
 	}

	/* Set initial values for the sensor struct. */
	memset(&s5k4ecgx_data, 0, sizeof(s5k4ecgx_data));
	s5k4ecgx_data.sensor_clk = devm_clk_get(dev, "csi_mclk");
	if (IS_ERR(s5k4ecgx_data.sensor_clk)) {
		/* assuming clock enabled by default */
		s5k4ecgx_data.sensor_clk = NULL;
		dev_err(dev, "clock-frequency missing or invalid\n");
		return PTR_ERR(s5k4ecgx_data.sensor_clk);
	}

	retval = of_property_read_u32(dev->of_node, "mclk",
					&(s5k4ecgx_data.mclk));
	if (retval) {
		dev_err(dev, "mclk missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "mclk_source",
					(u32 *) &(s5k4ecgx_data.mclk_source));
	if (retval) {
		dev_err(dev, "mclk_source missing or invalid\n");
		return retval;
	}

	retval = of_property_read_u32(dev->of_node, "csi_id",
					&(s5k4ecgx_data.csi));
	if (retval) {
		dev_err(dev, "csi id missing or invalid\n");
		return retval;
	}

	clk_prepare_enable(s5k4ecgx_data.sensor_clk);

	//s5k4ecgx_data.io_init = s5k4ecgx_reset;
	s5k4ecgx_data.i2c_client = client;
	s5k4ecgx_data.pix.pixelformat = V4L2_PIX_FMT_UYVY;
	s5k4ecgx_data.pix.width = 640;
	s5k4ecgx_data.pix.height = 480;
	s5k4ecgx_data.streamcap.capability = V4L2_MODE_HIGHQUALITY |
					   V4L2_CAP_TIMEPERFRAME;
	s5k4ecgx_data.streamcap.capturemode = 0;
	s5k4ecgx_data.streamcap.timeperframe.denominator = DEFAULT_FPS;
	s5k4ecgx_data.streamcap.timeperframe.numerator = 1;


	fxl6408_CamPower_init(s5k4ecgx_data.i2c_client);

	//s5k4ecgx_burst_write_regs(s5k4ecgx_init_reg, sizeof(s5k4ecgx_init_reg)/sizeof(s5k4ecgx_init_reg[0]),NULL);
#if 1
	s5k4ecgx_int_device.priv = &s5k4ecgx_data;
	retval = v4l2_int_device_register(&s5k4ecgx_int_device);

//	clk_disable_unprepare(s5k4ecgx_data.sensor_clk);
#endif

	pr_info("camera s5k4ecgx_mipi is found\n");
	return retval;
}

/*!
 * s5k4ecgx I2C detach function
 *
 * @param client            struct i2c_client *
 * @return  Error code indicating success or failure
 */
static int s5k4ecgx_remove(struct i2c_client *client)
{
//	v4l2_int_device_unregister(&s5k4ecgx_int_device);

	if (gpo_regulator)
		regulator_disable(gpo_regulator);

	if (analog_regulator)
		regulator_disable(analog_regulator);

	if (core_regulator)
		regulator_disable(core_regulator);

	if (io_regulator)
		regulator_disable(io_regulator);

	return 0;
}

/*!
 * s5k4ecgx init function
 * Called by insmod s5k4ecgx_camera.ko.
 *
 * @return  Error code indicating success or failure
 */
static __init int s5k4ecgx_init(void)
{
	u8 err;

	err = i2c_add_driver(&s5k4ecgx_i2c_driver);
	if (err != 0)
		pr_err("%s:driver registration failed, error=%d\n",
			__func__, err);

	return err;
}

/*!
 * s5k4ecgx cleanup function
 * Called on rmmod s5k4ecgx_camera.ko
 *
 * @return  Error code indicating success or failure
 */
static void __exit s5k4ecgx_clean(void)
{
	i2c_del_driver(&s5k4ecgx_i2c_driver);
}

//module_i2c_driver(s5k4ecgx_i2c_driver);
module_init(s5k4ecgx_init);
module_exit(s5k4ecgx_clean);

MODULE_AUTHOR("Freescale Semiconductor, Inc.");
MODULE_DESCRIPTION("s5k4ecgx MIPI Camera Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_ALIAS("CSI");
