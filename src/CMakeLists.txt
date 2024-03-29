find_package(glm REQUIRED)
find_package(EnTT REQUIRED)
find_package(SDL2 REQUIRED)

set(avcore_HEADERS
    ../include/glad/glad.h
    ../include/KHR/khrplatform.h
    ../include/av/core/app.hpp
    ../include/av/core/input.hpp
    ../include/av/core/graphics/mesh.hpp
    ../include/av/core/graphics/shader.hpp
)

set(avcore_SOURCES
    glad.c
    core/app.cpp
    core/input.cpp
    core/graphics/mesh.cpp
    core/graphics/shader.cpp
)

set(avutil_HEADERS
    ../include/av/util/expr_traits.hpp
    ../include/av/util/log.hpp
    ../include/av/util/task_queue.hpp
    ../include/av/util/time.hpp
    ../include/av/util/graphics/color.hpp
)

set(avutil_SOURCES
    util/log.cpp
)

set(packer_HEADERS
    ../include/stb/stb_image.h
    ../include/stb/stb_image_write.h
)

set(packer_SOURCES
    packer/packer.cpp
)

foreach(SUBMODULE avcore avutil packer)
    add_library(${SUBMODULE} ${${SUBMODULE}_HEADERS} ${${SUBMODULE}_SOURCES})
    add_library(AVocado::${SUBMODULE} ALIAS ${SUBMODULE})

    target_compile_features(${SUBMODULE} PUBLIC cxx_std_17)
    target_include_directories(${SUBMODULE}
        PUBLIC
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    foreach(HEADER ${${SUBMODULE}_HEADERS})
        string(REGEX REPLACE "[^\\/]+$" "" HEADER_DIR ${HEADER})
        install(FILES ${HEADER}
            DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${HEADER_DIR}
        )
    endforeach()
endforeach()

target_link_libraries(avutil
    PUBLIC
        glm::glm EnTT::EnTT
)

target_link_libraries(avcore
    PUBLIC
        avutil
        SDL2::SDL2main SDL2::SDL2-static
)

target_compile_features(packer PUBLIC cxx_std_17)
