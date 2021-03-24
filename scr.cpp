#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/kd.h>
#include <linux/fb.h>
#include "fbdev.h"
#include "font.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static fb_fix_screeninfo finfo;
static fb_var_screeninfo vinfo;

static s32 fbdev_fd;

FbDev *FbDev::initFbDev()
{
	s8 *fbdev = getenv("FRAMEBUFFER");

	if (fbdev) {
		fbdev_fd = open(fbdev, O_RDWR);
	} else {
		fbdev_fd = open("/dev/fb0", O_RDWR);
		if (fbdev_fd < 0) fbdev_fd = open("/dev/fb/0", O_RDWR);
	}

	if (fbdev_fd < 0) {
		fprintf(stderr, "can't open frame buffer device!\n");
		return 0;
	}

	fcntl(fbdev_fd, F_SETFD, fcntl(fbdev_fd, F_GETFD) | FD_CLOEXEC);
	ioctl(fbdev_fd, FBIOGET_FSCREENINFO, &finfo);
	ioctl(fbdev_fd, FBIOGET_VSCREENINFO, &vinfo);

	if (finfo.type != FB_TYPE_PACKED_PIXELS) {
		fprintf(stderr, "unsupported frame buffer device!\n");
		return 0;
	}

	switch (vinfo.bits_per_pixel) {
	case 8:
		if (finfo.visual != FB_VISUAL_PSEUDOCOLOR) {
			fprintf(stderr, "only support pseudo-color visual with 8bpp depth!\n");
			return 0;
		}
		break;

	case 15:
	case 16:
	case 32:
		if (finfo.visual != FB_VISUAL_TRUECOLOR && finfo.visual != FB_VISUAL_DIRECTCOLOR) {
			fprintf(stderr, "only support true-color/direct-color visual with 15/16/32bpp depth!\n");
			return 0;
		}
		break;

	default:
		fprintf(stderr, "only support frame buffer device with 8/15/16/32 color depth!\n");
		return 0;
	}

	return new FbDev();
}

FbDev::FbDev()
{
	mWidth = vinfo.xres;
	mHeight = vinfo.yres;
	mBitsPerPixel = vinfo.bits_per_pixel;
	mBytesPerLine = finfo.line_length;
	mVMemBase = (u8 *)mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev_fd, 0);


	if (mRotateType == Rotate0 || mRotateType == Rotate180) {
		bool ypan = (vinfo.yres_virtual > vinfo.yres && finfo.ypanstep && !(FH(1) % finfo.ypanstep));
		bool ywrap = (finfo.ywrapstep && !(FH(1) % finfo.ywrapstep));
		if (ywrap && !(vinfo.vmode & FB_VMODE_YWRAP)) {
			vinfo.vmode |= FB_VMODE_YWRAP;
			ioctl(fbdev_fd, FBIOPUT_VSCREENINFO, &vinfo);
			ywrap = (vinfo.vmode & FB_VMODE_YWRAP);
		}

		if ((ypan || ywrap) && !ioctl(fbdev_fd, FBIOPAN_DISPLAY, &vinfo)) {
			if (ywrap) {
				mScrollType = YWrap;
				mOffsetMax = vinfo.yres_virtual - 1;
			} else {
				mScrollType = YPan;
				mOffsetMax = vinfo.yres_virtual - vinfo.yres;
			}
		}
	}
}

FbDev::~FbDev()
{
	munmap(mVMemBase, finfo.smem_len);
	close(fbdev_fd);

	if (mScrollType != Redraw) {
		ioctl(STDIN_FILENO, KDSETMODE, KD_GRAPHICS);
		ioctl(STDIN_FILENO, KDSETMODE, KD_TEXT);
	}
}

const s8 *FbDev::drvId()
{
	return finfo.id;
}