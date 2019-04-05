/******************************************************************************
 *
 * Copyright(c) 2012 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2013 - 2015 Intel Mobile Communications GmbH
 * Copyright(c) 2015 - 2017 Intel Deutschland GmbH
 * Copyright(c) 2018 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#ifndef SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FW_API_RX_H_
#define SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FW_API_RX_H_

/* API for pre-9000 hardware */

#define IWL_RX_INFO_PHY_CNT 8
#define IWL_RX_INFO_ENERGY_ANT_ABC_IDX 1
#define IWL_RX_INFO_ENERGY_ANT_A_MSK 0x000000ff
#define IWL_RX_INFO_ENERGY_ANT_B_MSK 0x0000ff00
#define IWL_RX_INFO_ENERGY_ANT_C_MSK 0x00ff0000
#define IWL_RX_INFO_ENERGY_ANT_A_POS 0
#define IWL_RX_INFO_ENERGY_ANT_B_POS 8
#define IWL_RX_INFO_ENERGY_ANT_C_POS 16

enum iwl_mac_context_info {
    MAC_CONTEXT_INFO_NONE,
    MAC_CONTEXT_INFO_GSCAN,
};

/**
 * struct iwl_rx_phy_info - phy info
 * (REPLY_RX_PHY_CMD = 0xc0)
 * @non_cfg_phy_cnt: non configurable DSP phy data byte count
 * @cfg_phy_cnt: configurable DSP phy data byte count
 * @stat_id: configurable DSP phy data set ID
 * @reserved1: reserved
 * @system_timestamp: GP2  at on air rise
 * @timestamp: TSF at on air rise
 * @beacon_time_stamp: beacon at on-air rise
 * @phy_flags: general phy flags: band, modulation, ...
 * @channel: channel number
 * @non_cfg_phy: for various implementations of non_cfg_phy
 * @rate_n_flags: RATE_MCS_*
 * @byte_count: frame's byte-count
 * @frame_time: frame's time on the air, based on byte count and frame rate
 *  calculation
 * @mac_active_msk: what MACs were active when the frame was received
 * @mac_context_info: additional info on the context in which the frame was
 *  received as defined in &enum iwl_mac_context_info
 *
 * Before each Rx, the device sends this data. It contains PHY information
 * about the reception of the packet.
 */
struct iwl_rx_phy_info {
    uint8_t non_cfg_phy_cnt;
    uint8_t cfg_phy_cnt;
    uint8_t stat_id;
    uint8_t reserved1;
    __le32 system_timestamp;
    __le64 timestamp;
    __le32 beacon_time_stamp;
    __le16 phy_flags;
    __le16 channel;
    __le32 non_cfg_phy[IWL_RX_INFO_PHY_CNT];
    __le32 rate_n_flags;
    __le32 byte_count;
    uint8_t mac_active_msk;
    uint8_t mac_context_info;
    __le16 frame_time;
} __packed;

/*
 * TCP offload Rx assist info
 *
 * bits 0:3 - reserved
 * bits 4:7 - MIC CRC length
 * bits 8:12 - MAC header length
 * bit 13 - Padding indication
 * bit 14 - A-AMSDU indication
 * bit 15 - Offload enabled
 */
enum iwl_csum_rx_assist_info {
    CSUM_RXA_RESERVED_MASK = 0x000f,
    CSUM_RXA_MICSIZE_MASK = 0x00f0,
    CSUM_RXA_HEADERLEN_MASK = 0x1f00,
    CSUM_RXA_PADD = BIT(13),
    CSUM_RXA_AMSDU = BIT(14),
    CSUM_RXA_ENA = BIT(15)
};

/**
 * struct iwl_rx_mpdu_res_start - phy info
 * @byte_count: byte count of the frame
 * @assist: see &enum iwl_csum_rx_assist_info
 */
struct iwl_rx_mpdu_res_start {
    __le16 byte_count;
    __le16 assist;
} __packed; /* _RX_MPDU_RES_START_API_S_VER_2 */

/**
 * enum iwl_rx_phy_flags - to parse %iwl_rx_phy_info phy_flags
 * @RX_RES_PHY_FLAGS_BAND_24: true if the packet was received on 2.4 band
 * @RX_RES_PHY_FLAGS_MOD_CCK: modulation is CCK
 * @RX_RES_PHY_FLAGS_SHORT_PREAMBLE: true if packet's preamble was short
 * @RX_RES_PHY_FLAGS_NARROW_BAND: narrow band (<20 MHz) receive
 * @RX_RES_PHY_FLAGS_ANTENNA: antenna on which the packet was received
 * @RX_RES_PHY_FLAGS_ANTENNA_POS: antenna bit position
 * @RX_RES_PHY_FLAGS_AGG: set if the packet was part of an A-MPDU
 * @RX_RES_PHY_FLAGS_OFDM_HT: The frame was an HT frame
 * @RX_RES_PHY_FLAGS_OFDM_GF: The frame used GF preamble
 * @RX_RES_PHY_FLAGS_OFDM_VHT: The frame was a VHT frame
 */
enum iwl_rx_phy_flags {
    RX_RES_PHY_FLAGS_BAND_24 = BIT(0),
    RX_RES_PHY_FLAGS_MOD_CCK = BIT(1),
    RX_RES_PHY_FLAGS_SHORT_PREAMBLE = BIT(2),
    RX_RES_PHY_FLAGS_NARROW_BAND = BIT(3),
    RX_RES_PHY_FLAGS_ANTENNA = (0x7 << 4),
    RX_RES_PHY_FLAGS_ANTENNA_POS = 4,
    RX_RES_PHY_FLAGS_AGG = BIT(7),
    RX_RES_PHY_FLAGS_OFDM_HT = BIT(8),
    RX_RES_PHY_FLAGS_OFDM_GF = BIT(9),
    RX_RES_PHY_FLAGS_OFDM_VHT = BIT(10),
};

/**
 * enum iwl_mvm_rx_status - written by fw for each Rx packet
 * @RX_MPDU_RES_STATUS_CRC_OK: CRC is fine
 * @RX_MPDU_RES_STATUS_OVERRUN_OK: there was no RXE overflow
 * @RX_MPDU_RES_STATUS_SRC_STA_FOUND: station was found
 * @RX_MPDU_RES_STATUS_KEY_VALID: key was valid
 * @RX_MPDU_RES_STATUS_KEY_PARAM_OK: key parameters were usable
 * @RX_MPDU_RES_STATUS_ICV_OK: ICV is fine, if not, the packet is destroyed
 * @RX_MPDU_RES_STATUS_MIC_OK: used for CCM alg only. TKIP MIC is checked
 *  in the driver.
 * @RX_MPDU_RES_STATUS_TTAK_OK: TTAK is fine
 * @RX_MPDU_RES_STATUS_MNG_FRAME_REPLAY_ERR:  valid for alg = CCM_CMAC or
 *  alg = CCM only. Checks replay attack for 11w frames. Relevant only if
 *  %RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME is set.
 * @RX_MPDU_RES_STATUS_SEC_NO_ENC: this frame is not encrypted
 * @RX_MPDU_RES_STATUS_SEC_WEP_ENC: this frame is encrypted using WEP
 * @RX_MPDU_RES_STATUS_SEC_CCM_ENC: this frame is encrypted using CCM
 * @RX_MPDU_RES_STATUS_SEC_TKIP_ENC: this frame is encrypted using TKIP
 * @RX_MPDU_RES_STATUS_SEC_EXT_ENC: this frame is encrypted using extension
 *  algorithm
 * @RX_MPDU_RES_STATUS_SEC_CCM_CMAC_ENC: this frame is encrypted using CCM_CMAC
 * @RX_MPDU_RES_STATUS_SEC_ENC_ERR: this frame couldn't be decrypted
 * @RX_MPDU_RES_STATUS_SEC_ENC_MSK: bitmask of the encryption algorithm
 * @RX_MPDU_RES_STATUS_DEC_DONE: this frame has been successfully decrypted
 * @RX_MPDU_RES_STATUS_EXT_IV_BIT_CMP: extended IV (set with TKIP)
 * @RX_MPDU_RES_STATUS_KEY_ID_CMP_BIT: key ID comparison done
 * @RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME: this frame is an 11w management frame
 * @RX_MPDU_RES_STATUS_CSUM_DONE: checksum was done by the hw
 * @RX_MPDU_RES_STATUS_CSUM_OK: checksum found no errors
 * @RX_MPDU_RES_STATUS_STA_ID_MSK: station ID mask
 * @RX_MDPU_RES_STATUS_STA_ID_SHIFT: station ID bit shift
 */
enum iwl_mvm_rx_status {
    RX_MPDU_RES_STATUS_CRC_OK = BIT(0),
    RX_MPDU_RES_STATUS_OVERRUN_OK = BIT(1),
    RX_MPDU_RES_STATUS_SRC_STA_FOUND = BIT(2),
    RX_MPDU_RES_STATUS_KEY_VALID = BIT(3),
    RX_MPDU_RES_STATUS_KEY_PARAM_OK = BIT(4),
    RX_MPDU_RES_STATUS_ICV_OK = BIT(5),
    RX_MPDU_RES_STATUS_MIC_OK = BIT(6),
    RX_MPDU_RES_STATUS_TTAK_OK = BIT(7),
    RX_MPDU_RES_STATUS_MNG_FRAME_REPLAY_ERR = BIT(7),
    RX_MPDU_RES_STATUS_SEC_NO_ENC = (0 << 8),
    RX_MPDU_RES_STATUS_SEC_WEP_ENC = (1 << 8),
    RX_MPDU_RES_STATUS_SEC_CCM_ENC = (2 << 8),
    RX_MPDU_RES_STATUS_SEC_TKIP_ENC = (3 << 8),
    RX_MPDU_RES_STATUS_SEC_EXT_ENC = (4 << 8),
    RX_MPDU_RES_STATUS_SEC_CCM_CMAC_ENC = (6 << 8),
    RX_MPDU_RES_STATUS_SEC_ENC_ERR = (7 << 8),
    RX_MPDU_RES_STATUS_SEC_ENC_MSK = (7 << 8),
    RX_MPDU_RES_STATUS_DEC_DONE = BIT(11),
    RX_MPDU_RES_STATUS_EXT_IV_BIT_CMP = BIT(13),
    RX_MPDU_RES_STATUS_KEY_ID_CMP_BIT = BIT(14),
    RX_MPDU_RES_STATUS_ROBUST_MNG_FRAME = BIT(15),
    RX_MPDU_RES_STATUS_CSUM_DONE = BIT(16),
    RX_MPDU_RES_STATUS_CSUM_OK = BIT(17),
    RX_MDPU_RES_STATUS_STA_ID_SHIFT = 24,
    RX_MPDU_RES_STATUS_STA_ID_MSK = 0x1f << RX_MDPU_RES_STATUS_STA_ID_SHIFT,
};

/* 9000 series API */
enum iwl_rx_mpdu_mac_flags1 {
    IWL_RX_MDPU_MFLG1_ADDRTYPE_MASK = 0x03,
    IWL_RX_MPDU_MFLG1_MIC_CRC_LEN_MASK = 0xf0,
    /* shift should be 4, but the length is measured in 2-byte
     * words, so shifting only by 3 gives a byte result
     */
    IWL_RX_MPDU_MFLG1_MIC_CRC_LEN_SHIFT = 3,
};

enum iwl_rx_mpdu_mac_flags2 {
    /* in 2-byte words */
    IWL_RX_MPDU_MFLG2_HDR_LEN_MASK = 0x1f,
    IWL_RX_MPDU_MFLG2_PAD = 0x20,
    IWL_RX_MPDU_MFLG2_AMSDU = 0x40,
};

enum iwl_rx_mpdu_amsdu_info {
    IWL_RX_MPDU_AMSDU_SUBFRAME_IDX_MASK = 0x7f,
    IWL_RX_MPDU_AMSDU_LAST_SUBFRAME = 0x80,
};

enum iwl_rx_l3_proto_values {
    IWL_RX_L3_TYPE_NONE,
    IWL_RX_L3_TYPE_IPV4,
    IWL_RX_L3_TYPE_IPV4_FRAG,
    IWL_RX_L3_TYPE_IPV6_FRAG,
    IWL_RX_L3_TYPE_IPV6,
    IWL_RX_L3_TYPE_IPV6_IN_IPV4,
    IWL_RX_L3_TYPE_ARP,
    IWL_RX_L3_TYPE_EAPOL,
};

#define IWL_RX_L3_PROTO_POS 4

enum iwl_rx_l3l4_flags {
    IWL_RX_L3L4_IP_HDR_CSUM_OK = BIT(0),
    IWL_RX_L3L4_TCP_UDP_CSUM_OK = BIT(1),
    IWL_RX_L3L4_TCP_FIN_SYN_RST_PSH = BIT(2),
    IWL_RX_L3L4_TCP_ACK = BIT(3),
    IWL_RX_L3L4_L3_PROTO_MASK = 0xf << IWL_RX_L3_PROTO_POS,
    IWL_RX_L3L4_L4_PROTO_MASK = 0xf << 8,
    IWL_RX_L3L4_RSS_HASH_MASK = 0xf << 12,
};

enum iwl_rx_mpdu_status {
    IWL_RX_MPDU_STATUS_CRC_OK = BIT(0),
    IWL_RX_MPDU_STATUS_OVERRUN_OK = BIT(1),
    IWL_RX_MPDU_STATUS_SRC_STA_FOUND = BIT(2),
    IWL_RX_MPDU_STATUS_KEY_VALID = BIT(3),
    IWL_RX_MPDU_STATUS_KEY_PARAM_OK = BIT(4),
    IWL_RX_MPDU_STATUS_ICV_OK = BIT(5),
    IWL_RX_MPDU_STATUS_MIC_OK = BIT(6),
    IWL_RX_MPDU_RES_STATUS_TTAK_OK = BIT(7),
    IWL_RX_MPDU_STATUS_SEC_MASK = 0x7 << 8,
    IWL_RX_MPDU_STATUS_SEC_UNKNOWN = IWL_RX_MPDU_STATUS_SEC_MASK,
    IWL_RX_MPDU_STATUS_SEC_NONE = 0x0 << 8,
    IWL_RX_MPDU_STATUS_SEC_WEP = 0x1 << 8,
    IWL_RX_MPDU_STATUS_SEC_CCM = 0x2 << 8,
    IWL_RX_MPDU_STATUS_SEC_TKIP = 0x3 << 8,
    IWL_RX_MPDU_STATUS_SEC_EXT_ENC = 0x4 << 8,
    IWL_RX_MPDU_STATUS_SEC_GCM = 0x5 << 8,
    IWL_RX_MPDU_STATUS_DECRYPTED = BIT(11),
    IWL_RX_MPDU_STATUS_WEP_MATCH = BIT(12),
    IWL_RX_MPDU_STATUS_EXT_IV_MATCH = BIT(13),
    IWL_RX_MPDU_STATUS_KEY_ID_MATCH = BIT(14),
    IWL_RX_MPDU_STATUS_ROBUST_MNG_FRAME = BIT(15),
};

enum iwl_rx_mpdu_hash_filter {
    IWL_RX_MPDU_HF_A1_HASH_MASK = 0x3f,
    IWL_RX_MPDU_HF_FILTER_STATUS_MASK = 0xc0,
};

enum iwl_rx_mpdu_sta_id_flags {
    IWL_RX_MPDU_SIF_STA_ID_MASK = 0x1f,
    IWL_RX_MPDU_SIF_RRF_ABORT = 0x20,
    IWL_RX_MPDU_SIF_FILTER_STATUS_MASK = 0xc0,
};

#define IWL_RX_REORDER_DATA_INVALID_BAID 0x7f

enum iwl_rx_mpdu_reorder_data {
    IWL_RX_MPDU_REORDER_NSSN_MASK = 0x00000fff,
    IWL_RX_MPDU_REORDER_SN_MASK = 0x00fff000,
    IWL_RX_MPDU_REORDER_SN_SHIFT = 12,
    IWL_RX_MPDU_REORDER_BAID_MASK = 0x7f000000,
    IWL_RX_MPDU_REORDER_BAID_SHIFT = 24,
    IWL_RX_MPDU_REORDER_BA_OLD_SN = 0x80000000,
};

enum iwl_rx_mpdu_phy_info {
    IWL_RX_MPDU_PHY_8023 = BIT(0),
    IWL_RX_MPDU_PHY_AMPDU = BIT(5),
    IWL_RX_MPDU_PHY_AMPDU_TOGGLE = BIT(6),
    IWL_RX_MPDU_PHY_SHORT_PREAMBLE = BIT(7),
    IWL_RX_MPDU_PHY_TSF_OVERLOAD = BIT(8),
};

enum iwl_rx_mpdu_mac_info {
    IWL_RX_MPDU_PHY_MAC_INDEX_MASK = 0x0f,
    IWL_RX_MPDU_PHY_PHY_INDEX_MASK = 0xf0,
};

/* TSF overload low dword */
enum iwl_rx_phy_data0 {
    /* info type: HE any */
    IWL_RX_PHY_DATA0_HE_BEAM_CHNG = 0x00000001,
    IWL_RX_PHY_DATA0_HE_UPLINK = 0x00000002,
    IWL_RX_PHY_DATA0_HE_BSS_COLOR_MASK = 0x000000fc,
    IWL_RX_PHY_DATA0_HE_SPATIAL_REUSE_MASK = 0x00000f00,
    /* 1 bit reserved */
    IWL_RX_PHY_DATA0_HE_TXOP_DUR_MASK = 0x000fe000,
    IWL_RX_PHY_DATA0_HE_LDPC_EXT_SYM = 0x00100000,
    IWL_RX_PHY_DATA0_HE_PRE_FEC_PAD_MASK = 0x00600000,
    IWL_RX_PHY_DATA0_HE_PE_DISAMBIG = 0x00800000,
    IWL_RX_PHY_DATA0_HE_DOPPLER = 0x01000000,
    /* 6 bits reserved */
    IWL_RX_PHY_DATA0_HE_DELIM_EOF = 0x80000000,
};

enum iwl_rx_phy_info_type {
    IWL_RX_PHY_INFO_TYPE_NONE = 0,
    IWL_RX_PHY_INFO_TYPE_CCK = 1,
    IWL_RX_PHY_INFO_TYPE_OFDM_LGCY = 2,
    IWL_RX_PHY_INFO_TYPE_HT = 3,
    IWL_RX_PHY_INFO_TYPE_VHT_SU = 4,
    IWL_RX_PHY_INFO_TYPE_VHT_MU = 5,
    IWL_RX_PHY_INFO_TYPE_HE_SU = 6,
    IWL_RX_PHY_INFO_TYPE_HE_MU = 7,
    IWL_RX_PHY_INFO_TYPE_HE_TB = 8,
    IWL_RX_PHY_INFO_TYPE_HE_MU_EXT = 9,
    IWL_RX_PHY_INFO_TYPE_HE_TB_EXT = 10,
};

/* TSF overload high dword */
enum iwl_rx_phy_data1 {
    /*
     * check this first - if TSF overload is set,
     * see &enum iwl_rx_phy_info_type
     */
    IWL_RX_PHY_DATA1_INFO_TYPE_MASK = 0xf0000000,

    /* info type: HT/VHT/HE any */
    IWL_RX_PHY_DATA1_LSIG_LEN_MASK = 0x0fff0000,

    /* info type: HE MU/MU-EXT */
    IWL_RX_PHY_DATA1_HE_MU_SIGB_COMPRESSION = 0x00000001,
    IWL_RX_PHY_DATA1_HE_MU_SIBG_SYM_OR_USER_NUM_MASK = 0x0000001e,

    /* info type: HE any */
    IWL_RX_PHY_DATA1_HE_LTF_NUM_MASK = 0x000000e0,
    IWL_RX_PHY_DATA1_HE_RU_ALLOC_SEC80 = 0x00000100,
    /* trigger encoded */
    IWL_RX_PHY_DATA1_HE_RU_ALLOC_MASK = 0x0000fe00,

    /* info type: HE TB/TX-EXT */
    IWL_RX_PHY_DATA1_HE_TB_PILOT_TYPE = 0x00000001,
    IWL_RX_PHY_DATA1_HE_TB_LOW_SS_MASK = 0x0000000e,
};

/* goes into Metadata DW 7 */
enum iwl_rx_phy_data2 {
    /* info type: HE MU-EXT */
    /* the a1/a2/... is what the PHY/firmware calls the values */
    IWL_RX_PHY_DATA2_HE_MU_EXT_CH1_RU0 = 0x000000ff, /* a1 */
    IWL_RX_PHY_DATA2_HE_MU_EXT_CH1_RU2 = 0x0000ff00, /* a2 */
    IWL_RX_PHY_DATA2_HE_MU_EXT_CH2_RU0 = 0x00ff0000, /* b1 */
    IWL_RX_PHY_DATA2_HE_MU_EXT_CH2_RU2 = 0xff000000, /* b2 */

    /* info type: HE TB-EXT */
    IWL_RX_PHY_DATA2_HE_TB_EXT_SPTL_REUSE1 = 0x0000000f,
    IWL_RX_PHY_DATA2_HE_TB_EXT_SPTL_REUSE2 = 0x000000f0,
    IWL_RX_PHY_DATA2_HE_TB_EXT_SPTL_REUSE3 = 0x00000f00,
    IWL_RX_PHY_DATA2_HE_TB_EXT_SPTL_REUSE4 = 0x0000f000,
};

/* goes into Metadata DW 8 */
enum iwl_rx_phy_data3 {
    /* info type: HE MU-EXT */
    IWL_RX_PHY_DATA3_HE_MU_EXT_CH1_RU1 = 0x000000ff, /* c1 */
    IWL_RX_PHY_DATA3_HE_MU_EXT_CH1_RU3 = 0x0000ff00, /* c2 */
    IWL_RX_PHY_DATA3_HE_MU_EXT_CH2_RU1 = 0x00ff0000, /* d1 */
    IWL_RX_PHY_DATA3_HE_MU_EXT_CH2_RU3 = 0xff000000, /* d2 */
};

/* goes into Metadata DW 4 high 16 bits */
enum iwl_rx_phy_data4 {
    /* info type: HE MU-EXT */
    IWL_RX_PHY_DATA4_HE_MU_EXT_CH1_CTR_RU = 0x0001,
    IWL_RX_PHY_DATA4_HE_MU_EXT_CH2_CTR_RU = 0x0002,
    IWL_RX_PHY_DATA4_HE_MU_EXT_CH1_CRC_OK = 0x0004,
    IWL_RX_PHY_DATA4_HE_MU_EXT_CH2_CRC_OK = 0x0008,
    IWL_RX_PHY_DATA4_HE_MU_EXT_SIGB_MCS_MASK = 0x00f0,
    IWL_RX_PHY_DATA4_HE_MU_EXT_SIGB_DCM = 0x0100,
    IWL_RX_PHY_DATA4_HE_MU_EXT_PREAMBLE_PUNC_TYPE_MASK = 0x0600,
};

/**
 * struct iwl_rx_mpdu_desc_v1 - RX MPDU descriptor
 */
struct iwl_rx_mpdu_desc_v1 {
    /* DW7 - carries rss_hash only when rpa_en == 1 */
    union {
        /**
         * @rss_hash: RSS hash value
         */
        __le32 rss_hash;

        /**
         * @phy_data2: depends on info type (see @phy_data1)
         */
        __le32 phy_data2;
    };

    /* DW8 - carries filter_match only when rpa_en == 1 */
    union {
        /**
         * @filter_match: filter match value
         */
        __le32 filter_match;

        /**
         * @phy_data3: depends on info type (see @phy_data1)
         */
        __le32 phy_data3;
    };

    /* DW9 */
    /**
     * @rate_n_flags: RX rate/flags encoding
     */
    __le32 rate_n_flags;
    /* DW10 */
    /**
     * @energy_a: energy chain A
     */
    uint8_t energy_a;
    /**
     * @energy_b: energy chain B
     */
    uint8_t energy_b;
    /**
     * @channel: channel number
     */
    uint8_t channel;
    /**
     * @mac_context: MAC context mask
     */
    uint8_t mac_context;
    /* DW11 */
    /**
     * @gp2_on_air_rise: GP2 timer value on air rise (INA)
     */
    __le32 gp2_on_air_rise;
    /* DW12 & DW13 */
    union {
        /**
         * @tsf_on_air_rise:
         * TSF value on air rise (INA), only valid if
         * %IWL_RX_MPDU_PHY_TSF_OVERLOAD isn't set
         */
        __le64 tsf_on_air_rise;

        struct {
            /**
             * @phy_data0: depends on info_type, see @phy_data1
             */
            __le32 phy_data0;
            /**
             * @phy_data1: valid only if
             * %IWL_RX_MPDU_PHY_TSF_OVERLOAD is set,
             * see &enum iwl_rx_phy_data1.
             */
            __le32 phy_data1;
        };
    };
} __packed;

/**
 * struct iwl_rx_mpdu_desc_v3 - RX MPDU descriptor
 */
struct iwl_rx_mpdu_desc_v3 {
    /* DW7 - carries filter_match only when rpa_en == 1 */
    union {
        /**
         * @filter_match: filter match value
         */
        __le32 filter_match;

        /**
         * @phy_data2: depends on info type (see @phy_data1)
         */
        __le32 phy_data2;
    };

    /* DW8 - carries rss_hash only when rpa_en == 1 */
    union {
        /**
         * @rss_hash: RSS hash value
         */
        __le32 rss_hash;

        /**
         * @phy_data3: depends on info type (see @phy_data1)
         */
        __le32 phy_data3;
    };
    /* DW9 */
    /**
     * @partial_hash: 31:0 ip/tcp header hash
     *  w/o some fields (such as IP SRC addr)
     */
    __le32 partial_hash;
    /* DW10 */
    /**
     * @raw_xsum: raw xsum value
     */
    __le32 raw_xsum;
    /* DW11 */
    /**
     * @rate_n_flags: RX rate/flags encoding
     */
    __le32 rate_n_flags;
    /* DW12 */
    /**
     * @energy_a: energy chain A
     */
    uint8_t energy_a;
    /**
     * @energy_b: energy chain B
     */
    uint8_t energy_b;
    /**
     * @channel: channel number
     */
    uint8_t channel;
    /**
     * @mac_context: MAC context mask
     */
    uint8_t mac_context;
    /* DW13 */
    /**
     * @gp2_on_air_rise: GP2 timer value on air rise (INA)
     */
    __le32 gp2_on_air_rise;
    /* DW14 & DW15 */
    union {
        /**
         * @tsf_on_air_rise:
         * TSF value on air rise (INA), only valid if
         * %IWL_RX_MPDU_PHY_TSF_OVERLOAD isn't set
         */
        __le64 tsf_on_air_rise;

        struct {
            /**
             * @phy_data0: depends on info_type, see @phy_data1
             */
            __le32 phy_data0;
            /**
             * @phy_data1: valid only if
             * %IWL_RX_MPDU_PHY_TSF_OVERLOAD is set,
             * see &enum iwl_rx_phy_data1.
             */
            __le32 phy_data1;
        };
    };
    /* DW16 & DW17 */
    /**
     * @reserved: reserved
     */
    __le32 reserved[2];
} __packed; /* RX_MPDU_RES_START_API_S_VER_3 */

/**
 * struct iwl_rx_mpdu_desc - RX MPDU descriptor
 */
struct iwl_rx_mpdu_desc {
    /* DW2 */
    /**
     * @mpdu_len: MPDU length
     */
    __le16 mpdu_len;
    /**
     * @mac_flags1: &enum iwl_rx_mpdu_mac_flags1
     */
    uint8_t mac_flags1;
    /**
     * @mac_flags2: &enum iwl_rx_mpdu_mac_flags2
     */
    uint8_t mac_flags2;
    /* DW3 */
    /**
     * @amsdu_info: &enum iwl_rx_mpdu_amsdu_info
     */
    uint8_t amsdu_info;
    /**
     * @phy_info: &enum iwl_rx_mpdu_phy_info
     */
    __le16 phy_info;
    /**
     * @mac_phy_idx: MAC/PHY index
     */
    uint8_t mac_phy_idx;
    /* DW4 - carries csum data only when rpa_en == 1 */
    /**
     * @raw_csum: raw checksum (alledgedly unreliable)
     */
    __le16 raw_csum;

    union {
        /**
         * @l3l4_flags: &enum iwl_rx_l3l4_flags
         */
        __le16 l3l4_flags;

        /**
         * @phy_data4: depends on info type, see phy_data1
         */
        __le16 phy_data4;
    };
    /* DW5 */
    /**
     * @status: &enum iwl_rx_mpdu_status
     */
    __le16 status;
    /**
     * @hash_filter: hash filter value
     */
    uint8_t hash_filter;
    /**
     * @sta_id_flags: &enum iwl_rx_mpdu_sta_id_flags
     */
    uint8_t sta_id_flags;
    /* DW6 */
    /**
     * @reorder_data: &enum iwl_rx_mpdu_reorder_data
     */
    __le32 reorder_data;

    union {
        struct iwl_rx_mpdu_desc_v1 v1;
        struct iwl_rx_mpdu_desc_v3 v3;
    };
} __packed; /* RX_MPDU_RES_START_API_S_VER_3 */

#define IWL_RX_DESC_SIZE_V1 offsetofend(struct iwl_rx_mpdu_desc, v1)

#define IWL_CD_STTS_OPTIMIZED_POS 0
#define IWL_CD_STTS_OPTIMIZED_MSK 0x01
#define IWL_CD_STTS_TRANSFER_STATUS_POS 1
#define IWL_CD_STTS_TRANSFER_STATUS_MSK 0x0E
#define IWL_CD_STTS_WIFI_STATUS_POS 4
#define IWL_CD_STTS_WIFI_STATUS_MSK 0xF0

#define RX_NO_DATA_CHAIN_A_POS 0
#define RX_NO_DATA_CHAIN_A_MSK (0xff << RX_NO_DATA_CHAIN_A_POS)
#define RX_NO_DATA_CHAIN_B_POS 8
#define RX_NO_DATA_CHAIN_B_MSK (0xff << RX_NO_DATA_CHAIN_B_POS)
#define RX_NO_DATA_CHANNEL_POS 16
#define RX_NO_DATA_CHANNEL_MSK (0xff << RX_NO_DATA_CHANNEL_POS)

#define RX_NO_DATA_INFO_TYPE_POS 0
#define RX_NO_DATA_INFO_TYPE_MSK (0xff << RX_NO_DATA_INFO_TYPE_POS)
#define RX_NO_DATA_INFO_TYPE_NONE 0
#define RX_NO_DATA_INFO_TYPE_RX_ERR 1
#define RX_NO_DATA_INFO_TYPE_NDP 2
#define RX_NO_DATA_INFO_TYPE_MU_UNMATCHED 3
#define RX_NO_DATA_INFO_TYPE_HE_TB_UNMATCHED 4

#define RX_NO_DATA_INFO_ERR_POS 8
#define RX_NO_DATA_INFO_ERR_MSK (0xff << RX_NO_DATA_INFO_ERR_POS)
#define RX_NO_DATA_INFO_ERR_NONE 0
#define RX_NO_DATA_INFO_ERR_BAD_PLCP 1
#define RX_NO_DATA_INFO_ERR_UNSUPPORTED_RATE 2
#define RX_NO_DATA_INFO_ERR_NO_DELIM 3
#define RX_NO_DATA_INFO_ERR_BAD_MAC_HDR 4

#define RX_NO_DATA_FRAME_TIME_POS 0
#define RX_NO_DATA_FRAME_TIME_MSK (0xfffff << RX_NO_DATA_FRAME_TIME_POS)

#define RX_NO_DATA_RX_VEC0_HE_NSTS_MSK 0x03800000
#define RX_NO_DATA_RX_VEC0_VHT_NSTS_MSK 0x38000000

/**
 * struct iwl_rx_no_data - RX no data descriptor
 * @info: 7:0 frame type, 15:8 RX error type
 * @rssi: 7:0 energy chain-A,
 *  15:8 chain-B, measured at FINA time (FINA_ENERGY), 16:23 channel
 * @on_air_rise_time: GP2 during on air rise
 * @fr_time: frame time
 * @rate: rate/mcs of frame
 * @phy_info: &enum iwl_rx_phy_data0 and &enum iwl_rx_phy_info_type
 * @rx_vec: DW-12:9 raw RX vectors from DSP according to modulation type.
 *  for VHT: OFDM_RX_VECTOR_SIGA1_OUT, OFDM_RX_VECTOR_SIGA2_OUT
 *  for HE: OFDM_RX_VECTOR_HE_SIGA1_OUT, OFDM_RX_VECTOR_HE_SIGA2_OUT
 */
struct iwl_rx_no_data {
    __le32 info;
    __le32 rssi;
    __le32 on_air_rise_time;
    __le32 fr_time;
    __le32 rate;
    __le32 phy_info[2];
    __le32 rx_vec[2];
} __packed; /* RX_NO_DATA_NTFY_API_S_VER_1 */

/**
 * enum iwl_completion_desc_transfer_status -  transfer status (bits 1-3)
 * @IWL_CD_STTS_UNUSED: unused
 * @IWL_CD_STTS_UNUSED_2: unused
 * @IWL_CD_STTS_END_TRANSFER: successful transfer complete.
 *  In sniffer mode, when split is used, set in last CD completion. (RX)
 * @IWL_CD_STTS_OVERFLOW: In sniffer mode, when using split - used for
 *  all CD completion. (RX)
 * @IWL_CD_STTS_ABORTED: CR abort / close flow. (RX)
 * @IWL_CD_STTS_ERROR: general error (RX)
 */
enum iwl_completion_desc_transfer_status {
    IWL_CD_STTS_UNUSED,
    IWL_CD_STTS_UNUSED_2,
    IWL_CD_STTS_END_TRANSFER,
    IWL_CD_STTS_OVERFLOW,
    IWL_CD_STTS_ABORTED,
    IWL_CD_STTS_ERROR,
};

/**
 * enum iwl_completion_desc_wifi_status - wifi status (bits 4-7)
 * @IWL_CD_STTS_VALID: the packet is valid (RX)
 * @IWL_CD_STTS_FCS_ERR: frame check sequence error (RX)
 * @IWL_CD_STTS_SEC_KEY_ERR: error handling the security key of rx (RX)
 * @IWL_CD_STTS_DECRYPTION_ERR: error decrypting the frame (RX)
 * @IWL_CD_STTS_DUP: duplicate packet (RX)
 * @IWL_CD_STTS_ICV_MIC_ERR: MIC error (RX)
 * @IWL_CD_STTS_INTERNAL_SNAP_ERR: problems removing the snap (RX)
 * @IWL_CD_STTS_SEC_PORT_FAIL: security port fail (RX)
 * @IWL_CD_STTS_BA_OLD_SN: block ack received old SN (RX)
 * @IWL_CD_STTS_QOS_NULL: QoS null packet (RX)
 * @IWL_CD_STTS_MAC_HDR_ERR: MAC header conversion error (RX)
 * @IWL_CD_STTS_MAX_RETRANS: reached max number of retransmissions (TX)
 * @IWL_CD_STTS_EX_LIFETIME: exceeded lifetime (TX)
 * @IWL_CD_STTS_NOT_USED: completed but not used (RX)
 * @IWL_CD_STTS_REPLAY_ERR: pn check failed, replay error (RX)
 */
enum iwl_completion_desc_wifi_status {
    IWL_CD_STTS_VALID,
    IWL_CD_STTS_FCS_ERR,
    IWL_CD_STTS_SEC_KEY_ERR,
    IWL_CD_STTS_DECRYPTION_ERR,
    IWL_CD_STTS_DUP,
    IWL_CD_STTS_ICV_MIC_ERR,
    IWL_CD_STTS_INTERNAL_SNAP_ERR,
    IWL_CD_STTS_SEC_PORT_FAIL,
    IWL_CD_STTS_BA_OLD_SN,
    IWL_CD_STTS_QOS_NULL,
    IWL_CD_STTS_MAC_HDR_ERR,
    IWL_CD_STTS_MAX_RETRANS,
    IWL_CD_STTS_EX_LIFETIME,
    IWL_CD_STTS_NOT_USED,
    IWL_CD_STTS_REPLAY_ERR,
};

struct iwl_frame_release {
    uint8_t baid;
    uint8_t reserved;
    __le16 nssn;
};

enum iwl_rss_hash_func_en {
    IWL_RSS_HASH_TYPE_IPV4_TCP,
    IWL_RSS_HASH_TYPE_IPV4_UDP,
    IWL_RSS_HASH_TYPE_IPV4_PAYLOAD,
    IWL_RSS_HASH_TYPE_IPV6_TCP,
    IWL_RSS_HASH_TYPE_IPV6_UDP,
    IWL_RSS_HASH_TYPE_IPV6_PAYLOAD,
};

#define IWL_RSS_HASH_KEY_CNT 10
#define IWL_RSS_INDIRECTION_TABLE_SIZE 128
#define IWL_RSS_ENABLE 1

/**
 * struct iwl_rss_config_cmd - RSS (Receive Side Scaling) configuration
 *
 * @flags: 1 - enable, 0 - disable
 * @hash_mask: Type of RSS to use. Values are from %iwl_rss_hash_func_en
 * @reserved: reserved
 * @secret_key: 320 bit input of random key configuration from driver
 * @indirection_table: indirection table
 */
struct iwl_rss_config_cmd {
    __le32 flags;
    uint8_t hash_mask;
    uint8_t reserved[3];
    __le32 secret_key[IWL_RSS_HASH_KEY_CNT];
    uint8_t indirection_table[IWL_RSS_INDIRECTION_TABLE_SIZE];
} __packed; /* RSS_CONFIG_CMD_API_S_VER_1 */

#define IWL_MULTI_QUEUE_SYNC_MSG_MAX_SIZE 128
#define IWL_MULTI_QUEUE_SYNC_SENDER_POS 0
#define IWL_MULTI_QUEUE_SYNC_SENDER_MSK 0xf

/**
 * struct iwl_rxq_sync_cmd - RXQ notification trigger
 *
 * @flags: flags of the notification. bit 0:3 are the sender queue
 * @rxq_mask: rx queues to send the notification on
 * @count: number of bytes in payload, should be DWORD aligned
 * @payload: data to send to rx queues
 */
struct iwl_rxq_sync_cmd {
    __le32 flags;
    __le32 rxq_mask;
    __le32 count;
    uint8_t payload[];
} __packed; /* MULTI_QUEUE_DRV_SYNC_HDR_CMD_API_S_VER_1 */

/**
 * struct iwl_rxq_sync_notification - Notification triggered by RXQ
 * sync command
 *
 * @count: number of bytes in payload
 * @payload: data to send to rx queues
 */
struct iwl_rxq_sync_notification {
    __le32 count;
    uint8_t payload[];
} __packed; /* MULTI_QUEUE_DRV_SYNC_HDR_CMD_API_S_VER_1 */

/**
 * enum iwl_mvm_rxq_notif_type - Internal message identifier
 *
 * @IWL_MVM_RXQ_EMPTY: empty sync notification
 * @IWL_MVM_RXQ_NOTIF_DEL_BA: notify RSS queues of delBA
 */
enum iwl_mvm_rxq_notif_type {
    IWL_MVM_RXQ_EMPTY,
    IWL_MVM_RXQ_NOTIF_DEL_BA,
};

/**
 * struct iwl_mvm_internal_rxq_notif - Internal representation of the data sent
 * in &iwl_rxq_sync_cmd. Should be DWORD aligned.
 * FW is agnostic to the payload, so there are no endianity requirements.
 *
 * @type: value from &iwl_mvm_rxq_notif_type
 * @sync: ctrl path is waiting for all notifications to be received
 * @cookie: internal cookie to identify old notifications
 * @data: payload
 */
struct iwl_mvm_internal_rxq_notif {
    uint16_t type;
    uint16_t sync;
    uint32_t cookie;
    uint8_t data[];
} __packed;

/**
 * enum iwl_mvm_pm_event - type of station PM event
 * @IWL_MVM_PM_EVENT_AWAKE: station woke up
 * @IWL_MVM_PM_EVENT_ASLEEP: station went to sleep
 * @IWL_MVM_PM_EVENT_UAPSD: station sent uAPSD trigger
 * @IWL_MVM_PM_EVENT_PS_POLL: station sent PS-Poll
 */
enum iwl_mvm_pm_event {
    IWL_MVM_PM_EVENT_AWAKE,
    IWL_MVM_PM_EVENT_ASLEEP,
    IWL_MVM_PM_EVENT_UAPSD,
    IWL_MVM_PM_EVENT_PS_POLL,
}; /* PEER_PM_NTFY_API_E_VER_1 */

/**
 * struct iwl_mvm_pm_state_notification - station PM state notification
 * @sta_id: station ID of the station changing state
 * @type: the new powersave state, see &enum iwl_mvm_pm_event
 */
struct iwl_mvm_pm_state_notification {
    uint8_t sta_id;
    uint8_t type;
    /* private: */
    __le16 reserved;
} __packed; /* PEER_PM_NTFY_API_S_VER_1 */

#define BA_WINDOW_STREAMS_MAX 16
#define BA_WINDOW_STATUS_TID_MSK 0x000F
#define BA_WINDOW_STATUS_STA_ID_POS 4
#define BA_WINDOW_STATUS_STA_ID_MSK 0x01F0
#define BA_WINDOW_STATUS_VALID_MSK BIT(9)

/**
 * struct iwl_ba_window_status_notif - reordering window's status notification
 * @bitmap: bitmap of received frames [start_seq_num + 0]..[start_seq_num + 63]
 * @ra_tid: bit 3:0 - TID, bit 8:4 - STA_ID, bit 9 - valid
 * @start_seq_num: the start sequence number of the bitmap
 * @mpdu_rx_count: the number of received MPDUs since entering D0i3
 */
struct iwl_ba_window_status_notif {
    __le64 bitmap[BA_WINDOW_STREAMS_MAX];
    __le16 ra_tid[BA_WINDOW_STREAMS_MAX];
    __le32 start_seq_num[BA_WINDOW_STREAMS_MAX];
    __le16 mpdu_rx_count[BA_WINDOW_STREAMS_MAX];
} __packed; /* BA_WINDOW_STATUS_NTFY_API_S_VER_1 */

/**
 * struct iwl_rfh_queue_config - RX queue configuration
 * @q_num: Q num
 * @enable: enable queue
 * @reserved: alignment
 * @urbd_stts_wrptr: DMA address of urbd_stts_wrptr
 * @fr_bd_cb: DMA address of freeRB table
 * @ur_bd_cb: DMA address of used RB table
 * @fr_bd_wid: Initial index of the free table
 */
struct iwl_rfh_queue_data {
    uint8_t q_num;
    uint8_t enable;
    __le16 reserved;
    __le64 urbd_stts_wrptr;
    __le64 fr_bd_cb;
    __le64 ur_bd_cb;
    __le32 fr_bd_wid;
} __packed; /* RFH_QUEUE_CONFIG_S_VER_1 */

/**
 * struct iwl_rfh_queue_config - RX queue configuration
 * @num_queues: number of queues configured
 * @reserved: alignment
 * @data: DMA addresses per-queue
 */
struct iwl_rfh_queue_config {
    uint8_t num_queues;
    uint8_t reserved[3];
    struct iwl_rfh_queue_data data[];
} __packed; /* RFH_QUEUE_CONFIG_API_S_VER_1 */

#endif  // SRC_CONNECTIVITY_WLAN_DRIVERS_THIRD_PARTY_INTEL_IWLWIFI_FW_API_RX_H_
