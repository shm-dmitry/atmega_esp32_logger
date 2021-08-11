# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c init/wifi.c init/init.c gpio/gpio_listener.c socket/socket_server.c")
set(COMPONENT_ADD_INCLUDEDIRS "")

register_component()
