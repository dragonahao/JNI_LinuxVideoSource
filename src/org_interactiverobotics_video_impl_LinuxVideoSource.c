/*
 * org_interactiverobotics_video_linux_LinuxVideoSource.c
 *
 * Copyright (C) 2014  Pavel Prokhorov (pavelvpster@gmail.com)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "org_interactiverobotics_video_impl_LinuxVideoSource.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <linux/videodev2.h>
#include <libv4l2.h>


//
// V4L
//


static int xioctl(int fd, int requect, void* argp) {
	
	int r;
	
	do {
		
		r = v4l2_ioctl(fd, requect, argp);
		
	} while (-1 == r && EINTR == errno);
	
	return r;
}


/**
 * Эта функция открывает устройство.
 * 
 * @param deviceName имя устройства.
 * 
 * @return int описатель устройства.
 * 
 * В случае ошибки функция возвращает -1.
 * 
 */
static int openDevice(const char* deviceName) {

	fprintf(stdout, "Open video device...\n");
		
	// Получаем описание устройства
	
	struct stat st;
	
	if (-1 == stat(deviceName, &st)) {
		
		fprintf(stderr, "Unable to get device stat!\n");
		
		return -1;
	}
	
	// Проверяем, что это устройство, а не файл или директория
	
	if (!S_ISCHR(st.st_mode)) {
		
		fprintf(stderr, "Wrong device type!\n");
		
		return -1;
	}
	
	int r = v4l2_open(deviceName, O_RDWR | O_NONBLOCK, 0);
	
	if (-1 == r) {
		
		fprintf(stderr, "Open error!\n");
		
	} else {
		
		fprintf(stdout, "Done. Video device opened.\n");
	}
	
	return r;
}


/**
 * Эта функция проверяет возможности устройства.
 * 
 * @param fd описатель устройства.
 * 
 * @return int
 * 
 * Если необходимые возможности поддерживаются, функция возвращает 0;
 * в противном случае функция возвращает -1.
 * 
 */
static int checkCaps(int fd) {
	
	fprintf(stdout, "Check video device capabilities...\n");
		
	// Проверяем возможность захвата видео
	
	struct v4l2_capability cap;
	
	bzero(&cap, sizeof(cap));
	
	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
		
		fprintf(stderr, "Capabilities query error!\n");
		
		return -1;
	}
	
	if (0 == (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		
		fprintf(stderr, "Capture does not supported!\n");
		
		return -1;
	}
	
	fprintf(stdout, "Capture supported.\n");
	
	// Проверяем возможность чтения системной функцией read()
	
	if (0 == (cap.capabilities & V4L2_CAP_READWRITE)) {
		
		fprintf(stderr, "Read/Write does not supported!\n");
		
		return -1;
	}
	
	fprintf(stdout, "Read/Write supported.\n");
	
/*	// Проверяем возможность обрезки изображения (crop)
	
	struct v4l2_cropcap cropcap;
	
	bzero(&cropcap, sizeof(cropcap));
	
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
		
		struct v4l2_crop crop;
		
		bzero(&crop, sizeof(crop));
		
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		
		crop.c = cropcap.defrect;
		
		if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
			
			fprintf(stderr, "Crop does not supported!\n");
			
			return -1;
		}
	}
	
	fprintf(stdout, "Crop supported.\n");
*/	
	fprintf(stdout, "Done.\n");
	
	return 0;
}


/**
 * Эта функция устанавливает формат кадра JPEG.
 * 
 * @param fd описатель устройства,
 * 
 * @param width  ширина изображения,
 * @param height высота изображения.
 * 
 * @return int
 * 
 * Если формат кадра установлен, функция возвращает 0;
 * в противном случае функция возвращает -1.
 * 
 */
static int setJpegFormat(int fd, int width, int height) {

	fprintf(stdout, "Set JPEG format...\n");
	
	struct v4l2_format format;
	
	bzero(&format, sizeof(format));
	
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	format.fmt.pix.width       = width;
	format.fmt.pix.height      = height;
	format.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	
	if (-1 == xioctl(fd, VIDIOC_S_FMT, &format)) {
		
		fprintf(stderr, "Set format error!\n");
		
		return -1;
	}

	// Проверяем, что формат JPEG установился
	
	if (format.fmt.pix.pixelformat != V4L2_PIX_FMT_JPEG) {
		
		fprintf(stderr, "JPEG does not supported!\n");
		
		return -1;
	}
	
	fprintf(stdout, "Done.\n");
	
	return 0;
}


/**
 * Эта функция устанавливает формат кадра MJPEG.
 * 
 * @param fd описатель устройства,
 * 
 * @param width  ширина изображения,
 * @param height высота изображения.
 * 
 * @return int
 * 
 * Если формат кадра установлен, функция возвращает 0;
 * в противном случае функция возвращает -1.
 * 
 */
static int setMjpegFormat(int fd, int width, int height) {

	fprintf(stdout, "Set MJPEG format...\n");
	
	struct v4l2_format format;
	
	bzero(&format, sizeof(format));
	
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	format.fmt.pix.width       = width;
	format.fmt.pix.height      = height;
	format.fmt.pix.field       = V4L2_FIELD_INTERLACED;
	format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	
	if (-1 == xioctl(fd, VIDIOC_S_FMT, &format)) {
		
		fprintf(stderr, "Set format error!\n");
		
		return -1;
	}

	// Проверяем, что формат MJPEG установился
	
	if (format.fmt.pix.pixelformat != V4L2_PIX_FMT_MJPEG) {
		
		fprintf(stderr, "MJPEG does not supported!\n");
		
		return -1;
	}
	
	fprintf(stdout, "Done.\n");
	
	return 0;
}


/**
 * Эта функция устанавливает количество кадров в секунду.
 * 
 * @param fd описатель устройства,
 * 
 * @param fps количество кадров в секунду.
 * 
 * @return int
 * 
 * Если количество кадров в секунду установлено, функция возвращает 0;
 * в противном случае функция возвращает -1.
 * 
 */
static int setFps(int fd, int fps) {

	fprintf(stdout, "Set FPS = %d...\n", fps);
	
	struct v4l2_streamparm parm;
	
	bzero(&parm, sizeof(parm));
	
	parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	parm.parm.capture.timeperframe.numerator   = 1;
	parm.parm.capture.timeperframe.denominator = fps;

	int r = xioctl(fd, VIDIOC_S_PARM, &parm);
	
	if (-1 == r) {
		
		fprintf(stderr, "Parameter set error!\n");
		
	} else {

		fprintf(stdout, "Done.\n");
	}

	return r;
}


/**
 * Эта функция закрывает устройство.
 * 
 * @param fd описатель устройства.
 * 
 */
static void closeDevice(int fd) {
	
	v4l2_close(fd);
	
	fprintf(stdout, "Video device closed.\n");
}


/**
 * Эта функция читает кадр используя системную функцию read().
 * 
 * @param fd описатель устройства,
 * 
 * @param buf буфер,
 * 
 * @param len размер буфера.
 * 
 * @return int количество прочитанных байт.
 * 
 * Если прочитать кадр не удалось, функция возвращает -1.
 * 
 */
static int readFrame(int fd, void* buf, int len) {

	// Ожидаем кадра
	
	while (1) {
		
		fd_set fds;

		FD_ZERO(&fds);

		FD_SET(fd, &fds);

		struct timeval timeout;

		timeout.tv_sec  = 1;
		timeout.tv_usec = 0;

		int r = select(fd + 1, &fds, NULL, NULL, &timeout);

		if (-1 == r) {

			if (EINTR == errno) {

				continue ;

			} else {

				fprintf(stderr, "Frame read error!\n");
				
				return -1;
			}
			
		} else {
			
			break;
		}
	}

	// Читаем данные
	
	return v4l2_read(fd, buf, len);
}


//
// JNI
//


JNIEXPORT jint JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeOpen(JNIEnv* env, jobject object, jstring deviceName, jint width, jint height, jint fps) {
	
	// Открываем устройство
	
	const char* t = (*env)->GetStringUTFChars(env, deviceName, 0);
	
	int fd = openDevice(t);
	
	(*env)->ReleaseStringUTFChars(env, deviceName, t);
	
	if (-1 == fd) {
		
		return -1;
	}
	
	// Проверяем возможности устройства
	
	if (-1 == checkCaps(fd)) {
		
		closeDevice(fd);
		
		return -1;
	}
	
	// Устанавливаем формат JPEG
	
	if (-1 == setJpegFormat(fd, width, height)) {
		
		// MJPEG нам тоже подойдет
		
		if (-1 == setMjpegFormat(fd, width, height)) {
			
			// Устройство не поддерживает наши форматы

			closeDevice(fd);

			return -1;
		}
	}
	
	// Устанавливаем количество кадров в секунду
	
	if (-1 == setFps(fd, fps)) {
		
		closeDevice(fd);
		
		return -1;
	}

	// Готово
	
	return fd;
}


JNIEXPORT void JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeClose(JNIEnv* env, jobject object, jint fd) {
	
	closeDevice(fd);
}


JNIEXPORT jbyteArray JNICALL Java_org_interactiverobotics_video_impl_LinuxVideoSource_nativeReadFrame(JNIEnv* env, jobject object, jint fd) {
	
	// Создаем буфер
	
	void* buf = malloc(65535);
	
	// Читаем кадр
	
	int r = readFrame(fd, buf, 65535);
	
	if (-1 == r) {
		
		free(buf);
		
		return NULL;
	}

	// Создаем массив Java
	
	jbyteArray frameData;
	
	frameData = (*env)->NewByteArray(env, r);
	
	if (frameData == NULL) {
		
		free(buf);
	
		return NULL;
	}
	
	// Копируем туда данные
	
	(*env)->SetByteArrayRegion(env, frameData, 0, r, buf);
	
	free(buf);
	
	return frameData;
}
