target_sources(taller_common
    PRIVATE
    # .cpp files
    liberror.cpp
    common_protocol.cpp
    protocol_util.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    PUBLIC
    # .h files
    liberror.h
    common_protocol.h
    position.h
    queue.h
    protocol_util.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    )
