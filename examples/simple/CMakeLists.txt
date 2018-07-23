file(GLOB SOURCE_FILES *.cpp ../../src/*.cpp)

set(LIBRARIES SPI)

include_directories(../../src)

arduino(lwstreams-example-simple "${SOURCE_FILES}" "${LIBRARIES}")

target_compile_options(lwstreams-example-simple PRIVATE -Wall -Werror)
