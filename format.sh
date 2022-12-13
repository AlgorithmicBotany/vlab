 find . -type f -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" | xargs clang-format -style=file -i -fallback-style=none
