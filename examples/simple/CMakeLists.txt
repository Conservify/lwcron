file(GLOB SOURCE_FILES *.cpp ../../src/*.cpp)

set(LIBRARIES SPI)

include_directories(../../src)

arduino(lwcron-example-simple "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(lwcron-example-simple PRIVATE -Wall -Werror)
