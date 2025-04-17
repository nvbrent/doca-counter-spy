doca_samples_dir = doca_root_dir + '/samples'

# Flow shared counter example
flow_shared_counter_srcs = [
    doca_samples_dir + '/doca_flow/flow_shared_counter/flow_shared_counter_sample.c',
    doca_samples_dir + '/doca_flow/flow_shared_counter/flow_shared_counter_main.c',
    doca_samples_dir + '/doca_flow/flow_common.c',
    common_dir_path + '/dpdk_utils.c',
]

flow_shared_counter_inc_dirs = [
    include_directories(doca_samples_dir + '/doca_flow'),
    include_directories(common_dir_path),
]

flow_shared_counter_exe = executable(DOCA_PREFIX + 'flow_shared_counter',
    flow_shared_counter_srcs,
    dependencies : app_dependencies,
    include_directories : flow_shared_counter_inc_dirs,
    c_args : '-Wno-missing-braces',
    install: false)

test('flow_shared_counter', flow_shared_counter_exe,
    args: [
        '-a17:00.0,dv_flow_en=2', 
        '-a17:00.1,dv_flow_en=2', 
        '-c0x1'
    ],
    env: ['LD_PRELOAD=' + spy_lib.full_path()])
