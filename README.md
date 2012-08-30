# Xeric

Xeric is a simple server written in C. It only works on Linux. This is an experiment in C by a person who doesn't know C, so don't, er, actually use it, or anything like that.

Although, so far, it seems to work and it makes me wonder what exactly is inside Apache.

## Running

Xeric has two command-line options at the moment.

- `--port xx`. It specifies the port (not obvious at all). Defaults to 80.
- `--address xxx.xxx.xxx.xxx`. It specifies the address to listen at. Defaults to `0.0.0.0`.

Here's how you should run it for debugging (feel free to specify another port, 'course):

    make debug && ./xeric --port 8080 --address 127.0.0.1