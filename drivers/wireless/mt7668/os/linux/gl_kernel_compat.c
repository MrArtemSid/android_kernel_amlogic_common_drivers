// SPDX-License-Identifier: GPL-2.0
/*
 * cfg80211 callback wrappers for kernels with older signatures than the
 * driver uses; see os/linux/include/gl_kernel_compat.h. Each wrapper only
 * converts the arguments and calls the driver's own callback.
 */

#include "precomp.h"
#include "gl_cfg80211.h"
#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
#include "gl_p2p_ioctl.h"
#endif

#if !MTK_CFG80211_KEY_OPS_WDEV
int mtk_cfg80211_add_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr,
				struct key_params *params)
{
	return mtk_cfg80211_add_key(wiphy, ndev->ieee80211_ptr, link_id, key_index, pairwise,
				    mac_addr, params);
}

int mtk_cfg80211_get_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr, void *cookie,
				void (*callback)(void *cookie, struct key_params *))
{
	return mtk_cfg80211_get_key(wiphy, ndev->ieee80211_ptr, link_id, key_index, pairwise,
				    mac_addr, cookie, callback);
}

int mtk_cfg80211_del_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				u8 key_index, bool pairwise, const u8 *mac_addr)
{
	return mtk_cfg80211_del_key(wiphy, ndev->ieee80211_ptr, link_id, key_index, pairwise,
				    mac_addr);
}

#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
int mtk_p2p_cfg80211_add_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr,
				    struct key_params *params)
{
	return mtk_p2p_cfg80211_add_key(wiphy, ndev->ieee80211_ptr, link_id, key_index,
					pairwise, mac_addr, params);
}

int mtk_p2p_cfg80211_get_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr, void *cookie,
				    void (*callback)(void *cookie, struct key_params *))
{
	return mtk_p2p_cfg80211_get_key(wiphy, ndev->ieee80211_ptr, link_id, key_index,
					pairwise, mac_addr, cookie, callback);
}

int mtk_p2p_cfg80211_del_key_compat(struct wiphy *wiphy, struct net_device *ndev, int link_id,
				    u8 key_index, bool pairwise, const u8 *mac_addr)
{
	return mtk_p2p_cfg80211_del_key(wiphy, ndev->ieee80211_ptr, link_id, key_index,
					pairwise, mac_addr);
}

int mtk_p2p_cfg80211_set_mgmt_key_compat(struct wiphy *wiphy, struct net_device *ndev,
					 int link_id, u8 key_index)
{
	return mtk_p2p_cfg80211_set_mgmt_key(wiphy, ndev->ieee80211_ptr, link_id, key_index);
}
#endif
#endif /* !MTK_CFG80211_KEY_OPS_WDEV */

#if !MTK_CFG80211_STA_OPS_WDEV
int mtk_cfg80211_get_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    const u8 *mac, struct station_info *sinfo)
{
	return mtk_cfg80211_get_station(wiphy, ndev->ieee80211_ptr, mac, sinfo);
}

#if CFG_SUPPORT_TDLS
int mtk_cfg80211_change_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				       const u8 *mac, struct station_parameters *params)
{
	return mtk_cfg80211_change_station(wiphy, ndev->ieee80211_ptr, mac, params);
}

int mtk_cfg80211_add_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    const u8 *mac, struct station_parameters *params)
{
	return mtk_cfg80211_add_station(wiphy, ndev->ieee80211_ptr, mac, params);
}

int mtk_cfg80211_del_station_compat(struct wiphy *wiphy, struct net_device *ndev,
				    struct station_del_parameters *params)
{
	return mtk_cfg80211_del_station(wiphy, ndev->ieee80211_ptr, params);
}
#endif

#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
int mtk_p2p_cfg80211_get_station_compat(struct wiphy *wiphy, struct net_device *ndev,
					const u8 *mac, struct station_info *sinfo)
{
	return mtk_p2p_cfg80211_get_station(wiphy, ndev->ieee80211_ptr, mac, sinfo);
}

int mtk_p2p_cfg80211_del_station_compat(struct wiphy *wiphy, struct net_device *ndev,
					struct station_del_parameters *params)
{
	return mtk_p2p_cfg80211_del_station(wiphy, ndev->ieee80211_ptr, params);
}
#endif
#endif /* !MTK_CFG80211_STA_OPS_WDEV */

#if !MTK_CFG80211_ROC_RX_ADDR
int mtk_cfg80211_remain_on_channel_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					  struct ieee80211_channel *chan, unsigned int duration,
					  u64 *cookie)
{
	return mtk_cfg80211_remain_on_channel(wiphy, wdev, chan, duration, cookie, NULL);
}

#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
int mtk_p2p_cfg80211_remain_on_channel_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					      struct ieee80211_channel *chan,
					      unsigned int duration, u64 *cookie)
{
	return mtk_p2p_cfg80211_remain_on_channel(wiphy, wdev, chan, duration, cookie, NULL);
}
#endif
#endif /* !MTK_CFG80211_ROC_RX_ADDR */

#if !MTK_CFG80211_TDLS_MGMT_LINK_ID && CFG_SUPPORT_TDLS
int mtk_cfg80211_tdls_mgmt_compat(struct wiphy *wiphy, struct net_device *dev, const u8 *peer,
				  u8 action_code, u8 dialog_token, u16 status_code,
				  u32 peer_capability, bool initiator, const u8 *buf, size_t len)
{
	return mtk_cfg80211_tdls_mgmt(wiphy, dev, peer, -1, action_code, dialog_token,
				      status_code, peer_capability, initiator, buf, len);
}
#endif

#if CFG_ENABLE_WIFI_DIRECT_CFG_80211
#if !MTK_CFG80211_WIPHY_PARAMS_RADIO_IDX
int mtk_p2p_cfg80211_set_wiphy_params_compat(struct wiphy *wiphy, u32 changed)
{
	return mtk_p2p_cfg80211_set_wiphy_params(wiphy, -1, changed);
}
#endif

#if !MTK_CFG80211_SET_TX_POWER_RADIO_IDX
int mtk_p2p_cfg80211_set_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					enum nl80211_tx_power_setting type, int mbm)
{
	return mtk_p2p_cfg80211_set_txpower(wiphy, wdev, -1, type, mbm);
}
#endif

#if !MTK_CFG80211_GET_TX_POWER_RADIO_IDX
#if MTK_CFG80211_GET_TX_POWER_LINK_ID
int mtk_p2p_cfg80211_get_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					unsigned int link_id, int *dbm)
{
	return mtk_p2p_cfg80211_get_txpower(wiphy, wdev, -1, link_id, dbm);
}
#else
int mtk_p2p_cfg80211_get_txpower_compat(struct wiphy *wiphy, struct wireless_dev *wdev,
					int *dbm)
{
	return mtk_p2p_cfg80211_get_txpower(wiphy, wdev, -1, 0, dbm);
}
#endif
#endif

#if !MTK_CFG80211_RADAR_LINK_ID && (CFG_SUPPORT_DFS_MASTER == 1)
int mtk_p2p_cfg80211_start_radar_detection_compat(struct wiphy *wiphy, struct net_device *dev,
						  struct cfg80211_chan_def *chandef,
						  u32 cac_time_ms)
{
	return mtk_p2p_cfg80211_start_radar_detection(wiphy, dev, chandef, cac_time_ms, 0);
}
#endif
#endif /* CFG_ENABLE_WIFI_DIRECT_CFG_80211 */
