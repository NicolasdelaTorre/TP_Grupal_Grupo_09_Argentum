target_sources(taller_server
    PRIVATE
    # .cpp files
    aceptador.cpp
    client_handler.cpp
    gameloop.cpp
    main.cpp
    monitor_clientes.cpp
    protocolo_server.cpp
    receiver.cpp
    sender.cpp
    servidor.cpp
    PUBLIC
    # .h files
    aceptador.h
    client_handler.h
    gameloop.h
    monitor_clientes.h
    protocolo_server.h
    receiver.h
    sender.h
    servidor.h
    )
