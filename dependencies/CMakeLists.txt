include(FetchContent)

set(SFML_VERSION 3.0.0)
set(IMGUI_VERSION 1.91.1)

option(LINK_DEPS_STATIC CACHE ON)

# Don't build shared libs if we want to link to deps statically
if(LINK_DEPS_STATIC)
  set(BUILD_SHARED_LIBS OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()

# Get stripped down release zips
FetchContent_Declare(
 SFML
 URL "https://github.com/SFML/SFML/archive/${SFML_VERSION}.zip"
)

FetchContent_Declare(
 imgui
 URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
)

# imgui_sfml full repo
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        v3.0
)

# BUILDING

# SFML
option(SFML_BUILD_AUDIO "Build audio" OFF)
option(SFML_BUILD_NETWORK "Build network" OFF)
FetchContent_MakeAvailable(sfml)

# IMGUI
FetchContent_MakeAvailable(imgui)

# IMGUI-SFML
set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
option(IMGUI_SFML_IMGUI_DEMO "Build imgui_demo.cpp" ON)

FetchContent_MakeAvailable(imgui-sfml)

FetchContent_Declare( zeromq GIT_REPOSITORY https://github.com/zeromq/libzmq.git GIT_TAG        v4.3.5 )
FetchContent_Declare( cppzmq GIT_REPOSITORY https://github.com/zeromq/cppzmq.git GIT_TAG        v4.11.0 )
FetchContent_MakeAvailable(zeromq cppzmq)