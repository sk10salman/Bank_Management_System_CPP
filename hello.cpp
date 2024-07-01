#include <jni.h>
#include <android/log.h>
#include <media/NdkMediaCodec.h>
#include <media/NdkMediaFormat.h>

// Function to decode and encode a video using Android's MediaCodec and AMediaFormat
void decodeAndEncodeVideo(const char *inputFile, const char *outputFile) {
    // Initialize AMediaFormat for input format
    AMediaFormat *inputFormat = AMediaFormat_new();
    AMediaFormat_setString(inputFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(inputFormat, AMEDIAFORMAT_KEY_WIDTH, 640); // Replace with actual width
    AMediaFormat_setInt32(inputFormat, AMEDIAFORMAT_KEY_HEIGHT, 480); // Replace with actual height

    // Initialize AMediaFormat for output format
    AMediaFormat *outputFormat = AMediaFormat_new();
    AMediaFormat_setString(outputFormat, AMEDIAFORMAT_KEY_MIME, "video/avc");
    AMediaFormat_setInt32(outputFormat, AMEDIAFORMAT_KEY_WIDTH, 640); // Replace with desired output width
    AMediaFormat_setInt32(outputFormat, AMEDIAFORMAT_KEY_HEIGHT, 480); // Replace with desired output height
    AMediaFormat_setInt32(outputFormat, AMEDIAFORMAT_KEY_BIT_RATE, 400000); // Replace with desired bit rate
    AMediaFormat_setInt32(outputFormat, AMEDIAFORMAT_KEY_FRAME_RATE, 30); // Replace with desired frame rate

    // Create MediaCodec for decoding
    AMediaCodec *decoder = AMediaCodec_createDecoderByType("video/avc");
    AMediaCodec_configure(decoder, inputFormat, nullptr, nullptr, 0);
    AMediaCodec_start(decoder);

    // Create MediaCodec for encoding
    AMediaCodec *encoder = AMediaCodec_createEncoderByType("video/avc");
    AMediaCodec_configure(encoder, outputFormat, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);
    AMediaCodec_start(encoder);

    // Open input file for reading
    FILE *inputFd = fopen(inputFile, "rb");
    if (!inputFd) {
        __android_log_print(ANDROID_LOG_ERROR, "VideoProcessing", "Error opening input file: %s", inputFile);
        return;
    }

    // Open output file for writing
    FILE *outputFd = fopen(outputFile, "wb");
    if (!outputFd) {
        __android_log_print(ANDROID_LOG_ERROR, "VideoProcessing", "Error opening output file: %s", outputFile);
        fclose(inputFd);
        return;
    }

    // Read and process input frames
    ssize_t readBytes;
    uint8_t *inputBuffer = nullptr;
    ssize_t inputBufferSize = 0;
    int inputFrameIndex = 0;

    while ((readBytes = fread(inputBuffer, 1, inputBufferSize, inputFd)) > 0) {
        // Create input buffer for decoding
        ssize_t inputIndex = AMediaCodec_dequeueInputBuffer(decoder, -1);
        if (inputIndex >= 0) {
            size_t inputSize;
            uint8_t *inputBuffer = AMediaCodec_getInputBuffer(decoder, inputIndex, &inputSize);
            memcpy(inputBuffer, inputBuffer, readBytes);
            AMediaCodec_queueInputBuffer(decoder, inputIndex, 0, readBytes, 0);
        }

        // Dequeue and process decoded output frame
        AMediaCodecBufferInfo info;
        ssize_t outputIndex = AMediaCodec_dequeueOutputBuffer(decoder, &info, 0);
        if (outputIndex >= 0) {
            // Get decoded frame data
            size_t outputSize;
            uint8_t *outputBuffer = AMediaCodec_getOutputBuffer(decoder, outputIndex, &outputSize);
            
            // Encode the decoded frame
            ssize_t encodeIndex = AMediaCodec_dequeueInputBuffer(encoder, -1);
            if (encodeIndex >= 0) {
                size_t encodeSize;
                uint8_t *encodeBuffer = AMediaCodec_getInputBuffer(encoder, encodeIndex, &encodeSize);
                memcpy(encodeBuffer, outputBuffer, info.size);
                AMediaCodec_queueInputBuffer(encoder, encodeIndex, 0, info.size, 0);
            }

            // Get encoded output frame
            AMediaCodecBufferInfo encodeInfo;
            ssize_t encodeOutputIndex = AMediaCodec_dequeueOutputBuffer(encoder, &encodeInfo, 0);
            if (encodeOutputIndex >= 0) {
                // Write encoded frame to output file
                uint8_t *encodeOutputBuffer = AMediaCodec_getOutputBuffer(encoder, encodeOutputIndex, nullptr);
                fwrite(encodeOutputBuffer, 1, encodeInfo.size, outputFd);
                AMediaCodec_releaseOutputBuffer(encoder, encodeOutputIndex, false);
            }

            AMediaCodec_releaseOutputBuffer(decoder, outputIndex, false);
        }
    }

    // Clean up
    fclose(inputFd);
    fclose(outputFd);
    AMediaCodec_stop(decoder);
    AMediaCodec_delete(decoder);
    AMediaCodec_stop(encoder);
    AMediaCodec_delete(encoder);
    AMediaFormat_delete(inputFormat);
    AMediaFormat_delete(outputFormat);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_videoprocessing_MainActivity_processVideo(
        JNIEnv *env,
        jobject /* this */,
        jstring inputFile,
        jstring outputFile) {
    const char *inputFileCStr = env->GetStringUTFChars(inputFile, nullptr);
    const char *outputFileCStr = env->GetStringUTFChars(outputFile, nullptr);

    // Call decodeAndEncodeVideo function
    decodeAndEncodeVideo(inputFileCStr, outputFileCStr);

    env->ReleaseStringUTFChars(inputFile, inputFileCStr);
    env->ReleaseStringUTFChars(outputFile, outputFileCStr);
}
