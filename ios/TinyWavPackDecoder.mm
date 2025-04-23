#import "TinyWavPackDecoder.h"
#import "../src/tiny-wavpack/common/TinyWavPackDecoderInterface.h"
#import <React/RCTLog.h>
#import <ReactCommon/CallInvoker.h>
#import <jsi/jsi.h>

using namespace facebook::jsi;
using namespace facebook::react;

@implementation TinyWavPackDecoder

RCT_EXPORT_MODULE(TinyWavPackDecoderModule);

- (instancetype)init {
  self = [super init];
  if (self) {
    RCTLogInfo(@"TinyWavPackDecoderModule initialized");
  }
  return self;
}

- (dispatch_queue_t)methodQueue {
  static dispatch_queue_t queue = dispatch_queue_create("com.wavpackdecoder.decode", DISPATCH_QUEUE_CONCURRENT);
  return queue;
}

- (std::shared_ptr<TurboModule>)getTurboModule:(const ObjCTurboModule::InitParams &)params {
  return std::make_shared<NativeTinyWavPackDecoderSpecJSI>(params);
}

- (void)decodeWavPack:(NSString *)inputPath
           outputPath:(NSString *)outputPath
           maxSamples:(NSNumber *)maxSamples
        bitsPerSample:(NSNumber *)bitsPerSample
              resolve:(RCTPromiseResolveBlock)resolve
               reject:(RCTPromiseRejectBlock)reject {
  int maxSamplesInt = maxSamples != nil ? [maxSamples intValue] : -1;
  int bitsPerSampleInt = bitsPerSample != nil ? [bitsPerSample intValue] : 0; // Default to 16-bit
  const char *inputPathC = [inputPath UTF8String];
  const char *outputPathC = [outputPath UTF8String];
  
  DecoderResult result = decode_wavpack_to_wav(
                                               inputPathC,
                                               outputPathC,
                                               maxSamplesInt,
                                               bitsPerSampleInt,
                                               progressCallbackBridge,
                                               (__bridge void*)self
                                               );
  
  // Dispatch promise resolution/rejection to the main queue
  dispatch_async(dispatch_get_main_queue(), ^{
    if (result.success) {
      resolve(@"Success");
    } else {
      NSString *errorMessage = [NSString stringWithUTF8String:result.error];
      reject(@"decode_error", errorMessage, nil);
    }
  });
}

- (NSArray<NSString *> *)supportedEvents {
  return @[@"onProgressUpdate"];
}

static void progressCallbackBridge(float progress, void* context) {
  TinyWavPackDecoder* decoder = (__bridge TinyWavPackDecoder*)context;
  if (decoder.bridge) {
    [decoder sendEventWithName:@"onProgressUpdate" body:@{@"progress": @(progress)}];
  } else {
    RCTLogWarn(@"Cannot emit TinyWavPackDecoder progress event: bridge is nil");
  }
}

- (void)startObserving {
  RCTLogInfo(@"Starting event observation for TinyWavPackDecoder");
}

- (void)stopObserving {
  RCTLogInfo(@"Stopping event observation for TinyWavPackDecoder");
}

@end
