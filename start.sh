#!/bin/bash

watchexec -e cpp,hpp -r "cmake --build build && ./build/vendbunny_backend"
