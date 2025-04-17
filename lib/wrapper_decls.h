#pragma once

#include <doca_flow.h>

#ifndef COUNTER_SPY_DECL_NO_EXTERN // only defined in one source file!
#define COUNTER_SPY_EXTERN extern
#else
#define COUNTER_SPY_EXTERN
#endif

// Note this macro only works for certain C compilers; not for C++
#define DECL_PFUNC(func) COUNTER_SPY_EXTERN typeof(func) * p_ ## func

// Functions to wrap:
DECL_PFUNC(doca_flow_init);
DECL_PFUNC(doca_flow_destroy);
DECL_PFUNC(doca_flow_port_start);
DECL_PFUNC(doca_flow_port_stop);
DECL_PFUNC(doca_flow_port_pair);
DECL_PFUNC(doca_flow_port_priv_data);
DECL_PFUNC(doca_flow_port_operation_state_modify);
DECL_PFUNC(doca_flow_shared_resource_set_cfg);
DECL_PFUNC(doca_flow_shared_resources_bind);
DECL_PFUNC(doca_flow_shared_resources_query);
DECL_PFUNC(doca_flow_pipe_create);
DECL_PFUNC(doca_flow_pipe_resize);
DECL_PFUNC(doca_flow_pipe_add_entry);
DECL_PFUNC(doca_flow_pipe_update_entry);
DECL_PFUNC(doca_flow_pipe_control_add_entry);
DECL_PFUNC(doca_flow_pipe_lpm_add_entry);
DECL_PFUNC(doca_flow_pipe_lpm_update_entry);
DECL_PFUNC(doca_flow_pipe_ordered_list_add_entry);
DECL_PFUNC(doca_flow_pipe_acl_add_entry);
DECL_PFUNC(doca_flow_pipe_hash_add_entry);
DECL_PFUNC(doca_flow_pipe_remove_entry);
DECL_PFUNC(doca_flow_pipe_calc_hash);
DECL_PFUNC(doca_flow_pipe_destroy);
DECL_PFUNC(doca_flow_port_pipes_flush);
DECL_PFUNC(doca_flow_port_pipes_dump);
DECL_PFUNC(doca_flow_pipe_dump);
DECL_PFUNC(doca_flow_resource_query_entry);
DECL_PFUNC(doca_flow_resource_query_pipe_miss);
DECL_PFUNC(doca_flow_pipe_update_miss);
DECL_PFUNC(doca_flow_aging_handle);
DECL_PFUNC(doca_flow_entries_process);
DECL_PFUNC(doca_flow_pipe_entry_get_status);
DECL_PFUNC(doca_flow_port_switch_get);
DECL_PFUNC(doca_flow_mpls_label_encode);
DECL_PFUNC(doca_flow_mpls_label_decode);
DECL_PFUNC(doca_flow_parser_geneve_opt_create);
DECL_PFUNC(doca_flow_parser_geneve_opt_destroy);
DECL_PFUNC(doca_flow_get_target);
DECL_PFUNC(doca_flow_cfg_create);
DECL_PFUNC(doca_flow_cfg_destroy);
DECL_PFUNC(doca_flow_cfg_set_pipe_queues);
DECL_PFUNC(doca_flow_cfg_set_nr_counters);
DECL_PFUNC(doca_flow_cfg_set_nr_meters);
DECL_PFUNC(doca_flow_cfg_set_nr_acl_collisions);
DECL_PFUNC(doca_flow_cfg_set_mode_args);
DECL_PFUNC(doca_flow_cfg_set_nr_shared_resource);
DECL_PFUNC(doca_flow_cfg_set_queue_depth);
DECL_PFUNC(doca_flow_cfg_set_cb_pipe_process);
DECL_PFUNC(doca_flow_cfg_set_cb_entry_process);
DECL_PFUNC(doca_flow_cfg_set_cb_shared_resource_unbind);
DECL_PFUNC(doca_flow_cfg_set_rss_key);
DECL_PFUNC(doca_flow_cfg_set_default_rss);
DECL_PFUNC(doca_flow_cfg_set_definitions);
DECL_PFUNC(doca_flow_port_cfg_create);
DECL_PFUNC(doca_flow_port_cfg_destroy);
DECL_PFUNC(doca_flow_port_cfg_set_devargs);
DECL_PFUNC(doca_flow_port_cfg_set_priv_data_size);
DECL_PFUNC(doca_flow_port_cfg_set_dev);
DECL_PFUNC(doca_flow_port_cfg_set_rss_cfg);
DECL_PFUNC(doca_flow_port_cfg_set_ipsec_sn_offload_disable);
DECL_PFUNC(doca_flow_port_cfg_set_operation_state);
DECL_PFUNC(doca_flow_port_cfg_set_actions_mem_size);
DECL_PFUNC(doca_flow_port_cfg_set_service_threads_core);
DECL_PFUNC(doca_flow_port_cfg_set_service_threads_cycle);
DECL_PFUNC(doca_flow_pipe_cfg_create);
DECL_PFUNC(doca_flow_pipe_cfg_destroy);
DECL_PFUNC(doca_flow_pipe_cfg_set_match);
DECL_PFUNC(doca_flow_pipe_cfg_set_actions);
DECL_PFUNC(doca_flow_pipe_cfg_set_monitor);
DECL_PFUNC(doca_flow_pipe_cfg_set_ordered_lists);
DECL_PFUNC(doca_flow_pipe_cfg_set_name);
DECL_PFUNC(doca_flow_pipe_cfg_set_type);
DECL_PFUNC(doca_flow_pipe_cfg_set_domain);
DECL_PFUNC(doca_flow_pipe_cfg_set_is_root);
DECL_PFUNC(doca_flow_pipe_cfg_set_nr_entries);
DECL_PFUNC(doca_flow_pipe_cfg_set_is_resizable);
// DECL_PFUNC(doca_flow_pipe_cfg_set_enable_strict_matching);
DECL_PFUNC(doca_flow_pipe_cfg_set_dir_info);
DECL_PFUNC(doca_flow_pipe_cfg_set_miss_counter);
DECL_PFUNC(doca_flow_pipe_cfg_set_congestion_level_threshold);
DECL_PFUNC(doca_flow_pipe_cfg_set_user_ctx);
DECL_PFUNC(doca_flow_pipe_cfg_set_hash_map_algorithm);


