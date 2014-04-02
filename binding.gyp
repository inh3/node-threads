{
    'targets': [

    # node threads .node
    {
        'target_name': 'node-threads',
        'sources': [
            'node-threads.cc',
            'src/node-threads/environment.cc',
            'src/node-threads/node-threads-object.cc',
            'src/node-threads/web-worker.cc',
            'src/node-threads/node-threads-factory.cc',

            'src/file/file_info.cc',

            'src/emulation/process_emulation.cc',
            'src/emulation/require.cc',

            'src/thread/thread.cc',
            'src/thread/thread-isolate.cc',
            'src/thread/callback-manager.cc',

            'src/work-items/work-item.cc',
            'src/work-items/function-work-item.cc',
            
            'src/utilities/utilities.cc',
            'src/utilities/error-handling.cc',
            'src/utilities/json.cc'
        ],

        'include_dirs': [
            './src',
            './src/node-threads',
            './src/file',
            './src/thread',
            './src/work-items',
            './src/emulation',
            './src/utilities',
            './src/threadpool',
            '<!(node -e \"require(\'nan\')\")'
        ],

        'dependencies': [
            'threadpool'
        ],

        'conditions': [
            ['OS=="linux"', {
                'cflags': [
                    '-std=c++0x'
                ]
            }],
            ['OS=="mac"', {
                'cflags': [
                    '-std=c++11',
                    '-stdlib=libc++'
                ]
            }]
        ]
    },

    # thread pool library
    {
        'target_name': 'threadpool',
        'type': 'static_library',

        'include_dirs': [
            './src/threadpool'
        ],

        'sources': [
            './src/threadpool/synchronize.c',
            './src/threadpool/task_queue.c',
            './src/threadpool/thread_pool.c'
        ],

        'conditions': [
            ['OS=="win"', {
                'defines': [
                    '_WIN32'
                ]
            }],
            ['OS=="linux"', {
                'ldflags': [
                    '-pthread'
                ]
            }],
            ['OS=="mac"', {
                'ldflags': [
                    '-pthread'
                ]
            }]
        ]
    }]
}