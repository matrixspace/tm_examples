add_executable(serialization_test SerializationTest.cpp)
target_link_libraries(serialization_test ${TMInfra_LIBRARY} ${TMBasic_LIBRARY})
set_target_properties(serialization_test PROPERTIES FOLDER "serialization_test")

add_executable(proto_interop_test ProtoInteropTest.cpp)
target_link_libraries(proto_interop_test protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite ${TMInfra_LIBRARY} ${TMBasic_LIBRARY})
set_target_properties(proto_interop_test PROPERTIES FOLDER "serialization_test")