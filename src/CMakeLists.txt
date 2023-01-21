set(littlelog_SRCS 
    Buffer.cpp
    QueueBuffer.cpp
    Write_to_file.cpp
    LittleLogger.cpp
    LittleLog.cpp
    )

add_library(littlelog ${littlelog_SRCS})
target_link_libraries(littlelog pthread)

install(TARGETS littlelog DESTINATION lib)
file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/littlelog)


