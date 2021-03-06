/*
	File:  AVSampleBufferDisplayLayer.h

	Framework:  AVFoundation
 
	Copyright 2011-2013 Apple Inc. All rights reserved.

*/

/*!
    @class			AVSampleBufferDisplayLayer

    @abstract		AVSampleBufferDisplayLayer is a subclass of CALayer that can decompress and display compressed or uncompressed video frames.
*/

#import <AVFoundation/AVBase.h>
#import <AVFoundation/AVAnimation.h>
#import <QuartzCore/CoreAnimation.h>
#import <CoreMedia/CMSync.h>
#import <CoreMedia/CMSampleBuffer.h>

@class AVSampleBufferDisplayLayerInternal;

NS_CLASS_AVAILABLE(10_8, 6_0)
@interface AVSampleBufferDisplayLayer : CALayer
{
@private
	AVSampleBufferDisplayLayerInternal		*_sampleBufferDisplayLayerInternal;
}

/*!
	@property		controlTimebase
	@abstract		The layer's control timebase, which governs how time stamps are interpreted.
	@discussion		By default, this property is NULL, in which case time stamps will be interpreted
					according to the host time clock (mach_absolute_time with the appropriate timescale
					conversion; this is the same as Core Animation's CACurrentMediaTime).  With no 
					control timebase, once frames are enqueued, it is not possible to adjust exactly 
					when they are displayed.
					
					If a non-NULL control timebase is set, it will be used to interpret time stamps.
					You can control the timing of frame display by setting the rate and time of the
					control timebase.  
					If you are synchronizing video to audio, you can use a timebase whose master clock
					is a CMAudioDeviceClock for the appropriate audio device to prevent drift.
					
					The control timebase can not be changed after enqueueSampleBuffer: is called.
*/
@property (retain) __attribute__((NSObject)) CMTimebaseRef controlTimebase;

/*!
	@property		videoGravity
	@abstract		A string defining how the video is displayed within an AVSampleBufferDisplayLayer bounds rect.
	@discusssion	Options are AVLayerVideoGravityResizeAspect, AVLayerVideoGravityResizeAspectFill 
 					and AVLayerVideoGravityResize. AVLayerVideoGravityResizeAspect is default. 
					See <AVFoundation/AVAnimation.h> for a description of these options.
 */
@property(copy) NSString *videoGravity;

@end


@interface AVSampleBufferDisplayLayer (AVSampleBufferDisplayLayerQueueManagement)

/*!
	@method			enqueueSampleBuffer:
	@abstract		Sends a sample buffer for display.
	@discussion		If sampleBuffer has the kCMSampleAttachmentKey_DoNotDisplay attachment set to
					kCFBooleanTrue, the frame will be decoded but not displayed.
					Otherwise, if sampleBuffer has the kCMSampleAttachmentKey_DisplayImmediately
					attachment set to kCFBooleanTrue, the decoded image will be displayed as soon 
					as possible, replacing all previously enqueued images regardless of their timestamps.
					Otherwise, the decoded image will be displayed at sampleBuffer's output presentation
					timestamp, as interpreted by the control timebase (or the mach_absolute_time timeline
					if there is no control timebase).
					
					To schedule the removal of previous images at a specific timestamp, enqueue 
					a marker sample buffer containing no samples, with the
					kCMSampleBufferAttachmentKey_EmptyMedia attachment set to kCFBooleanTrue.
					
					IMPORTANT NOTE: attachments with the kCMSampleAttachmentKey_ prefix must be set via
					CMSampleBufferGetSampleAttachmentsArray and CFDictionarySetValue. 
					Attachments with the kCMSampleBufferAttachmentKey_ prefix must be set via
					CMSetAttachment.
*/
- (void)enqueueSampleBuffer:(CMSampleBufferRef)sampleBuffer;

/*!
	@method			flush
	@abstract		Instructs the layer to discard pending enqueued sample buffers.
	@discussion		It is not possible to determine which sample buffers have been decoded, 
					so the next frame passed to enqueueSampleBuffer: should be an IDR frame
					(also known as a key frame or sync sample).
*/
- (void)flush;

/*!
	@method			flushAndRemoveImage
	@abstract		Instructs the layer to discard pending enqueued sample buffers and remove any
					currently displayed image.
	@discussion		It is not possible to determine which sample buffers have been decoded, 
					so the next frame passed to enqueueSampleBuffer: should be an IDR frame
					(also known as a key frame or sync sample).
*/
- (void)flushAndRemoveImage;

/*!
	@property		readyForMoreMediaData
	@abstract		Indicates the readiness of the layer to accept more sample buffers.
	@discussion		AVSampleBufferDisplayLayer keeps track of the occupancy levels of its internal queues
					for the benefit of clients that enqueue sample buffers from non-real-time sources --
					i.e., clients that can supply sample buffers faster than they are consumed, and so
					need to decide when to hold back.
					
					Clients enqueueing sample buffers from non-real-time sources may hold off from
					generating or obtaining more sample buffers to enqueue when the value of
					readyForMoreMediaData is NO.  
					
					It is safe to call enqueueSampleBuffer: when readyForMoreMediaData is NO, but 
					it is a bad idea to enqueue sample buffers without bound.
					
					To help with control of the non-real-time supply of sample buffers, such clients can use
					-requestMediaDataWhenReadyOnQueue:usingBlock
					in order to specify a block that the layer should invoke whenever it's ready for 
					sample buffers to be appended.
 
					The value of readyForMoreMediaData will often change from NO to YES asynchronously, 
					as previously supplied sample buffers are decoded and displayed.
	
					This property is not key value observable.
*/
@property (readonly, getter=isReadyForMoreMediaData) BOOL readyForMoreMediaData;

/*!
	@method			requestMediaDataWhenReadyOnQueue:usingBlock:
	@abstract		Instructs the target to invoke a client-supplied block repeatedly, 
					at its convenience, in order to gather sample buffers for display.
	@discussion		The block should enqueue sample buffers to the layer either until the layer's
					readyForMoreMediaData property becomes NO or until there is no more data 
					to supply. When the layer has decoded enough of the media data it has received 
					that it becomes ready for more media data again, it will invoke the block again 
					in order to obtain more.
					If this function is called multiple times, only the last call is effective.
					Call stopRequestingMediaData to cancel this request.
					Each call to requestMediaDataWhenReadyOnQueue:usingBlock: should be paired
					with a corresponding call to stopRequestingMediaData:. Releasing the
					AVSampleBufferDisplayLayer without a call to stopRequestingMediaData will result
					in undefined behavior.
*/
- (void)requestMediaDataWhenReadyOnQueue:(dispatch_queue_t)queue usingBlock:(void (^)(void))block;

/*!
	@method			stopRequestingMediaData
	@abstract		Cancels any current requestMediaDataWhenReadyOnQueue:usingBlock: call.
	@discussion		This method may be called from outside the block or from within the block.
*/
- (void)stopRequestingMediaData;

@end
