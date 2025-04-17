#include <signal.h>

#include <dpdk_utils.h>
#include <utils.h>

#include <doca_argp.h>
#include <doca_log.h>
#include <doca_flow.h>

DOCA_LOG_REGISTER(sample);

volatile bool force_quit = false;

static void
signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGTERM)
    {
        printf("\n\nSignal %d received, preparing to exit...\n", signum);
        force_quit = true;
    }
}

static void install_signal_handler(void)
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

////////////////////////////////////////////////////////////////////////////////
// DOCA Flow

static struct doca_flow_port *
port_init(uint16_t port_id)
{
    char port_id_str[128];
    snprintf(port_id_str, sizeof(port_id_str), "%d", port_id);

    struct doca_flow_port_cfg *port_cfg = NULL;
    doca_error_t res = doca_flow_port_cfg_create(&port_cfg);
#if 0
    {
		.port_id = port_id,
		.type = DOCA_FLOW_PORT_DPDK_BY_ID,
		.devargs = port_id_str,
	};
#endif
    struct doca_flow_port *port;
    res = doca_flow_port_start(port_cfg, &port);
    if (port == NULL)
    {
        DOCA_LOG_ERR("failed to initialize doca flow port: %s", doca_error_get_descr(res));
        return NULL;
    }
    doca_flow_port_cfg_destroy(port_cfg);
    return port;
}

int flow_init(
    struct application_dpdk_config *dpdk_config,
    struct doca_flow_port *ports[])
{
    struct doca_flow_cfg *arp_sc_flow_cfg = NULL;
    doca_error_t res = doca_flow_cfg_create(&arp_sc_flow_cfg);
    if (res != DOCA_SUCCESS)
    {
        DOCA_LOG_ERR("failed to create doca flow config: %s", doca_error_get_descr(res));
        return -1;
    }
#if 0
    {
		.flags = DOCA_FLOW_CFG_PIPE_MISS_MON,
		.queues = dpdk_config->port_config.nb_queues,
		.resource.nb_counters = 1024,
		.mode_args = "vnf,hws",
        .queue_depth = 128,
        .nr_shared_resources = {
            [DOCA_FLOW_SHARED_RESOURCE_COUNT] = 1024,
        },
	};
#endif
    res = doca_flow_init(arp_sc_flow_cfg);
    doca_flow_cfg_destroy(arp_sc_flow_cfg);
    if (res != DOCA_SUCCESS)
    {
        DOCA_LOG_ERR("failed to init doca: %s", doca_error_get_descr(res));
        return -1;
    }
    DOCA_LOG_DBG("DOCA flow init done");

    for (uint16_t port_id = 0; port_id < dpdk_config->port_config.nb_ports; port_id++)
    {
        ports[port_id] = port_init(port_id);
    }

    /* pair the two ports together for hairpin forwarding */
    for (uint16_t port_id = 0; port_id + 1 < dpdk_config->port_config.nb_ports; port_id += 2)
    {
        if (doca_flow_port_pair(ports[port_id], ports[port_id + 1]))
        {
            DOCA_LOG_ERR("DOCA Flow port pairing failed");
            return -1;
        }
    }

    /* bind some shared counters to the ports */
    for (uint16_t port_id = 0; port_id < dpdk_config->port_config.nb_ports; port_id++)
    {
        const uint32_t N_SHARED = 4;
        uint32_t shared_counter_id[N_SHARED];
        for (uint32_t i = 0; i < N_SHARED; i++)
        {
            shared_counter_id[i] = (port_id + 1) * 10 + i;
        }

        struct doca_flow_shared_resource_cfg counter_cfg = {
            .domain = DOCA_FLOW_PIPE_DOMAIN_DEFAULT,
        };
        for (uint32_t i = 0; i < N_SHARED; i++)
        {
            doca_flow_shared_resource_set_cfg(DOCA_FLOW_SHARED_RESOURCE_COUNTER, shared_counter_id[i], &counter_cfg);
        }

        if (doca_flow_shared_resources_bind(
                DOCA_FLOW_SHARED_RESOURCE_COUNTER, shared_counter_id, N_SHARED, ports[port_id]))
        {
            DOCA_LOG_ERR("DOCA Flow port shared-counter-bind failed");
            return -1;
        }
    }

    DOCA_LOG_DBG("DOCA flow init done");
    return 0;
}

void flow_destroy(struct doca_flow_port *ports[], uint16_t num_ports)
{
    for (uint16_t port_id = 0; port_id < num_ports; port_id++)
    {
        doca_flow_port_pipes_flush(ports[port_id]);
        doca_flow_port_stop(ports[port_id]);
    }
    doca_flow_destroy();
}

void create_flows(
    struct doca_flow_port *ports[],
    uint16_t num_ports)
{
    for (uint16_t port_id = 0; port_id < num_ports; port_id++)
    {
        doca_error_t res;
        struct doca_flow_match match = {};
        struct doca_flow_monitor mon = {.counter_type = DOCA_FLOW_RESOURCE_TYPE_NON_SHARED};
        uint16_t rss_queues[] = {0};
        struct doca_flow_fwd fwd = {.type = DOCA_FLOW_FWD_RSS, .rss_type = DOCA_FLOW_RESOURCE_TYPE_NON_SHARED, .rss = {.nr_queues = 1, .queues_array = rss_queues}};
        struct doca_flow_fwd miss = {.type = DOCA_FLOW_FWD_DROP};
        struct doca_flow_pipe_cfg *cfg = NULL;
        res = doca_flow_pipe_cfg_create(&cfg, ports[port_id]);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe config: %s", doca_error_get_descr(res));
            return;
        }
#if 0
        cfg->attr = {
            .name = "SAMPLE_PIPE",
        };
        cfg->port = ports[port_id];
        cfg->match = &match;
        cfg->monitor = &mon;
        };
#endif
        struct doca_flow_pipe *pipe = NULL;

        res = doca_flow_pipe_create(cfg, &fwd, &miss, &pipe);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe: %s", doca_error_get_descr(res));
        }

        // Binding shared_counters to pipes in VNF mode does not appear to work under DOCA 2.2+
#if 0
        /* bind some shared counters to the first pipe */
        const uint32_t N_SHARED = 3;
        uint32_t shared_counter_id[N_SHARED];
        for (uint32_t i=0; i<N_SHARED; i++) {
            shared_counter_id[i] = (port_id+5) * 10 + i;
        }

        struct doca_flow_shared_resource_cfg counter_cfg = {
            .domain = DOCA_FLOW_PIPE_DOMAIN_DEFAULT,
        };
        for (uint32_t i=0; i<N_SHARED; i++) {
            doca_flow_shared_resource_set_cfg(DOCA_FLOW_SHARED_RESOURCE_COUNTER, shared_counter_id[i], &counter_cfg);
        }

        if (doca_flow_shared_resources_bind(
                DOCA_FLOW_SHARED_RESOURCE_COUNTER, shared_counter_id, N_SHARED, pipe)) {
            DOCA_LOG_ERR("DOCA Flow port shared-counter-bind failed");
        }
#endif

        for (int j = 0; j < 5; j++)
        {
            struct doca_flow_pipe_entry *entry = NULL;
            res = doca_flow_pipe_add_entry(0, pipe, NULL, NULL, NULL, NULL, 0, NULL, &entry);
        }

        struct doca_flow_fwd fwd_next = {.type = DOCA_FLOW_FWD_PIPE, .next_pipe = pipe};
        struct doca_flow_pipe_cfg *cfg_next = NULL;
        res = doca_flow_pipe_cfg_create(&cfg_next, ports[port_id]);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe config: %s", doca_error_get_descr(res));
            return;
        }
#if 0
        cfg_next->attr = {
            .name = "NEXT_PIPE",
        };
            .port = ports[port_id],
            .match = &match,
            .monitor = &mon,
        };
#endif
        res = doca_flow_pipe_create(cfg_next, &fwd_next, &miss, &pipe);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe: %s", doca_error_get_descr(res));
        }

        for (int j = 0; j < 2; j++)
        {
            struct doca_flow_pipe_entry *entry = NULL;
            res = doca_flow_pipe_add_entry(0, pipe, NULL, NULL, NULL, NULL, 0, NULL, &entry);
            if (res != DOCA_SUCCESS)
            {
                DOCA_LOG_ERR("Failed to create Pipe entry: %s", doca_error_get_descr(res));
            }
        }

        struct doca_flow_pipe_cfg *cfg_root = NULL;
        res = doca_flow_pipe_cfg_create(&cfg_root, ports[port_id]);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe config: %s", doca_error_get_descr(res));
            return;
        }
#if 0
        cfg_root->attr = {
            .name = "ROOT_PIPE",
                .is_root = true,
            },
            .port = ports[port_id],
            .match = &match,
            .monitor = NULL, // "Counter action on root table is not supported in HW steering mode"
        };
#endif
        struct doca_flow_fwd fwd_root = {.type = DOCA_FLOW_FWD_PIPE, .next_pipe = pipe};
        res = doca_flow_pipe_create(cfg_root, &fwd_root, &miss, &pipe);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe: %s", doca_error_get_descr(res));
        }
        struct doca_flow_pipe_entry *entry = NULL;
        res = doca_flow_pipe_add_entry(0, pipe, NULL, NULL, NULL, NULL, 0, NULL, &entry);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to create Pipe entry: %s", doca_error_get_descr(res));
        }

        res = doca_flow_entries_process(ports[port_id], 0, 100 * 1000, 8);
        if (res != DOCA_SUCCESS)
        {
            DOCA_LOG_ERR("Failed to process Pipe entries: %s", doca_error_get_descr(res));
        }
    }
}

int main(int argc, char *argv[])
{
    struct doca_log_backend *sdk_log;

    // Register a logger backend
    doca_error_t result = doca_log_backend_create_standard();
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    // Register a logger backend for internal SDK errors and warnings
    result = doca_log_backend_create_with_file_sdk(stdout, &sdk_log);
    if (result != DOCA_SUCCESS)
        return EXIT_FAILURE;

    doca_log_backend_set_sdk_level(sdk_log, DOCA_LOG_LEVEL_WARNING);

    struct application_dpdk_config dpdk_config = {
        .port_config.nb_ports = 2,
        .port_config.nb_queues = 1,
        .port_config.nb_hairpin_q = 1,
    };

    /* Parse cmdline/json arguments */
    doca_argp_init("SAMPLE", &dpdk_config);
    doca_argp_set_dpdk_program(dpdk_init);
    // sample_register_argp_params();
    doca_argp_start(argc, argv);

    install_signal_handler();

    dpdk_queues_and_ports_init(&dpdk_config);

    struct doca_flow_port *ports[dpdk_config.port_config.nb_ports];
    flow_init(&dpdk_config, ports);

    create_flows(ports, dpdk_config.port_config.nb_ports);

    while (!force_quit)
    {
        printf("sleeping...\n");
        sleep(8);
    }

    flow_destroy(ports, dpdk_config.port_config.nb_ports);
    doca_argp_destroy();

    return 0;
}
