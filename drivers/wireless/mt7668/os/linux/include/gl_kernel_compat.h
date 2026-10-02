/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Kernel API compatibility for the MT7668 driver.
 *
 * The driver sources follow the current kernel API (6.18). Older kernels,
 * including Android common kernels that carry only part of the newer
 * cfg80211, are handled here:
 *  - plain shims for renamed helpers;
 *  - MTK_CFG80211_* probes, set by the Makefile from this kernel's
 *    include/net/cfg80211.h (0 = old signature);
 *  - MTK_CFG80211_OP(): picks the driver's own cfg80211 callback when
 *    the signatures match, or a wrapper from gl_kernel_compat.c that
 *    adapts the old signature to it.
 */
#ifndef _GL_KERNEL_COMPAT_H
#define _GL_KERNEL_COMPAT_H

#include <linux/version.h>
#include <linux/timer.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>

/* timer_delete_sync() appeared in 6.2 (del_timer_sync() until then). */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 2, 0)
#define timer_delete_sync(timer) del_timer_sync(timer)
#endif

/*
 * Until 5.18 netif_rx() only queues the skb; process context had to use
 * netif_rx_ni() to run the pending softirq. Since then netif_rx() works
 * in any context.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 18, 0)
#define kal_netif_rx_any(skb) (in_interrupt() ? netif_rx(skb) : netif_rx_ni(skb))
#else
#define kal_netif_rx_any(skb) netif_rx(skb)
#endif

/* Symbol namespaces are string literals since 6.13. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
#define KAL_MODULE_IMPORT_NS(ns) MODULE_IMPORT_NS(#ns)
#else
#define KAL_MODULE_IMPORT_NS(ns) MODULE_IMPORT_NS(ns)
#endif

/* Probes from the Makefile; default to the current API. */
#ifndef MTK_CFG80211_KEY_OPS_WDEV
#define MTK_CFG80211_KEY_OPS_WDEV 1
#endif
#ifndef MTK_CFG80211_STA_OPS_WDEV
#define MTK_CFG80211_STA_OPS_WDEV 1
#endif
#ifndef MTK_CFG80211_ROC_RX_ADDR
#define MTK_CFG80211_ROC_RX_ADDR 1
#endif
#ifndef MTK_CFG80211_TDLS_MGMT_LINK_ID
#define MTK_CFG80211_TDLS_MGMT_LINK_ID 1
#endif
#ifndef MTK_CFG80211_AP_UPDATE
#define MTK_CFG80211_AP_UPDATE 1
#endif
#ifndef MTK_CFG80211_WIPHY_PARAMS_RADIO_IDX
#define MTK_CFG80211_WIPHY_PARAMS_RADIO_IDX 1
#endif
#ifndef MTK_CFG80211_SET_TX_POWER_RADIO_IDX
#define MTK_CFG80211_SET_TX_POWER_RADIO_IDX 1
#endif
#ifndef MTK_CFG80211_GET_TX_POWER_RADIO_IDX
#define MTK_CFG80211_GET_TX_POWER_RADIO_IDX 1
#endif
#ifndef MTK_CFG80211_GET_TX_POWER_LINK_ID
#define MTK_CFG80211_GET_TX_POWER_LINK_ID 1
#endif
#ifndef MTK_CFG80211_RADAR_LINK_ID
#define MTK_CFG80211_RADAR_LINK_ID 1
#endif
#ifndef MTK_CFG80211_STA_EVENT_WDEV
#define MTK_CFG80211_STA_EVENT_WDEV 1
#endif
#ifndef MTK_CFG80211_CAC_EVENT_LINK_ID
#define MTK_CFG80211_CAC_EVENT_LINK_ID 1
#endif
#ifndef MTK_CFG80211_CH_SWITCH_LINK_ID
#define MTK_CFG80211_CH_SWITCH_LINK_ID 1
#endif
#ifndef MTK_CFG80211_CH_SWITCH_PUNCT
#define MTK_CFG80211_CH_SWITCH_PUNCT 0
#endif

/*
 * MTK_CFG80211_OP(cond, fn): fn when the kernel has the current signature
 * (cond = 1), else the wrapper fn##_compat. cond must be a probe macro
 * (a literal 0 or 1). get_tx_power is current once it has radio_idx,
 * which came after link_id.
 */
#define __MTK_CFG80211_OP0(fn) fn##_compat
#define __MTK_CFG80211_OP1(fn) fn
#define __MTK_CFG80211_OP(cond, fn) __MTK_CFG80211_OP##cond(fn)
#define MTK_CFG80211_OP(cond, fn) __MTK_CFG80211_OP(cond, fn)

/* Event helpers whose arguments changed. */
#if MTK_CFG80211_STA_EVENT_WDEV
#define kal_cfg80211_new_sta(ndev, mac, sinfo, gfp) \
	cfg80211_new_sta((ndev)->ieee80211_ptr, mac, sinfo, gfp)
#define kal_cfg80211_del_sta(ndev, mac, gfp) \
	cfg80211_del_sta((ndev)->ieee80211_ptr, mac, gfp)
#else
#define kal_cfg80211_new_sta(ndev, mac, sinfo, gfp) \
	cfg80211_new_sta(ndev, mac, sinfo, gfp)
#define kal_cfg80211_del_sta(ndev, mac, gfp) \
	cfg80211_del_sta(ndev, mac, gfp)
#endif

#if MTK_CFG80211_CAC_EVENT_LINK_ID
#define kal_cfg80211_cac_event(ndev, chandef, event, gfp) \
	cfg80211_cac_event(ndev, chandef, event, gfp, 0)
#else
#define kal_cfg80211_cac_event(ndev, chandef, event, gfp) \
	cfg80211_cac_event(ndev, chandef, event, gfp)
#endif

#if MTK_CFG80211_CH_SWITCH_PUNCT
#define kal_cfg80211_ch_switch_notify(ndev, chandef) \
	cfg80211_ch_switch_notify(ndev, chandef, 0, 0)
#elif MTK_CFG80211_CH_SWITCH_LINK_ID
#define kal_cfg80211_ch_switch_notify(ndev, chandef) \
	cfg80211_ch_switch_notify(ndev, chandef, 0)
#else
#define kal_cfg80211_ch_switch_notify(ndev, chandef) \
	cfg80211_ch_switch_notify(ndev, chandef)
#endif

/* Wrappers (gl_kernel_compat.c), declared only where they are built. */
#if !MTK_CFG80211_KEY_OPS_WDEV
int mtk_cfg80211_add_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr,
				struct key_params *params);
int mtk_cfg80211_get_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr, void *cookie,
				void (*callback)(void *cookie, struct key_params *));
int mtk_cfg80211_del_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr);
int mtk_p2p_cfg80211_add_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr,
				    struct key_params *params);
int mtk_p2p_cfg80211_get_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr, void *cookie,
				    void (*callback)(void *cookie, struct key_params *));
int mtk_p2p_cfg80211_del_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr);
int mtk_p2p_cfg80211_set_mgmt_key_compat(struct wiphy *wiphy, struct net_device *ndev,
					 int link_id, u8 key_index);
#endif

#if !MTK_CFG80211_STA_OPS_WDEV
int mtk_cfg80211_get_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    const u8 *mac, struct station_info *sinfo);
int mtk_cfg80211_change_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				       const u8 *mac, struct station_parameters *params);
int mtk_cfg80211_add_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    const u8 *mac, struct station_parameters *params);
int mtk_cfg80211_del_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    struct station_del_parameters *params);
int mtk_p2p_cfg80211_get_station_compat(struct wiphy *wiphy, struct net_device *ndev,
					const u8 *mac, struct station_info *sinfo);
int mtk_p2p_cfg80211_del_station_compat(struct wiphy *wiphy, struct net_device *ndev,
					struct station_del_parameters *params);
#endif

#if !MTK_CFG80211_ROC_RX_ADDR
int mtk_cfg80211_remain_on_channel_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					  struct ieee80211_channel *chan, unsigned int duration,
					  u64 *cookie);
int mtk_p2p_cfg80211_remain_on_channel_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					      struct ieee80211_channel *chan,
					      unsigned int duration, u64 *cookie);
#endif

#if !MTK_CFG80211_TDLS_MGMT_LINK_ID
int mtk_cfg80211_tdls_mgmt_compat(struct wiphy *wiphy, struct net_device *dev, const u8 *peer,
				  u8 action_code, u8 dialog_token, u16 status_code,
				  u32 peer_capability, bool initiator, const u8 *buf, size_t len);
#endif

#if !MTK_CFG80211_WIPHY_PARAMS_RADIO_IDX
int mtk_p2p_cfg80211_set_wiphy_params_compat(struct wiphy *wiphy, u32 changed);
#endif

#if !MTK_CFG80211_SET_TX_POWER_RADIO_IDX
int mtk_p2p_cfg80211_set_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					enum nl80211_tx_power_setting type, int mbm);
#endif

#if !MTK_CFG80211_GET_TX_POWER_RADIO_IDX
#if MTK_CFG80211_GET_TX_POWER_LINK_ID
int mtk_p2p_cfg80211_get_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					unsigned int link_id, int *dbm);
#else
int mtk_p2p_cfg80211_get_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					int *dbm);
#endif
#endif

#if !MTK_CFG80211_RADAR_LINK_ID
int mtk_p2p_cfg80211_start_radar_detection_compat(struct wiphy *wiphy, struct net_device *dev,
						  struct cfg80211_chan_def *chandef,
						  u32 cac_time_ms);
#endif

#endif /* _GL_KERNEL_COMPAT_H */
