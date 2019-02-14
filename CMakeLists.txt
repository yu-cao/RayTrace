cmake_minimum_required(VERSION 3.13)
project(RayTrace)

set(CMAKE_CXX_STANDARD 17)

add_executable(RayTrace src/main.cpp src/vec3.h src/rays.h src/hitable.h src/sphere.h src/hitable_list.h src/camera.h src/material.h src/aabb.h src/moving_sphere.h src/bvh.h)