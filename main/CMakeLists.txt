set(srcs "main.c"
	     "connect_wifi.c"
         "DHT11.C"
         "HD44780.c")
idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ".")