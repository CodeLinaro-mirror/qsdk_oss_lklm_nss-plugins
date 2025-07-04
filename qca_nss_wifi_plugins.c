/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

/*
 * nss_plugins.c
 *	NSS Plugin
 */
#include <linux/of.h>
#include <linux/module.h>
#include <linux/list.h>
#include <linux/netdevice.h>

#include <ppe_drv_public.h>
#include <ppe_vp_public.h>
#include <ppe_drv_iface.h>
#include "qca_nss_wifi_plugins.h"

/*
 * qca_nss_wifi_plugin_ds_gbl_ctx
 * 	global context
 */
struct qca_nss_wifi_plugin_ds_gbl gbl_plugin_ds = {0};

/*
 * qca_nss_wifi_plugin_ds_alloc
 * 	plugin_ds_obj alloc
 */
struct qca_nss_wifi_plugin_ds_node *qca_nss_wifi_plugin_ds_alloc(void)
{
	return kzalloc(sizeof(struct qca_nss_wifi_plugin_ds_node), GFP_ATOMIC);
}

/*
 * qca_nss_wifi_plugin_ds_free
 * 	plugin_ds_obj free
 */
void qca_nss_wifi_plugin_ds_free(struct qca_nss_wifi_plugin_ds_node *node_obj)
{
	kfree(node_obj);
}

/*
 * qca_nss_wifi_plugin_get_ds_node
 * 	get ds node
 */
struct ppe_ds *qca_nss_wifi_plugin_get_ds_node(int ppeds_node_id)
{
	struct qca_nss_wifi_plugin_ds_gbl *g_plugin_ds = &gbl_plugin_ds;
	struct qca_nss_wifi_plugin_ds_node *plugin_ds_obj;
	struct ppe_ds * ppe_ds_node = NULL;

	spin_lock_bh(&g_plugin_ds->lock);
	if (!list_empty(&g_plugin_ds->active_nodes)) {
		list_for_each_entry(plugin_ds_obj, &g_plugin_ds->active_nodes, list) {
			int node_id = plugin_ds_obj->ppeds_node_id;
			if (node_id == ppeds_node_id) {
				ppe_ds_node = plugin_ds_obj->node;
				break;
			}
		}
	}

	spin_unlock_bh(&g_plugin_ds->lock);
	return ppe_ds_node;
}

/*
 * qca_nss_wifi_plugin_ds_wlan_inst_alloc
 * 	inst alloc
 */
int qca_nss_wifi_plugin_ds_wlan_inst_alloc(struct ppe_ds_wlan_ops *ops, size_t priv_size)
{
	struct qca_nss_wifi_plugin_ds_gbl *plugin_ds_gbl = &gbl_plugin_ds;
	struct qca_nss_wifi_plugin_ds_node *plugin_ds_obj;
	struct ppe_ds *node;
	uint32_t node_id;

	plugin_ds_obj = qca_nss_wifi_plugin_ds_alloc();
	if (!plugin_ds_obj) {
		pr_err("Failed to allocate memory\n");
		return -1;
	}

	spin_lock_init(&plugin_ds_gbl->lock);
	node = ppe_ds_wlan_inst_alloc(ops, priv_size);
	if (!node) {
		qca_nss_wifi_plugin_ds_free(plugin_ds_obj);
		return -1;
	}

	node_id = ppe_ds_get_node_id(node);
	plugin_ds_obj->ppeds_node_id = node_id;
	plugin_ds_obj->node = node;
	list_add(&plugin_ds_obj->list, &plugin_ds_gbl->active_nodes);

	return node_id;
}

/*
 * qca_nss_wifi_plugin_ds_wlan_inst_start
 * 	inst start
 */
int qca_nss_wifi_plugin_ds_wlan_inst_start(struct ppe_ds_wlan_ctx_info_handle *wlan_info_hd, int ppeds_node_id)
{
	struct ppe_ds *ppeds_node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_wlan_instance_start(ppeds_node, wlan_info_hd);
}

/*
 * qca_nss_wifi_plugin_ds_wlan_inst_register
 * 	instance register
 */
bool qca_nss_wifi_plugin_ds_wlan_inst_register(struct ppe_ds_wlan_reg_info *reg_info, int ppeds_node_id)
{
	struct ppe_ds *ppeds_node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_wlan_inst_register(ppeds_node, reg_info);
}

/*
 * qca_nss_wifi_plugin_ds_wlan_instance_stop
 * 	instance stop
 */
void qca_nss_wifi_plugin_ds_wlan_instance_stop(struct ppe_ds_wlan_ctx_info_handle *wlan_info_hdl, int ppeds_node_id)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_wlan_instance_stop(node, wlan_info_hdl);
}

/*
 * qca_nss_wifi_plugin_ds_wlan_inst_free
 * 	instance free
 */
void qca_nss_wifi_plugin_ds_wlan_inst_free(int ppeds_node_id)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_wlan_inst_free(node);
}

/*
 * qca_nss_wifi_plugin_ds_wlan_get_intr_ctxt
 * 	get interrupt context
 */
void *qca_nss_wifi_plugin_ds_wlan_get_intr_ctxt(int ppeds_node_id)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_wlan_get_intr_ctxt(node);
}

/*
 * qca_nss_wifi_plugin_ds_ppe2tcl_wlan_handle_intr
 * 	ppe2tcl intr handle
 */
int qca_nss_wifi_plugin_ds_ppe2tcl_wlan_handle_intr(int ppeds_node_id)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_ppe2tcl_wlan_handle_intr(node);
}

/*
 * qca_nss_wifi_plugin_ds_reo2ppe_wlan_handle_intr
 * 	reo2ppe intr handle
 */
int qca_nss_wifi_plugin_ds_reo2ppe_wlan_handle_intr(int ppeds_node_id)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	return ppe_ds_reo2ppe_wlan_handle_intr(node);
}

/*
 * qca_nss_wifi_plugin_ds_wlan_service_status_update
 *	service status update
 */
void qca_nss_wifi_plugin_ds_wlan_service_status_update(int ppeds_node_id, bool enable)
{
	struct ppe_ds *node = qca_nss_wifi_plugin_get_ds_node(ppeds_node_id);
	ppe_ds_wlan_service_status_update(node, enable);
}

/*
 * qca_nss_wifi_plugin_vp_cfg_update
 * 	vp cfg update
 */
ppe_vp_status_t qca_nss_wifi_plugin_vp_cfg_update(struct net_device *ath_dev, struct ppe_vp_ui *vpui)
{
	int16_t vp_num = ppe_drv_port_num_from_dev(ath_dev);

	if (unlikely((vp_num < PPE_DRV_VIRTUAL_START) || (vp_num >= PPE_DRV_VIRTUAL_END))) {
		pr_err("\nNot a valid Virtual Port number %d dev %s\n", vp_num, ath_dev->name);
		return PPE_VP_STATUS_PORT_INVALID;
	}

	/*
	 * Passive VP needs to be pushed through RFS
	 */
	if (unlikely(vpui->usr_type == PPE_VP_USER_TYPE_PASSIVE)) {
		pr_err("Dev: %s Passive VP: %d needs to be removed\n", ath_dev->name, vp_num);

		/*
		 * release the reference taken during register
		 */
		dev_put(ath_dev);
		return ppe_vp_free(vp_num);
	}

	if (ppe_vp_update_vp_stats_cb(vp_num, vpui->stats_cb) == PPE_VP_STATUS_SUCCESS) {
		pr_err("VP at port num : %u, updated with stats_cb", vp_num);
	}

	return ppe_vp_cfg_update(vp_num, vpui);
}

/*
 * qca_nss_wifi_plugin_get_vp_num
 * 	Return the VP num for the dev
 */
int16_t qca_nss_wifi_plugin_get_vp_num(struct net_device *dev)
{
	int16_t vp_num = ppe_drv_port_num_from_dev(dev);

	if (unlikely((vp_num < PPE_DRV_VIRTUAL_START) || (vp_num >= PPE_DRV_VIRTUAL_END))) {
		pr_err("\nCouldn't find a valid Virtual Port number for dev %s\n", dev->name);
		return -1;
	}

	return vp_num;
}

/*
 * qca_nss_wifi_plugins_ops
 */
struct nss_plugins_ops plugins_ops = {
	.ds_inst_alloc = qca_nss_wifi_plugin_ds_wlan_inst_alloc,
	.ds_inst_start = qca_nss_wifi_plugin_ds_wlan_inst_start,
	.ds_inst_register = qca_nss_wifi_plugin_ds_wlan_inst_register,
	.ds_inst_stop = qca_nss_wifi_plugin_ds_wlan_instance_stop,
	.ds_inst_free = qca_nss_wifi_plugin_ds_wlan_inst_free,
	.ds_inst_get_ctx = qca_nss_wifi_plugin_ds_wlan_get_intr_ctxt,
	.ds_inst_ppe2tcl_intr = qca_nss_wifi_plugin_ds_ppe2tcl_wlan_handle_intr,
	.ds_inst_reo2ppe_intr = qca_nss_wifi_plugin_ds_reo2ppe_wlan_handle_intr,
	.service_status_update = qca_nss_wifi_plugin_ds_wlan_service_status_update,
	.vp_cfg_update = qca_nss_wifi_plugin_vp_cfg_update,
	.get_vp_num = qca_nss_wifi_plugin_get_vp_num,
};

/*
 * nss_plugins_get_ops
 * 	get ops
 */
struct nss_plugins_ops * qca_nss_wifi_plugins_get_ops(void)
{
	return &plugins_ops;
}
EXPORT_SYMBOL(qca_nss_wifi_plugins_get_ops);

/*
 * nss_plugins_exit_module()
 *	nss_plugins module exit function
 */
static void __exit qca_nss_wifi_plugins_exit_module(void)
{
	nss_plugins_info("NSS Plugins Module unloaded\n");
}

/*
 * nss_plugins_init_module()
 *	nss_plugings module init function
 */
static int __init qca_nss_wifi_plugins_init_module(void)
{
	struct qca_nss_wifi_plugin_ds_gbl *plugin_ds_gbl = &gbl_plugin_ds;
	INIT_LIST_HEAD(&plugin_ds_gbl->active_nodes);
	nss_plugins_info("NSS Plugins Module loaded\n");
	return 0;
}

module_init(qca_nss_wifi_plugins_init_module);
module_exit(qca_nss_wifi_plugins_exit_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS Plugins");
