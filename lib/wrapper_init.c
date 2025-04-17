/*
 * Copyright (c) 2023 NVIDIA CORPORATION & AFFILIATES, ALL RIGHTS RESERVED.
 *
 * This software product is a proprietary product of NVIDIA CORPORATION &
 * AFFILIATES (the "Company") and all right, title, and interest in and to the
 * software product, including all associated intellectual property rights, are
 * and shall remain exclusively with the Company.
 *
 * This software product is governed by the End User License Agreement
 * provided with the software product.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>

#define COUNTER_SPY_DECL_NO_EXTERN
#include <wrapper_decls.h>
#include <counter_spy_c.h>

#define DOCA_FLOW_SO "libdoca_flow.so"

void __attribute__ ((constructor)) init_logger(void);
void __attribute__ ((destructor)) close_logger(void);

void * log_dlopen(const char *file, int mode)
{
    void * handle = dlopen(file, mode);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    return handle;
}

void * log_dlsym(void *handle, const char *name)
{
    dlerror();
    void * res = dlsym(handle, name);
    char * error = dlerror();
    if (error) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }
    return res;
}

void load_wrappers(void)
{
    void * handle_doca_flow = log_dlopen(DOCA_FLOW_SO, RTLD_LAZY);

#define INIT_PFUNC(func, lib_handle) p_ ## func = log_dlsym(lib_handle, #func)
    // Core DOCA Flow functions
    INIT_PFUNC(doca_flow_init,                   handle_doca_flow);
    INIT_PFUNC(doca_flow_destroy,                handle_doca_flow);
    
    // Port functions
    INIT_PFUNC(doca_flow_port_start,             handle_doca_flow);
    INIT_PFUNC(doca_flow_port_stop,              handle_doca_flow);
    INIT_PFUNC(doca_flow_port_pair,              handle_doca_flow);
    INIT_PFUNC(doca_flow_port_priv_data,         handle_doca_flow);
    INIT_PFUNC(doca_flow_port_operation_state_modify, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_pipes_flush,       handle_doca_flow);
    INIT_PFUNC(doca_flow_port_pipes_dump,        handle_doca_flow);
    INIT_PFUNC(doca_flow_port_switch_get,        handle_doca_flow);
    
    // Shared resource functions
    INIT_PFUNC(doca_flow_shared_resource_set_cfg, handle_doca_flow);
    INIT_PFUNC(doca_flow_shared_resources_bind,  handle_doca_flow);
    INIT_PFUNC(doca_flow_shared_resources_query, handle_doca_flow);
    
    // Pipe functions
    INIT_PFUNC(doca_flow_pipe_create,            handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_resize,            handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_add_entry,         handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_update_entry,      handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_control_add_entry, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_lpm_add_entry,     handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_lpm_update_entry,  handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_ordered_list_add_entry, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_acl_add_entry,     handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_hash_add_entry,    handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_remove_entry,      handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_calc_hash,         handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_destroy,           handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_dump,              handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_update_miss,       handle_doca_flow);
    
    // Resource query functions
    INIT_PFUNC(doca_flow_resource_query_entry,   handle_doca_flow);
    INIT_PFUNC(doca_flow_resource_query_pipe_miss, handle_doca_flow);
    
    // Aging and entry processing
    INIT_PFUNC(doca_flow_aging_handle,           handle_doca_flow);
    INIT_PFUNC(doca_flow_entries_process,        handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_entry_get_status,  handle_doca_flow);
    
    // MPLS functions
    INIT_PFUNC(doca_flow_mpls_label_encode,      handle_doca_flow);
    INIT_PFUNC(doca_flow_mpls_label_decode,      handle_doca_flow);
    
    // GENEVE functions
    INIT_PFUNC(doca_flow_parser_geneve_opt_create, handle_doca_flow);
    INIT_PFUNC(doca_flow_parser_geneve_opt_destroy, handle_doca_flow);
    
    // Target functions
    INIT_PFUNC(doca_flow_get_target,             handle_doca_flow);
    
    // Configuration functions
    INIT_PFUNC(doca_flow_cfg_create,             handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_destroy,            handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_pipe_queues,    handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_nr_counters,    handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_nr_meters,      handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_nr_acl_collisions, handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_mode_args,      handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_nr_shared_resource, handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_queue_depth,    handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_cb_pipe_process, handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_cb_entry_process, handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_cb_shared_resource_unbind, handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_rss_key,        handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_default_rss,    handle_doca_flow);
    INIT_PFUNC(doca_flow_cfg_set_definitions,    handle_doca_flow);
    
    // Port configuration functions
    INIT_PFUNC(doca_flow_port_cfg_create,        handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_destroy,       handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_devargs,   handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_priv_data_size, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_dev,       handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_rss_cfg,   handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_ipsec_sn_offload_disable, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_operation_state, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_actions_mem_size, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_service_threads_core, handle_doca_flow);
    INIT_PFUNC(doca_flow_port_cfg_set_service_threads_cycle, handle_doca_flow);
    
    // Pipe configuration functions
    INIT_PFUNC(doca_flow_pipe_cfg_create,        handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_destroy,       handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_match,     handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_actions,   handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_monitor,   handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_ordered_lists, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_name,      handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_type,      handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_domain,    handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_is_root,   handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_nr_entries, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_is_resizable, handle_doca_flow);
    // INIT_PFUNC(doca_flow_pipe_cfg_set_enable_strict_matching, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_dir_info,  handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_miss_counter, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_congestion_level_threshold, handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_user_ctx,  handle_doca_flow);
    INIT_PFUNC(doca_flow_pipe_cfg_set_hash_map_algorithm, handle_doca_flow);
}

void init_logger(void)
{
    load_wrappers();
}

void close_logger(void)
{
    counter_spy_stop_service();
}
