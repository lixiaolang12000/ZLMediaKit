#ifndef MK_FFMPEG_SOURCE_H_
#define MK_FFMPEG_SOURCE_H_

#include "mk_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ffmpeg播放结束的回调
 * @param user_data 用户数据指针
 * @param key 流的唯一标识
 */
typedef void(API_CALL *on_mk_ffmpeg_close)(void *user_data, const char* key);

/**
 * 添加FFmpeg代理流
 * @param src_url 拉流地址
 * @param dst_url 推流地址
 * @param timeout_ms 拉流超时时间
 * @param cb 回调函数指针,不得为null
 * @param user_data 用户数据指针
 * @return 流的唯一标识
 */
API_EXPORT const char* API_CALL mk_add_ffmpeg_source(const char* src_url, const char* dst_url, int timeout_ms, on_mk_ffmpeg_close cb, void* user_data);

/**
 * 添加FFmpeg代理流
 * @param src_url 拉流地址
 * @param dst_url 推流地址
 * @param ffmpeg_cmd ffmpeg命令
 * @param timeout_ms 拉流超时时间
 * @param cb 回调函数指针,不得为null
 * @param user_data 用户数据指针
 * @return 流的唯一标识
 */
API_EXPORT const char* API_CALL mk_add_ffmpeg_source_cmd(const char* src_url, const char* dst_url, const char* ffmpeg_cmd, int timeout_ms, on_mk_ffmpeg_close cb, void* user_data);

/**
 * 添加FFmpeg代理流
 * @param src_url 拉流地址
 * @param dst_url 推流地址 
 * @param cb 回调函数指针,不得为null
 * @param user_data 用户数据指针
 * @return 流的唯一标识
 */
API_EXPORT const char* API_CALL mk_add_ffmpeg_source_default(const char* src_url, const char* dst_url, int timeout_ms, on_mk_ffmpeg_close cb, void* user_data);

/**
 * 删除FFmpeg代理流
 * @param key 流的唯一标识
 */
API_EXPORT void API_CALL mk_del_ffmpeg_source(const char* key);

#ifdef __cplusplus
}
#endif

#endif  // MK_FFMPEG_SOURCE_H_
