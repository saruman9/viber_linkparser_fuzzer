# LibAFL-based Fuzzer for Viber's native library `liblinkparser.so`

Here you can find an example of a fuzzer implementation for the library `liblinkparser.so` of Rakuten Viber for Android and the harness.

## Building

1. Install the [Android NDK](https://developer.android.com/ndk)
2. Make sure that the paths to cross-compiler are listed in the `PATH` environment variable.
3. `rustup target add x86_64-linux-android`
4. `cargo build --release --target=x86_64-linux-android`
5. Copy the shared dependency libraries to the `lib` directory.
6. Build the harness:
   - For fuzzing:
   ```console
   $ cmake -B build -S . -DANDROID_PLATFORM=${YOUR_ANDROID_PLATFORM_NUMBER_HERE} \
      -DCMAKE_TOOLCHAIN_FILE=${SPECIFIC_ANDROID_NDK_TOOLCHAIN_PATH_HERE}/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=x86_64
   $ cmake --build build
   ```

   - For triaging and debugging:
   ```console
   $ cmake -B build -S . -DTRIAGE -DANDROID_PLATFORM=${YOUR_ANDROID_PLATFORM_NUMBER_HERE} \
      -DCMAKE_TOOLCHAIN_FILE=${SPECIFIC_ANDROID_NDK_TOOLCHAIN_PATH_HERE}/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=x86_64
   $ cmake --build build
   ```
7. Copy everything to the device or to the emulator:

```shell
adb push ./corpus ./lib/* ./build/*harness* ./target/x86_64-linux-android/release/frida_fuzzer /data/local/tmp
```

## Using

See `cargo run -- --help`.

For example,

```shell
./frida_fuzzer -c 0-6 -A -H ./libharness.so --asan-cores=0,1 -F fuzz -l ./libharness.so -l ./liblinkparser.so -l ./libicuBinder.so -C --cmplog-cores=2,3 -d
```

or debug mode:

```shell
RUST_BACKTRACE=1 LIBAFL_DEBUG_OUTPUT=1 RUST_LOG=debug ./frida_fuzzer -H ./libharness.so -F fuzz -l ./libharness.so -l ./liblinkparser.so -l ./libicuBinder.so
```
