cmake_minimum_required(VERSION 3.29)
project(Lab1_3_OpenGLatHome)

set(CMAKE_CXX_STANDARD 20)

add_executable(Lab1_3_OpenGLatHome main.cpp
        libs/tgaimage.cpp
        camera.cpp
        shader.cpp
        texture.cpp
        light.cpp
        geometry.h
        model.h
        model.cpp
        shadertoy.h)
