# Basically just wrap the .csproj into something CMake can handle
# Inspired by https://github.com/novelrt/NovelRT/blob/master/src/CMakeLists.txt

set(SANITY_ENGINE_DOTNET_CSPROJ ${CMAKE_CURRENT_LIST_DIR}/SanityEngine.NET.csproj)

if("${CMAKE_BUILD_TYPE}" AND NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(SANITY_ENGINE_DOTNET_CONFIGURATION Release)
else()
    set(SANITY_ENGINE_DOTNET_CONFIGURATION Debug)
endif()

file(GLOB_RECURSE SANITY_ENGINE_DOTNET_INPUTS CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_LIST_DIR}/*.cs
    ${CMAKE_CURRENT_LIST_DIR}/SanityEngine.NET.runtimeconfig.json
  )
list(FILTER SANITY_ENGINE_DOTNET_INPUTS EXCLUDE REGEX SanityEngine\.NET/obj/.*)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SANITY_ENGINE_DOTNET_INPUTS})

set(SANITY_ENGINE_DOTNET_OUTPUTS
    ${CMAKE_CURRENT_BINARY_DIR}/SanityEngine.NET.txt
  )

add_custom_command(OUTPUT ${SANITY_ENGINE_DOTNET_OUTPUTS}
    COMMAND dotnet build ${SANITY_ENGINE_DOTNET_CSPROJ} /p:Configuration=${SANITY_ENGINE_DOTNET_CONFIGURATION} /p:OutDir=$<TARGET_FILE_DIR:SanityEngine>
    COMMAND ${CMAKE_COMMAND} -E touch ${SANITY_ENGINE_DOTNET_OUTPUTS}
    MAIN_DEPENDENCY ${SANITY_ENGINE_DOTNET_CSPROJ}
    DEPENDS ${SANITY_ENGINE_DOTNET_INPUTS}
  )

add_custom_target(SanityEngineDotNet ALL DEPENDS ${SANITY_ENGINE_DOTNET_OUTPUTS} SOURCES ${SANITY_ENGINE_DOTNET_INPUTS})

