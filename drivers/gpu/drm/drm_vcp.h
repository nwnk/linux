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
 */

/*
 * Virtual Control Panel codes defined in the VESA Monitor Control Command
 * Set specification.  Used by both DDC/CI and the USB monitor control
 * class.
 */

#ifndef DRM_VCP_H
#define DRM_VCP_H

/* Preset operations */
#define VCP_RESTORE_DEFAULTS	0x04        /* meaning, all defaults */
#define VCP_RESTORE_LUMINANCE	0x05
#define VCP_RESTORE_GEOMETRY	0x06
#define VCP_RESTORE_COLOR	0x08
#define VCP_RESTORE_TV		0x0A
#define VCP_SETTINGS		0xB0

/* Image adjustments */
#define VCP_AUTO_COLOR_SETUP	0x1F
#define VCP_AUTO_SETUP		0x1E
#define VCP_AUTO_SETUP_ENABLE	0xA2
#define VCP_BACKLIGHT		0x13
#define VCP_BLACK_LEVEL_BLUE	0x70
#define VCP_BLACK_LEVEL_GREEN	0x6E
#define VCP_BLACK_LEVEL_RED	0x6C
#define VCP_BLOCK_LUT		0x75
#define VCP_CLOCK		0x0E
#define VCP_CLOCK_PHASE		0x3E
#define VCP_COLOR_PRESET	0x14
#define VCP_CONTRAST		0x12
#define VCP_DISPLAY_APPLICATION	0xDC
#define VCP_FLESHTONE		0x11
#define VCP_FOCUS		0x1C
#define VCP_GAMMA		0x72
#define VCP_GRAYSCALE_EXPAND	0x2E
#define VCP_HORIZONTAL_MOIRE	0x56
#define VCP_HUE			0x90
#define VCP_HUE_BLUE		0x9F
#define VCP_HUE_CYAN		0x9E
#define VCP_HUE_GREEN		0x9D
#define VCP_HUE_MAGENTA		0xA0
#define VCP_HUE_RED		0x9B
#define VCP_HUE_YELLOW		0x9C
#define VCP_LUMINANCE		0x10
#define VCP_LUT_SIZE		0x73
#define VCP_ORIENTATION		0xAA
#define VCP_SATURATION		0x8A
#define VCP_SATURATION_BLUE	0x5D
#define VCP_SATURATION_CYAN	0x5C
#define VCP_SATURATION_GREEN	0x5B
#define VCP_SATURATION_MAGENTA	0x5E
#define VCP_SATURATION_RED	0x5A
#define VCP_SATURATION_YELLOW	0x59
#define VCP_SHARPNESS		0x87
#define VCP_SINGLE_POINT_LUT	0x74
#define VCP_STEREO		0xD4
#define VCP_TEMPERATURE		0x0C
#define VCP_TEMPERATURE_DELTA	0x0B
#define VCP_TV_BLACK_LEVEL	0x92
#define VCP_TV_CONTRAST		0x8E
#define VCP_TV_SHARPNESS	0x8C
#define VCP_VELOCITY_MODULATION	0x88
#define VCP_VERTICAL_MOIRE	0x58
#define VCP_VIDEO_GAIN_BLUE	0x1A
#define VCP_VIDEO_GAIN_GREEN	0x18
#define VCP_VIDEO_GAIN_RED	0x16
#define VCP_VISION_COMPENSATION	0x17
#define VCP_WINDOW_BACKGROUND	0x9A
#define VCP_WINDOW_CONTROL	0xA4
#define VCP_WINDOW_SELECT	0xA5
#define VCP_ZOOM		0x7C

/* Display control */
#define VCP_DISPLAY_CONTROLLER	    0xC8
#define VCP_DISPLAY_FIRMWARE	    0xC9
#define VCP_DISPLAY_USAGE_TIME	    0xC6
#define VCP_HORIZONTAL_FREQUENCY    0xAC
#define VCP_IMAGE_MODE		    0xDB
#define VCP_OSD			    0xCA
#define VCP_OSD_LANGUAGE	    0xCC
#define VCP_POWER		    0xD6
#define VCP_SOURCE_COLOR_CODING	    0xB5
#define VCP_SOURCE_TIMING_MODE	    0xB4
#define VCP_VERSION		    0xDF
#define VCP_VERTICAL_FREQUENCY	    0xAE

/* Geometry control */
#define VCP_BOTTOM_CORNER_FLARE		    0x4A
#define VCP_BOTTOM_CORNER_HOOK		    0x4C
#define VCP_DISPLAY_SCALING		    0x86
#define VCP_HORIZONTAL_CONVERGENCE_RB	    0x28
#define VCP_HORIZONTAL_CONVERGENCE_MG	    0x29
#define VCP_HORIZONTAL KEYSTONE		    0x42
#define VCP_HORIZONTAL_LINEARITY	    0x2A
#define VCP_HORIZONTAL_LINEARITY_BALANCE    0x2C
#define VCP_HORIZONTAL_MIRROR		    0x82
#define VCP_HORIZONTAL_PARALLELOGRAM	    0x40
#define VCP_HORIZONTAL_PINCUSHION	    0x24
#define VCP_HORIZONTAL_PINCUSHION_BALANCE   0x26
#define VCP_HORIZONTAL_POSITION		    0x20
#define VCP_HORIZONTAL_SIZE		    0x22
#define VCP_ROTATION			    0x44
#define VCP_SCAN_MODE			    0xDA
#define VCP_TOP_CORNER_FLARE		    0x46
#define VCP_TOP_CORNER_HOOK		    0x48
#define VCP_VERTICAL_CONVERGENCE_RB	    0x38
#define VCP_VERTICAL_CONVERGENCE_MG	    0x39
#define VCP_VERTICAL_KEYSTONE		    0x43
#define VCP_VERTICAL_LINEARITY		    0x3A
#define VCP_VERTICAL_LINEARITY_BALANCE	    0x3C
#define VCP_VERTICAL_MIRROR		    0x84
#define VCP_VERTICAL_PARALLELOGRAM	    0x41
#define VCP_VERTICAL_PINCUSHION		    0x34
#define VCP_VERTICAL_PINCUSHION_BALANCE	    0x36
#define VCP_VERTICAL_POSITION		    0x30
#define VCP_VERTICAL_SIZE		    0x32
#define VCP_WINDOW_POSITION_TL_X	    0x95
#define VCP_WINDOW_POSITION_TL_Y	    0x96
#define VCP_WINDOW_POSITION_BR_X	    0x97
#define VCP_WINDOW_POSITION_BR_Y	    0x98

/* Miscellaneous functions */
#define VCP_ACTIVE_CONTROL		0x52
#define VCP_AMBIENT_LIGHT_SENSOR	0x66
#define VCP_APPLICATION_ENABLE_KEY	0xC6
#define VCP_ASSET_TAG			0xD2
#define VCP_AUX_DISPLAY_DATA		0xCF
#define VCP_AUX_DISPLAY_SIZE		0xCE
#define VCP_AUX_POWER_OUTPUT		0xD7
#define VCP_DEGAUSS			0x01
#define VCP_DISPLAY_DESCRIPTOR		0xC4
#define VCP_DISPLAY_DESCRIPTOR_LENGTH	0xC2
#define VCP_DISPLAY_DESCRIPTOR_SET	0xC3
#define VCP_DISPLAY_TECHNOLOGY		0xB6
#define VCP_EDID			0x78
#define VCP_INPUT_SOURCE		0x60
#define VCP_NEW_CONTROL_VALUE		0x02
#define VCP_OUTPUT_SELECT		0xD0
#define VCP_PERFORMANCE_PRESERVATION	0x54
#define VCP_RPC				0x76
#define VCP_SCRATCH_PAD			0xDE
#define VCP_SOFT_CONTROLS		0x03
#define VCP_STATUS_INDICATORS		0xCD
#define VCP_SUBPIXEL_LAYOUT		0xB2
#define VCP_TV_CHANNEL_UP_DOWN		0x8B

/* Audio functions */
#define VCP_AUDIO_BALANCE	    0x93
#define VCP_AUDIO_BASS		    0x91
#define VCP_AUDIO_MIC_VOLUME	    0x64
#define VCP_AUDIO_MUTE		    0x8D
#define VCP_AUDIO_PROCESSOR_MODE    0x94
#define VCP_AUDIO_SPEAKER_SELECT    0x63
#define VCP_AUDIO_SPEAKER_VOLUME    0x62
#define VCP_AUDIO_TREBLE	    0x8F

/* DPVL functions */
#define VCP_BODY_CRC_ERRORS	    0xBC
#define VCP_CLIENT_ID		    0xBD
#define VCP_HEADER_ERROR_COUNT	    0xBB
#define VCP_LINK_CONTROL	    0xBE
#define VCP_MONITOR_STATUS	    0xB7
#define VCP_MONITOR_X_ORIGIN	    0xB9
#define VCP_MONITOR_Y_ORIGIN	    0xBA
#define VCP_PACKET_COUNT	    0xB8

/* E0-FF inclusive are reserved for manufacturer VCPs */

#endif
