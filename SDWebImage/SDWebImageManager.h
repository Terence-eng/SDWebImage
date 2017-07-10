/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import "SDWebImageCompat.h"
#import "SDWebImageOperation.h"
#import "SDWebImageDownloader.h"
#import "SDImageCache.h"

typedef NS_OPTIONS(NSUInteger, SDWebImageOptions) {
    /**
     * By default, when a URL fail to be downloaded, the URL is blacklisted so the library won't keep trying.
     * This flag disable this blacklisting.
     * 默认情况下如果一个url下载失败，会被加入黑名单，防止再次下载
     * 下载失败了会再次尝试下载
     */
    SDWebImageRetryFailed = 1 << 0,

    /**
     * By default, image downloads are started during UI interactions, this flags disable this feature,
     * leading to delayed download on UIScrollView deceleration for instance.
     * 你滑动的时候scrollview不下载,你手从屏幕上移走,scrollview开始减速的时候才会开始下载图片
     */
    SDWebImageLowPriority = 1 << 1,

    /**
     * This flag disables on-disk caching
     * 这个flag禁止磁盘缓存,只有内存缓存
     */
    SDWebImageCacheMemoryOnly = 1 << 2,

    /**
     * This flag enables progressive download, the image is displayed progressively during download as a browser would do.
     * By default, the image is only displayed once completely downloaded.
     * 这个flag会在图片下载的时候就显示(就像你用浏览器浏览网页的时候那种图片下载,一截一截的显示(待确认))
     */
    SDWebImageProgressiveDownload = 1 << 3,

    /**
     * Even if the image is cached, respect the HTTP response cache control, and refresh the image from remote location if needed.
     * The disk caching will be handled by NSURLCache instead of SDWebImage leading to slight performance degradation.
     * This option helps deal with images changing behind the same request URL, e.g. Facebook graph api profile pics.
     * If a cached image is refreshed, the completion block is called once with the cached image and again with the final image.
     *
     * Use this flag only if you can't make your URLs static with embedded cache busting parameter.
     * 一个图片缓存了,还是会重新请求.并且缓存侧略依据NSURLCache而不是SDWebImage.URL不变,图片会更新时使用
     */
    SDWebImageRefreshCached = 1 << 4,

    /**
     * In iOS 4+, continue the download of the image if the app goes to background. This is achieved by asking the system for
     * extra time in background to let the request finish. If the background task expires the operation will be cancelled.
     * 启动后台下载,加入你进入一个页面,有一张图片正在下载，这时候你让app进入后台,图片还是会继续下载(这个估计要开backgroundfetch才有用)
     */
    SDWebImageContinueInBackground = 1 << 5,

    /**
     * Handles cookies stored in NSHTTPCookieStore by setting
     * NSMutableURLRequest.HTTPShouldHandleCookies = YES;
     * 可以控制存在NSHTTPCookieStore的cookies.
     */
    SDWebImageHandleCookies = 1 << 6,

    /**
     * Enable to allow untrusted SSL certificates.
     * Useful for testing purposes. Use with caution in production.
     * 允许不安全的SSL证书,在正式环境中慎用
     */
    SDWebImageAllowInvalidSSLCertificates = 1 << 7,

    /**
     * By default, images are loaded in the order in which they were queued. This flag moves them to
     * the front of the queue.
     * 默认情况下,image在装载的时候是按照他们在队列中的顺序装载的(就是先进先出).这个flag会把他们移动到队列的前端,并且立刻装载,而不是等到当前队列装载的时候再装载.
     */
    SDWebImageHighPriority = 1 << 8,
    
    /**
     * By default, placeholder images are loaded while the image is loading. This flag will delay the loading
     * of the placeholder image until after the image has finished loading.
     * 默认情况下,占位图会在图片下载的时候显示.这个flag开启会延迟占位图显示的时间,等到图片下载完成之后才会显示占位图.
     */
    SDWebImageDelayPlaceholder = 1 << 9,

    /**
     * We usually don't call transformDownloadedImage delegate method on animated images,
     * as most transformation code would mangle it.
     * Use this flag to transform them anyway.
     */
    SDWebImageTransformAnimatedImage = 1 << 10,
    
    /**
     * By default, image is added to the imageView after download. But in some cases, we want to
     * have the hand before setting the image (apply a filter or add it with cross-fade animation for instance)
     * Use this flag if you want to manually set the image in the completion when success
     * 就是给你一个机会在下载完成的时候可以对原图片做些操作，系统就不自动帮你设置了
     */
    SDWebImageAvoidAutoSetImage = 1 << 11,
    
    /**
     * By default, images are decoded respecting their original size. On iOS, this flag will scale down the
     * images to a size compatible with the constrained memory of devices.
     * If `SDWebImageProgressiveDownload` flag is set the scale down is deactivated.
     * 对比较大的图片进行压缩处理
     */
    SDWebImageScaleDownLargeImages = 1 << 12
};

typedef void(^SDExternalCompletionBlock)(UIImage * _Nullable image, NSError * _Nullable error, SDImageCacheType cacheType, NSURL * _Nullable imageURL);

typedef void(^SDInternalCompletionBlock)(UIImage * _Nullable image, NSData * _Nullable data, NSError * _Nullable error, SDImageCacheType cacheType, BOOL finished, NSURL * _Nullable imageURL);

typedef NSString * _Nullable (^SDWebImageCacheKeyFilterBlock)(NSURL * _Nullable url);


@class SDWebImageManager;

// SDWebImageManagerDelegate代理
@protocol SDWebImageManagerDelegate <NSObject>

@optional

/**
 * Controls which image should be downloaded when the image is not found in the cache.
 *
 * @param imageManager The current `SDWebImageManager`
 * @param imageURL     The url of the image to be downloaded
 *
 * @return Return NO to prevent the downloading of the image on cache misses. If not implemented, YES is implied.
 * 主要作用是当缓存里没有发现某张图片的缓存时,是否选择下载这张图片(默认是yes),可以选择no,那么sdwebimage在缓存中没有找到这张图片的时候不会选择下载
 */
- (BOOL)imageManager:(nonnull SDWebImageManager *)imageManager shouldDownloadImageForURL:(nullable NSURL *)imageURL;

/**
 * Allows to transform the image immediately after it has been downloaded and just before to cache it on disk and memory.
 * NOTE: This method is called from a global queue in order to not to block the main thread.
 *
 * @param imageManager The current `SDWebImageManager`
 * @param image        The image to transform
 * @param imageURL     The url of the image to transform
 *
 * @return The transformed image object.
 */
/**
 *在图片下载完成并且还没有加入磁盘缓存或者内存缓存的时候就transform这个图片.这个方法是在异步线程执行的,防治阻塞主线程.
 *至于为什么在异步执行很简单,对一张图片纠正方向(也就是transform)是很耗资源的,一张2M大小的图片纠正方向你可以用instrument测试一下耗时.
 *很恐怖
 */
- (nullable UIImage *)imageManager:(nonnull SDWebImageManager *)imageManager transformDownloadedImage:(nullable UIImage *)image withURL:(nullable NSURL *)imageURL;

@end

/**
 * The SDWebImageManager is the class behind the UIImageView+WebCache category and likes.
 * It ties the asynchronous downloader (SDWebImageDownloader) with the image cache store (SDImageCache).
 * You can use this class directly to benefit from web image downloading with caching in another context than
 * a UIView.
 *
 * Here is a simple example of how to use SDWebImageManager:
 *
 * @code

SDWebImageManager *manager = [SDWebImageManager sharedManager];
[manager loadImageWithURL:imageURL
                  options:0
                 progress:nil
                completed:^(UIImage *image, NSError *error, SDImageCacheType cacheType, BOOL finished, NSURL *imageURL) {
                    if (image) {
                        // do something with image
                    }
                }];

 * @endcode
 */
@interface SDWebImageManager : NSObject
// 代理
@property (weak, nonatomic, nullable) id <SDWebImageManagerDelegate> delegate;
// 缓存
@property (strong, nonatomic, readonly, nullable) SDImageCache *imageCache;
// 下载器
@property (strong, nonatomic, readonly, nullable) SDWebImageDownloader *imageDownloader;

/**
 * The cache filter is a block used each time SDWebImageManager need to convert an URL into a cache key. This can
 * be used to remove dynamic part of an image URL.
 *
 * The following example sets a filter in the application delegate that will remove any query-string from the
 * URL before to use it as a cache key:
 *
 * @code

[[SDWebImageManager sharedManager] setCacheKeyFilter:^(NSURL *url) {
    url = [[NSURL alloc] initWithScheme:url.scheme host:url.host path:url.path];
    return [url absoluteString];
}];

 * @endcode
 */
/*
 * 这个cacheKeyFilter是干嘛的呢?很简单.1他是一个block.2.这个block的作用就是生成一个image的key.因为sdwebimage的缓存原理你可以当成是一个字典,每一个字典的value就是一张image,那么这个value对应的key是什么呢?就是cacheKeyFilter根据某个规则对这个图片的url做一些操作生成的.上面的示例就显示了怎么利用这个block把image的url重新组合生成一个key.以后当sdwebimage检测到你
 */
@property (nonatomic, copy, nullable) SDWebImageCacheKeyFilterBlock cacheKeyFilter;

/**
 * Returns global SDWebImageManager instance.
 *
 * @return SDWebImageManager shared instance
 * 生成一个单例
 */
+ (nonnull instancetype)sharedManager;

/**
 * Allows to specify instance of cache and image downloader used with image manager.
 * @return new instance of `SDWebImageManager` with specified cache and downloader.
 */
- (nonnull instancetype)initWithCache:(nonnull SDImageCache *)cache downloader:(nonnull SDWebImageDownloader *)downloader NS_DESIGNATED_INITIALIZER;

/**
 * Downloads the image at the given URL if not present in cache or return the cached version otherwise.
 *
 * @param url            The URL to the image
 * @param options        A mask to specify options to use for this request
 * @param progressBlock  A block called while image is downloading
 *                       @note the progress block is executed on a background queue
 * @param completedBlock A block called when operation has been completed.
 *
 *   This parameter is required.
 * 
 *   This block has no return value and takes the requested UIImage as first parameter and the NSData representation as second parameter.
 *   In case of error the image parameter is nil and the third parameter may contain an NSError.
 *
 *   The forth parameter is an `SDImageCacheType` enum indicating if the image was retrieved from the local cache
 *   or from the memory cache or from the network.
 *
 *   The fith parameter is set to NO when the SDWebImageProgressiveDownload option is used and the image is
 *   downloading. This block is thus called repeatedly with a partial image. When image is fully downloaded, the
 *   block is called a last time with the full image and the last parameter set to YES.
 *
 *   The last parameter is the original image URL
 *
 * @return Returns an NSObject conforming to SDWebImageOperation. Should be an instance of SDWebImageDownloaderOperation
 */
/*
 * 这个方法主要就是SDWebImage下载图片的方法了.
 * 第一个参数是必须要的,就是image的url
 * 第二个参数就是我们上面的Options,你可以定制化各种各样的操作.详情参上.
 * 第三个参数是一个回调block,用于图片在下载过程中的回调.(英文注释应该是有问题的.)
 * 第四个参数是一个下载完成的回调.会在图片下载完成后回调.
 * 返回值是一个NSObject类,并且这个NSObject类是conforming一个协议这个协议叫做SDWebImageOperation,这个协议很简单,就是一个cancel掉operation的协议.
 */
- (nullable id <SDWebImageOperation>)loadImageWithURL:(nullable NSURL *)url
                                              options:(SDWebImageOptions)options
                                             progress:(nullable SDWebImageDownloaderProgressBlock)progressBlock
                                            completed:(nullable SDInternalCompletionBlock)completedBlock;

/**
 * Saves image to cache for given URL
 *
 * @param image The image to cache
 * @param url   The URL to the image
 * 将图片存入cache的方法,类似于字典的setValue: forKey:
 */

- (void)saveImageToCache:(nullable UIImage *)image forURL:(nullable NSURL *)url;

/**
 * Cancel all current operations
 * 取消掉当前所有的下载图片的operation
 */
- (void)cancelAll;

/**
 * Check one or more operations running
 * check一下是否有一个或者多个operation正在执行(简单来说就是check是否有图片在下载)
 */
- (BOOL)isRunning;

/**
 *  Async check if image has already been cached
 *
 *  @param url              image url
 *  @param completionBlock  the block to be executed when the check is finished
 *  
 *  @note the completion block is always executed on the main queue
 */
/**
 * 如果检测到图片已经被缓存,那么执行回调block.这个block会永远执行在主线程.也就是你可以在这个回调block里更新ui.
 */
- (void)cachedImageExistsForURL:(nullable NSURL *)url
                     completion:(nullable SDWebImageCheckCacheCompletionBlock)completionBlock;

/**
 *  Async check if image has already been cached on disk only
 *
 *  @param url              image url
 *  @param completionBlock  the block to be executed when the check is finished
 *
 *  @note the completion block is always executed on the main queue
 */
/**
 * 如果检测到图片已经被缓存在磁盘(存且仅存在disk),那么执行回调block.这个block会永远执行在主线程.也就是你可以在这个回调block里更新ui.
 */
- (void)diskImageExistsForURL:(nullable NSURL *)url
                   completion:(nullable SDWebImageCheckCacheCompletionBlock)completionBlock;


/**
 * Return the cache key for a given URL
 * 通过image的url返回image存在缓存里的key.有人会问了,为什么不直接把图片的url当做image的key来使用呢?而是非要对url做一些处理才能当做key.我的解释是,我也不太清楚.可能为了防止重复吧.
 */
- (nullable NSString *)cacheKeyForURL:(nullable NSURL *)url;

@end
