cmake_minimum_required(VERSION 3.12)

project(Project)

include(../../CMake/CommonMacros.txt)

SETUP_APP(Ch2_02_GLM "Chapter 02")

target_link_libraries(Ch2_02_GLM glad glfw)