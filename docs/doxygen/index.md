Each winss utility uses a common library *winss.lib* which is included
statically. Most of the components were designed to be used in a modular way
such as the [WaitMultiplexer]. The library is purposely single threaded with
ownership well defined. Each utility entry point will own the life cycle of the
components use the [WaitMultiplexer] to dispatch the events to the correct
component. The idea is that it makes the implementation more like a robust
state machine and no change of dead locks. The explicit ownership and no
malloc's helps alleviate problematic memory leaks and handle leaks that are
likely to occur.

Examples of how to use this lib can be found [here][bin]. To use it in your
application you will have to download from [github] or build it yourself using
[readme].

[WaitMultiplexer]: @ref winss::WaitMultiplexer "WaitMultiplexer"
[bin]: https://github.com/Morgan-Stanley/winss/tree/master/bin
[github]: https://github.com/Morgan-Stanley/winss/releases/latest "github"
[readme]: https://github.com/Morgan-Stanley/winss/blob/master/README.md "readme"
