set(COMPONENT_SRCS
  driver/atomvm_esp32cam.c
  )

set(COMPONENT_ADD_INCLUDEDIRS
  driver/include
  )

set(COMPONENT_REQUIRES driver)

register_component()
