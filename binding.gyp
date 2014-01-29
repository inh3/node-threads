{
    'targets': [

    # npool .node
    {
        'target_name': 'node-threads',
        'sources': [
            'node-threads.cc',
            'src/node-threads/node-threads-object.cc',
            'src/utilities/utilities.cc'
        ],

        'include_dirs': [
            './src',
            './src/node-threads',
            './src/utilities',
            '<!(node -e \"require(\'nan\')\")'
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
    }]
}