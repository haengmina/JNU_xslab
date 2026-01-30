
- # How to Build
    - ### Release Mode Build
        ```shell
        - $ cmake -S . -B build -G Ninja
        - $ ninja -C build -j$(nproc)
        ```

    - ### Debug Mode Build
        ```shell
        - $ cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
        - $ ninja -C build -j$(nproc)
        ```

