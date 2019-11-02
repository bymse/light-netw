## Light network utility

This is a simple cli win-socket utility that can share files using IPv6 + TCP. It was created for educational purposes and for some practice with `C` and network-programming. If you have any comments about this project (technical remarks, code quality, etc.) -> contact me, I'll be glad to discuss them.

#### How does it work?

There are two modes : server and client. 

You can run server and specify directory:

```bash
light-netw -t sd -i Y:\path\to\dir
```

And if you call client like this:

```bash
light-netw -t cf -a localhost -i folder\file.txt -o Y:\path\to\some_file.txt
```

Then the server will try to find file ``Y:\path\to\dir\folder\file.txt`` and send it to the client. The client will write it to ``Y:\path\to\some_file.txt``. If  ``Y:\path\to\dir\folder\file.txt`` doesn't exist the server will create it.

| FLAG | Meaning                                          |
| ---- | ------------------------------------------------ |
| -a   | Address (domain or IPv6)                         |
| -p   | Port (default 3490)                              |
| -t   | Run type: sd (server dirshare), cf (client file) |
| -i   | Input path, dir for server, file name for client |
| -o   | Output path, output file for client              |

