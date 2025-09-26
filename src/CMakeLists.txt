add_executable(cppzmq_chat
 main.cpp
)

target_link_libraries(cppzmq_chat
 PUBLIC
  ImGui-SFML::ImGui-SFML
)

# Do we need this?
# include(Install.cmake)
