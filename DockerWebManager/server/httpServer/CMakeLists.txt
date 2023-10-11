find_package(Qt6 REQUIRED COMPONENTS httpServer)
target_link_libraries(mytarget PRIVATE Qt6::HttpServer)