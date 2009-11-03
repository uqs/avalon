
ADD_EXECUTABLE(
	main
	main.cpp
	Motor.cpp
)

TARGET_LINK_LIBRARIES(main epos can-serial) 


