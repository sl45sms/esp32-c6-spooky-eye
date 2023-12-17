set(srcs "main.c"
    "decode_jpg.c"
    "decode_gif.c"
    )

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS "."
                    EMBED_FILES image_logo_240x240.jpg blackeye.gif)
