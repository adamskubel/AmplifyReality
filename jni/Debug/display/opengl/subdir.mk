################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../display/opengl/OpenGLRenderer.cpp 

OBJS += \
./display/opengl/OpenGLRenderer.o 

CPP_DEPS += \
./display/opengl/OpenGLRenderer.d 


# Each subdirectory must supply rules for building sources it contributes
display/opengl/%.o: ../display/opengl/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C++ Compiler'
	g++ -I"C:\Android\android-ndk-r6b\platforms\android-9\arch-arm\usr\include" -I"C:\Android\android-ndk-r6b\sources\android\native_app_glue" -I"C:\Android\FinalProject\OpenCV\OpenCV-2.3.1\include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


