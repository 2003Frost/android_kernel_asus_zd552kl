/*
 *
 * FocalTech TouchScreen driver.
 *
 * Copyright (c) 2010-2016, Focaltech Ltd. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
/*****************************************************************************
*
* File Name: focaltech_core.h

* Author: Focaltech Driver Team
*
* Created: 2016-08-08
*
* Abstract:
*
* Reference:
*
*****************************************************************************/

#ifndef __LINUX_FOCALTECH_CORE_H__
#define __LINUX_FOCALTECH_CORE_H__
/*****************************************************************************
* Included header files
*****************************************************************************/
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/firmware.h>
#include <linux/debugfs.h>
//#include <linux/sensors.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/workqueue.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mount.h>
#include <linux/netdevice.h>
#include <linux/unistd.h>
#include <linux/ioctl.h>
#include "focaltech_common.h"
#include "focaltech_flash.h"
//<ASUS-BSP Robert_He 20170310> add switch touch firmware node ++++++
#include <linux/switch.h>
//<ASUS-BSP Robert_He 20170310> add switch touch firmware node ------
//<ASUS-BSP Robert_He 20170609> add wake_up lock to ensure suspend/resume++++++
#include <linux/wakelock.h>
//<ASUS-BSP Robert_He 20170609> add wake_up lock to ensure suspend/resume------


/*****************************************************************************
* Private constant and macro definitions using #define
*****************************************************************************/
#define LEN_FLASH_ECC_MAX                   0xFFFE

#if 0
#define FTS_WORKQUEUE_NAME                  "fts_wq"
#endif

#define FTS_KEY_WIDTH                       50
#define FTS_ONE_TCH_LEN                     6
#define POINT_READ_BUF  (3 + FTS_ONE_TCH_LEN * FTS_MAX_POINTS)

#define FTS_MAX_ID                          0x0F
#define FTS_TOUCH_X_H_POS                   3
#define FTS_TOUCH_X_L_POS                   4
#define FTS_TOUCH_Y_H_POS                   5
#define FTS_TOUCH_Y_L_POS                   6
#define FTS_TOUCH_PRE_POS                   7
#define FTS_TOUCH_AREA_POS                  8
#define FTS_TOUCH_POINT_NUM                 2
#define FTS_TOUCH_EVENT_POS                 3
#define FTS_TOUCH_ID_POS                    5
#define FTS_COORDS_ARR_SIZE                 4

#define FTS_TOUCH_DOWN      0
#define FTS_TOUCH_UP        1
#define FTS_TOUCH_CONTACT   2

#define FTS_SYSFS_ECHO_ON(buf)		((strnicmp(buf, "1", 1)  == 0) || \
										(strnicmp(buf, "on", 2) == 0))
#define FTS_SYSFS_ECHO_OFF(buf)		((strnicmp(buf, "0", 1)  == 0) || \
										(strnicmp(buf, "off", 3) == 0))

/*****************************************************************************
* Private enumerations, structures and unions using typedef
*****************************************************************************/

struct fts_ts_platform_data {
    u32 irq_gpio;
    u32 irq_gpio_flags;
    u32 reset_gpio;
    u32 reset_gpio_flags;
    bool have_key;
    u32 key_number;
    u32 keys[4];
    u32 key_y_coord;
    u32 key_x_coords[4];
    u32 x_max;
    u32 y_max;
    u32 x_min;
    u32 y_min;
    u32 max_touch_number;
};

struct ts_event {
    u16 au16_x[FTS_MAX_POINTS]; /*x coordinate */
    u16 au16_y[FTS_MAX_POINTS]; /*y coordinate */
    u16 pressure[FTS_MAX_POINTS];
    u8 au8_touch_event[FTS_MAX_POINTS]; /* touch event: 0 -- down; 1-- up; 2 -- contact */
    u8 au8_finger_id[FTS_MAX_POINTS];   /*touch ID */
    u8 area[FTS_MAX_POINTS];
    u8 touch_point;
    u8 point_num;
};

struct fts_ts_data {
    struct i2c_client *client;
    struct input_dev *input_dev;
    struct ts_event event;
    const struct fts_ts_platform_data *pdata;
#if FTS_PSENSOR_EN
    struct fts_psensor_platform_data *psensor_pdata;
#endif
    struct work_struct touch_event_work;
    struct workqueue_struct *ts_workqueue;
    struct regulator *vdd;
    struct regulator *vcc_i2c;
//<Focal driver-team libaojian 20170828> add define about irq_lock and mutex ++++++
    spinlock_t irq_lock;
	bool irq_disable;
    struct mutex report_mutex;
//<Focal driver-team libaojian 20170828> add define about irq_lock and mutex -------
    u16 addr;
    bool suspended;
    u8 fw_ver[3];
    u8 fw_vendor_id;
    int touchs;
	//<ASUS-BSP Robert_He 20170310> add switch touch firmware node ++++++
	struct switch_dev touch_sdev;
	//<ASUS-BSP Robert_He 20170310> add switch touch firmware node ------
#if defined(CONFIG_FB)
    struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
    struct early_suspend early_suspend;
#endif
//<ASUS-BSP Robert_He 20170525> add touch resume/suspend workqueue ++++++
	struct workqueue_struct *tp_suspend_resume_wk;
	struct work_struct touch_resume_work;
	struct work_struct touch_suspend_work;
//<ASUS-BSP Robert_He 20170525> add touch resume/suspend workqueue ------
//<ASUS-BSP Robert_He 20170609> add wake_up lock to ensure suspend/resume++++++
    struct wake_lock touch_wake_lock;
//<ASUS-BSP Robert_He 20170609> add wake_up lock to ensure suspend/resume++++++

};

#if FTS_PSENSOR_EN
struct fts_psensor_platform_data {
    struct input_dev *input_psensor_dev;
    struct sensors_classdev ps_cdev;
    int tp_psensor_opened;
    char tp_psensor_data;       /* 0 near, 1 far */
    struct fts_ts_data *data;
};

int fts_sensor_init(struct fts_ts_data *data);
int fts_sensor_read_data(struct fts_ts_data *data);
int fts_sensor_suspend(struct fts_ts_data *data);
int fts_sensor_resume(struct fts_ts_data *data);
int fts_sensor_remove(struct fts_ts_data *data);
#endif

/*****************************************************************************
* Static variables
*****************************************************************************/
extern struct i2c_client *fts_i2c_client;
extern struct fts_ts_data *fts_wq_data;
extern struct input_dev *fts_input_dev;

//upgrade functions
extern void fts_update_fw_vendor_id(struct fts_ts_data *data);
extern void fts_update_fw_ver(struct fts_ts_data *data);
#endif /* __LINUX_FOCALTECH_CORE_H__ */
