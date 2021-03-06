/*
 * Copyright 2009 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 2004, 2005 David Young.  All rights reserved.
 *
 * Programmed for NetBSD by David Young.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of David Young may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY David Young ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL David
 * Young BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */
/* Macros for bit twiddling. */

#ifndef _RTW_REG_H_
#define	_RTW_REG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BIT_TWIDDLE
#define	_BIT_TWIDDLE
/*
 * nth bit, BIT(0) == 0x1.
 */
#define	BIT(n) (((n) == 32) ? 0 : ((uint32_t)1 << (n)))

/*
 * bits m through n, m < n.
 */
#define	BITS(m, n) ((BIT(MAX((m), (n)) + 1) - 1) ^ (BIT(MIN((m), (n))) - 1))

/*
 * find least significant bit that is set
 */
#define	LOWEST_SET_BIT(x) ((((x) - 1) & (x)) ^ (x))

/*
 * for x a power of two and p a non-negative integer, is x a greater
 * power than 2**p?
 */
#define	GTEQ_POWER(x, p) (((ulong_t)(x) >> (p)) != 0)

#define	MASK_TO_SHIFT2(m) (GTEQ_POWER(LOWEST_SET_BIT((m)), 1) ? 1 : 0)

#define	MASK_TO_SHIFT4(m) \
	(GTEQ_POWER(LOWEST_SET_BIT((m)), 2) \
	    ? 2 + MASK_TO_SHIFT2((m) >> 2) \
	    : MASK_TO_SHIFT2((m)))

#define	MASK_TO_SHIFT8(m) \
	(GTEQ_POWER(LOWEST_SET_BIT((m)), 4) \
	    ? 4 + MASK_TO_SHIFT4((m) >> 4) \
	    : MASK_TO_SHIFT4((m)))

#define	MASK_TO_SHIFT16(m) \
	(GTEQ_POWER(LOWEST_SET_BIT((m)), 8) \
	    ? 8 + MASK_TO_SHIFT8((m) >> 8) \
	    : MASK_TO_SHIFT8((m)))

#define	MASK_TO_SHIFT(m) \
	(GTEQ_POWER(LOWEST_SET_BIT((m)), 16) \
	    ? 16 + MASK_TO_SHIFT16((m) >> 16) \
	    : MASK_TO_SHIFT16((m)))

#define	MASK_AND_RSHIFT(x, mask) (((x) & (mask)) >> MASK_TO_SHIFT(mask))
#define	LSHIFT(x, mask) ((x) << MASK_TO_SHIFT(mask))
#define	MASK_AND_REPLACE(reg, val, mask) ((reg & ~mask) | LSHIFT(val, mask))
#define	PRESHIFT(m) MASK_AND_RSHIFT((m), (m))

#endif /* _BIT_TWIDDLE */

/* RTL8180L Host Control and Status Registers */

/*
 * ID Register: MAC addr, 6 bytes.
 * Auto-loaded from EEPROM. Read by byte, by word, or by double word,
 * but write only by double word.
 */
#define	RTW_IDR0	0x00
#define	RTW_IDR1	0x04

#define	RTW_MAR0	0x08	/* Multicast filter, 64b. */
#define	RTW_MAR1	0x0c

/*
 * Timing Synchronization Function Timer Register,
 * low word, 32b, read-only.
 */
#define	RTW_TSFTRL	0x18
#define	RTW_TSFTRH	0x1c	/* High word, 32b, read-only. */
/*
 * Transmit Low Priority Descriptors Start Address,
 * 32b, 256-byte alignment.
 */
#define	RTW_TLPDA	0x20
/*
 * Transmit Normal Priority Descriptors Start  Address,
 * 32b, 256-byte alignment.
 */
#define	RTW_TNPDA	0x24
/*
 * Transmit High Priority Descriptors Start Address,
 * 32b, 256-byte alignment.
 */
#define	RTW_THPDA	0x28

#define	RTW_BRSR	0x2c	/* Basic Rate Set Register, 16b */
/*
 * 1: use short PLCP header for CTS/ACK packet,
 * 0: use long PLCP header
 */
#define	RTW_BRSR_BPLCP	BIT(8)
#define	RTW_BRSR_MBR8180_MASK	BITS(1, 0)	/* Maximum Basic Service Rate */
#define	RTW_BRSR_MBR8180_1MBPS	LSHIFT(0, RTW_BRSR_MBR8180_MASK)
#define	RTW_BRSR_MBR8180_2MBPS	LSHIFT(1, RTW_BRSR_MBR8180_MASK)
#define	RTW_BRSR_MBR8180_5MBPS	LSHIFT(2, RTW_BRSR_MBR8180_MASK)
#define	RTW_BRSR_MBR8180_11MBPS	LSHIFT(3, RTW_BRSR_MBR8180_MASK)

/*
 * 8181 and 8180 docs conflict!
 */
#define	RTW_BRSR_MBR8181_1MBPS	BIT(0)
#define	RTW_BRSR_MBR8181_2MBPS	BIT(1)
#define	RTW_BRSR_MBR8181_5MBPS	BIT(2)
#define	RTW_BRSR_MBR8181_11MBPS	BIT(3)

#define	RTW_BSSID	0x2e
/*
 * BSSID, 6 bytes
 */
#define	RTW_BSSID16	0x2e		/* first two bytes */
#define	RTW_BSSID32	(0x2e + 4)	/* remaining four bytes */
#define	RTW_BSSID0	RTW_BSSID16		/* BSSID[0], 8b */
#define	RTW_BSSID1	(RTW_BSSID0 + 1)	/* BSSID[1], 8b */
#define	RTW_BSSID2	(RTW_BSSID1 + 1)	/* BSSID[2], 8b */
#define	RTW_BSSID3	(RTW_BSSID2 + 1)	/* BSSID[3], 8b */
#define	RTW_BSSID4	(RTW_BSSID3 + 1)	/* BSSID[4], 8b */
#define	RTW_BSSID5	(RTW_BSSID4 + 1)	/* BSSID[5], 8b */

#define	RTW_CR		0x37	/* Command Register, 8b */
/*
 * Reset: host sets to 1 to disable
 * transmitter & receiver, reinitialize FIFO.
 * RTL8180L sets to 0 to signal completion.
 */
#define	RTW_CR_RST	BIT(4)
/*
 * Receiver Enable: host enables receiver
 * by writing 1. RTL8180L indicates receiver
 * is active with 1. After power-up, host
 * must wait for reset before writing.
 */
#define	RTW_CR_RE	BIT(3)
/*
 * Transmitter Enable: host enables transmitter
 * by writing 1. RTL8180L indicates transmitter
 * is active with 1. After power-up, host
 * must wait for reset before writing.
 */
#define	RTW_CR_TE	BIT(2)
/*
 * PCI Multiple Read/Write enable:
 * 1 enables,
 * 0 disables. XXX RTL8180, only?
 */
#define	RTW_CR_MULRW	BIT(0)

#define	RTW_IMR		0x3c	/* Interrupt Mask Register, 16b */
#define	RTW_ISR		0x3e	/* Interrupt status register, 16b */

#define	RTW_INTR_TXFOVW	BIT(15)		/* Tx FIFO Overflow */
/*
 * Time Out: 1 indicates RTW_TSFTR[0:31] = RTW_TINT
 */
#define	RTW_INTR_TIMEOUT	BIT(14)
/*
 * Beacon Time Out: time for host to prepare beacon:
 * RTW_TSFTR % (RTW_BCNITV_BCNITV * TU) =
 * (RTW_BCNITV_BCNITV * TU - RTW_BINTRITV)
 */
#define	RTW_INTR_BCNINT	BIT(13)
/*
 * ATIM Time Out: ATIM interval will pass,
 * RTW_TSFTR % (RTW_BCNITV_BCNITV * TU) =
 * (RTW_ATIMWND_ATIMWND * TU - RTW_ATIMTRITV)
 */
#define	RTW_INTR_ATIMINT	BIT(12)
/*
 * Tx Beacon Descriptor Error:
 * beacon transmission aborted because
 * frame Rx'd
 */
#define	RTW_INTR_TBDER	BIT(11)
#define	RTW_INTR_TBDOK	BIT(10)	/* Tx Beacon Descriptor OK */
/*
 * Tx High Priority Descriptor Error:
 * reached short/long retry limit
 */
#define	RTW_INTR_THPDER	BIT(9)
#define	RTW_INTR_THPDOK	BIT(8)	/* Tx High Priority Descriptor OK */
/*
 * Tx Normal Priority Descriptor Error:
 * reached short/long retry limit
 */
#define	RTW_INTR_TNPDER	BIT(7)
#define	RTW_INTR_TNPDOK	BIT(6)	/* Tx Normal Priority Descriptor OK */
/*
 * Rx FIFO Overflow: either RDU (see below)
 * or PCI bus too slow/busy
 */
#define	RTW_INTR_RXFOVW	BIT(5)
#define	RTW_INTR_RDU	BIT(4)	/* Rx Descriptor Unavailable */
/*
 * Tx Low Priority Descriptor Error
 * reached short/long retry limit
 */
#define	RTW_INTR_TLPDER	BIT(3)
#define	RTW_INTR_TLPDOK	BIT(2)	/* Tx Low Priority Descriptor OK */
#define	RTW_INTR_RER	BIT(1)	/* Rx Error: CRC32 or ICV error */
#define	RTW_INTR_ROK	BIT(0)	/* Rx OK */

/*
 * Convenient interrupt conjunctions.
 */
#define	RTW_INTR_RX	(RTW_INTR_RER|RTW_INTR_ROK | \
			RTW_INTR_RDU |RTW_INTR_RXFOVW)
#define	RTW_INTR_TX	(RTW_INTR_TLPDER|RTW_INTR_TLPDOK|RTW_INTR_THPDER|\
			RTW_INTR_THPDOK|RTW_INTR_TNPDER|RTW_INTR_TNPDOK|\
			RTW_INTR_TBDER|RTW_INTR_TBDOK)
#define	RTW_INTR_BEACON	(RTW_INTR_BCNINT)
#define	RTW_INTR_IOERROR	(RTW_INTR_TXFOVW|RTW_INTR_RXFOVW|RTW_INTR_RDU)

#define	RTW_TCR		0x40	/* Transmit Configuration Register, 32b */
#define	RTW_TCR_CWMIN	BIT(31)	/* 1: CWmin = 8, 0: CWmin = 32. */
/*
 * 1: host assigns 802.11 sequence number,
 * 0: hardware assigns sequence number
 */
#define	RTW_TCR_SWSEQ	BIT(30)
/* Hardware version ID, read-only */
#define	RTW_TCR_HWVERID_MASK	BITS(29, 25)
#define	RTW_TCR_HWVERID_D	LSHIFT(26, RTW_TCR_HWVERID_MASK)
#define	RTW_TCR_HWVERID_F	LSHIFT(27, RTW_TCR_HWVERID_MASK)
#define	RTW_TCR_HWVERID_RTL8180	RTW_TCR_HWVERID_F

/*
 * Set ACK/CTS Timeout (EIFS).
 * 1: ACK rate = max(RTW_BRSR_MBR, Rx rate) (XXX not min? typo in datasheet?)
 * 0: ACK rate = 1Mbps
 */
#define	RTW_TCR_SAT	BIT(24)
/* Max DMA Burst Size per Tx DMA Burst */
#define	RTW_TCR_MXDMA_MASK	BITS(23, 21)
#define	RTW_TCR_MXDMA_16	LSHIFT(0, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_32	LSHIFT(1, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_64	LSHIFT(2, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_128	LSHIFT(3, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_256	LSHIFT(4, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_512	LSHIFT(5, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_1024	LSHIFT(6, RTW_TCR_MXDMA_MASK)
#define	RTW_TCR_MXDMA_2048	LSHIFT(7, RTW_TCR_MXDMA_MASK)

#define	RTW_TCR_DISCW		BIT(20)	/* disable 802.11 random backoff */

/*
 * host lets RTL8180 append ICV to WEP packets
 */
#define	RTW_TCR_ICV		BIT(19)

/*
 * Loopback Test: disables TXI/TXQ outputs.
 */
#define	RTW_TCR_LBK_MASK	BITS(18, 17)
#define	RTW_TCR_LBK_NORMAL	LSHIFT(0, RTW_TCR_LBK_MASK) /* normal ops */
#define	RTW_TCR_LBK_MAC		LSHIFT(1, RTW_TCR_LBK_MASK) /* MAC loopback */
#define	RTW_TCR_LBK_BBP		LSHIFT(2, RTW_TCR_LBK_MASK) /* baseband loop. */
#define	RTW_TCR_LBK_CONT	LSHIFT(3, RTW_TCR_LBK_MASK) /* continuous Tx */

/*
 * 0: RTL8180 appends CRC32
 * 1: host appends CRC32
 *
 * (I *think* this is right.  The docs have a mysterious
 *  description in the  passive voice.)
 */
#define	RTW_TCR_CRC	BIT(16)
#define	RTW_TCR_SRL_MASK	BITS(15, 8)	/* Short Retry Limit */
#define	RTW_TCR_LRL_MASK	BITS(7, 0)	/* Long Retry Limit */

#define	RTW_RCR		0x44	/* Receive Configuration Register, 32b */
/*
 * only do Early Rx on packets longer than 1536 bytes
 */
#define	RTW_RCR_ONLYERLPKT	BIT(31)
#define	RTW_RCR_ENCS2		BIT(30)	/* enable carrier sense method 2 */
#define	RTW_RCR_ENCS1		BIT(29)	/* enable carrier sense method 1 */
#define	RTW_RCR_ENMARP		BIT(28)	/* enable MAC auto-reset PHY */
/*
 * Check BSSID/ToDS/FromDS: set "Link On" when received BSSID
 * matches RTW_BSSID and received ToDS/FromDS are appropriate
 * according to RTW_MSR_NETYPE.
 */
#define	RTW_RCR_CBSSID		BIT(23)
#define	RTW_RCR_APWRMGT		BIT(22)	/* accept packets w/ PWRMGMT bit set */
/*
 * when RTW_MSR_NETYPE ==  RTW_MSR_NETYPE_INFRA_OK, accept
 * broadcast/multicast packets whose 3rd address matches RTL8180's MAC.
 */
#define	RTW_RCR_ADD3		BIT(21)
#define	RTW_RCR_AMF		BIT(20)	/* accept management frames */
#define	RTW_RCR_ACF		BIT(19)	/* accept control frames */
#define	RTW_RCR_ADF		BIT(18)	/* accept data frames */
/*
 * Rx FIFO Threshold: RTL8180 begins PCI transfer when this many data
 * bytes are received
 */
#define	RTW_RCR_RXFTH_MASK	BITS(15, 13)
#define	RTW_RCR_RXFTH_64	LSHIFT(2, RTW_RCR_RXFTH_MASK)
#define	RTW_RCR_RXFTH_128	LSHIFT(3, RTW_RCR_RXFTH_MASK)
#define	RTW_RCR_RXFTH_256	LSHIFT(4, RTW_RCR_RXFTH_MASK)
#define	RTW_RCR_RXFTH_512	LSHIFT(5, RTW_RCR_RXFTH_MASK)
#define	RTW_RCR_RXFTH_1024	LSHIFT(6, RTW_RCR_RXFTH_MASK)
#define	RTW_RCR_RXFTH_WHOLE	LSHIFT(7, RTW_RCR_RXFTH_MASK)

#define	RTW_RCR_AICV		BIT(12)	/* accept frames w/ ICV errors */

/*
 * Max DMA Burst Size per Rx DMA Burst
 */
#define	RTW_RCR_MXDMA_MASK	BITS(10, 8)
#define	RTW_RCR_MXDMA_16	LSHIFT(0, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_32	LSHIFT(1, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_64	LSHIFT(2, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_128	LSHIFT(3, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_256	LSHIFT(4, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_512	LSHIFT(5, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_1024	LSHIFT(6, RTW_RCR_MXDMA_MASK)
#define	RTW_RCR_MXDMA_UNLIMITED	LSHIFT(7, RTW_RCR_MXDMA_MASK)

/*
 * EEPROM type, read-only. 1: EEPROM is 93c56, 0: 93c46
 */
#define	RTW_RCR_9356SEL		BIT(6)

#define	RTW_RCR_ACRC32		BIT(5)	/* accept frames w/ CRC32 errors */
#define	RTW_RCR_AB		BIT(3)	/* accept broadcast frames */
#define	RTW_RCR_AM		BIT(2)	/* accept multicast frames */
/*
 * accept physical match frames. XXX means PLCP header ok?
 */
#define	RTW_RCR_APM		BIT(1)
#define	RTW_RCR_AAP		BIT(0)	/* accept frames w/ destination */

/*
 * Additional bits to set in monitor mode.
 */
#define	RTW_RCR_MONITOR (		\
    RTW_RCR_AAP |			\
    RTW_RCR_ACF |			\
    RTW_RCR_ACRC32 |			\
    RTW_RCR_AICV |			\
    0)

/*
 * The packet filter bits.
 */
#define	RTW_RCR_PKTFILTER_MASK (\
    RTW_RCR_ENCS1|RTW_RCR_ENCS2|\
    RTW_RCR_AAP |		\
    RTW_RCR_AB |		\
    RTW_RCR_ACF |		\
    RTW_RCR_ACRC32 |		\
    RTW_RCR_ADD3 |		\
    RTW_RCR_ADF |		\
    RTW_RCR_AICV |		\
    RTW_RCR_AM |		\
    RTW_RCR_AMF |		\
    RTW_RCR_APM |		\
    RTW_RCR_APWRMGT |		\
    0)

/*
 * Receive power-management frames and mgmt/ctrl/data frames.
 */
#define	RTW_RCR_PKTFILTER_DEFAULT	(	\
    RTW_RCR_ONLYERLPKT |			\
    RTW_RCR_ENCS1 |				\
    RTW_RCR_CBSSID |				\
    RTW_RCR_ADF |				\
    RTW_RCR_AMF |				\
    RTW_RCR_APM |				\
    RTW_RCR_AM |		\
    RTW_RCR_AB |		\
    0)
#define	RTW_RCR_PROMIC (	\
    RTW_RCR_AAP |		\
    0)

#define	RTW_TINT	0x48	/* Timer Interrupt Register, 32b */
/*
 * Transmit Beacon Descriptor Start Address,
 *  32b, 256-byte alignment
 */
#define	RTW_TBDA	0x4c
#define	RTW_9346CR	0x50	/* 93c46/93c56 Command Register, 8b */
#define	RTW_9346CR_EEM_MASK	BITS(7, 6)	/* Operating Mode */
#define	RTW_9346CR_EEM_NORMAL	LSHIFT(0, RTW_9346CR_EEM_MASK)
/*
 * Load the EEPROM. Reset registers to defaults.
 * Takes ~2ms. RTL8180 indicates completion with RTW_9346CR_EEM_NORMAL.
 * XXX RTL8180 only?
 */
#define	RTW_9346CR_EEM_AUTOLOAD	LSHIFT(1, RTW_9346CR_EEM_MASK)
/*
 * Disable network & bus-master operations and enable
 * _EECS, _EESK, _EEDI, _EEDO.
 * XXX RTL8180 only?
 */
#define	RTW_9346CR_EEM_PROGRAM	LSHIFT(2, RTW_9346CR_EEM_MASK)
/* Enable RTW_CONFIG[0123] registers. */
#define	RTW_9346CR_EEM_CONFIG	LSHIFT(3, RTW_9346CR_EEM_MASK)
/*
 * EEPROM pin status/control in _EEM_CONFIG, _EEM_AUTOLOAD modes.
 * XXX RTL8180 only?
 */
#define	RTW_9346CR_EECS	BIT(3)
#define	RTW_9346CR_EESK	BIT(2)
#define	RTW_9346CR_EEDI	BIT(1)
#define	RTW_9346CR_EEDO	BIT(0)	/* read-only */

#define	RTW_CONFIG0	0x51	/* Configuration Register 0, 8b */
/*
 * implements 40-bit WEP, XXX RTL8180 only?
 */
#define	RTW_CONFIG0_WEP40	BIT(7)
/*
 * implements 104-bit WEP, from EEPROM, read-only XXX RTL8180 only?
 */
#define	RTW_CONFIG0_WEP104	BIT(6)
/*
 * 1: RTW_PSR_LEDGPO[01] control LED[01] pins.
 * 0: LED behavior defined by RTW_CONFIG1_LEDS10_MASK
 * XXX RTL8180 only?
 */
#define	RTW_CONFIG0_LEDGPOEN	BIT(4)
/*
 * auxiliary power is present, read-only
 */
#define	RTW_CONFIG0_AUXPWR	BIT(3)
/*
 * Geographic Location, read-only
 */
#define	RTW_CONFIG0_GL_MASK		BITS(1, 0)
/*
 * _RTW_CONFIG0_GL_* is what the datasheet says, but RTW_CONFIG0_GL_*
 * work.
 */
#define	_RTW_CONFIG0_GL_USA		LSHIFT(3, RTW_CONFIG0_GL_MASK)
#define	RTW_CONFIG0_GL_EUROPE		LSHIFT(2, RTW_CONFIG0_GL_MASK)
#define	RTW_CONFIG0_GL_JAPAN		LSHIFT(1, RTW_CONFIG0_GL_MASK)
#define	RTW_CONFIG0_GL_USA		LSHIFT(0, RTW_CONFIG0_GL_MASK)
/*
 * RTL8181 datasheet says RTW_CONFIG0_GL_JAPAN = 0.
 */

#define	RTW_CONFIG1	0x52	/* Configuration Register 1, 8b */

/*
 * LED configuration. From EEPROM. Read/write.
 *
 * Setting				LED0		LED1
 * -------				----		----
 * RTW_CONFIG1_LEDS_ACT_INFRA		Activity	Infrastructure
 * RTW_CONFIG1_LEDS_ACT_LINK		Activity	Link
 * RTW_CONFIG1_LEDS_TX_RX		Tx		Rx
 * RTW_CONFIG1_LEDS_LINKACT_INFRA	Link/Activity	Infrastructure
 */
#define	RTW_CONFIG1_LEDS_MASK	BITS(7, 6)
#define	RTW_CONFIG1_LEDS_ACT_INFRA	LSHIFT(0, RTW_CONFIG1_LEDS_MASK)
#define	RTW_CONFIG1_LEDS_ACT_LINK	LSHIFT(1, RTW_CONFIG1_LEDS_MASK)
#define	RTW_CONFIG1_LEDS_TX_RX		LSHIFT(2, RTW_CONFIG1_LEDS_MASK)
#define	RTW_CONFIG1_LEDS_LINKACT_INFRA	LSHIFT(3, RTW_CONFIG1_LEDS_MASK)

/*
 * LWAKE Output Signal. Only applicable to Cardbus. Pulse width is 150ms.
 *
 *                                   RTW_CONFIG1_LWACT
 *				0			1
 * RTW_CONFIG4_LWPTN	0	active high		active low
 *			1	positive pulse		negative pulse
 */
#define	RTW_CONFIG1_LWACT	BIT(4)

#define	RTW_CONFIG1_MEMMAP	BIT(3)	/* using PCI memory space, read-only */
#define	RTW_CONFIG1_IOMAP	BIT(2)	/* using PCI I/O space, read-only */
/*
 * if set, VPD from offsets 0x40-0x7f in EEPROM are at
 * registers 0x60-0x67 of PCI Configuration Space ( XXX huh? )
 */
#define	RTW_CONFIG1_VPD		BIT(1)
#define	RTW_CONFIG1_PMEN	BIT(0)	/* Power Management Enable: TBD */

#define	RTW_CONFIG2	0x53	/* Configuration Register 2, 8b */
/*
 * clocks are locked, read-only:
 * Tx frequency & symbol clocks are derived from the same OSC
 */
#define	RTW_CONFIG2_LCK	BIT(7)
#define	RTW_CONFIG2_ANT	BIT(6)	/* diversity enabled, read-only */
/*
 * Descriptor Polling State: enable test mode.
 */
#define	RTW_CONFIG2_DPS	BIT(3)
#define	RTW_CONFIG2_PAPESIGN		BIT(2)		/* TBD, from EEPROM */
#define	RTW_CONFIG2_PAPETIME_MASK	BITS(1, 0)	/* TBD, from EEPROM */

#define	RTW_ANAPARM	0x54	/* Analog parameter, 32b */
/*
 * undocumented bits which appear to control the power state of the RF
 * components
 */
#define	RTW_ANAPARM_RFPOW0_MASK	BITS(30, 28)
#define	RTW_ANAPARM_RFPOW_MASK	\
	(RTW_ANAPARM_RFPOW0_MASK|RTW_ANAPARM_RFPOW1_MASK)

/*
 * 1: disable Tx DAC,
 * 0: enable
 */
#define	RTW_ANAPARM_TXDACOFF	BIT(27)
/*
 * undocumented bits which appear to control the power state of the RF
 * components
 */
#define	RTW_ANAPARM_RFPOW1_MASK	BITS(26, 20)

/*
 * Maxim On/Sleep/Off control
 */
#define	RTW_ANAPARM_RFPOW_MAXIM_ON	LSHIFT(0x8, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_MAXIM_SLEEP	LSHIFT(0x378, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_MAXIM_OFF	LSHIFT(0x379, RTW_ANAPARM_RFPOW1_MASK)

/*
 * RFMD On/Sleep/Off control
 */
#define	RTW_ANAPARM_RFPOW_RFMD_ON	LSHIFT(0x408, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_RFMD_SLEEP	LSHIFT(0x378, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_RFMD_OFF	LSHIFT(0x379, RTW_ANAPARM_RFPOW1_MASK)

/*
 * Philips On/Sleep/Off control
 */
#define	RTW_ANAPARM_RFPOW_ANA_PHILIPS_ON	\
    LSHIFT(0x328, RTW_ANAPARM_RFPOW1_MASK)
#define	RTW_ANAPARM_RFPOW_DIG_PHILIPS_ON	\
    LSHIFT(0x008, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_PHILIPS_SLEEP\
    LSHIFT(0x378, RTW_ANAPARM_RFPOW1_MASK)

/*
 * reg[RTW_ANAPARM] |= RTW_ANAPARM_TXDACOFF;
 */
#define	RTW_ANAPARM_RFPOW_PHILIPS_OFF\
    LSHIFT(0x379, RTW_ANAPARM_RFPOW1_MASK)

#define	RTW_ANAPARM_RFPOW_PHILIPS_ON	LSHIFT(0x328, RTW_ANAPARM_RFPOW1_MASK)

/*
 * undocumented card-specific bits from the EEPROM.
 */
#define	RTW_ANAPARM_CARDSP_MASK	BITS(19, 0)

#define	RTW_MSR		0x58	/* Media Status Register, 8b */
/*
 * Network Type and Link Status
 */
#define	RTW_MSR_NETYPE_MASK	BITS(3, 2)
/*
 * AP, XXX RTL8181 only?
 */
#define	RTW_MSR_NETYPE_AP_OK	LSHIFT(3, RTW_MSR_NETYPE_MASK)
/*
 * infrastructure link ok
 */
#define	RTW_MSR_NETYPE_INFRA_OK	LSHIFT(2, RTW_MSR_NETYPE_MASK)
/*
 * ad-hoc link ok
 */
#define	RTW_MSR_NETYPE_ADHOC_OK	LSHIFT(1, RTW_MSR_NETYPE_MASK)
/*
 * no link
 */
#define	RTW_MSR_NETYPE_NOLINK	LSHIFT(0, RTW_MSR_NETYPE_MASK)

#define	RTW_CONFIG3	0x59	/* Configuration Register 3, 8b */
#define	RTW_CONFIG3_GNTSEL	BIT(7)	/* Grant Select, read-only */
/*
 * Set RTW_CONFIG3_PARMEN and RTW_9346CR_EEM_CONFIG to
 * allow RTW_ANAPARM writes.
 */
#define	RTW_CONFIG3_PARMEN	BIT(6)
/*
 * Valid when RTW_CONFIG1_PMEN is set. If set, RTL8180 wakes up
 * OS when Magic Packet is Rx'd.
 */
#define	RTW_CONFIG3_MAGIC	BIT(5)
/*
 * Cardbus-related registers and functions are enabled,
 * read-only. XXX RTL8180 only.
 */
#define	RTW_CONFIG3_CARDBEN	BIT(3)
/*
 * CLKRUN enabled, read-only. XXX RTL8180 only.
 */
#define	RTW_CONFIG3_CLKRUNEN	BIT(2)
/*
 * Function Registers Enabled, read-only. XXX RTL8180 only.
 */
#define	RTW_CONFIG3_FUNCREGEN	BIT(1)
/*
 * Fast back-to-back enabled, read-only.
 */
#define	RTW_CONFIG3_FBTBEN	BIT(0)
#define	RTW_CONFIG4	0x5A	/* Configuration Register 4, 8b */
/*
 * VCO Power Down
 * 0: normal operation
 *    (power-on default)
 * 1: power-down VCO, RF front-end,
 *    and most RTL8180 components.
 */
#define	RTW_CONFIG4_VCOPDN	BIT(7)
/*
 * Power Off
 * 0: normal operation
 *    (power-on default)
 * 1: power-down RF front-end,
 *    and most RTL8180 components,
 *    but leave VCO on.
 *
 * XXX RFMD front-end only?
 */
#define	RTW_CONFIG4_PWROFF	BIT(6)
/*
 * Power Management
 * 0: normal operation
 *    (power-on default)
 * 1: set Tx packet's PWRMGMT bit.
 */
#define	RTW_CONFIG4_PWRMGT	BIT(5)
/*
 * LANWAKE vs. PMEB: Cardbus-only
 * 0: LWAKE & PMEB asserted
 *    simultaneously
 * 1: LWAKE asserted only if
 *    both PMEB is asserted and
 *    ISOLATEB is low.
 * XXX RTL8180 only.
 */
#define	RTW_CONFIG4_LWPME	BIT(4)
/*
 * see RTW_CONFIG1_LWACT XXX RTL8180 only.
 */
#define	RTW_CONFIG4_LWPTN	BIT(2)
/*
 * Radio Front-End Programming Method
 */
#define	RTW_CONFIG4_RFTYPE_MASK	BITS(1, 0)
#define	RTW_CONFIG4_RFTYPE_INTERSIL	LSHIFT(1, RTW_CONFIG4_RFTYPE_MASK)
#define	RTW_CONFIG4_RFTYPE_RFMD		LSHIFT(2, RTW_CONFIG4_RFTYPE_MASK)
#define	RTW_CONFIG4_RFTYPE_PHILIPS	LSHIFT(3, RTW_CONFIG4_RFTYPE_MASK)

#define	RTW_TESTR	0x5B	/* TEST mode register, 8b */

#define	RTW_PSR		0x5e	/* Page Select Register, 8b */
#define	RTW_PSR_GPO	BIT(7)	/* Control/status of pin 52. */
#define	RTW_PSR_GPI	BIT(6)	/* Status of pin 64. */
/*
 * Status/control of LED1 pin if RTW_CONFIG0_LEDGPOEN is set.
 */
#define	RTW_PSR_LEDGPO1	BIT(5)
/*
 * Status/control of LED0 pin if RTW_CONFIG0_LEDGPOEN is set.
 */
#define	RTW_PSR_LEDGPO0	BIT(4)
#define	RTW_PSR_UWF	BIT(1)	/* Enable Unicast Wakeup Frame */
#define	RTW_PSR_PSEN	BIT(0)	/* 1: page 1, 0: page 0 */

#define	RTW_SCR		0x5f	/* Security Configuration Register, 8b */
#define	RTW_SCR_KM_MASK	BITS(5, 4)	/* Key Mode */
#define	RTW_SCR_KM_WEP104	LSHIFT(1, RTW_SCR_KM_MASK)
#define	RTW_SCR_KM_WEP40	LSHIFT(0, RTW_SCR_KM_MASK)
/*
 * Enable Tx WEP. Invalid if neither RTW_CONFIG0_WEP40 nor
 * RTW_CONFIG0_WEP104 is set.
 */
#define	RTW_SCR_TXSECON		BIT(1)
/*
 * Enable Rx WEP. Invalid if neither RTW_CONFIG0_WEP40 nor
 * RTW_CONFIG0_WEP104 is set.
 */
#define	RTW_SCR_RXSECON		BIT(0)

#define	RTW_BCNITV	0x70	/* Beacon Interval Register, 16b */
/*
 * TU between TBTT, written by host.
 */
#define	RTW_BCNITV_BCNITV_MASK	BITS(9, 0)
#define	RTW_ATIMWND	0x72	/* ATIM Window Register, 16b */
/*
 * ATIM Window length in TU, written by host.
 */
#define	RTW_ATIMWND_ATIMWND	BITS(9, 0)

#define	RTW_BINTRITV	0x74	/* Beacon Interrupt Interval Register, 16b */
/*
 * RTL8180 wakes host with RTW_INTR_BCNINT at BINTRITV
 * microseconds before TBTT
 */
#define	RTW_BINTRITV_BINTRITV	BITS(9, 0)
#define	RTW_ATIMTRITV	0x76	/* ATIM Interrupt Interval Register, 16b */
/*
 * RTL8180 wakes host with RTW_INTR_ATIMINT at ATIMTRITV
 * microseconds before end of ATIM Window
 */
#define	RTW_ATIMTRITV_ATIMTRITV	BITS(9, 0)

#define	RTW_PHYDELAY	0x78	/* PHY Delay Register, 8b */
/*
 * Rev. C magic from reference  driver
 */
#define	RTW_PHYDELAY_REVC_MAGIC	BIT(3)
/*
 * microsecond Tx delay between MAC and RF front-end
 */
#define	RTW_PHYDELAY_PHYDELAY	BITS(2, 0)
#define	RTW_CRCOUNT	0x79	/* Carrier Sense Counter, 8b */
#define	RTW_CRCOUNT_MAGIC	0x4c

#define	RTW_CRC16ERR	0x7a	/* CRC16 error count, 16b, XXX RTL8181 only? */

#define	RTW_BB	0x7c		/* Baseband interface, 32b */
/*
 * used for writing RTL8180's integrated baseband processor
 */
#define	RTW_BB_RD_MASK		BITS(23, 16)	/* data to read */
#define	RTW_BB_WR_MASK		BITS(15, 8)	/* data to write */
#define	RTW_BB_WREN		BIT(7)		/* write enable */
#define	RTW_BB_ADDR_MASK	BITS(6, 0)	/* address */

#define	RTW_PHYADDR	0x7c	/* Address register for PHY interface, 8b */
#define	RTW_PHYDATAW	0x7d	/* Write data to PHY, 8b, write-only */
#define	RTW_PHYDATAR	0x7e	/* Read data from PHY, 8b (?), read-only */

#define	RTW_PHYCFG	0x80	/* PHY Configuration Register, 32b */
/*
 * if !RTW_PHYCFG_HST, host sets. MAC clears after banging bits.
 */
#define	RTW_PHYCFG_MAC_POLL	BIT(31)
/*
 * 1: host bangs bits
 * 0: MAC bangs bits
 */
#define	RTW_PHYCFG_HST		BIT(30)
#define	RTW_PHYCFG_MAC_RFTYPE_MASK	BITS(29, 28)
#define	RTW_PHYCFG_MAC_RFTYPE_INTERSIL	LSHIFT(0, RTW_PHYCFG_MAC_RFTYPE_MASK)
#define	RTW_PHYCFG_MAC_RFTYPE_RFMD	LSHIFT(1, RTW_PHYCFG_MAC_RFTYPE_MASK)
#define	RTW_PHYCFG_MAC_RFTYPE_GCT	RTW_PHYCFG_MAC_RFTYPE_RFMD
#define	RTW_PHYCFG_MAC_RFTYPE_PHILIPS	LSHIFT(3, RTW_PHYCFG_MAC_RFTYPE_MASK)
#define	RTW_PHYCFG_MAC_PHILIPS_ADDR_MASK	BITS(27, 24)
#define	RTW_PHYCFG_MAC_PHILIPS_DATA_MASK	BITS(23, 0)
#define	RTW_PHYCFG_MAC_MAXIM_LODATA_MASK	BITS(27, 24)
#define	RTW_PHYCFG_MAC_MAXIM_ADDR_MASK		BITS(11, 8)
#define	RTW_PHYCFG_MAC_MAXIM_HIDATA_MASK	BITS(7, 0)
#define	RTW_PHYCFG_HST_EN		BIT(2)
#define	RTW_PHYCFG_HST_CLK		BIT(1)
#define	RTW_PHYCFG_HST_DATA		BIT(0)

#define	RTW_MAXIM_HIDATA_MASK	BITS(11, 4)
#define	RTW_MAXIM_LODATA_MASK	BITS(3, 0)

/*
 * 0x84 - 0xD3, page 1, selected when RTW_PSR[PSEN] == 1.
 */

#define	RTW_WAKEUP0L	0x84	/* Power Management Wakeup Frame */
#define	RTW_WAKEUP0H	0x88	/* 32b */

#define	RTW_WAKEUP1L	0x8c
#define	RTW_WAKEUP1H	0x90

#define	RTW_WAKEUP2LL	0x94
#define	RTW_WAKEUP2LH	0x98

#define	RTW_WAKEUP2HL	0x9c
#define	RTW_WAKEUP2HH	0xa0

#define	RTW_WAKEUP3LL	0xa4
#define	RTW_WAKEUP3LH	0xa8

#define	RTW_WAKEUP3HL	0xac
#define	RTW_WAKEUP3HH	0xb0

#define	RTW_WAKEUP4LL	0xb4
#define	RTW_WAKEUP4LH	0xb8

#define	RTW_WAKEUP4HL	0xbc
#define	RTW_WAKEUP4HH	0xc0

#define	RTW_CRC0	0xc4	/* CRC of wakeup frame 0, 16b */
#define	RTW_CRC1	0xc6	/* CRC of wakeup frame 1, 16b */
#define	RTW_CRC2	0xc8	/* CRC of wakeup frame 2, 16b */
#define	RTW_CRC3	0xca	/* CRC of wakeup frame 3, 16b */
#define	RTW_CRC4	0xcc	/* CRC of wakeup frame 4, 16b */

/*
 * 0x84 - 0xD3, page 0, selected when RTW_PSR[PSEN] == 0.
 */

/*
 * Default Key Registers, each 128b
 *
 * If RTW_SCR_KM_WEP104, 104 lsb are the key.
 * If RTW_SCR_KM_WEP40, 40 lsb are the key.
 */
#define	RTW_DK0		0x90	/* Default Key 0 Register, 128b */
#define	RTW_DK1		0xa0	/* Default Key 1 Register, 128b */
#define	RTW_DK2		0xb0	/* Default Key 2 Register, 128b */
#define	RTW_DK3		0xc0	/* Default Key 3 Register, 128b */

#define	RTW_CONFIG5	0xd8	/* Configuration Register 5, 8b */
#define	RTW_CONFIG5_TXFIFOOK	BIT(7)	/* Tx FIFO self-test pass, read-only */
#define	RTW_CONFIG5_RXFIFOOK	BIT(6)	/* Rx FIFO self-test pass, read-only */
/*
 * 1: start calibration cycle and raise AGCRESET pin.
 * 0: lower AGCRESET pin
 */
#define	RTW_CONFIG5_CALON	BIT(5)
#define	RTW_CONFIG5_EACPI	BIT(2)	/* Enable ACPI Wake up, default 0 */
/*
 * Enable LAN Wake signal, from EEPROM
 */
#define	RTW_CONFIG5_LANWAKE	BIT(1)
/*
 * 1: both software & PCI Reset reset PME_Status
 * 0: only software resets PME_Status
 *
 * From EEPROM.
 */
#define	RTW_CONFIG5_PMESTS	BIT(0)

/*
 * Transmit Priority Polling Register, 8b, write-only.
 */
#define	RTW_TPPOLL	0xd9
/*
 * RTL8180 clears to notify host of a beacon
 * Tx. Host writes have no effect.
 */
#define	RTW_TPPOLL_BQ	BIT(7)
/*
 * Host writes 1 to notify RTL8180 of high-priority Tx packets, RTL8180 clears
 * to after high-priority Tx is complete.
 */
#define	RTW_TPPOLL_HPQ	BIT(6)
/*
 * If RTW_CONFIG2_DPS is set, host writes 1 to notify RTL8180 of
 * normal-priority Tx packets, RTL8180 clears
 * after normal-priority Tx is complete.
 *
 * If RTW_CONFIG2_DPS is clear, host writes have no effect. RTL8180 clears after
 * normal-priority Tx is complete.
 */
#define	RTW_TPPOLL_NPQ	BIT(5)
/*
 * Host writes 1 to notify RTL8180 of low-priority Tx packets, RTL8180 clears
 * after low-priority Tx is complete.
 */
#define	RTW_TPPOLL_LPQ	BIT(4)
/*
 * Host writes 1 to tell RTL8180 to stop beacon DMA. This bit is invalid
 * when RTW_CONFIG2_DPS is set.
 */
#define	RTW_TPPOLL_SBQ	BIT(3)
/*
 * Host writes 1 to tell RTL8180 to stop high-priority DMA.
 */
#define	RTW_TPPOLL_SHPQ	BIT(2)
/*
 * Host writes 1 to tell RTL8180 to stop normal-priority DMA.
 * This bit is invalid when RTW_CONFIG2_DPS is set.
 */
#define	RTW_TPPOLL_SNPQ	BIT(1)
/*
 * Host writes 1 to tell RTL8180 to stop low-priority DMA.
 */
#define	RTW_TPPOLL_SLPQ	BIT(0)

/* Start all queues. */
#define	RTW_TPPOLL_ALL	(RTW_TPPOLL_BQ | RTW_TPPOLL_HPQ | \
			RTW_TPPOLL_NPQ | RTW_TPPOLL_LPQ)

/* Start queues solaris required. */
#define	RTW_TPPOLL_LN	(RTW_TPPOLL_NPQ | RTW_TPPOLL_LPQ)

/* Stop all queues. */
#define	RTW_TPPOLL_SALL	(RTW_TPPOLL_SBQ | RTW_TPPOLL_SHPQ | \
			RTW_TPPOLL_SNPQ | RTW_TPPOLL_SLPQ)

#define	RTW_CWR		0xdc	/* Contention Window Register, 16b, read-only */
/*
 * Contention Window: indicates number of contention windows before Tx
 */
#define	RTW_CWR_CW	BITS(9, 0)

/*
 * Retry Count Register, 16b, read-only
 */
#define	RTW_RETRYCTR	0xde
/*
 * Retry Count: indicates number of retries after Tx
 */
#define	RTW_RETRYCTR_RETRYCT	BITS(7, 0)

/*
 * Receive descriptor Start Address Register,
 * 32b, 256-byte alignment.
 */
#define	RTW_RDSAR	0xe4
/*
 * Function Event Register, 32b, Cardbus only. Only valid when
 * both RTW_CONFIG3_CARDBEN and RTW_CONFIG3_FUNCREGEN are set.
 */
#define	RTW_FER		0xf0
#define	RTW_FER_INTR	BIT(15)	/* set when RTW_FFER_INTR is set */
#define	RTW_FER_GWAKE	BIT(4)	/* General Wakeup */
/*
 * Function Event Mask Register, 32b, Cardbus only. Only valid when
 * both RTW_CONFIG3_CARDBEN and RTW_CONFIG3_FUNCREGEN are set.
 */
#define	RTW_FEMR	0xf4
#define	RTW_FEMR_INTR	BIT(15)	/* set when RTW_FFER_INTR is set */
#define	RTW_FEMR_WKUP	BIT(14)	/* Wakeup Mask */
#define	RTW_FEMR_GWAKE	BIT(4)	/* General Wakeup */
/*
 * Function Present State Register, 32b, read-only, Cardbus only.
 * Only valid when both RTW_CONFIG3_CARDBEN and RTW_CONFIG3_FUNCREGEN
 * are set.
 */
#define	RTW_FPSR	0xf8
#define	RTW_FPSR_INTR	BIT(15)	/* TBD */
#define	RTW_FPSR_GWAKE	BIT(4)	/* General Wakeup: TBD */
/*
 * Function Force Event Register, 32b, write-only, Cardbus only.
 * Only valid when both RTW_CONFIG3_CARDBEN and RTW_CONFIG3_FUNCREGEN
 * are set.
 */
#define	RTW_FFER	0xfc
#define	RTW_FFER_INTR	BIT(15)	/* TBD */
#define	RTW_FFER_GWAKE	BIT(4)	/* General Wakeup: TBD */

/*
 * Serial EEPROM offsets
 */
#define	RTW_SR_ID	0x00	/* 16b */
#define	RTW_SR_VID	0x02	/* 16b */
#define	RTW_SR_DID	0x04	/* 16b */
#define	RTW_SR_SVID	0x06	/* 16b */
#define	RTW_SR_SMID	0x08	/* 16b */
#define	RTW_SR_MNGNT	0x0a
#define	RTW_SR_MXLAT	0x0b
#define	RTW_SR_RFCHIPID	0x0c
#define	RTW_SR_CONFIG3	0x0d
#define	RTW_SR_MAC	0x0e	/* 6 bytes */
#define	RTW_SR_CONFIG0	0x14
#define	RTW_SR_CONFIG1	0x15
#define	RTW_SR_PMC	0x16	/* Power Management Capabilities, 16b */
#define	RTW_SR_CONFIG2	0x18
#define	RTW_SR_CONFIG4	0x19
#define	RTW_SR_ANAPARM	0x1a	/* Analog Parameters, 32b */
#define	RTW_SR_TESTR	0x1e
#define	RTW_SR_CONFIG5	0x1f
#define	RTW_SR_TXPOWER1		0x20
#define	RTW_SR_TXPOWER2		0x21
#define	RTW_SR_TXPOWER3		0x22
#define	RTW_SR_TXPOWER4		0x23
#define	RTW_SR_TXPOWER5		0x24
#define	RTW_SR_TXPOWER6		0x25
#define	RTW_SR_TXPOWER7		0x26
#define	RTW_SR_TXPOWER8		0x27
#define	RTW_SR_TXPOWER9		0x28
#define	RTW_SR_TXPOWER10	0x29
#define	RTW_SR_TXPOWER11	0x2a
#define	RTW_SR_TXPOWER12	0x2b
#define	RTW_SR_TXPOWER13	0x2c
#define	RTW_SR_TXPOWER14	0x2d
#define	RTW_SR_CHANNELPLAN	0x2e	/* bitmap of channels to scan */
#define	RTW_SR_ENERGYDETTHR	0x2f	/* energy-detect threshold */
#define	RTW_SR_ENERGYDETTHR_DEFAULT	0x0c	/* use this if old SROM */
#define	RTW_SR_CISPOINTER	0x30	/* 16b */
#define	RTW_SR_RFPARM		0x32	/* RF-specific parameter */
#define	RTW_SR_RFPARM_DIGPHY	BIT(0)		/* 1: digital PHY */
#define	RTW_SR_RFPARM_DFLANTB	BIT(1)		/* 1: antenna B is default */
#define	RTW_SR_RFPARM_CS_MASK	BITS(2, 3)	/* carrier-sense type */
#define	RTW_SR_VERSION		0x3c	/* EEPROM content version, 16b */
#define	RTW_SR_CRC		0x3e	/* EEPROM content CRC, 16b */
#define	RTW_SR_VPD		0x40	/* Vital Product Data, 64 bytes */
#define	RTW_SR_CIS		0x80	/* CIS Data, 93c56 only, 128 bytes */

/*
 * RTL8180 Transmit/Receive Descriptors
 */

/*
 * the first descriptor in each ring must be on a 256-byte boundary
 */
#define	RTW_DESC_ALIGNMENT 256

/*
 * Tx descriptor
 */
struct rtw_txdesc {
	uint32_t	td_ctl0;
	uint32_t	td_ctl1;
	uint32_t	td_buf;
	uint32_t	td_len;
	uint32_t	td_next;
	uint32_t	td_rsvd[3];
};

#define	td_stat td_ctl0

#define	RTW_TXCTL0_OWN			BIT(31)		/* 1: ready to Tx */
#define	RTW_TXCTL0_RSVD0		BIT(30)		/* reserved */
#define	RTW_TXCTL0_FS			BIT(29)		/* first segment */
#define	RTW_TXCTL0_LS			BIT(28)		/* last segment */

#define	RTW_TXCTL0_RATE_MASK		BITS(27, 24)	/* Tx rate */
#define	RTW_TXCTL0_RATE_1MBPS		LSHIFT(0, RTW_TXCTL0_RATE_MASK)
#define	RTW_TXCTL0_RATE_2MBPS		LSHIFT(1, RTW_TXCTL0_RATE_MASK)
#define	RTW_TXCTL0_RATE_5MBPS		LSHIFT(2, RTW_TXCTL0_RATE_MASK)
#define	RTW_TXCTL0_RATE_11MBPS		LSHIFT(3, RTW_TXCTL0_RATE_MASK)

#define	RTW_TXCTL0_RTSEN		BIT(23)		/* RTS Enable */

#define	RTW_TXCTL0_RTSRATE_MASK		BITS(22, 19)	/* Tx rate */
#define	RTW_TXCTL0_RTSRATE_1MBPS	LSHIFT(0, RTW_TXCTL0_RTSRATE_MASK)
#define	RTW_TXCTL0_RTSRATE_2MBPS	LSHIFT(1, RTW_TXCTL0_RTSRATE_MASK)
#define	RTW_TXCTL0_RTSRATE_5MBPS	LSHIFT(2, RTW_TXCTL0_RTSRATE_MASK)
#define	RTW_TXCTL0_RTSRATE_11MBPS	LSHIFT(3, RTW_TXCTL0_RTSRATE_MASK)

#define	RTW_TXCTL0_BEACON		BIT(18)	/* packet is a beacon */
#define	RTW_TXCTL0_MOREFRAG		BIT(17)	/* another fragment follows */
/*
 * add short PLCP preamble and header
 */
#define	RTW_TXCTL0_SPLCP		BIT(16)
#define	RTW_TXCTL0_KEYID_MASK		BITS(15, 14)	/* default key id */
#define	RTW_TXCTL0_RSVD1_MASK		BITS(13, 12)	/* reserved */
/*
 * Tx packet size in bytes
 */
#define	RTW_TXCTL0_TPKTSIZE_MASK	BITS(11, 0)

#define	RTW_TXSTAT_OWN		RTW_TXCTL0_OWN
#define	RTW_TXSTAT_RSVD0	RTW_TXCTL0_RSVD0
#define	RTW_TXSTAT_FS		RTW_TXCTL0_FS
#define	RTW_TXSTAT_LS		RTW_TXCTL0_LS
#define	RTW_TXSTAT_RSVD1_MASK	BITS(27, 16)
#define	RTW_TXSTAT_TOK		BIT(15)
#define	RTW_TXSTAT_RTSRETRY_MASK	BITS(14, 8)	/* RTS retry count */
#define	RTW_TXSTAT_DRC_MASK		BITS(7, 0)	/* Data retry count */

/*
 * supplements _LENGTH in packets sent 5.5Mb/s or faster
 */
#define	RTW_TXCTL1_LENGEXT	BIT(31)
#define	RTW_TXCTL1_LENGTH_MASK	BITS(30, 16)	/* PLCP length (microseconds) */
/*
 * RTS Duration (microseconds)
 */
#define	RTW_TXCTL1_RTSDUR_MASK	BITS(15, 0)

#define	RTW_TXLEN_LENGTH_MASK	BITS(11, 0)	/* Tx buffer length in bytes */

/*
 * Rx descriptor
 */
struct rtw_rxdesc {
    uint32_t	rd_ctl;
    uint32_t	rd_rsvd0;
    uint32_t	rd_buf;
    uint32_t	rd_rsvd1;
};

#define	rd_stat rd_ctl
#define	rd_rssi rd_rsvd0
#define	rd_tsftl rd_buf		/* valid only when RTW_RXSTAT_LS is set */
#define	rd_tsfth rd_rsvd1	/* valid only when RTW_RXSTAT_LS is set */

#define	RTW_RXCTL_OWN		BIT(31)		/* 1: owned by NIC */
#define	RTW_RXCTL_EOR		BIT(30)		/* end of ring */
#define	RTW_RXCTL_FS		BIT(29)		/* first segment */
#define	RTW_RXCTL_LS		BIT(28)		/* last segment */
#define	RTW_RXCTL_RSVD0_MASK	BITS(29, 12)	/* reserved */
#define	RTW_RXCTL_LENGTH_MASK	BITS(11, 0)	/* Rx buffer length */

#define	RTW_RXSTAT_OWN		RTW_RXCTL_OWN
#define	RTW_RXSTAT_EOR		RTW_RXCTL_EOR
#define	RTW_RXSTAT_FS		RTW_RXCTL_FS	/* first segment */
#define	RTW_RXSTAT_LS		RTW_RXCTL_LS	/* last segment */
#define	RTW_RXSTAT_DMAFAIL	BIT(27)		/* DMA failure on this pkt */
/*
 * buffer overflow XXX means FIFO exhausted?
 */
#define	RTW_RXSTAT_BOVF		BIT(26)
/*
 * Rx'd with short preamble and PLCP header
 */
#define	RTW_RXSTAT_SPLCP	BIT(25)
#define	RTW_RXSTAT_RSVD1	BIT(24)		/* reserved */
#define	RTW_RXSTAT_RATE_MASK	BITS(23, 20)	/* Rx rate */
#define	RTW_RXSTAT_RATE_1MBPS	LSHIFT(0, RTW_RXSTAT_RATE_MASK)
#define	RTW_RXSTAT_RATE_2MBPS	LSHIFT(1, RTW_RXSTAT_RATE_MASK)
#define	RTW_RXSTAT_RATE_5MBPS	LSHIFT(2, RTW_RXSTAT_RATE_MASK)
#define	RTW_RXSTAT_RATE_11MBPS	LSHIFT(3, RTW_RXSTAT_RATE_MASK)
#define	RTW_RXSTAT_MIC		BIT(19)		/* XXX from reference driver */
#define	RTW_RXSTAT_MAR		BIT(18)		/* is multicast */
#define	RTW_RXSTAT_PAR		BIT(17)		/* matches RTL8180's MAC */
#define	RTW_RXSTAT_BAR		BIT(16)		/* is broadcast */
/*
 * error summary. valid when RTW_RXSTAT_LS set. indicates
 * that either RTW_RXSTAT_CRC32 or RTW_RXSTAT_ICV is set.
 */
#define	RTW_RXSTAT_RES		BIT(15)
#define	RTW_RXSTAT_PWRMGT	BIT(14)		/* 802.11 PWRMGMT bit is set */
/*
 * XXX CRC16 error, from reference driver
 */
#define	RTW_RXSTAT_CRC16	BIT(14)
#define	RTW_RXSTAT_CRC32	BIT(13)		/* CRC32 error */
#define	RTW_RXSTAT_ICV		BIT(12)		/* ICV error */
/*
 * frame length, including CRC32
 */
#define	RTW_RXSTAT_LENGTH_MASK	BITS(11, 0)

/*
 * Convenient status conjunction.
 */
#define	RTW_RXSTAT_ONESEG	(RTW_RXSTAT_FS|RTW_RXSTAT_LS)
/*
 * Convenient status disjunctions.
 */
#define	RTW_RXSTAT_IOERROR	(RTW_RXSTAT_DMAFAIL|RTW_RXSTAT_BOVF)
#define	RTW_RXSTAT_DEBUG	(RTW_RXSTAT_SPLCP|RTW_RXSTAT_MAR|\
				RTW_RXSTAT_PAR|RTW_RXSTAT_BAR|\
				RTW_RXSTAT_PWRMGT|RTW_RXSTAT_CRC32|\
				RTW_RXSTAT_ICV)


#define	RTW_RXRSSI_VLAN		BITS(32, 16)	/* XXX from reference driver */
/*
 * for Philips RF front-ends
 */
#define	RTW_RXRSSI_RSSI		BITS(15, 8)	/* RF energy at the PHY */
/*
 * for RF front-ends by Intersil, Maxim, RFMD
 */
#define	RTW_RXRSSI_IMR_RSSI	BITS(15, 9)	/* RF energy at the PHY */
#define	RTW_RXRSSI_IMR_LNA	BIT(8)		/* 1: LNA activated */
#define	RTW_RXRSSI_SQ		BITS(7, 0)	/* Barker code-lock quality */

#define	RTW_READ8(regs, ofs)						\
	ddi_get8((regs)->r_handle,					\
	(uint8_t *)((regs)->r_base + (ofs)))

#define	RTW_READ16(regs, ofs)						\
	ddi_get16((regs)->r_handle,					\
	(uint16_t *)((uintptr_t)(regs)->r_base + (ofs)))

#define	RTW_READ(regs, ofs)						\
	ddi_get32((regs)->r_handle,					\
	(uint32_t *)((uintptr_t)(regs)->r_base + (ofs)))

#define	RTW_WRITE8(regs, ofs, val)					\
	ddi_put8((regs)->r_handle,					\
	(uint8_t *)((regs)->r_base + (ofs)), val)

#define	RTW_WRITE16(regs, ofs, val)					\
	ddi_put16((regs)->r_handle,					\
	(uint16_t *)((uintptr_t)(regs)->r_base + (ofs)), val)

#define	RTW_WRITE(regs, ofs, val)					\
	ddi_put32((regs)->r_handle,					\
	(uint32_t *)((uintptr_t)(regs)->r_base + (ofs)), val)

#define	RTW_ISSET(regs, reg, mask)					\
	(RTW_READ((regs), (reg)) & (mask))

#define	RTW_CLR(regs, reg, mask)					\
	RTW_WRITE((regs), (reg), RTW_READ((regs), (reg)) & ~(mask))

/*
 * bus_space(9) lied?
 */
#ifndef	BUS_SPACE_BARRIER_SYNC
#define	BUS_SPACE_BARRIER_SYNC (BUS_SPACE_BARRIER_READ|BUS_SPACE_BARRIER_WRITE)
#endif

#ifndef	BUS_SPACE_BARRIER_READ_BEFORE_READ
#define	BUS_SPACE_BARRIER_READ_BEFORE_READ BUS_SPACE_BARRIER_READ
#endif

#ifndef	BUS_SPACE_BARRIER_READ_BEFORE_WRITE
#define	BUS_SPACE_BARRIER_READ_BEFORE_WRITE BUS_SPACE_BARRIER_READ
#endif

#ifndef	BUS_SPACE_BARRIER_WRITE_BEFORE_READ
#define	BUS_SPACE_BARRIER_WRITE_BEFORE_READ BUS_SPACE_BARRIER_WRITE
#endif

#ifndef	BUS_SPACE_BARRIER_WRITE_BEFORE_WRITE
#define	BUS_SPACE_BARRIER_WRITE_BEFORE_WRITE BUS_SPACE_BARRIER_WRITE
#endif

/*
 * Bus barrier
 *
 * Complete outstanding read and/or write ops on [reg0, reg1]
 * ([reg1, reg0]) before starting new ops on the same region. See
 * acceptable bus_space_barrier(9) for the flag definitions.
 */
#define	RTW_BARRIER(regs, reg0, reg1, flags)
/*
 *	***just define a dummy macro here in solaris***
 *	bus_space_barrier((regs)->r_bh, (regs)->r_bt,		\
 *	    MIN(reg0, reg1), MAX(reg0, reg1) - MIN(reg0, reg1) + 4, flags)
 */
/*
 * Barrier convenience macros.
 */
/*
 * sync
 */
#define	RTW_SYNC(regs, reg0, reg1)				\
	RTW_BARRIER(regs, reg0, reg1, BUS_SPACE_BARRIER_SYNC)

/*
 * write-before-write
 */
#define	RTW_WBW(regs, reg0, reg1)				\
	RTW_BARRIER(regs, reg0, reg1, BUS_SPACE_BARRIER_WRITE_BEFORE_WRITE)

/*
 * write-before-read
 */
#define	RTW_WBR(regs, reg0, reg1)				\
	RTW_BARRIER(regs, reg0, reg1, BUS_SPACE_BARRIER_WRITE_BEFORE_READ)

/*
 * read-before-read
 */
#define	RTW_RBR(regs, reg0, reg1)				\
	RTW_BARRIER(regs, reg0, reg1, BUS_SPACE_BARRIER_READ_BEFORE_READ)

/*
 * read-before-read
 */
#define	RTW_RBW(regs, reg0, reg1)				\
	RTW_BARRIER(regs, reg0, reg1, BUS_SPACE_BARRIER_READ_BEFORE_WRITE)

#define	RTW_WBRW(regs, reg0, reg1)				\
		RTW_BARRIER(regs, reg0, reg1,			\
		    BUS_SPACE_BARRIER_WRITE_BEFORE_READ |	\
		    BUS_SPACE_BARRIER_WRITE_BEFORE_WRITE)

/*
 * Registers for RTL8180L's built-in baseband modem.
 */
#define	RTW_BBP_SYS1		0x00
#define	RTW_BBP_TXAGC		0x03	/* guess: transmit auto gain control */
/*
 * guess: low-noise amplifier activation threshold
 */
#define	RTW_BBP_LNADET		0x04
/*
 * guess: intermediate frequency (IF)
 * auto-gain control (AGC) initial value
 */
#define	RTW_BBP_IFAGCINI	0x05
#define	RTW_BBP_IFAGCLIMIT	0x06	/* guess: IF AGC maximum value */
/*
 * guess: activation threshold for IF AGC loop
 */
#define	RTW_BBP_IFAGCDET	0x07

#define	RTW_BBP_ANTATTEN	0x10	/* guess: antenna & attenuation */
#define	RTW_BBP_ANTATTEN_PHILIPS_MAGIC		0x91
#define	RTW_BBP_ANTATTEN_INTERSIL_MAGIC		0x92
#define	RTW_BBP_ANTATTEN_RFMD_MAGIC		0x93
#define	RTW_BBP_ANTATTEN_MAXIM_MAGIC		0xb3
#define	RTW_BBP_ANTATTEN_DFLANTB		0x40
#define	RTW_BBP_ANTATTEN_CHAN14			0x0c

/*
 * guess: transmit/receive switch latency
 */
#define	RTW_BBP_TRL			0x11
#define	RTW_BBP_SYS2			0x12
#define	RTW_BBP_SYS2_ANTDIV		0x80	/* enable antenna diversity */
/*
 * loopback rate?
 * 0: 1Mbps
 * 1: 2Mbps
 * 2: 5.5Mbps
 * 3: 11Mbps
 */
#define	RTW_BBP_SYS2_RATE_MASK		BITS(5, 4)
#define	RTW_BBP_SYS3			0x13
/*
 * carrier-sense threshold
 */
#define	RTW_BBP_SYS3_CSTHRESH_MASK	BITS(0, 3)
/*
 * guess: channel energy-detect threshold
 */
#define	RTW_BBP_CHESTLIM	0x19
/*
 * guess: channel signal-quality threshold
 */
#define	RTW_BBP_CHSQLIM		0x1a

#define	RTW_EPROM_CMD_OPERATING_MODE_MASK	((1<<7)|(1<<6))
#define	RTW_EPROM_CMD_OPERATING_MODE_SHIFT	6
#define	RTW_EPROM_CS_SHIFT	3
#define	RTW_EPROM_CK_SHIFT	2
#define	RTW_EPROM_CMD_CONFIG	0x3
#define	RTW_EPROM_CMD_NORMAL	0
#define	RTW_EPROM_CMD_LOAD 1
#define	RTW_TX_DMA_POLLING_HIPRIORITY_SHIFT 6
#define	RTW_TX_DMA_POLLING_NORMPRIORITY_SHIFT 5
#define	RTW_TX_DMA_POLLING_LOWPRIORITY_SHIFT 4
#define	RTW_CONFIG2_DMA_POLLING_MODE_SHIFT 3

#define	RTW_CMD_RST_SHIFT (4)
#define	RTW_TX_DMA_STOP_BEACON_SHIFT 3
#ifdef __cplusplus
}
#endif

#endif /* _RTW_REG_H_ */
