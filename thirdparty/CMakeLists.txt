# Adding some hardcoded thirdparty libs
add_subdirectory(GSL)
include_directories(GSL/include)

add_subdirectory(glm)
include_directories(glm)

add_subdirectory(assimp)
include_directories(assimp)

# Disable warnings from thirdparty libs
target_compile_options(GSL INTERFACE -Wno-error)
target_compile_options(glm INTERFACE -Wno-error)
target_compile_options(assimp PRIVATE -Wno-error)
target_compile_options(assimp INTERFACE -Wno-error)
