file(TO_CMAKE_PATH "$ENV{TM_KIT_HOME}" TM_KIT_HOME)
file(TO_CMAKE_PATH "$ENV{TM_KIT_DIR}" TM_KIT_DIR)

set(TMTransport_INCLUDE_SEARCH_PATHS
  ${TM_KIT_HOME}
  ${TM_KIT_HOME}/include
  ${TM_KIT_DIR}
  ${TM_KIT_DIR}/include
  /usr/include
  /usr/include/tm_kit
  /usr/local/include
  /usr/local/include/tm_kit
  /opt/tm_kit/include
)

set(TMTransport_LIB_SEARCH_PATHS
  ${TM_KIT_HOME}
  ${TM_KIT_HOME}/lib
  ${TM_KIT_DIR}
  ${TM_KIT_DIR}/lib
  /usr/lib
  /usr/lib/tm_kit
  /usr/local/lib
  /usr/local/lib/tm_kit
  /opt/tm_kit/lib
)

FIND_PATH(TMTransport_INCLUDE_DIR NAMES tm_kit/transport/MultiTransportRemoteFacility.hpp HINTS ${TMTransport_INCLUDE_SEARCH_PATHS})

set(TMTransport_NAMES tm_kit_transport tm_transport tm_transport.a tm_transport.lib)
set(TMTransport_NAMES_DEBUG tm_kit_transport_debug tm_transport_debug tm_transport_d)
if(NOT TMTransport_LIBRARY)
    find_library(TMTransport_LIBRARY_RELEASE NAMES ${TMTransport_NAMES} HINTS ${TMTransport_LIB_SEARCH_PATHS} NAMES_PER_DIR)
    find_library(TMTransport_LIBRARY_DEBUG NAMES ${TMTransport_NAMES_DEBUG} HINTS ${TMTransport_LIB_SEARCH_PATHS} NAMES_PER_DIR)
    include(SelectLibraryConfigurations)
    select_library_configurations(TMTransport)
    mark_as_advanced(TMTransport_LIBRARY_RELEASE TMTransport_LIBRARY_DEBUG)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TMTransport REQUIRED_VARS TMTransport_LIBRARY TMTransport_INCLUDE_DIR)