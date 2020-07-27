set(COMPONENT_SRCS
  driver/esp32cam_driver.c
  )

set(COMPONENT_ADD_INCLUDEDIRS
  driver/include
  )

set(COMPONENT_REQUIRES driver)

register_component()
