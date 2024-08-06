cmake_minimum_required(VERSION 3.12)

project(Project)

include(../../CMake/CommonMacros.txt)

SETUP_APP(Ch2_01_GLFW "Chapter 02")

target_link_libraries(Ch2_01_GLFW glad glfw)