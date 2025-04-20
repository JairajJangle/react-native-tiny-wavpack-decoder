#import <React/RCTBridgeModule.h>
#import <ReactCommon/RCTTurboModule.h>
#import <jsi/jsi.h>
#import <RNTinyWavPackDecoderSpec.h>
#import <React/RCTEventEmitter.h>

@interface TinyWavPackDecoder : RCTEventEmitter <NativeTinyWavPackDecoderSpec>
@end