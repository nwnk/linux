/*
 * Copyright 2012 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * them Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTIBILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *	Adam Jackson <ajax@redhat.com>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/export.h>
#include <linux/ctype.h>
#include "drmP.h"
#include "drm_ddcci.h"
#include "drm_vcp.h"

/*
 * Some notes on the protocol, since the docs are... special:
 *
 * The "write" buffer as represented in this file does not include the standard
 * I2C destination address (0x6E on the wire, 0x37 in ddcci_write_read).  It
 * does, however, include a "virtual source address" of 0x51, in every message.
 * This is because DDC/CI inherits from ACCESS.bus, which wanted some kind of
 * address assignment scheme when used as a proper bus.  We never assign
 * addresses here, we just use the 0x51 default.
 *
 * The second byte of the write buffer is the "length" of the write, but with
 * the high bit always set.  Again this is A.b being weird, that bit
 * distinguishes commands from bulk data stream (a hilarious idea on a 100kbit
 * bus).  The length includes the command (the CI_* defines below) and the
 * payload of the command, not including the final checksum byte.
 *
 * Both writes and reads tend to have a turnaround time: the host must wait
 * both for the write to be interpreted before reading, and after the read
 * has completed before submitting another command.
 *
 * DDC sets the maximum transfer size to 128 bytes to limit I2C bus
 * contention.  DDC/CI thinks that doesn't go too far enough, and sets the
 * MTU to 32 bytes to limit I2C bus contention.  More fundamentally this
 * means the monitor's buffer is usually not bigger than 32 bytes, so you
 * really do need to do things in chunks.  That 32 bytes appears to mean
 * the "length" as computed in the length field of the reply, ie, not
 * including address and checksum framing.
 */

/* ACCESS.bus protocol details */
#define CI_FEATURE_REQ		0x01
#define CI_FEATURE_REP		0x02
#define CI_SET_FEATURE_REQ	0x03
#define CI_TIMING_REP		0x06
#define CI_TIMING_REQ		0x07
#define CI_SET_FEATURE_REP	0x09
#define CI_SAVE			0x0C
#define CI_IDENTIFICATION_REP	0xE1
#define CI_TABLE_READ_REQ	0xE2
#define CI_CAPABILITY_REP	0xE3
#define CI_TABLE_READ_REP	0xE4
#define CI_TABLE_WRITE		0xE7
#define CI_IDENTIFICATION_REQ	0xF1
#define CI_CAPABILITY_REQ	0xF3
#define CI_ENABLE_APP_REPORT	0xF5

static u8
writeChecksum(u8 *buf, int len)
{
	u8 x = 0x6E;
	int i;

	for (i = 0; i < len; i++)
		x ^= buf[i];

	return x;
}

#define checksum(x) do { \
	x[ARRAY_SIZE(x) - 1] = writeChecksum(x, ARRAY_SIZE(x) - 1); \
} while (0)

/* Basic protocol interface */

/* XXX add retry here?  or in callers?  probably will be clear once you
 * have table r/w */
static bool
ddcci_write_read(struct i2c_adapter *i2c, u8 *wbuf, int wlen,
		 u8 *rbuf, int rlen, int wait)
{
	int ret;
	struct i2c_msg rmsg = {
		.addr = 0x37,
		.flags = 0,
		.len = wlen,
		.buf = wbuf,
	};
	struct i2c_msg wmsg = {
		.addr = 0x37,
		.flags = I2C_M_RD,
		.len = rlen,
		.buf = rbuf,
	};

	ret = i2c_transfer(i2c, &rmsg, 1);
	if (ret != 1)
		return false;

	if (wait)
		msleep(wait);

	ret = i2c_transfer(i2c, &wmsg, 1);
	if (ret != 1)
		return false;

	return true;
}

struct ddcci_feature {
	u8 opcode;
	u8 type;
	u16 max_value;
	u16 current_value;
};

static bool
ddcci_get_vcp_feature(struct i2c_adapter *i2c, u8 opcode,
		      struct ddcci_feature *ret)
{
	u8 wbuf[5];
	u8 rbuf[12];

	wbuf[0] = 0x51;
	wbuf[1] = 0x82;
	wbuf[2] = CI_FEATURE_REQ;
	wbuf[3] = opcode;
	checksum(wbuf);

	if (!ddcci_write_read(i2c, wbuf, ARRAY_SIZE(wbuf),
			      rbuf, ARRAY_SIZE(rbuf), 40))
		return false;

	if (rbuf[3] != 0) /* unsupported VCP code */
		return false;

	ret->opcode = rbuf[4];
	ret->type = rbuf[5];
	ret->max_value = (rbuf[6] << 8) + rbuf[7];
	ret->current_value = (rbuf[8] << 8) + rbuf[9];

	return ret;
}

static bool
ddcci_set_vcp_feature(struct i2c_adapter *i2c, u8 opcode, u16 val)
{
	u8 wbuf[7];

	wbuf[0] = 0x51;
	wbuf[1] = 0x84;
	wbuf[2] = CI_SET_FEATURE_REQ;
	wbuf[3] = opcode;
	wbuf[4] = (val & 0xff00) >> 8;
	wbuf[5] = val & 0xff;
	checksum(wbuf);

	return ddcci_write_read(i2c, wbuf, ARRAY_SIZE(wbuf), NULL, 0, 50);
}

/**
 * Fetch the DDC/CI capability string
 *
 * @connector: Connector to probe
 * @adapter: Associated i2c bus
 *
 * Attempt to fetch the DDC/CI capability string from the connected display.
 * Parsing that beast is a whole other problem.  Returns the string, or
 * NULL on error.
 */
static u8 *
ddcci_get_vcp_capabilities(struct i2c_adapter *i2c)
{
	u8 *ret = NULL, *new = NULL;
	u8 wbuf[6];
	u8 rbuf[64];
	int offset = 0;

	wbuf[0] = 0x51;
	wbuf[1] = 0x83;
	wbuf[2] = CI_CAPABILITY_REQ;

	while (1) {
		wbuf[3] = (offset & 0xff00) >> 8;
		wbuf[4] = offset & 0xff;
		checksum(wbuf);

		/*
		 * The 50ms turnaround isn't actually documented in the spec,
		 * but anything faster seems to be unreliable.
		 */
		if (!ddcci_write_read(i2c, wbuf, ARRAY_SIZE(wbuf),
				      rbuf, ARRAY_SIZE(rbuf), 50)) {
			kfree(ret);
			break;
		}

		if (rbuf[1] <= 0x83)
			break;

		offset += rbuf[1] - 0x83;
		new = krealloc(ret, offset + 1, GFP_KERNEL);
		if (!new) {
			kfree(ret);
			break;
		} else if (!ret) {
		    new[0] = '\0'; /* for first time through */
		}
		ret = new;

		if (rbuf[1] >= 0x83)
			strncat(ret, rbuf + 5, rbuf[1] - 0x83);
	}

	return ret;
}

static bool
ddcci_application_reports(struct i2c_adapter *i2c, bool on)
{
	u8 wbuf[5];

	wbuf[0] = 0x51;
	wbuf[1] = 0x81;
	wbuf[2] = CI_ENABLE_APP_REPORT;
	wbuf[3] = on;
	checksum(wbuf);

	return ddcci_write_read(i2c, wbuf, ARRAY_SIZE(wbuf), NULL, 0, 50);
}

struct ddcci_timing_report {
	u8 status;
	u16 hsync;
	u16 vsync;
};

/*
 * Timing reports don't work until you turn application reports on.  You
 * want not to leave application reports on, because on some monitors that
 * will move DPMS control into DDC/CI, which leads to weird things like the
 * power button on the display not working.
 */
static bool
ddcci_get_timing_report(struct i2c_adapter *i2c,
			struct ddcci_timing_report *ret)
{
	u8 wbuf[4];
	u8 rbuf[32];

	wbuf[0] = 0x51;
	wbuf[1] = 0x81;
	wbuf[2] = CI_TIMING_REQ;
	checksum(wbuf);

	if (!ddcci_application_reports(i2c, true))
		return false;

	if (!ddcci_write_read(i2c, wbuf, ARRAY_SIZE(wbuf),
			      rbuf, ARRAY_SIZE(rbuf), 40))
		return false;

	ret->status = rbuf[3];
	ret->hsync = (rbuf[4] << 8) + rbuf[5];
	ret->vsync = (rbuf[6] << 8) + rbuf[7];

	ddcci_application_reports(i2c, false);
	return true;
}

/* property glue */

/* DRM_MODE_PROP_IMMUTABLE */

/*
 * Things that probably want special handling, or hiding:
 * VCP_SATURATION_* and VCP_HUE_* for 6-axis control
 * VCP_WINDOW_*
 * VCP_*_FREQUENCY
 * VCP_DISPLAY_FIRMWARE
 */

#define IMMUTABLE DRM_MODE_PROP_IMMUTABLE

struct vcp_info {
	u8 vcp;
	int flags;
	const char *name;
} vcp_info[] = {
	{ VCP_BACKLIGHT, 0, "Backlight" },
	{ VCP_BLACK_LEVEL_BLUE, 0, "Black level (blue)" },
	{ VCP_BLACK_LEVEL_GREEN, 0, "Black level (green)" },
	{ VCP_BLACK_LEVEL_RED, 0, "Black level (red)" },
	{ VCP_CLOCK, 0, "Clock" },
	{ VCP_CLOCK_PHASE, 0, "Clock phase" },
	{ VCP_CONTRAST, 0, "Contrast" },
	{ VCP_DISPLAY_USAGE_TIME, IMMUTABLE, "Display usage time (hours)" },
	{ VCP_FOCUS, 0, "Focus" },
	{ VCP_HORIZONTAL_MOIRE, 0, "Horizontal moire" },
	{ VCP_HUE, 0, "Hue" },
	{ VCP_LUMINANCE, 0, "Luminance" },
	{ VCP_SATURATION, 0, "Saturation" },
	{ VCP_SHARPNESS, 0, "Sharpness" },
	{ VCP_TV_BLACK_LEVEL, 0, "TV Black level" },
	{ VCP_TV_CONTRAST, 0, "TV Contrast" },
	{ VCP_TV_SHARPNESS, 0, "TV Sharpness" },
	{ VCP_VELOCITY_MODULATION, 0, "Velocity modulation" },
	{ VCP_VERTICAL_MOIRE, 0, "Vertical moire" },
	{ VCP_VIDEO_GAIN_BLUE, 0, "Video gain (blue)" },
	{ VCP_VIDEO_GAIN_GREEN, 0, "Video gain (green)" },
	{ VCP_VIDEO_GAIN_RED, 0, "Video gain (red)" },
	{ VCP_VISION_COMPENSATION, 0, "Vision compensation" },
	{ VCP_ZOOM, 0, "Zoom" },
};

static bool
ddcci_vcp_is_table(u8 vcp)
{
	switch (vcp) {
	case VCP_INPUT_SOURCE:
	case VCP_LUT_SIZE:
	case VCP_SINGLE_POINT_LUT:
	case VCP_BLOCK_LUT:
	case VCP_RPC:
	case VCP_EDID:
	case VCP_WINDOW_CONTROL:
	case VCP_SOURCE_TIMING_MODE:
	case VCP_DISPLAY_DESCRIPTOR:
	case VCP_AUX_DISPLAY_DATA:
	case VCP_OUTPUT_SELECT:
	case VCP_ASSET_TAG:
		return true;
	default:
		return false;
	}
}

static struct vcp_info *
ddcci_get_vcp_info(u8 vcp)
{
	int i;

	if (vcp >= 0xe0)
		return NULL; /* no vendor-specific vcp yet */

	if (ddcci_vcp_is_table(vcp))
		return NULL; /* no table r/w support yet */

	for (i = 0; i < ARRAY_SIZE(vcp_info); i++)
		if (vcp_info[i].vcp == vcp)
			return &vcp_info[i];

	return NULL;
}

/* High-level API */

struct ddcci_context {
	struct drm_device *dev;
	struct i2c_adapter *i2c;
	uint32_t vcp[256]; /* VCP IDs */

	/* quirks, etc. */
};

/* works a byte at a time, since spaces are optional */
static u8
ddcci_strtou8(const char *in)
{
	u8 buf[3] = { *in, *(in + 1), 0 };

	return simple_strtoul(buf, NULL, 16);
}

static void
ddcci_make_property(struct ddcci_context *ctx, u8 vcp)
{
	struct vcp_info *v;
	struct ddcci_feature f;
	struct drm_property *prop;

	if (!(v = ddcci_get_vcp_info(vcp)))
		return;

	if (!ddcci_get_vcp_feature(ctx->i2c, vcp, &f))
		return;

	if (!(prop = drm_property_create_range(ctx->dev, v->flags, v->name,
					       0, f.max_value)))
		return;

	ctx->vcp[vcp] = prop->base.id;
}

static u8 *
ddcci_parse_enum(struct ddcci_context *ctx, u8 *i)
{
	/* we don't handle these yet */
	char *closeparen = strchr(i, ')');

	return closeparen ? closeparen + 1 : NULL;
}

static bool
ddcci_parse_caps(struct ddcci_context *ctx, struct edid *edid)
{
	bool ret = false;
	u8 *caps = NULL, *i = NULL;
	int vcp = -1, len;

	caps = ddcci_get_vcp_capabilities(ctx->i2c);
	if (!caps)
		goto out;
	len = strlen(caps);

	/* not an elegant parser */
	if (!(i = strstr(caps, "vcp(")))
		goto out;

	for (i += 4; i && *i && (i < caps + len); i++) {
		if (isspace(*i))
			continue;

		if (!(isxdigit(*i) && isxdigit(*(i+1))))
			break; /* we must be done */

		vcp = ddcci_strtou8(i);

		if (*(i+2) == '(') {
			i = ddcci_parse_enum(ctx, i);
		} else {
			ddcci_make_property(ctx, vcp);
			i += 2;
		}
	}

out:
	kfree(caps);
	return ret;
}

/**
 * Probe for DDC/CI support
 *
 * @i2c: Associated i2c bus
 * @edid: EDID of the connected display
 *
 * Asks whether DDC/CI is usable on this bus.  Some displays require poking
 * before DDC/CI will work.  This routine will attempt to enable DDC/CI
 * using sink-specific methods if necessary, based on the vendor/model
 * tuple in EDID.
 *
 * Returns a context for future DDC/CI operation, since other commands may
 * require knowing additional quirks.  The context is opaque, and is freed
 * with kfree().
 *
 * TODO:
 *	actually do enable quirks.
 *	get VCP string, parse, fill in ->vcp
 */
struct ddcci_context *
ddcci_probe(struct drm_device *dev, struct i2c_adapter *i2c, struct edid *edid)
{
	u8 buf = 0;
	struct ddcci_context *ctx = NULL;

	if (!ddcci_write_read(i2c, &buf, 1, &buf, 1, 0))
		goto out;

	ctx = kmalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		goto out;

	if (!ddcci_parse_caps(ctx, edid))
		goto out;

	ctx->dev = dev;

	return ctx;

out:
	kfree(ctx);
	return NULL;
}
EXPORT_SYMBOL(ddcci_probe);

/**
 * Verify sink sync
 *
 * @i2c: Associated i2c bus
 * @mode: The mode we're attempting to set.
 *
 * Ask the monitor whether it has synced to what we're sending it.  @mode
 * should be the adjusted mode, as opposed to the user mode.
 *
 * TODO: The 'mode' argument is not used. We're not checking the sync polarity
 * bits, though we could.  At least on some DP monitors the sync bits are
 * fictitious, since DP doesn't really have the same concept of sync
 * signalling.  We're also not checking the reported frequencies, which are in
 * odd units (hsync in tens of Hz, vsync in hundredths of Hz) that are more
 * precise than what drm_mode_{hsync, vrefresh} will give us.
 */
enum ddcci_sync_state
ddcci_get_sync_state(struct ddcci_context *ctx, struct drm_display_mode *mode)
{
	struct ddcci_timing_report report;

	if (!ddcci_get_timing_report(ctx->i2c, &report))
		return ddcci_sync_state_unknown;

	if (!(report.status & 0x80))
		return ddcci_sync_state_failed;

	if (report.status & 0x40)
		return ddcci_sync_state_unstable;

	return ddcci_sync_state_synced;
}
EXPORT_SYMBOL(ddcci_get_sync_state);

/**
 * Get DDC/CI's notion of sink power
 *
 * @ctx: DDC/CI context
 *
 * Looks at the VCP for power state.  1-4 are DPMS on/standy/suspend/off,
 * but 0 and 5 (depending on sink) are used for "physically powered off".
 * Usually the driver wants to treat that as if it were disconnected.
 */
enum drm_connector_status
ddcci_get_sink_power(struct ddcci_context *ctx)
{
	struct ddcci_feature dpms;

#if 0
	/*
	 * TODO: once we fill in ->vcp, we should be sure to mask off
	 * VCP_POWER for displays where we don't get an HPD when the power
	 * button is pushed.  Otherwise we'd have to poll to get state
	 * changes right, and nobody likes polling.
	 */
	if (ctx->vcp[VCP_POWER] == 0)
		return connector_status_unknown;
#endif

	if (!ddcci_get_vcp_feature(ctx->i2c, VCP_POWER, &dpms))
		return connector_status_unknown;

	switch (dpms.current_value) {
	case 0:
	case 5:
		return connector_status_disconnected;
	default:
		return connector_status_connected;
	}
}
EXPORT_SYMBOL(ddcci_get_sink_power);
