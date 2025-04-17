#include <string>
#include <map>
#include <mutex>
#include <random> // only used for faking stats
#include <thread>

#include <doca_flow.h>
#include <counter_spy_c.h>
#include <counter_spy.h>

#include <counter_spy.pb.h>
#include <counter_spy.grpc.pb.h>

const std::string COUNTER_SPY_FAKE_STATS_ENV_VAR = "COUNTER_SPY_FAKE_STATS";

bool fake_stats_enabled = false;
std::mt19937 rng; // don't bother with non-default seed
std::normal_distribution<> pkts_random_dist(0, 10000);
std::normal_distribution<> bytes_random_dist(0, 1000000);

std::map<const struct doca_flow_port *const, PortMon> ports;

// Used for looking up port_id, etc.
struct port_cfg {
	uint16_t port_id;
	doca_flow_port *port;
};
std::map<const doca_flow_port_cfg *, port_cfg> port_cfg_map;

void counter_spy_set_port_cfg_port_id(const doca_flow_port_cfg *cfg, uint16_t port_id)
{
	port_cfg_map[cfg].port_id = port_id;
}
void counter_spy_set_port_cfg_port(const doca_flow_port_cfg *cfg, doca_flow_port *port)
{
	port_cfg_map[cfg].port = port;
}

// Used for looking up pipe_id, name, etc.
struct pipe_cfg {
	std::string name;
	bool is_root;
	bool miss_counter_enabled;
	doca_flow_pipe *pipe;
};
std::map<const doca_flow_pipe_cfg *, pipe_cfg> pipe_cfg_map;

void counter_spy_set_pipe_cfg_name(
    const struct doca_flow_pipe_cfg *cfg, 
    const char *name)
{
    pipe_cfg_map[cfg].name = name;
}

void counter_spy_set_pipe_cfg_is_root(
    const struct doca_flow_pipe_cfg *cfg, 
    bool is_root)
{
    pipe_cfg_map[cfg].is_root = is_root;
}

void counter_spy_set_pipe_cfg_miss_counter(
    const struct doca_flow_pipe_cfg *cfg, 
    bool miss_counter_enabled)
{
    pipe_cfg_map[cfg].miss_counter_enabled = miss_counter_enabled;
}

void counter_spy_set_pipe_cfg_pipe(
    const struct doca_flow_pipe_cfg *cfg, 
    struct doca_flow_pipe *pipe)
{
    pipe_cfg_map[cfg].pipe = pipe;
}

struct doca_flow_resource_query fake_stats()
{
    struct doca_flow_resource_query stats;
    stats.counter.total_bytes = (uint64_t)std::abs(bytes_random_dist(rng));
    stats.counter.total_pkts  = (uint64_t)std::abs(pkts_random_dist(rng));
    return stats;
}

struct doca_flow_resource_query operator+(
    const struct doca_flow_resource_query &a, 
    const struct doca_flow_resource_query &b)
{
    struct doca_flow_resource_query c;
    c.counter.total_bytes = a.counter.total_bytes + b.counter.total_bytes;
    c.counter.total_pkts = a.counter.total_pkts + b.counter.total_pkts;
    return c;
}

struct doca_flow_resource_query operator-(
    const struct doca_flow_resource_query &a, 
    const struct doca_flow_resource_query &b)
{
    struct doca_flow_resource_query c;
    c.counter.total_bytes = a.counter.total_bytes - b.counter.total_bytes;
    c.counter.total_pkts = a.counter.total_pkts - b.counter.total_pkts;
    return c;
}

EntryMon::EntryMon() = default;

EntryMon::EntryMon(
    const struct doca_flow_pipe_entry *entry_ptr, 
    const struct doca_flow_monitor *mon_settings) : entry_ptr(entry_ptr)
{ 
    if (mon_settings) {
        this->mon = *mon_settings; // copy
    }
}

EntryMon::EntryMon(
    const struct doca_flow_pipe *pipe_ptr,
    const struct doca_flow_monitor *mon_settings) : pipe_ptr(pipe_ptr)
{    
    if (mon_settings) {
        this->mon = *mon_settings; // copy
    }
}

EntryFlowStats
EntryMon::query_entry()
{
    EntryFlowStats result = {};
    result.entry_ptr = entry_ptr;
    result.shared_counter_id = 0;

    if (fake_stats_enabled) {
        // Generate a fake delta and accumulate the total
        result.delta = fake_stats();
        this->stats = this->stats + result.delta;
        result.total = this->stats;
        result.valid = true;
    } else if (entry_ptr) {
        // Query the total and compute the delta
        auto prev_stats = this->stats;
        auto res = doca_flow_resource_query_entry(
            const_cast<struct doca_flow_pipe_entry *>(entry_ptr), 
            &result.total);
        result.valid = res == DOCA_SUCCESS;
        if (result.valid) {
            result.delta = result.total - prev_stats;
            this->stats = result.total;
        }
    } else if (pipe_ptr) {
        // Query the total and compute the delta
        auto prev_stats = this->stats;
        auto res = doca_flow_resource_query_pipe_miss(
                const_cast<struct doca_flow_pipe*>(pipe_ptr),
                &result.total);
        result.valid = res == DOCA_SUCCESS;
        if (result.valid) {
            result.delta = result.total - prev_stats;
            this->stats = result.total;
        } else {
            printf("Failed to query miss counter for pipe %p; error=%d\n", pipe_ptr, res);
        }
    }
    return result;
}

SharedCounterMon::SharedCounterMon() = default;

bool SharedCounterMon::is_empty() const
{
    return shared_counter_ids.empty();
}

void SharedCounterMon::shared_counters_bound(
    uint32_t *res_array,
    uint32_t res_array_len)
{
    for (uint32_t i=0; i<res_array_len; i++) {
        shared_counter_ids[res_array[i]].shared_counter_id = res_array[i];
    }
}

FlowStatsList SharedCounterMon::query_entries()
{
    FlowStatsList result;

    size_t n_shared = shared_counter_ids.size();

    if (n_shared == 0) {
        return result;
    }

    // Keep a copy of the counters before the query so
    // the delta can be computed
    auto prev_counters = shared_counter_ids; // copy

    // Prepare the vector of counter IDs for the batch query operation
    std::vector<uint32_t> counter_ids;
    counter_ids.reserve(n_shared);
    for (const auto &shared_ctr : shared_counter_ids) {
        counter_ids.push_back(shared_ctr.first);
    }

    // Prepare a vector for the output of the batch query:
    std::vector<doca_flow_resource_query> query_results_array(n_shared);

    if (!fake_stats_enabled) {
        auto res = doca_flow_shared_resources_query(
            DOCA_FLOW_SHARED_RESOURCE_COUNTER, counter_ids.data(),
            query_results_array.data(), n_shared);

        if (res != DOCA_SUCCESS) {
            return result;
        }
    }

    result.reserve(result.size() + n_shared);

    for (size_t i=0; i<n_shared; i++) {
        auto &shared_ctr = shared_counter_ids[counter_ids[i]];
        const auto &prev_ctr = prev_counters[counter_ids[i]];
        auto &result_stat = result.emplace_back();
        result_stat.shared_counter_id = counter_ids[i];

        if (fake_stats_enabled) {
            // Generate a fake delta and accumulate the total
            result_stat.delta = fake_stats();
            shared_ctr.total = shared_ctr.total + result_stat.delta;
            result_stat.total = shared_ctr.total;
        } else {
            // Update our internal state
            shared_ctr.total.counter.total_bytes = query_results_array[i].counter.total_bytes;
            shared_ctr.total.counter.total_pkts = query_results_array[i].counter.total_pkts;
            shared_ctr.delta.counter.total_bytes = shared_ctr.total.counter.total_bytes - prev_ctr.total.counter.total_bytes;
            shared_ctr.delta.counter.total_pkts = shared_ctr.total.counter.total_pkts - prev_ctr.total.counter.total_pkts;

            // Update the output message
            result_stat.total = shared_ctr.total;
            result_stat.delta = shared_ctr.delta;
        }
    }

    return result;
}

PipeMon::PipeMon() = default;

PipeMon::PipeMon(
    const doca_flow_pipe *pipe,
    std::string name,
    doca_flow_pipe_type type,
    bool is_root,
    bool miss_counter_enabled,
    const doca_flow_monitor *pipe_mon) : 
        name_(name),
        type_(type),
        is_root_(is_root),
        miss_counter_enabled_(miss_counter_enabled),
        pipe_(pipe), 
        miss_entry_(pipe, pipe_mon)
{
    if (pipe_mon) {
        mon_ = *pipe_mon; // copy
    }
}

std::string PipeMon::name() const
{
    return name_;
}

bool PipeMon::is_root() const
{
    return is_root_;
}

doca_flow_pipe_type PipeMon::type() const
{
    return type_;
}

bool PipeMon::is_counter_active(const struct doca_flow_monitor *mon)
{
	return mon && mon->counter_type != DOCA_FLOW_RESOURCE_TYPE_NONE;
}

PipeStats
PipeMon::query_entries()
{
    PipeStats result;
    result.pipe_mon = this;
    result.pipe_stats.reserve(entries_.size());

    //printf("Query: Pipe %s\n", attr_name.c_str());
    for (auto &entry : entries_) {
        auto stats = entry.second.query_entry();
        if (stats.valid) {
            result.pipe_stats.emplace_back(stats);
        }
    }

    result.pipe_shared_counters = std::move(shared_counters_.query_entries());

    result.pipe_miss_counter = miss_entry_.query_entry();

    return result;
}

void PipeMon::shared_counters_bound(
    uint32_t *res_array,
    uint32_t res_array_len)
{
    shared_counters_.shared_counters_bound(res_array, res_array_len);
}

void PipeMon::entry_added(
    const struct doca_flow_pipe *pipe, 
    const struct doca_flow_monitor *entry_monitor, 
    const struct doca_flow_pipe_entry *entry)
{
    if (is_counter_active(entry_monitor) ||
        is_counter_active(&mon_)) 
    {
        entries_[entry] = EntryMon(entry, entry_monitor);
    }
}

PortMon::PortMon() : port_id_(0)
{
}

PortMon::PortMon(
    uint16_t port_id,
    const struct doca_flow_port *port) :
        port_id_(port_id),
        port_(port)
{
}

PortMon::~PortMon()
{
    port_flushed();
}

void PortMon::port_flushed()
{
    std::lock_guard<std::mutex> lock(mutex_);
    pipes_.clear();
}

uint16_t PortMon::port_id() const
{
    return port_id_;
}

PortStats
PortMon::query()
{
    std::lock_guard<std::mutex> lock(mutex_);
    //printf("Query: Port %d\n", _port_id);
    PortStats stats;
    stats.port_mon = this;
    for (auto &pipe : pipes_) {
        auto entries = pipe.second.query_entries();
        if (!entries.pipe_stats.empty()) {
            stats.port_stats[pipe.second.name()] = std::move(entries);
        }
    }

    stats.port_shared_counters = std::move(shared_counters_.query_entries());

    return stats;
}

void PortMon::shared_counters_bound(
    uint32_t *res_array,
    uint32_t res_array_len)
{
    shared_counters_.shared_counters_bound(res_array, res_array_len);
}

void PortMon::pipe_created(
    const struct doca_flow_pipe_cfg *cfg, 
    const doca_flow_pipe *pipe)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // TODO: get pipe_cfg attrs, monitor from cfg
    // pipes[pipe] = PipeMon(pipe, cfg->attr, cfg->monitor);
}

std::mutex& PortMon::get_mutex() const
{
    return mutex_;
}

PipeMon *PortMon::find_pipe(const doca_flow_pipe *pipe)
{
    auto pipe_counters_iter = pipes_.find(pipe);
    return (pipe_counters_iter == pipes_.end()) ? nullptr : &pipe_counters_iter->second;
}

void counter_spy_port_started(
    const struct doca_flow_port * port)
{
    for (auto &port_cfg_iter : port_cfg_map) {
        if (port_cfg_iter.second.port == port) {
            uint16_t port_id = port_cfg_iter.second.port_id;
            ports.emplace(std::piecewise_construct,
                std::forward_as_tuple(port),
                std::forward_as_tuple(port_id, port));
            return;
        }
    }
}

void counter_spy_port_stopped(
    const struct doca_flow_port *port)
{
    ports.erase(port);
}

void counter_spy_port_flushed(
    const struct doca_flow_port *port)
{
    ports[port].port_flushed();
}

void counter_spy_pipe_created(
    const struct doca_flow_pipe_cfg *cfg, 
    const doca_flow_pipe *pipe)
{
    doca_flow_port *port = nullptr; // TODO: get port from cfg
    auto &port_counters = ports[port];
    port_counters.pipe_created(cfg, pipe);
}

void counter_spy_entry_added(
    const struct doca_flow_pipe *pipe, 
    const struct doca_flow_monitor *monitor, 
    const struct doca_flow_pipe_entry *entry)
{
    for (auto &port_counters_iter : ports) {
        auto &port_counters = port_counters_iter.second;
        std::lock_guard<std::mutex> lock(port_counters.get_mutex());

        auto pipe_counters = port_counters.find_pipe(pipe);
        if (!pipe_counters) {
            continue; // pipe not found
        }

        pipe_counters->entry_added(pipe, monitor, entry);
        break;
    }
}

void counter_spy_shared_counters_bound(
    enum doca_flow_shared_resource_type type, 
    uint32_t *res_array,
    uint32_t res_array_len, 
    void *bindable_obj)
{
    // Determine whether bindable_obj is a port or pipe
    auto find_port = ports.find((doca_flow_port*)bindable_obj);
    if (find_port != ports.end()) {
        // found a port!
        find_port->second.shared_counters_bound(res_array, res_array_len);
        return;
    }

    for (auto &port_iter : ports) {
        auto &port_counters = port_iter.second;

        std::lock_guard<std::mutex> lock(port_counters.get_mutex());

        auto pipe_counters = port_counters.find_pipe((PipePtr)bindable_obj);
        if (!pipe_counters) {
            continue; // pipe not found
        }

        pipe_counters->shared_counters_bound(res_array, res_array_len);
        break;
    }
}
