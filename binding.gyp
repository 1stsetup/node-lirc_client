{
  "targets": [
    {
      "target_name": "lirc_client",
      "sources": [ "src/lirc_client.cc" ],
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
