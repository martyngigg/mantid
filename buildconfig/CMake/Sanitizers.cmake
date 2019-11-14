#################################################
# Controls various project wide sanitizer options
#################################################

set(USE_SANITIZER "Off" CACHE STRING "Sanitizer mode to enable")
set_property(CACHE USE_SANITIZER PROPERTY STRINGS
             Off Address Address+Leak Leak Memory Thread Undefined)

if(NOT ${USE_SANITIZER} MATCHES "Off")
    if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
        # Version 13 is needed for add_link_options, but not all platforms
        # currently have it
        message(FATAL_ERROR "CMake 3.13 onwards is required to run the sanitizers")
    endif()

    if(WIN32)
        message(FATAL_ERROR "Windows does not support sanitizers")
    endif()

    # Check and warn if we are not in a mode without debug symbols
    string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
    if(${build_type_lower} MATCHES "release" OR ${build_type_lower} MATCHES "minsizerel" )
        message(WARNING "You are running address sanitizers without debug information, try RelWithDebInfo")

    elseif(${build_type_lower} MATCHES "relwithdebinfo")
        # RelWithDebug runs with -o2 which performs inlining reducing the usefulness
        message("Replacing -O2 flag with -O1 to preserve stack trace on sanitizers")
        string(REPLACE "-O2" "-O1" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
        string(REPLACE "-O2" "-O1" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO})

        set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)
        set(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "" FORCE)

        add_compile_options(-fno-omit-frame-pointer -fno-optimize-sibling-calls)
    endif()

    # Allow all instrumented code to continue beyond the first error
    add_compile_options(-fsanitize-recover=all)

    # N.b. we can switch this to add_link_options in CMake 3.13
    # rather than have to check the linker options etc
    if (USE_SANITIZER STREQUAL "Address")
        message(STATUS "Enabling address sanitizer")

        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)

    elseif (USE_SANITIZER STREQUAL "Address+Leak")
        message(STATUS "Enabling address and leak sanitizer")

        add_compile_options(-fsanitize=address,leak)
        add_link_options(-fsanitize=address,leak)

    elseif (USE_SANITIZER STREQUAL "Leak")
        message(STATUS "Enabling leak sanitizer")

        add_compile_options(-fsanitize=leak)
        add_link_options(-fsanitize=leak)

    elseif (USE_SANITIZER STREQUAL "Memory")
        # Requires Clang > 10 and libc++ (rather than libstdc++)
        # so we will wire up later
        message(FATAL_ERROR "Not Enabled Yet")
        message(STATUS "Enabling Memory sanitizer")

    elseif (USE_SANITIZER STREQUAL "Thread")
        message(STATUS "Enabling Thread sanitizer")

        add_compile_options(-fsanitize=thread)
        add_link_options(-fsanitize=thread)

    elseif (USE_SANITIZER STREQUAL "Undefined")
        message(STATUS "Enabling undefined behaviour sanitizer")

        add_compile_options(
            -fsanitize=undefined
            # RTTI information is not exported for some classes causing the
            # linker to fail whilst adding vptr instrumentation
            -fno-sanitize=vptr
        )
        add_link_options(-fsanitize=undefined)
    else()
        message(FATAL_ERROR "Unrecognised Sanitizer Option")
    endif()

endif()
