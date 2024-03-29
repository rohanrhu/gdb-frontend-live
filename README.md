# `gdb-frontend-live`: online ide/debugger platform based on [gdb-frontend](https://github.com/rohanrhu/gdb-frontend) 🛸👽🌌
GDBFrontendLive is a server that creates GDBFrontend instances and provides an online sharable IDEs for each individiual users.

[![GitHub release](https://img.shields.io/github/release/rohanrhu/gdb-frontend-live.svg?style=flat-square&color=informational)](https://github.com/rohanrhu/gdb-frontend-live/releases)
[![GitHub issues](https://img.shields.io/github/issues/rohanrhu/gdb-frontend-live?style=flat-square&color=red)](https://github.com/rohanrhu/gdb-frontend-live/issues)
[![GitHub forks](https://img.shields.io/github/forks/rohanrhu/gdb-frontend-live?style=flat-square)](https://github.com/rohanrhu/gdb-frontend-live/network)
[![GitHub stars](https://img.shields.io/github/stars/rohanrhu/gdb-frontend-live?style=flat-square)](https://github.com/rohanrhu/gdb-frontend-live/stargazers)
[![Gitter](https://img.shields.io/badge/chat-on%20gitter-blue.svg?style=flat-square&logo=gitter)](https://gitter.im/gdb-frontend/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Donate](https://img.shields.io/liberapay/receives/EvrenselKisilik.svg?logo=liberapay&style=flat-square&color=green)](https://liberapay.com/EvrenselKisilik/donate)

![gdb-frontend-live](https://oguzhaneroglu.com/static/images/gflive-ss-1.png "GDBFrontendLive is a server that creates GDBFrontend instances and provides an online sharable IDEs for each individiual users.")

## Architecture
```
+----------------+        +------------------------+
|                +-------->                        |
|    Client      |        |    GDBFrontendLive     |
|   (Browser)    |        |         Server         |
|                <--------+                        |
+--^--+----------+        | +-----Instances------+ |
   |  |                   | |                    | |
   |  |                   | | +----------------+ | |
   |  |                   | | |                | | |
   |  +----------------------->                | | |
   |                      | | |  GDBFrontend   | | |
   +--------------------------+                | | |
                          | | |                | | |
                          | | +----------------+ | |
                          | |        ...         | |
                          | +--------------------+ |
                          +------------------------+
```

## What is this exactly?
GFLive provides a platform that creates online shareble IDE/debuggers for each individual user. Once you create an IDE instance, you can share your session and debug together.

This project may useful for especially educational institutions or teams; they can setup their own GFLive platforms and provide online shareble IDEs. Also, of course it may be useful for individual usages; you can create a lot of debugger instances and use and share them to your friends.

GDBFrontend is a debugger in fact for now but it provides basic editing features and full access to terminal on a `tmux` session. So you can use even Vim in the terminal or do something else. With debugging features, editing/IDE features will be improved in-time. You can contribute to project, all PRs are welcome.

**Note:** Still both projects GFlive and GF are very new and on alpha stages.

## Security
GFLive provides full access on instance user and group that are set by `--instance-user`/`--instance-group`, so the IDE instances can access each other and internet! Also you must be careful for your open ports or DMZ option on your router; if you dont use a router, you must use firewall or setup a complex autharization architecture based on users and ports.

## Other Parts
### GDBFrontend: [gdb-frontend](https://github.com/rohanrhu/gdb-frontend)
GDBFrontend to create instances.

### Adapter: [gdb-frontend-live-adapter](https://github.com/rohanrhu/gdb-frontend-live-adapter)
GDBFrontend adapter plugin for adopting instance to GDBFrontendLive.

### Client: [gdb-frontend-live-client](https://github.com/rohanrhu/gdb-frontend-live-client)
Frontend client that connects to GDBFrontendLive server.

## Build
You can build it with GNU Make.

```bash
make clean; make
```

## Requirements
GFLive does not require anything except [GDBFrontend Requirements](https://github.com/rohanrhu/gdb-frontend#requirements). **You must provide requirements for GDBFrontend.**

## Usage
```
# ./server -h
GDBFrontendLive v0.0.1-alpha
Usage: gdbfrontendlive [options]
Options:
        --help, -h:                             Shows this help text.
        --version, -v:                          Shows GDBFrontendLive and GDBFrontend versions.
        --host-address=IP, -H IP:               Specifies bind address.
        --bind-address=IP, -l IP:               Specifies host address.
        --ws-port=PORT, -wsp PORT:              Specifies GDBFrontend websocket server's port.
        --instance-user=USER, -u USER:          Sets the user that runs instances.
        --instance-group=GROUP, -g GROUP:       Sets the group that runs instances.
```

Start server with an instance user:
```bash
./server --instance-user=USERNAME
```

### Notices
* You must run the server as root because it sets uids and gids of instance processes.
* CWD must be the directory of server executable.

## Client API/Interface
GFLive provides a WebSocket server to clients. The server uses serialized JSONs for messaging. HTTP server for client is a TODO; you can use [gdb-frontend-live-client](https://github.com/rohanrhu/gdb-frontend-live-client) on `file://` (without a http server) or serve it a simple http server like Apache HTTPD.

## Does it support Windows?
No. You can use it on Linux.

### Does it support WSL?
GDBFrontend supports WSL but GFLive does not.
### Issues about WSL
* GFLive needs random ports and on WSL 1, Random port option is not usable on WSL becasue `/proc/net/tcp` interface is not working on WSL. (WSL 2 does not has this problem.)
## Versioning
Since v0.1.0-git, GFLive switched to a new versioning strategy.

### Reading Versions
In `vX.Y.Z-STABILITY`:
* `X` is **major** versions, changes long term with major features and enhancements.
* `Y` is **main** versions that include new features and enhancements.
* `Z` is **bugfix** releases of main versions.
* `STABILITY` is stability level of the release. (`alpha`, `beta`, `rcN`, `stable`)

## Documentation
Documentation is TODO yet.

## Contributing
All PRs are welcome.

## Donate
You can donate to support the project.

<a href="https://liberapay.com/EvrenselKisilik/donate"><img alt="Donate using Liberapay" src="https://liberapay.com/assets/widgets/donate.svg"></a>

or you can make donation with Bitcoin:

| QR Code | Bitcoin address for donations |
|---|---|
| ![Bitcoin address QR code for donate](media/btc-donation-qr.png) | **3KBtYfaAT42uVFd6D2XFRDTAoErLz73vpL** |

## License
GNU General Public License v3 (GPL-3)

You may copy, distribute and modify the software as long as you track changes/dates in source files. Any modifications to or software including (via compiler) GPL-licensed code must also be made available under the GPL along with build & install instructions.