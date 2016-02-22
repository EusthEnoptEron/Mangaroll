LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include ../../../../../cflags.mk

LOCAL_MODULE			:= mangaroll
LOCAL_SRC_FILES			:= ../../../Src/Mangaroll.cpp \
                           ../../../Src/Page.cpp \
                           ../../../Src/PageCarousel.cpp \
                           ../../../Src/RemotePage.cpp\
                           ../../../Src/Helpers.cpp
LOCAL_STATIC_LIBRARIES	:= vrsound vrmodel vrlocale vrgui vrappframework systemutils libovrkernel
LOCAL_SHARED_LIBRARIES	:= vrapi

include $(BUILD_SHARED_LIBRARY)

$(call import-module,LibOVRKernel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/SystemUtils/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppFramework/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrGui/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrLocale/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrModel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrSound/Projects/AndroidPrebuilt/jni)