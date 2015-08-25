live555 = "live555"
live555_modules = ["BasicUsageEnvironment", "groupsock", "liveMedia", "UsageEnvironment"]
live555_include_dirs = live555_modules
live555_libraries = ["lib" + library + ".a" for library in live555_modules]

x264 = "x264"
x264_include_dirs = ["."]
x264_library = "libx264.a"

def includes():
    return {x264 : x264_include_dirs,
            live555 : live555_include_dirs}

def libraries():
    return {x264 : [x264_library],
            live555 : live555_libraries}
