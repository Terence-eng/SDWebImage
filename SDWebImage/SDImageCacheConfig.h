/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"

// SDImageCacheConfig 用来管理“缓存配置信息”的类。

@interface SDImageCacheConfig : NSObject

/**
 * Decompressing images that are downloaded and cached can improve performance but can consume lot of memory.
 * Defaults to YES. Set this to NO if you are experiencing a crash due to excessive memory consumption.
 解压缩图像下载和缓存可以提高性能，但是会消耗大量内存， 默认是YES ，如果因为消耗内存过大而崩溃，可以置为NO。
 shouldDecompressImages 是否解压图像，默认是 YES
 */
@property (assign, nonatomic) BOOL shouldDecompressImages;

/**
 *  disable iCloud backup [defaults to YES]
 *  shouldDisableiCloud 是否禁用 iCloud 备份，默认是 YES
 */
@property (assign, nonatomic) BOOL shouldDisableiCloud;

/**
 * use memory cache [defaults to YES]
 * shouldCacheImagesInMemory 是否缓存到内存中，默认是YES
 */
@property (assign, nonatomic) BOOL shouldCacheImagesInMemory;

/**
 * The maximum length of time to keep an image in the cache, in seconds
 * maxCacheAge  在缓存中图像保存时间的最大长度，以秒为单位 默认是一周时间
 */
@property (assign, nonatomic) NSInteger maxCacheAge;

/**
 * The maximum size of the cache, in bytes.
 * maxCacheSize  缓存的最大大小，以字节为单位
 */
@property (assign, nonatomic) NSUInteger maxCacheSize;

@end
