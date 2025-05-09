#include <doca_flow.h>
#include <wrapper_decls.h>
#include <counter_spy_c.h>

// Wrapper functions from doca_flow.h

doca_error_t
doca_flow_init(struct doca_flow_cfg *cfg)
{
	doca_error_t res = (*p_doca_flow_init)(cfg);
	if (res == DOCA_SUCCESS) {
		counter_spy_start_service();
	}
	return res;
}

void
doca_flow_destroy(void)
{
	(*p_doca_flow_destroy)();
	counter_spy_stop_service();
}

doca_error_t doca_flow_port_cfg_set_devargs(struct doca_flow_port_cfg *cfg, const char *devargs)
{
	doca_error_t res = (*p_doca_flow_port_cfg_set_devargs)(cfg, devargs);
	if (res == DOCA_SUCCESS) {
		counter_spy_set_port_cfg_port_id(cfg, atoi(devargs));
	}
	return res;
}

doca_error_t doca_flow_pipe_cfg_set_name(struct doca_flow_pipe_cfg *cfg, const char *name)
{
	doca_error_t res = (*p_doca_flow_pipe_cfg_set_name)(cfg, name);
	if (res == DOCA_SUCCESS) {
		counter_spy_set_pipe_cfg_name(cfg, name);
	}
	return res;
}

doca_error_t doca_flow_pipe_cfg_set_miss_counter(struct doca_flow_pipe_cfg *cfg, bool enabled)
{
	doca_error_t res = (*p_doca_flow_pipe_cfg_set_miss_counter)(cfg, enabled);
	if (res == DOCA_SUCCESS) {
		counter_spy_set_pipe_cfg_miss_counter(cfg, enabled);
	}
	return res;
}

doca_error_t doca_flow_pipe_cfg_set_is_root(struct doca_flow_pipe_cfg *cfg, bool is_root)
{
	doca_error_t res = (*p_doca_flow_pipe_cfg_set_is_root)(cfg, is_root);
	if (res == DOCA_SUCCESS) {
		counter_spy_set_pipe_cfg_is_root(cfg, is_root);
	}
	return res;
}

doca_error_t
doca_flow_port_start(const struct doca_flow_port_cfg *cfg,
		     struct doca_flow_port **port)
{
	doca_error_t res = (*p_doca_flow_port_start)(cfg, port);
	if (res == DOCA_SUCCESS) {
		counter_spy_set_port_cfg_port(cfg, *port);
		counter_spy_port_started(*port);
	}
	return res;
}

doca_error_t
doca_flow_port_stop(struct doca_flow_port *port)
{
	doca_error_t res = (*p_doca_flow_port_stop)(port);
	if (res == DOCA_SUCCESS) {
		counter_spy_port_stopped(port);
	}
	return res;
}

void
doca_flow_port_pipes_flush(struct doca_flow_port *port)
{
	(*p_doca_flow_port_pipes_flush)(port);
	counter_spy_port_flushed(port);
}

doca_error_t
doca_flow_pipe_create(const struct doca_flow_pipe_cfg *cfg,
		const struct doca_flow_fwd *fwd,
		const struct doca_flow_fwd *fwd_miss,
		struct doca_flow_pipe **pipe)
{
    doca_error_t res = (*p_doca_flow_pipe_create)(cfg, fwd, fwd_miss, pipe);
	if (res == DOCA_SUCCESS && pipe) {
		counter_spy_set_pipe_cfg_pipe(cfg, *pipe);
		counter_spy_pipe_created(cfg, *pipe);
	}
    return res;
}

doca_error_t
doca_flow_pipe_add_entry(uint16_t pipe_queue,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_match *match,
			const struct doca_flow_actions *actions,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			uint32_t flags,
			void *usr_ctx,
			struct doca_flow_pipe_entry **entry)
{
    doca_error_t res = (*p_doca_flow_pipe_add_entry)(
        pipe_queue, pipe, match, actions, monitor, fwd, 
        flags, usr_ctx, entry);
	if (res == DOCA_SUCCESS && entry) {
		counter_spy_entry_added(pipe, monitor, *entry);
	}
	return res;
}

doca_error_t
doca_flow_pipe_hash_add_entry(uint16_t pipe_queue,
			      struct doca_flow_pipe *pipe,
			      uint32_t entry_index,
			      const struct doca_flow_actions *actions,
			      const struct doca_flow_monitor *monitor,
			      const struct doca_flow_fwd *fwd,
			      const enum doca_flow_flags_type flags,
			      void *usr_ctx,
			      struct doca_flow_pipe_entry **entry)
{
    doca_error_t res = (*p_doca_flow_pipe_hash_add_entry)(
        pipe_queue, pipe, entry_index, actions, monitor, fwd, 
        flags, usr_ctx, entry);
	if (res == DOCA_SUCCESS && entry) {
		counter_spy_entry_added(pipe, monitor, *entry);
	}
	return res;
}

doca_error_t
doca_flow_pipe_control_add_entry(uint16_t pipe_queue,
			uint32_t priority,
			struct doca_flow_pipe *pipe,
			const struct doca_flow_match *match,
			const struct doca_flow_match *match_mask,
			const struct doca_flow_match_condition *condition,
			const struct doca_flow_actions *actions,
			const struct doca_flow_actions *actions_mask,
			const struct doca_flow_action_descs *action_descs,
			const struct doca_flow_monitor *monitor,
			const struct doca_flow_fwd *fwd,
			void *usr_ctx,
			struct doca_flow_pipe_entry **entry)
{
    doca_error_t res = (*p_doca_flow_pipe_control_add_entry)(
        pipe_queue, priority, pipe, match, match_mask, condition,
		actions, actions_mask, action_descs, monitor, fwd, 
		usr_ctx, entry);
	if (res == DOCA_SUCCESS && entry) {
		counter_spy_entry_added(pipe, monitor, *entry);
	}
	return res;
}

doca_error_t
doca_flow_shared_resources_bind(enum doca_flow_shared_resource_type type,
					     uint32_t *res_array,
					     uint32_t res_array_len,
					     void *bindable_obj)
{
	doca_error_t res = (*p_doca_flow_shared_resources_bind)(
		type, res_array, res_array_len, bindable_obj);
	if (res == DOCA_SUCCESS && type == DOCA_FLOW_SHARED_RESOURCE_COUNTER) {
		counter_spy_shared_counters_bound(
			type, res_array, res_array_len, bindable_obj);
	}
	return res;
}
