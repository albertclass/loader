find_package(nlohmann_json CONFIG REQUIRED)
aux_source_directory(. LOADER_SOURCE_FILES)

file(GLOB LOADER_HEADER_FILES *.h)
file(GLOB LOADER_RESOURCE_FILES *.rc)

source_group("Header Files" FILES ${LOADER_HEADER_FILES})
source_group("Source Files" FILES ${LOADER_SOURCE_FILES})
source_group("Resource Files" FILES ${LOADER_RESOURCE_FILES})

# add the executable
add_executable(loader WIN32 ${LOADER_HEADER_FILES} ${LOADER_SOURCE_FILES} ${LOADER_RESOURCE_FILES})
set_target_properties(loader
	PROPERTIES
		CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
)

target_link_libraries(loader PRIVATE nlohmann_json nlohmann_json::nlohmann_json)
