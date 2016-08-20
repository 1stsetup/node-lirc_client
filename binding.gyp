{
  "targets": [
    {
      "variables": {
        "node_version": '"<!(node -e "console.log(process.versions.node)")',
      },
      "target_name": "lirc_client",
      "conditions": [
	['node_version=="0.10.0"', {
		"sources": [ "src/lirc_client-node-0.10.cc" ],
		} ],
	['node_version=="4.4.3"', {
		"sources": [ "src/lirc_client-node-4.4.cc" ],
		} ],
	
      ],
      "sources": [ "src/lirc_client-node-0.10.cc" ],
      "ldflags": [
        "-llirc_client"
      ],
      'cflags!': [ '-O2' ],
      'cflags+': [ '-O3' ],
      'cflags_cc!': [ '-O2' ],
      'cflags_cc+': [ '-O3' ],
      'cflags_c!': [ '-O2' ],
      'cflags_c+': [ '-O3' ],
    }
  ]
}
