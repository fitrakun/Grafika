#include <linux/fb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <math.h>

#define PI 3.14159265

typedef int bool;
#define true 1
#define false 0

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

int fb_fd;
long screensize;
uint8_t *fbp;

int width;
int height;
int block = 1;

inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo *vinfo)
{
    return (r<<vinfo->red.offset) | (g<<vinfo->green.offset) | (b<<vinfo->blue.offset);
}

void clear(){

    int x,y;
    /*for (x=0;x<vinfo.xres;x++){
        for (y=0;y<vinfo.yres;y++)*/
    for (x=0;x<width;x++){
        for (y=0;y<height;y++)
        {
            long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
            *((uint32_t*)(fbp + location)) = pixel_color(0xFF,0x5F,0x00, &vinfo);
        }
    }
}

void colorLine(int i, int j){

    int x,y;
    for (x=i*block;x<i*block+block;x++){
        for (y=j*block;y<j*block+block;y++)
        {
            if((x<=width && x>=0) && (y<=height && y>=0)){
                long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
                *((uint32_t*)(fbp + location)) = pixel_color(0xFF,0xFF,0xFF, &vinfo);
            }
        }
    }
}

void drawLine(int x1, int y1, int x2, int y2){
    int i, j;
    int cur_x = x1;
    int cur_y = y1;
    if (x1 == x2) {
        while(cur_y != y2){
            i = cur_x;
            j = height/block - cur_y;
            colorLine(i,j);
            cur_y++;
        }
    }
    else 
    if (y1 == y2) {
        while(cur_x != x2){
            i = cur_x;
            j = height/block - cur_y;
            colorLine(i,j);
            cur_x++;
        }
    }else {
        int dx = x2 - x1;
        if(dx < 0){
            dx = -dx;
        }
        int dy = y2 - y1;
        if(dy < dx){
            int pk = (2 * dy) - dx;
            int cur_x = x1;
            int cur_y = y1;
            i = cur_x;
            j = height/block - cur_y;
            colorLine(i,j);
            while((cur_x != x2) && (cur_y != y2)){
                if (pk < 0) {
                    if(x2 < x1){
                        cur_x--;
                    } else {
                        cur_x++;
                    }
                } else {
                    if(x2 < x1){
                        cur_x--;
                    } else {
                        cur_x++;
                    }
                    cur_y++;
                }
                i = cur_x;
                j = height/block - cur_y;
                colorLine(i,j);
                int a = 2 * dy;
                int b = (2 * dy) - (2 * dx);
                if (pk >= 0){
                    pk += b;
                } else {
                    pk += a;
                }
            }
        } else {
            int pk = (2 * dx) - dy;
            int cur_x = x1;
            int cur_y = y1;
            i = cur_x;
            j = height/block - cur_y;
            colorLine(i,j);
            while((cur_x != x2) && (cur_y != y2)){
                if (pk < 0) {
                    cur_y++;

                } else {
                    if(x2 < x1){
                        cur_x--;
                    } else {
                        cur_x++;
                    }
                    cur_y++;
                }
                i = cur_x;
                j = height/block - cur_y;
                colorLine(i,j);
                int a = 2 * dx;
                int b = (2 * dx) - (2 * dy);
                if (pk >= 0){
                    pk += b;
                } else {
                    pk += a;
                }
            }
        }

    }
}
void start()
{
    fb_fd = open("/dev/fb0",O_RDWR);

    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.grayscale=0;
    vinfo.bits_per_pixel=32;
    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);

    screensize = vinfo.yres_virtual * finfo.line_length;

    width = vinfo.xres-1;
    height = vinfo.yres-1;

    fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
}

void font_F(int offset_x, int offset_y, int size)
{
    int x = offset_x;
    int y = offset_y;
    int x1 = offset_x+(size/5);
    int y1 = offset_y-size;
    int x2 = offset_x+(size/2);
    int y2 = y1+(size/5);
    drawLine(x,y,x1+1,y);// -
    drawLine(x,y1,x,y); // |
    drawLine(x,y1,x2,y1); // _
    drawLine(x2,y1,x2,y2); // |
    drawLine(x1,y2,x2+1,y2); // -
    drawLine(x1,y2,x1,y); // |
}

int main()
{
    start();
    clear();
    font_F(600,600,500);
    return 0;
}
