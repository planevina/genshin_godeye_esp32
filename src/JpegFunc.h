#ifndef _JPEGFUNC_H_
#define _JPEGFUNC_H_

#include <JPEGDEC.h>

static JPEGDEC _jpeg;

static void jpegDraw(
    File &file, JPEG_DRAW_CALLBACK *jpegDrawCallback, bool useBigEndian,
    int x, int y, int widthLimit, int heightLimit)
{
    int _x; //实际x偏差
    int _y; //实际y偏差
    int _scale; //缩小倍率
    int iMaxMCUs; //切分块数
    _jpeg.open(file,jpegDrawCallback);
    //float ratio = (float)_jpeg.getWidth() / widthLimit; //以宽度缩放
    float ratio = (float)_jpeg.getHeight() / heightLimit; //以高度缩放
    if (ratio <= 1.2)
    {
        _scale = 0;
        iMaxMCUs = widthLimit / 16;
        _x = x + (widthLimit - _jpeg.getWidth()) / 2;
        _y = y + (heightLimit - _jpeg.getHeight()) / 2;
        //printf("[JPG] xoff:%d,yoff:%d,scale (%d) \n",_x,_y,_scale);
    }
    else if (ratio <= 2.4)
    {
        _scale = JPEG_SCALE_HALF;
        iMaxMCUs = widthLimit / 8;
        _x = x + (widthLimit - _jpeg.getWidth() / _scale) / 2;
        _y = y + (heightLimit - _jpeg.getHeight() / _scale) / 2;
        //printf("[JPG] xoff:%d,yoff:%d,scale (%d) \n",_x,_y,_scale);
    }
    else if (ratio <= 4.8)
    {
        _scale = JPEG_SCALE_QUARTER;
        iMaxMCUs = widthLimit / 4;
        _x = x + (widthLimit - _jpeg.getWidth() / _scale) / 2;
        _y = y + (heightLimit - _jpeg.getHeight() / _scale) / 2;
        //printf("[JPG] xoff:%d,yoff:%d,scale (%d) \n",_x,_y,_scale);
    }
    else
    {
        _scale = JPEG_SCALE_EIGHTH;
        iMaxMCUs = widthLimit / 2;
        _x = x + (widthLimit - _jpeg.getWidth() / _scale) / 2;
        _y = y + (heightLimit - _jpeg.getHeight() / _scale) / 2;
        //printf("[JPG] xoff:%d,yoff:%d,scale (%d) \n",_x,_y,_scale);
    }
    _jpeg.setMaxOutputSize(iMaxMCUs);
    if (useBigEndian)
    {
        _jpeg.setPixelType(RGB565_BIG_ENDIAN);
    }
    _jpeg.decode(_x, _y, _scale);
    _jpeg.close();
}

#endif
