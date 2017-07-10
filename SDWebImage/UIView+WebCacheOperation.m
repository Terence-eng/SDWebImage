/*
 * This file is part of the SDWebImage package.
 * (c) Olivier Poitrey <rs@dailymotion.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#import "UIView+WebCacheOperation.h"

#if SD_UIKIT || SD_MAC

#import "objc/runtime.h"

static char loadOperationKey;

// 字典
typedef NSMutableDictionary<NSString *, id> SDOperationsDictionary;

@implementation UIView (WebCacheOperation)

// runtime 创建operationDictionary
- (SDOperationsDictionary *)operationDictionary {
    SDOperationsDictionary *operations = objc_getAssociatedObject(self, &loadOperationKey);
    if (operations) {
        return operations;
    }
    operations = [NSMutableDictionary dictionary];
    objc_setAssociatedObject(self, &loadOperationKey, operations, OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    return operations;
}

- (void)sd_setImageLoadOperation:(nullable id)operation forKey:(nullable NSString *)key {
    if (key) {
        [self sd_cancelImageLoadOperationWithKey:key];
        if (operation) {
            SDOperationsDictionary *operationDictionary = [self operationDictionary];
            operationDictionary[key] = operation;
        }
    }
}

- (void)sd_cancelImageLoadOperationWithKey:(nullable NSString *)key {
    // Cancel in progress downloader from queue
    // 取消正在进行的下载队列
    
    SDOperationsDictionary *operationDictionary = [self operationDictionary];
    // 根据传入的类名拿到对应的value,预计应该是某个对象
    id operations = operationDictionary[key];
    if (operations) {
        if ([operations isKindOfClass:[NSArray class]]) {
            // 遍历数组
            for (id <SDWebImageOperation> operation in operations) {
                if (operation) {
                    [operation cancel];
                }
            }
        } else if ([operations conformsToProtocol:@protocol(SDWebImageOperation)]){
            // conformsToProtocol 判断 operations 是否有实现 SDWebImageOperation 协议
            [(id<SDWebImageOperation>) operations cancel];
        }
        // 从字典中移除对应的key
        [operationDictionary removeObjectForKey:key];
    }
}

- (void)sd_removeImageLoadOperationWithKey:(nullable NSString *)key {
    if (key) {
        SDOperationsDictionary *operationDictionary = [self operationDictionary];
        [operationDictionary removeObjectForKey:key];
    }
}

@end

#endif
