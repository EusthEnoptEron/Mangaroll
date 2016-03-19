LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ARM_MODE  := arm					# full speed arm instead of thumb
LOCAL_ARM_NEON  := true					# compile with neon support enabled
LOCAL_MODULE := jpeg
LOCAL_SRC_FILES := libjpeg.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

include ../../../../../cflags.mk

LOCAL_MODULE			:= mangaroll
LOCAL_SRC_FILES			:= ../../../Src/Mangaroll.cpp \
                           ../../../Src/Page.cpp \
                           ../../../Src/Manga.cpp \
                           ../../../Src/RemotePage.cpp\
                           ../../../Src/Helpers.cpp \
                           ../../../Src/MangaCarousel.cpp \
                           ../../../Src/MangaSettingsView.cpp \
                           ../../../Src/ShaderManager.cpp \
                           ../../../Src/View.cpp \
                           ../../../Src/ViewManager.cpp \
                           ../../../Src/MangaSelectionView.cpp \
                           ../../../Src/AsyncTexture.cpp \
                           ../../../Src/OVR_TurboJpeg.cpp \
                           ../../../Src/MangaProvider.cpp \
                           ../../../Src/AnimationManager.cpp \
                           ../../../Src/UICheckbox.cpp \
                           ../../../Src/Config.cpp \
                           ../../../Src/Web.cpp

LOCAL_STATIC_LIBRARIES	:= vrsound vrmodel vrlocale vrgui vrappframework systemutils libovrkernel jpeg stbs
LOCAL_SHARED_LIBRARIES	:= vrapi
LOCAL_STATIC_LIBRARIES  += vrcapture

include $(BUILD_SHARED_LIBRARY)

$(call import-module,LibOVRKernel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/SystemUtils/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppFramework/Projects/AndroidPrebuilt/jni)
# $(call import-module,VrAppSupport/VrGui/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrLocale/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrModel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrSound/Projects/AndroidPrebuilt/jni)

include ../../../../../VrCapture/Projects/Android/jni/Android.mk
include ../../../../../VrAppSupport/VrGUI/Projects/Android/jni/Android.mk