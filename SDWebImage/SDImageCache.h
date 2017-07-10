/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"
#import "SDImageCacheConfig.h"

typedef NS_ENUM(NSInteger, SDImageCacheType) {
    /**
     * The image wasn't available the SDWebImage caches, but was downloaded from the web.
     * 直接从网上下载
     */
    SDImageCacheTypeNone,
    /**
     * The image was obtained from the disk cache.
     * 从本地沙盒中获取图片
     */
    SDImageCacheTypeDisk,
    /**
     * The image was obtained from the memory cache.
     * 从内存中获取图片
     */
    SDImageCacheTypeMemory
};

typedef void(^SDCacheQueryCompletedBlock)(UIImage * _Nullable image, NSData * _Nullable data, SDImageCacheType cacheType);

typedef void(^SDWebImageCheckCacheCompletionBlock)(BOOL isInCache);

typedef void(^SDWebImageCalculateSizeBlock)(NSUInteger fileCount, NSUInteger totalSize);


/**
 * SDImageCache maintains a memory cache and an optional disk cache. Disk cache write operations are performed
 * asynchronous so it doesn’t add unnecessary latency to the UI.
 */
@interface SDImageCache : NSObject

#pragma mark - Properties

/**
 *  Cache Config object - storing all kind of settings
 *  缓存配置对象
 */
@property (nonatomic, nonnull, readonly) SDImageCacheConfig *config;

/**
 * The maximum "total cost" of the in-memory image cache. The cost function is the number of pixels held in memory.
 *
 */
@property (assign, nonatomic) NSUInteger maxMemoryCost;

/**
 * The maximum number of objects the cache should hold.
 * 缓存中应该持有的对象的最大数量
 */
@property (assign, nonatomic) NSUInteger maxMemoryCountLimit;

#pragma mark - Singleton and initialization

/**
 * Returns global shared cache instance
 *
 * @return SDImageCache global instance
 * 获取一个单例
 */
+ (nonnull instancetype)sharedImageCache;

/**
 * Init a new cache store with a specific namespace
 *
 * @param ns The namespace to use for this cache store
 * 初始化一个带有特定名称空间的新缓存存储
 */
- (nonnull instancetype)initWithNamespace:(nonnull NSString *)ns;

/**
 * Init a new cache store with a specific namespace and directory
 *
 * @param ns        The namespace to use for this cache store
 * @param directory Directory to cache disk images in
 * 使用特定的名称空间和目录初始化一个新的缓存存储
 * ns 为这个缓存商店使用的名称空间
 * directory 目录
 */
- (nonnull instancetype)initWithNamespace:(nonnull NSString *)ns
                       diskCacheDirectory:(nonnull NSString *)directory NS_DESIGNATED_INITIALIZER;

#pragma mark - Cache paths

// 返回沙盒 Caches/fullNamespace 路径，
- (nullable NSString *)makeDiskCachePath:(nonnull NSString*)fullNamespace;

/**
 * Add a read-only cache path to search for images pre-cached by SDImageCache
 * Useful if you want to bundle pre-loaded images with your app
 *
 * @param path The path to use for this read-only cache path
 */
- (void)addReadOnlyCachePath:(nonnull NSString *)path;

#pragma mark - Store Ops

/**
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param completionBlock A block executed after the operation is finished
 * 异步将图像存储在给定键的内存 磁盘缓存中。
 */
- (void)storeImage:(nullable UIImage *)image
            forKey:(nullable NSString *)key
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param toDisk          Store the image to disk cache if YES
 * @param completionBlock A block executed after the operation is finished
 * 异步将图像存储在给定键的内存和磁盘缓存中。
 * toDisk将图像存储到磁盘缓存中
 */
- (void)storeImage:(nullable UIImage *)image
            forKey:(nullable NSString *)key
            toDisk:(BOOL)toDisk
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Asynchronously store an image into memory and disk cache at the given key.
 *
 * @param image           The image to store
 * @param imageData       The image data as returned by the server, this representation will be used for disk storage
 *                        instead of converting the given image object into a storable/compressed image format in order
 *                        to save quality and CPU
 * @param key             The unique image cache key, usually it's image absolute URL
 * @param toDisk          Store the image to disk cache if YES
 * @param completionBlock A block executed after the operation is finished
 * 异步将图片存储到给定键的内容和磁盘缓存中
 * image 图像 
 * image data 数据是由服务器返回的，这个表示将被用于磁盘存储而不是将给定的图像对象转换成可存储/压缩的图像格式，以节省质量和CPU。
 * key 是唯一的图像缓存键，通常是图像绝对URL
 * toDisk  将图像存储到磁盘缓存中
 * completionBlock 完成的回调
 */
- (void)storeImage:(nullable UIImage *)image
         imageData:(nullable NSData *)imageData
            forKey:(nullable NSString *)key
            toDisk:(BOOL)toDisk
        completion:(nullable SDWebImageNoParamsBlock)completionBlock;

/**
 * Synchronously store image NSData into disk cache at the given key.
 *
 * @warning This method is synchronous, make sure to call it from the ioQueue
 *
 * @param imageData  The image data to store
 * @param key        The unique image cache key, usually it's image absolute URL
 *
 * 同步将图像的NSData存储到给定键的磁盘缓存中。
 * 警告这个方法是同步的，确保从ioQueue调用它
 * imageData 数据存储
 * key 是唯一的图像缓存键，通常是图像绝对URL
 */
- (void)storeImageDataToDisk:(nullable NSData *)imageData forKey:(nullable NSString *)key;

#pragma mark - Query and Retrieve Ops

/**
 *  Async check if image exists in disk cache already (does not load the image)
 *
 *  @param key             the key describing the url
 *  @param completionBlock the block to be executed when the check is done.
 *  @note the completion block will be always executed on the main queue
 *  异步检查是否已经存在于磁盘缓存中(不加载图像)
 *  key
 *  completionBlock 完成块，在主队列上执行
 */
- (void)diskImageExistsWithKey:(nullable NSString *)key completion:(nullable SDWebImageCheckCacheCompletionBlock)completionBlock;

/**
 * Operation that queries the cache asynchronously and call the completion when done.
 *
 * @param key       The unique key used to store the wanted image
 * @param doneBlock The completion block. Will not get called if the operation is cancelled
 *
 * @return a NSOperation instance containing the cache op
 * 操作异步查询高速缓存，并在完成时调用doneBlock。
 */
- (nullable NSOperation *)queryCacheOperationForKey:(nullable NSString *)key done:(nullable SDCacheQueryCompletedBlock)doneBlock;

/**
 * Query the memory cache synchronously.
 *
 * @param key The unique key used to store the image
 
 * 同步地查询内存缓存。
 */
- (nullable UIImage *)imageFromMemoryCacheForKey:(nullable NSString *)key;

/**
 * Query the disk cache synchronously.
 *
 * @param key The unique key used to store the image
 * 同步地查询磁盘缓存。
 */
- (nullable UIImage *)imageFromDiskCacheForKey:(nullable NSString *)key;

/**
 * Query the cache (memory and or disk) synchronously after checking the memory cache.
 *
 * @param key The unique key used to store the image
 * 在检查内存缓存之后，以同步方式查询缓存(内存或磁盘)。
 */
- (nullable UIImage *)imageFromCacheForKey:(nullable NSString *)key;

#pragma mark - Remove Ops

/**
 * Remove the image from memory and disk cache asynchronously
 *
 * @param key             The unique image cache key
 * @param completion      A block that should be executed after the image has been removed (optional)
 将图像从内存和磁盘缓存中删除
 */
- (void)removeImageForKey:(nullable NSString *)key withCompletion:(nullable SDWebImageNoParamsBlock)completion;

/**
 * Remove the image from memory and optionally disk cache asynchronously
 *
 * @param key             The unique image cache key
 * @param fromDisk        Also remove cache entry from disk if YES
 * @param completion      A block that should be executed after the image has been removed (optional)
 * 将图像从内存中移除，并可选地进行磁盘缓存
 * key 是唯一的图像缓存键
 * fromDisk 如果YES，也可以进入磁盘中移除图像
 */
- (void)removeImageForKey:(nullable NSString *)key fromDisk:(BOOL)fromDisk withCompletion:(nullable SDWebImageNoParamsBlock)completion;

#pragma mark - Cache clean Ops

/**
 * Clear all memory cached images
 * 清除所有内存缓存图像
 */
- (void)clearMemory;

/**
 * Async clear all disk cached images. Non-blocking method - returns immediately.
 * @param completion    A block that should be executed after cache expiration completes (optional)
 异步清除所有磁盘缓存映像。非阻塞方法-立即返回。
 */
- (void)clearDiskOnCompletion:(nullable SDWebImageNoParamsBlock)completion;

/**
 * Async remove all expired cached image from disk. Non-blocking method - returns immediately.
 * @param completionBlock A block that should be executed after cache expiration completes (optional)
 
 * 异步将所有过期的缓存映像从磁盘中删除。非阻塞方法-立即返回。
 */
- (void)deleteOldFilesWithCompletionBlock:(nullable SDWebImageNoParamsBlock)completionBlock;

#pragma mark - Cache Info

/**
 * Get the size used by the disk cache
 * 获取磁盘缓存所使用的大小
 */
- (NSUInteger)getSize;

/**
 * Get the number of images in the disk cache
 * 获取磁盘缓存中的图像数量
 */
- (NSUInteger)getDiskCount;

/**
 * Asynchronously calculate the disk cache's size.
 * 异步地计算磁盘缓存的大小。
 */
- (void)calculateSizeWithCompletionBlock:(nullable SDWebImageCalculateSizeBlock)completionBlock;

#pragma mark - Cache Paths

/**
 *  Get the cache path for a certain key (needs the cache path root folder)
 *
 *  @param key  the key (can be obtained from url using cacheKeyForURL)
 *  @param path the cache path root folder
 *
 *  @return the cache path
 
 * 获取某个键的缓存路径(需要缓存路径根文件夹)
 * key (可以从url中使用cacheKeyForURL获得)
 * path 缓存路径根文件夹
 * 返回缓存路径
 */
- (nullable NSString *)cachePathForKey:(nullable NSString *)key inPath:(nonnull NSString *)path;

/**
 *  Get the default cache path for a certain key
 *
 *  @param key the key (can be obtained from url using cacheKeyForURL)
 *
 *  @return the default cache path
 *  获取某个键的默认缓存路径
 *  key 关键字(可以从url中使用cacheKeyForURL获得)
 *  返回默认的缓存路径
 */
- (nullable NSString *)defaultCachePathForKey:(nullable NSString *)key;

@end
