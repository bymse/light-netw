## Light network utility

This is a simple cli win-socket utility that can share files using IPv4 (IPv6) + TCP. It was created for educational purposes and for some practice with `C` and `network-programming`. If you have any comments about this project (technical remarks, code quality, etc.) -> contact me via [egorstrelov@gmail.com](mailto:egorstrelov@gmail.com), I'll be glad to discuss them.

#### How does it work?

There are two modes : server and client. 

You can run utility in server mode and spec a dir for it. Then in client mode can request any file from this dir and the server will response with it.

#### Options

| FLAG | Meaning                                          | DEFAULT             |
| ---- | ------------------------------------------------ | ------------------- |
| -a   | Address (domain or IP)                           |                     |
| -p   | Port                                             | 3490                |
| -t   | Run type: sd (server dirshare), cf (client file) |                     |
| -i   | Input path, dir for server, file name for client |                     |
| -o   | Output path, output file for client              |                     |
| -r   | Routing protocol, v4 for IPv4, v6 for IPv6       | v4                  |
| -l   | Path to file for logs                            | light-netw.logs.log |

#### Examples

You can run the server:

```bash
light-netw -t sd -r v6 -i path\to\dir
```

And the client:

```bash
light-netw -t cf -r v6 -a localhost -i folder\file.txt -o path\to\some_file.txt
```

The server will:

1. get ``"folder\file.txt"`` from client via IP`v6` and port `3490`
2. try to find file ``path\to\dir\folder\file.txt`` 
3. send it to the client, if it exists, and error code otherwise 

The client will:

- write it to ``path\to\some_file.txt``

#### Error codes

You can check [source code](https://github.com/bymse/light-netw/blob/master/common/netwtypes.h#L6) 