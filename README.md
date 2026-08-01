# any-logger
Provides an abstract logger implementation to be used in C applications.

## License

This project is licensed under the GNU Lesser General Public License v3.0
or later (LGPL-3.0-or-later). See [LICENSE](LICENSE) for details.

## Project Notes

- Current planning lives in `docs/project_plan.md`.
- The local pthread smoke example lives in `examples/test_logger`.

## Tests

Run the dependency-free C17 unit tests with:

```sh
make -C tests test
```

The test target uses strict compiler warnings as errors. An UndefinedBehaviorSanitizer target is also available:

```sh
make -C tests ubsan
```

AddressSanitizer can be included with `make -C tests sanitizers` when supported by the host toolchain.

The buffer tests verify FIFO behavior, overwrite-oldest behavior, and reset. The logger tests verify channel registration/reuse and asynchronous enqueueing.
