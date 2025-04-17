#pragma once

#include <doca_flow.h>

#ifdef __cplusplus
extern "C" {
#endif

void counter_spy_start_service(void);

void counter_spy_stop_service(void);

void counter_spy_port_started(
    const struct doca_flow_port *port);

void counter_spy_port_stopped(
    const struct doca_flow_port *port);

void counter_spy_port_flushed(
    const struct doca_flow_port *port);

void counter_spy_pipe_created(
    const struct doca_flow_pipe_cfg *cfg, 
    const struct doca_flow_pipe *pipe);

void counter_spy_entry_added(
    const struct doca_flow_pipe *pipe, 
    const struct doca_flow_monitor *monitor, 
    const struct doca_flow_pipe_entry *entry);

void counter_spy_shared_counters_bound(
    enum doca_flow_shared_resource_type type, 
    uint32_t *res_array,
    uint32_t res_array_len, 
    void *bindable_obj);

void counter_spy_set_port_cfg_port_id(
    const struct doca_flow_port_cfg *cfg, 
    uint16_t port_id);
void counter_spy_set_port_cfg_port(
    const struct doca_flow_port_cfg *cfg, 
    struct doca_flow_port *port);

void counter_spy_set_pipe_cfg_name(
    const struct doca_flow_pipe_cfg *cfg, 
    const char *name);
void counter_spy_set_pipe_cfg_is_root(
    const struct doca_flow_pipe_cfg *cfg, 
    bool is_root);
void counter_spy_set_pipe_cfg_miss_counter(
    const struct doca_flow_pipe_cfg *cfg, 
    bool miss_counter_enabled);
void counter_spy_set_pipe_cfg_pipe(
    const struct doca_flow_pipe_cfg *cfg, 
    struct doca_flow_pipe *pipe);

#ifdef __cplusplus
}
#endif
