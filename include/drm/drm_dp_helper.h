/*
 * Copyright © 2008 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#ifndef _DRM_DP_HELPER_H_
#define _DRM_DP_HELPER_H_

#include <linux/types.h>
#include <linux/i2c.h>

/* From the VESA DisplayPort spec */

#define AUX_NATIVE_WRITE	0x8
#define AUX_NATIVE_READ		0x9
#define AUX_I2C_WRITE		0x0
#define AUX_I2C_READ		0x1
#define AUX_I2C_STATUS		0x2
#define AUX_I2C_MOT		0x4

#define AUX_NATIVE_REPLY_ACK	(0x0 << 4)
#define AUX_NATIVE_REPLY_NACK	(0x1 << 4)
#define AUX_NATIVE_REPLY_DEFER	(0x2 << 4)
#define AUX_NATIVE_REPLY_MASK	(0x3 << 4)

#define AUX_I2C_REPLY_ACK	(0x0 << 6)
#define AUX_I2C_REPLY_NACK	(0x1 << 6)
#define AUX_I2C_REPLY_DEFER	(0x2 << 6)
#define AUX_I2C_REPLY_MASK	(0x3 << 6)

/* AUX CH addresses */
/* DPCD */
#define DP_DPCD_REV                         0x000

#define DP_MAX_LINK_RATE                    0x001

#define DP_MAX_LANE_COUNT                   0x002
# define DP_MAX_LANE_COUNT_MASK		    0x1f
# define DP_TPS3_SUPPORTED		    (1 << 6)
# define DP_ENHANCED_FRAME_CAP		    (1 << 7)

#define DP_MAX_DOWNSPREAD                   0x003
# define DP_NO_AUX_HANDSHAKE_LINK_TRAINING  (1 << 6)

#define DP_NORP                             0x004

#define DP_DOWNSTREAMPORT_PRESENT           0x005
# define DP_DWN_STRM_PORT_PRESENT           (1 << 0)
# define DP_DWN_STRM_PORT_TYPE_MASK         0x06
# define DP_DWN_STRM_PORT_TYPE_DP	    (0 << 1)
# define DP_DWN_STRM_PORT_TYPE_ANALOG	    (1 << 1)
# define DP_DWN_STRM_PORT_TYPE_TMDS	    (2 << 1)
# define DP_DWN_STRM_PORT_TYPE_OTHER	    (3 << 1)
# define DP_FORMAT_CONVERSION               (1 << 3)

#define DP_MAIN_LINK_CHANNEL_CODING         0x006

#define DP_DOWN_STREAM_PORT_COUNT	    0x007
# define DP_DWN_STREAM_PORT_COUNT_MASK	    0x0f
# define DP_OUI_SUPPORT			    (1 << 7)

#define DP_RECEIVE_PORT0_CAP_0		    0x008
#define DP_RECEIVE_PORT0_CAP_1		    0x009
#define DP_RECEIVE_PORT1_CAP_0		    0x00a
#define DP_RECEIVE_PORT1_CAP_1		    0x00b
/* receiver port capabilities */
# define DP_RECEIVE_PORT_CAP0_LOCAL_EDID_PRESENT	(1 << 1)
# define DP_RECEIVE_PORT_CAP0_ASSOCIATED_TO_PREVIOUS	(1 << 2)
# define DP_RECEIVE_PORT_CAP1_BUFFER_SIZE(x)		((x + 1) * 32)

#define DP_I2C_SPEED_CAP		    0x00c
# define DP_I2C_SPEED_1K		    (1 << 0)
# define DP_I2C_SPEED_5K		    (1 << 1)
# define DP_I2C_SPEED_10K		    (1 << 2)
# define DP_I2C_SPEED_100K		    (1 << 3)
# define DP_I2C_SPEED_400K		    (1 << 4)
# define DP_I2C_SPEED_1M		    (1 << 5)

#define DP_EDP_CONFIGURATION_CAP	    0x00d
# define DP_EDP_ALTERNATE_SCRAMBER_RESET_ENABLE	(1 << 0)
# define DP_EDP_FRAMING_CHANGE_CAPABLE		(1 << 1)

#define DP_TRAINING_AUX_RD_INTERVAL         0x00e

#define DP_MSTM_CAP			    0x021
# define DP_MST_CAPABLE			    (1 << 0)

#define DP_DOWN_STREAM_PORT_CAP(x)	    (0x080 + (x & 0x7f))
# define DP_DWN_STRM_PORT_TYPE_DP	    (0 << 0)
# define DP_DWN_STRM_PORT_TYPE_VGA	    (1 << 0)
# define DP_DWN_STRM_PORT_TYPE_DVI	    (2 << 0)
# define DP_DWN_STRM_PORT_TYPE_HDMI	    (3 << 0)
# define DP_DWN_STRM_PORT_TYPE_NON_EDID	    (4 << 0)
# define DP_DWN_STRM_PORT_TYPE_MASK	    0x07
# define DP_DWN_STRM_PORT_HPD		    (1 << 3)

/* link configuration */
#define	DP_LINK_BW_SET		            0x100
# define DP_LINK_BW_1_62		    0x06
# define DP_LINK_BW_2_7			    0x0a
# define DP_LINK_BW_5_4			    0x14

#define DP_LANE_COUNT_SET	            0x101
# define DP_LANE_COUNT_MASK		    0x0f
# define DP_LANE_COUNT_ENHANCED_FRAME_EN    (1 << 7)

#define DP_TRAINING_PATTERN_SET	            0x102
# define DP_TRAINING_PATTERN_DISABLE	    (0 << 0)
# define DP_TRAINING_PATTERN_1		    (1 << 0)
# define DP_TRAINING_PATTERN_2		    (2 << 0)
# define DP_TRAINING_PATTERN_3		    (3 << 0)
# define DP_TRAINING_PATTERN_MASK	    0x3
# define DP_LINK_QUAL_PATTERN_DISABLE	    (0 << 2)
# define DP_LINK_QUAL_PATTERN_D10_2	    (1 << 2)
# define DP_LINK_QUAL_PATTERN_ERROR_RATE    (2 << 2)
# define DP_LINK_QUAL_PATTERN_PRBS7	    (3 << 2)
# define DP_LINK_QUAL_PATTERN_MASK	    (3 << 2)
# define DP_RECOVERED_CLOCK_OUT_EN	    (1 << 4)
# define DP_LINK_SCRAMBLING_DISABLE	    (1 << 5)
# define DP_SYMBOL_ERROR_COUNT_BOTH	    (0 << 6)
# define DP_SYMBOL_ERROR_COUNT_DISPARITY    (1 << 6)
# define DP_SYMBOL_ERROR_COUNT_SYMBOL	    (2 << 6)
# define DP_SYMBOL_ERROR_COUNT_MASK	    (3 << 6)

#define DP_TRAINING_LANE0_SET		    0x103
#define DP_TRAINING_LANE1_SET		    0x104
#define DP_TRAINING_LANE2_SET		    0x105
#define DP_TRAINING_LANE3_SET		    0x106
# define DP_TRAIN_VOLTAGE_SWING_MASK	    0x3
# define DP_TRAIN_VOLTAGE_SWING_SHIFT	    0
# define DP_TRAIN_MAX_SWING_REACHED	    (1 << 2)
# define DP_TRAIN_VOLTAGE_SWING_400	    (0 << 0)
# define DP_TRAIN_VOLTAGE_SWING_600	    (1 << 0)
# define DP_TRAIN_VOLTAGE_SWING_800	    (2 << 0)
# define DP_TRAIN_VOLTAGE_SWING_1200	    (3 << 0)
# define DP_TRAIN_PRE_EMPHASIS_MASK	    (3 << 3)
# define DP_TRAIN_PRE_EMPHASIS_0	    (0 << 3)
# define DP_TRAIN_PRE_EMPHASIS_3_5	    (1 << 3)
# define DP_TRAIN_PRE_EMPHASIS_6	    (2 << 3)
# define DP_TRAIN_PRE_EMPHASIS_9_5	    (3 << 3)
# define DP_TRAIN_PRE_EMPHASIS_SHIFT	    3
# define DP_TRAIN_MAX_PRE_EMPHASIS_REACHED  (1 << 5)

#define DP_DOWNSPREAD_CTRL		    0x107
# define DP_SPREAD_AMP_0_5		    (1 << 4)

#define DP_MAIN_LINK_CHANNEL_CODING_SET	    0x108
# define DP_SET_ANSI_8B10B		    (1 << 0)

/* values as in DP_I2C_SPEED_CAP */
#define DP_I2C_SPEED_CONTROL		    0x109

#define DP_EDP_CONFIGURATION_SET	    0x10a
# define DP_EDP_ALTERNATE_SCRAMBLER_RESET_ENABLE    (1 << 0)
# define DP_EDP_FRAMING_CHANGE_ENABLE		    (1 << 1)
# define DP_EDP_PANEL_SELF_TEST_EABLE		    (1 << 7)

#define DP_MSTM_CTRL			    0x111
# define DP_MST_ENABLE			    (1 << 0)
# define DP_UP_REQ_ENABLE		    (1 << 1)
# define DP_UPSTREAM_IS_SRC		    (1 << 2)

#define DP_SINK_COUNT			    0x200
# define DP_SINK_COUNT_MASK		    0x3f
# define DP_SINK_CP_READY		    (1 << 6)

#define DP_DEVICE_SERVICE_IRQ_VECTOR	    0x201
# define DP_IRQ_REMOTE_CONTROL_COMMAND_PENDING	    (1 << 0)
# define DP_IRQ_AUTOMATED_TEST_REQUEST		    (1 << 1)
# define DP_IRQ_CP_IRQ				    (1 << 2)
# define DP_IRQ_SINK_SPECIFIC_IRQ		    (1 << 6)

#define DP_LANE0_1_STATUS		    0x202
#define DP_LANE2_3_STATUS		    0x203
# define DP_LANE_CR_DONE		    (1 << 0)
# define DP_LANE_CHANNEL_EQ_DONE	    (1 << 1)
# define DP_LANE_SYMBOL_LOCKED		    (1 << 2)

#define DP_CHANNEL_EQ_BITS (DP_LANE_CR_DONE |		\
			    DP_LANE_CHANNEL_EQ_DONE |	\
			    DP_LANE_SYMBOL_LOCKED)

#define DP_LANE_ALIGN_STATUS_UPDATED	    0x204
# define DP_INTERLANE_ALIGN_DONE	    (1 << 0)
# define DP_DOWNSTREAM_PORT_STATUS_CHANGED  (1 << 6)
# define DP_LINK_STATUS_UPDATED		    (1 << 7)

#define DP_SINK_STATUS			    0x205
# define DP_RECEIVE_PORT_0_STATUS	    (1 << 0)
# define DP_RECEIVE_PORT_1_STATUS	    (1 << 1)

#define DP_ADJUST_REQUEST_LANE0_1	    0x206
#define DP_ADJUST_REQUEST_LANE2_3	    0x207
# define DP_ADJUST_VOLTAGE_SWING_LANE0_MASK  0x03
# define DP_ADJUST_VOLTAGE_SWING_LANE0_SHIFT 0
# define DP_ADJUST_PRE_EMPHASIS_LANE0_MASK   0x0c
# define DP_ADJUST_PRE_EMPHASIS_LANE0_SHIFT  2
# define DP_ADJUST_VOLTAGE_SWING_LANE1_MASK  0x30
# define DP_ADJUST_VOLTAGE_SWING_LANE1_SHIFT 4
# define DP_ADJUST_PRE_EMPHASIS_LANE1_MASK   0xc0
# define DP_ADJUST_PRE_EMPHASIS_LANE1_SHIFT  6

#define DP_TRAINING_SCORE_LANE0		    0x208
#define DP_TRAINING_SCORE_LANE1		    0x209
#define DP_TRAINING_SCORE_LANE2		    0x20a
#define DP_TRAINING_SCORE_LANE3		    0x20b

/* register pairs for a 15-bit saturated error count */
#define DP_SYMBOL_ERROR_COUNT_LANE0_LOW	    0x210
#define DP_SYMBOL_ERROR_COUNT_LANE0_HIGH    0x211
#define DP_SYMBOL_ERROR_COUNT_LANE1_LOW	    0x212
#define DP_SYMBOL_ERROR_COUNT_LANE1_HIGH    0x213
#define DP_SYMBOL_ERROR_COUNT_LANE2_LOW	    0x214
#define DP_SYMBOL_ERROR_COUNT_LANE2_HIGH    0x215
#define DP_SYMBOL_ERROR_COUNT_LANE3_LOW	    0x216
#define DP_SYMBOL_ERROR_COUNT_LANE4_HIGH    0x217
/* ... on the HIGH register */
# define DP_SYMBOL_ERROR_COUNT_VALID	    (1 << 7)

#define DP_SOURCE_IEEE_OUI_7_0		    0x300
#define DP_SOURCE_IEEE_OUI_15_8		    0x301
#define DP_SOURCE_IEEE_OUI_23_16    	    0x302

#define DP_SINK_IEEE_OUI_7_0		    0x400
#define DP_SINK_IEEE_OUI_15_8		    0x401
#define DP_SINK_IEEE_OUI_23_16    	    0x402

#define DP_BRANCH_IEEE_OUI_7_0		    0x500
#define DP_BRANCH_IEEE_OUI_15_8		    0x501
#define DP_BRANCH_IEEE_OUI_23_16    	    0x502

#define DP_SET_POWER                        0x600
# define DP_SET_POWER_D0                    0x1
# define DP_SET_POWER_D3                    0x2

#define DP_HDCP_BKSV			    0x68000
#define DP_HDCP_R0			    0x68005
#define DP_HDCP_AKSV			    0x68007
#define DP_HDCP_AN			    0x6800c
#define DP_HDCP_V_H0			    0x68014
#define DP_HDCP_V_H1			    0x68018
#define DP_HDCP_V_H2			    0x6801c
#define DP_HDCP_V_H3			    0x68020
#define DP_HDCP_V_H4			    0x68024
#define DP_HDCP_BCAPS			    0x68028
# define DP_HDCP_CAPABLE		    (1 << 0)
# define DP_HDCP_REPEATER		    (1 << 1)
#define DP_HDCP_BSTATUS			    0x68029
# define DP_HDCP_READY			    (1 << 0)
# define DP_HDCP_R0_AVAILABLE		    (1 << 1)
# define DP_HDCP_LINK_INTEGRITY_FAILURE	    (1 << 2)
# define DP_HDCP_REAUTHENTICATION_REQUEST   (1 << 3)
#define DP_HDCP_BINFO			    0x6802a
# define DP_HDCP_DEVICE_COUNT_MASK	    0x7f
# define DP_HDCP_MAX_DEVS_EXCEEDED	    (1 << 7)
#define DP_HDCP_BINFO2			    0x6802b
# define DP_HDCP_DEPTH_MASK		    0x07
# define DP_HDCP_MAX_CASCADE_EXCEEDED	    (1 << 3)
#define DP_HDCP_KSV_FIFO		    0x6802c
#define DP_HDCP_AINFO			    0x6803b
# define DP_HDCP_REAUTHENTICATION_ENABLE_IRQ_HPD    (1 << 0)

#define MODE_I2C_START	1
#define MODE_I2C_WRITE	2
#define MODE_I2C_READ	4
#define MODE_I2C_STOP	8

struct i2c_algo_dp_aux_data {
	bool running;
	u16 address;
	int (*aux_ch) (struct i2c_adapter *adapter,
		       int mode, uint8_t write_byte,
		       uint8_t *read_byte);
};

int
i2c_dp_aux_add_bus(struct i2c_adapter *adapter);

#endif /* _DRM_DP_HELPER_H_ */
