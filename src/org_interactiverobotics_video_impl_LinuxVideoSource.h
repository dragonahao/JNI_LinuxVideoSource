/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_interactiverobotics_video_impl_LinuxVideoSource */

#ifndef _Included_org_interactiverobotics_video_impl_LinuxVideoSource
#define _Included_org_interactiverobotics_video_impl_LinuxVideoSource
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_interactiverobotics_video_impl_LinuxVideoSource
 * Method:    nativeOpen
 * Signature: (Ljava/lang/String;III)I
 */
JNIEXPORT jint JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeOpen
  (JNIEnv *, jobject, jstring, jint, jint, jint);

/*
 * Class:     org_interactiverobotics_video_impl_LinuxVideoSource
 * Method:    nativeClose
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeClose
  (JNIEnv *, jobject, jint);

/*
 * Class:     org_interactiverobotics_video_impl_LinuxVideoSource
 * Method:    nativeReadFrame
 * Signature: (I)[B
 */
JNIEXPORT jbyteArray JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeReadFrame
  (JNIEnv *, jobject, jint);

#ifdef __cplusplus
}
#endif
#endif
