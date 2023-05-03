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
6. `adb push ./target/x86_64-linux-android/release/frida_fuzzer harness.so /data/loca/tmp`

## Using

See `cargo run -- --help`.

For example,

```
./frida_fuzzer -c 6 -A -H ./harness.so --asan-cores=1,2 -F fuzz -l ./harness.so -l ./liblinkparser.so -l libicuBinder.so -C --cmplog-cores=3,4 -d
```
