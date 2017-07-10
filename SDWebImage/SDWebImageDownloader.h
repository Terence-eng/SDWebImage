/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import <Foundation/Foundation.h>
#import "SDWebImageCompat.h"
#import "SDWebImageOperation.h"

typedef NS_OPTIONS(NSUInteger, SDWebImageDownloaderOptions) {
    //这个属于默认的使用模式了,前往下载,返回进度block信息,完成时调用completedBlock
    SDWebImageDownloaderLowPriority = 1 << 0,
    
    //渐进式下载 ,如果设置了这个选项,会在下载过程中,每次接收到一段返回数据就会调用一次完成回调,回调中的image参数为未下载完成的部分图像,可以实现将图片一点点显示出来的功能
    SDWebImageDownloaderProgressiveDownload = 1 << 1,

    /**
     * By default, request prevent the use of NSURLCache. With this flag, NSURLCache
     * is used with default policies.
     * 通常情况下request阻止使用NSURLCache.这个选项会默认使用NSURLCache
     */
    SDWebImageDownloaderUseNSURLCache = 1 << 2,

    /**
     * Call completion block with nil image/imageData if the image was read from NSURLCache
     * (to be combined with `SDWebImageDownloaderUseNSURLCache`).
     * I think this option should be renamed to 'SDWebImageDownloaderUsingCachedResponseDontLoad'
     * 如果从NSURLCache中读取图片,会在调用完成block的时候,传递空的image或者imageData
     */
    SDWebImageDownloaderIgnoreCachedResponse = 1 << 3,
    
    /**
     * In iOS 4+, continue the download of the image if the app goes to background. This is achieved by asking the system for
     * extra time in background to let the request finish. If the background task expires the operation will be cancelled.
     *  系统为iOS 4+时候,如果应用进入后台,继续下载.这个选项是为了实现在后台申请额外的时间来完成请求.如果后台任务到期,操作也会被取消
     */
    SDWebImageDownloaderContinueInBackground = 1 << 4,

    /**
     * Handles cookies stored in NSHTTPCookieStore by setting 
     * NSMutableURLRequest.HTTPShouldHandleCookies = YES;
     * 通过设置 NSMutableURLRequest.HTTPShouldHandleCookies = YES的方式来处理存储在NSHTTPCookieStore的cookies
     */
    SDWebImageDownloaderHandleCookies = 1 << 5,

    /**
     * Enable to allow untrusted SSL certificates.
     * Useful for testing purposes. Use with caution in production.
     * 允许不受信任的SSL证书，在测试环境中很有用，在生产环境中要谨慎使用
     */
    SDWebImageDownloaderAllowInvalidSSLCertificates = 1 << 6,

    /**
     * Put the image in the high priority queue.
     * 将图片下载放到高优先级队列中
     */
    SDWebImageDownloaderHighPriority = 1 << 7,
    
    /**
     * Scale down the image
     * 裁剪大图片
     */
    SDWebImageDownloaderScaleDownLargeImages = 1 << 8,
};

typedef NS_ENUM(NSInteger, SDWebImageDownloaderExecutionOrder) {
    /**
     * Default value. All download operations will execute in queue style (first-in-first-out).
     * 默认值。所有的下载操作都将以队列方式执行(先入先出)。
     */
    SDWebImageDownloaderFIFOExecutionOrder,

    /**
     * All download operations will execute in stack style (last-in-first-out).
     * 所有的下载操作都将以堆栈样式执行(最后一种方式)，先入后出
     */
    SDWebImageDownloaderLIFOExecutionOrder
};

extern NSString * _Nonnull const SDWebImageDownloadStartNotification;
extern NSString * _Nonnull const SDWebImageDownloadStopNotification;
/**
 下载进度block

 @param receivedSize 本次收到的size
 @param expectedSize 总共的size
 @param targetURL URL
 */
typedef void(^SDWebImageDownloaderProgressBlock)(NSInteger receivedSize, NSInteger expectedSize, NSURL * _Nullable targetURL);

/**
 下载完成block

 @param image 图像
 @param data 图像数据
 @param error 报错
 @param finished 是否完成
 */
typedef void(^SDWebImageDownloaderCompletedBlock)(UIImage * _Nullable image, NSData * _Nullable data, NSError * _Nullable error, BOOL finished);

typedef NSDictionary<NSString *, NSString *> SDHTTPHeadersDictionary;
typedef NSMutableDictionary<NSString *, NSString *> SDHTTPHeadersMutableDictionary;

typedef SDHTTPHeadersDictionary * _Nullable (^SDWebImageDownloaderHeadersFilterBlock)(NSURL * _Nullable url, SDHTTPHeadersDictionary * _Nullable headers);

/**
 *  A token associated with each download. Can be used to cancel a download
 *  token 与每个下载相关联，可以用来取消下载
 */
@interface SDWebImageDownloadToken : NSObject

@property (nonatomic, strong, nullable) NSURL *url;
@property (nonatomic, strong, nullable) id downloadOperationCancelToken;

@end


/**
 * Asynchronous downloader dedicated and optimized for image loading.
 */
@interface SDWebImageDownloader : NSObject

/**
 * Decompressing images that are downloaded and cached can improve performance but can consume lot of memory.
 * Defaults to YES. Set this to NO if you are experiencing a crash due to excessive memory consumption.
 下载和缓存的解压图像可以提高性能，但可以消耗大量内存。
 默认值为YES。如果您正在经历由于内存过度消耗而导致的崩溃，那么将它设置为NO。
 */
@property (assign, nonatomic) BOOL shouldDecompressImages;

/**
 *  The maximum number of concurrent downloads
 *  并发最大下载数量
 */
@property (assign, nonatomic) NSInteger maxConcurrentDownloads;

/**
 * Shows the current amount of downloads that still need to be downloaded
 * 显示当前可以下载的下载数量
 */
@property (readonly, nonatomic) NSUInteger currentDownloadCount;


/**
 *  The timeout value (in seconds) for the download operation. Default: 15.0.
 *  下载超时时间，默认15秒
 */
@property (assign, nonatomic) NSTimeInterval downloadTimeout;


/**
 * Changes download operations execution order. Default value is `SDWebImageDownloaderFIFOExecutionOrder`.
 * 更改下载操作的执行顺序。默认值是“SDWebImageDownloaderFIFOExecutionOrder”。
 */
@property (assign, nonatomic) SDWebImageDownloaderExecutionOrder executionOrder;

/**
 *  Singleton method, returns the shared instance
 *
 *  @return global shared instance of downloader class
 *  返回一个单例
 */
+ (nonnull instancetype)sharedDownloader;

/**
 *  Set the default URL credential to be set for request operations.
 */
@property (strong, nonatomic, nullable) NSURLCredential *urlCredential;

/**
 * Set username
 */
@property (strong, nonatomic, nullable) NSString *username;

/**
 * Set password
 */
@property (strong, nonatomic, nullable) NSString *password;

/**
 * Set filter to pick headers for downloading image HTTP request.
 *
 * This block will be invoked for each downloading image request, returned
 * NSDictionary will be used as headers in corresponding HTTP request.
 */
@property (nonatomic, copy, nullable) SDWebImageDownloaderHeadersFilterBlock headersFilter;

/**
 * Creates an instance of a downloader with specified session configuration.
 * *Note*: `timeoutIntervalForRequest` is going to be overwritten.
 * @return new instance of downloader class
 */
- (nonnull instancetype)initWithSessionConfiguration:(nullable NSURLSessionConfiguration *)sessionConfiguration NS_DESIGNATED_INITIALIZER;

/**
 * Set a value for a HTTP header to be appended to each download HTTP request.
 *
 * @param value The value for the header field. Use `nil` value to remove the header.
 * @param field The name of the header field to set.
 * 为每个下载HTTP请求附加一个HTTP头的值
 * value  header字段的值。使用nil值来删除header。
 * field  要设置header字段的名称
 */
- (void)setValue:(nullable NSString *)value forHTTPHeaderField:(nullable NSString *)field;

/**
 * Returns the value of the specified HTTP header field.
 *
 * @return The value associated with the header field field, or `nil` if there is no corresponding header field.
 */
- (nullable NSString *)valueForHTTPHeaderField:(nullable NSString *)field;

/**
 * Sets a subclass of `SDWebImageDownloaderOperation` as the default
 * `NSOperation` to be used each time SDWebImage constructs a request
 * operation to download an image.
 *
 * @param operationClass The subclass of `SDWebImageDownloaderOperation` to set 
 *        as default. Passing `nil` will revert to `SDWebImageDownloaderOperation`.
 */
- (void)setOperationClass:(nullable Class)operationClass;

/**
 * Creates a SDWebImageDownloader async downloader instance with a given URL
 *
 * The delegate will be informed when the image is finish downloaded or an error has happen.
 *
 * @see SDWebImageDownloaderDelegate
 *
 * @param url            The URL to the image to download
 * @param options        The options to be used for this download
 * @param progressBlock  A block called repeatedly while the image is downloading
 *                       @note the progress block is executed on a background queue
 * @param completedBlock A block called once the download is completed.
 *                       If the download succeeded, the image parameter is set, in case of error,
 *                       error parameter is set with the error. The last parameter is always YES
 *                       if SDWebImageDownloaderProgressiveDownload isn't use. With the
 *                       SDWebImageDownloaderProgressiveDownload option, this block is called
 *                       repeatedly with the partial image object and the finished argument set to NO
 *                       before to be called a last time with the full image and finished argument
 *                       set to YES. In case of error, the finished argument is always YES.
 *
 * @return A token (SDWebImageDownloadToken) that can be passed to -cancel: to cancel this operation
 */
- (nullable SDWebImageDownloadToken *)downloadImageWithURL:(nullable NSURL *)url
                                                   options:(SDWebImageDownloaderOptions)options
                                                  progress:(nullable SDWebImageDownloaderProgressBlock)progressBlock
                                                 completed:(nullable SDWebImageDownloaderCompletedBlock)completedBlock;

/**
 * Cancels a download that was previously queued using -downloadImageWithURL:options:progress:completed:
 *
 * @param token The token received from -downloadImageWithURL:options:progress:completed: that should be canceled.
 */
- (void)cancel:(nullable SDWebImageDownloadToken *)token;

/**
 * Sets the download queue suspension state
 * 设置下载队列暂停状态
 */
- (void)setSuspended:(BOOL)suspended;

/**
 * Cancels all download operations in the queue
 * 取消队列中的所有下载操作
 */
- (void)cancelAllDownloads;

@end
