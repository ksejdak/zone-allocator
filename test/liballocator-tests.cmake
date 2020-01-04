set(LINUX_ARM_TOOLCHAIN_8_PATH      "/opt/toolchains/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf")
set(LINUX_ARM_TOOLCHAIN_9_PATH      "/opt/toolchains/gcc-arm-9.2-2019.12-x86_64-arm-none-linux-gnueabihf")
set(BAREMETAL_ARM_TOOLCHAIN_PATH    "/opt/toolchains/gcc-arm-none-eabi-9-2019-q4-major")

set(FREERTOS_VERSION                freertos-10.2.1)
set(FREERTOS_PORTABLE               ARM_CM4F)

if (${PLATFORM} STREQUAL freertos-arm)
    set(APP_C_FLAGS                 "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb" CACHE INTERNAL "")
    set(APP_CXX_FLAGS               "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb" CACHE INTERNAL "")
endif ()
