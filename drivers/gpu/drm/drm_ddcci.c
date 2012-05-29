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
#include "drmP.h"
#include "drm_ddcci.h"

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

/* High-level API */

struct ddcci_context {
	struct drm_device *dev;
	struct i2c_adapter *i2c;
	uint32_t vcp[256]; /* VCP IDs */

	/* quirks, etc. */
};

static bool
ddcci_parse_caps(struct ddcci_context *ctx, struct edid *edid)
{
	bool ret = false;
	u8 *caps = NULL;

	caps = ddcci_get_vcp_capabilities(ctx->i2c);
	if (!caps)
		goto out;

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
