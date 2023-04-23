# LibAFL-based Fuzzer for Viber's native library `liblinkparser.so`

Here you can find an example of a fuzzer implementation for the library `liblinkparser.so` of Rakuten Viber for Android and the harness.

## Building

1. Install the [Android NDK](https://developer.android.com/ndk)
2. Make sure that the paths to cross-compiler are listed in the `PATH` environment variable.
3. `rustup target add x86_64-linux-android`
4. `cargo build --release --target=x86_64-linux-android`

## Using

See `cargo run --release -- --help`.
