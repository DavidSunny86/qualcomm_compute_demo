# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#set(CMAKE_C_COMPILER E:/libs/ARMCompiler6.12/bin/armclang.exe)
#set(CMAKE_CXX_COMPILER E:/libs/ARMCompiler6.12/bin/armclang.exe)

# set(CMAKE_C_COMPILER E:/libs/android-ndk-r16b/toolchains/llvm-Snapdragon_LLVM_for_Android_6.0/prebuilt/windows-x86_64/bin/clang.exe)
# set(CMAKE_CXX_COMPILER E:/libs/android-ndk-r16b/toolchains/llvm-Snapdragon_LLVM_for_Android_6.0/prebuilt/windows-x86_64/bin/clang++.exe)

# set(CMAKE_C_LINK_EXECUTABLE E:/libs/android-ndk-r16b/toolchains/llvm-Snapdragon_LLVM_for_Android_6.0/prebuilt/windows-x86_64/bin/clang-cl.exe)
# set(CMAKE_CXX_LINK_EXECUTABLE  E:/libs/android-ndk-r16b/toolchains/llvm-Snapdragon_LLVM_for_Android_6.0/prebuilt/windows-x86_64/bin/clang-cl.exe)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.


#add_library( # Sets the name of the library.
#        native-lib
#        # Sets the library as a shared library.
#        SHARED
#        # Provides a relative path to your source file(s).
#        native-lib.cpp)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

#add_library(crashpad SHARED IMPORTED)

add_library(aga SHARED IMPORTED)
add_library(mgd SHARED IMPORTED)
add_library(hetCompute-1.0.0 SHARED IMPORTED)
add_library(hetcompute_dsp SHARED IMPORTED)
add_library(hetcompute_dsp_skel SHARED IMPORTED)
#set_property(TARGET crashpad PROPERTY IMPORTED_LOCATION "E:/export/TetsNdk/app/src/main/jniLibs/${ANDROID_ABI}/libcrashpad_snapshot_test_module.so")
set_property(TARGET aga PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libAGA.so")
set_property(TARGET mgd PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libMGD.so")
set_property(TARGET hetCompute-1.0.0 PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libhetCompute-1.0.0.so")
set_property(TARGET hetcompute_dsp PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libhetcompute_dsp.so")
set_property(TARGET hetcompute_dsp_skel PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libhetcompute_dsp_skel.so")

set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS} -DHETCOMPUTE_DISABLE_EXCEPTIONS
 -DHAVE_CONFIG_H=1 -DHAVE_ANDROID_LOG_H=1
 -DHETCOMPUTE_HAVE_RTTI=1 DHETCOMPUTE_HAVE_OPENCL=1
 -DHETCOMPUTE_HAVE_GPU=1 -DHETCOMPUTE_HAVE_GLES=1
 -DHETCOMPUTE_HAVE_QTI_DSP=1
 DHETCOMPUTE_THROW_ON_API_ASSERT=1
 -DHETCOMPUTE_LOG_FIRE_EVENT=1
")


# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -pthread -std=c++11 -stdlib=libstdc++ -fexceptions  -frtti -Ofast -Osize  -DHETCOMPUTE_HAVE_QTI_DSP=1") # -fmerge-functions  -mfpu=neon  -std=gnu++11  -stdlib=libstdc++

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# -Wall -Werror

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.

#${CMAKE_ANDROID_ARCH_ABI}

if(${ANDROID_ABI} EQUAL "arm64-v8a")
    set(CMAKE_SHARED_LINKER_FLAGS #
            "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate   -Wl,--allow-shlib-undefined")
else()
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate   -Wl,--allow-shlib-undefined ")
endif()

add_library(native-activity SHARED main.cpp)

target_include_directories(native-activity PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue)
target_include_directories(native-activity PRIVATE
#        ${PROJECT_SOURCE_DIR}/main/cpp/include
        ${PROJECT_SOURCE_DIR}/include
)


target_link_libraries( # Specifies the target library.
        native-activity
        #native-lib
        native_app_glue
#        crashpad
        #aga
        #mgd
        EGL
        GLESv3
        GLESv1_CM
        android
        hetCompute-1.0.0
        hetcompute_dsp
        #hetcompute_dsp_skel
        log

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})