# LibAFL-based Fuzzer for Viber's native library `liblinkparser.so`

Here you can find an example of a fuzzer implementation for the library `liblinkparser.so` of Rakuten Viber for Android and the harness.

## Building

1. Install the [Android NDK](https://developer.android.com/ndk)
2. Make sure that the paths to cross-compiler are listed in the `PATH` environment variable.
3. `rustup target add x86_64-linux-android`
4. `cargo build --release --target=x86_64-linux-android`
5. Build the harness:
   - `x86_64-linux-android-clang -O3 harness.c -shared -o harness.so` (for fuzzing)
   - `x86_64-linux-android-clang -DTRIAGE -O0 harness.c -o harness` (for triaging and debugging)
6. Libraries: `adb push libc++_shared.so libicuBinder.so liblinkparser.so /data/local/tmp`
7. The fuzzer: `adb push ./target/x86_64-linux-android/release/frida_fuzzer harness.so /data/local/tmp`
8. The corpus: `adb push ./corpus /data/local/tmp`

## Using

See `cargo run -- --help`.

For example,

```shell
./frida_fuzzer -c 0-6 -A -H ./harness.so --asan-cores=0,1 -F fuzz -l ./harness.so -l ./liblinkparser.so -l ./libicuBinder.so -C --cmplog-cores=2,3 -d
```

or debug mode:

```shell
COLORBT_SHOW_HIDDEN=1 RUST_BACKTRACE=full LD_LIBRARY_PATH=. LIBAFL_DEBUG_OUTPUT=1 ./frida_fuzzer -H ./harness.so -F fuzz -l ./harness.so -l ./liblinkparser.so -l ./libicuBinder.so
```
