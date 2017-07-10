/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 * (c) Fabrice Aneche
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"

// 图片格式
typedef NS_ENUM(NSInteger, SDImageFormat) {
    SDImageFormatUndefined = -1, //未知
    SDImageFormatJPEG = 0, //JPG
    SDImageFormatPNG,      //PNG
    SDImageFormatGIF,      //GIF
    SDImageFormatTIFF,     //TIFF
    SDImageFormatWebP      //WebP
};

@interface NSData (ImageContentType)

/**
 *  Return image format
 *
 *  @param data the input image data
 *
 *  @return the image format as `SDImageFormat` (enum)
 */
+ (SDImageFormat)sd_imageFormatForImageData:(nullable NSData *)data;

@end
