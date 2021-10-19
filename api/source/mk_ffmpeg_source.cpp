#include "mk_ffmpeg_source.h"
#include "FFmpegSource.h"
#include "Util/MD5.h"
#include <thread>
#include <chrono>

//FFmpeg拉流代理器列表
static unordered_map<string, FFmpegSource::Ptr> s_ffmpegMap;
static recursive_mutex s_ffmpegMapMtx;

struct FFmpegSourceContext {
    on_mk_ffmpeg_close cb;
    void *user_data;
    std::string key;
    FFmpegSourceContext(): user_data(nullptr) {

    }
};

static unordered_map<string, FFmpegSourceContext> s_ffmpegContextMap;
static recursive_mutex s_ffmpegContextMapMtx;

const char* mk_add_ffmpeg_source(const char* src_url, const char* dst_url, int timeout_ms, on_mk_ffmpeg_close cb, void* user_data) {

    //const char *ffmpeg_cmd = "%s -rtsp_transport tcp  -i %s -vcodec copy -acodec copy -f flv %s";
    //const char *ffmpeg_cmd = "%s -re -rtsp_transport tcp -i %s -vcodec h264 -acodec copy -f flv %s";
    // 10秒超时
    const char *ffmpeg_cmd = "%s -re -rtsp_transport tcp -i %s -vcodec libx264  -bf 0 -b:v 2048k -tune:v zerolatency -profile:v baseline -preset:v ultrafast -an -rw_timeout 10000000 -stimeout 10000000  -f flv %s";
   
    auto key = MD5(dst_url).hexdigest();
    {
        lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
        if (s_ffmpegMap.find(key) != s_ffmpegMap.end()) {
            //已经在拉流了
            return "";
        }
    }

    InfoL << "new ffmpeg key:" << key;

    FFmpegSource::Ptr ffmpeg = std::make_shared<FFmpegSource>();
    s_ffmpegMap[key] = ffmpeg;

    ffmpeg->setOnClose([key]() {
        {
            lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
            s_ffmpegMap.erase(key);
        }
        
        {
            lock_guard<decltype(s_ffmpegContextMapMtx)> lck(s_ffmpegContextMapMtx);
            auto iter = s_ffmpegContextMap.find(key);
            if (iter != s_ffmpegContextMap.end()) {
                if (iter->second.cb) {
                    iter->second.cb(iter->second.user_data, key.c_str());
                }
                s_ffmpegContextMap.erase(iter);
            }
            
        }
    });
    ffmpeg->setupRecordFlag(false, false);

    bool finished = false;
    const char* result_key = nullptr;
    ffmpeg->play(ffmpeg_cmd, src_url, dst_url, timeout_ms, [cb, user_data, key, &result_key, &finished](const SockException &ex) {
        if (ex) {
            WarnL << "mk_add_ffmpeg_source:" << ex.what();

            lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
            s_ffmpegMap.erase(key);        
        } else {
            FFmpegSourceContext ctx;
            ctx.cb = cb;
            ctx.user_data = user_data;
            ctx.key = key;            

            InfoL << "ctx.key:" << ctx.key << " result_key:" << result_key;
            lock_guard<decltype(s_ffmpegContextMapMtx)> lck(s_ffmpegContextMapMtx);
            s_ffmpegContextMap[key] = ctx;            
            result_key = s_ffmpegContextMap[key].key.c_str();
        }
        finished = true;   
    });

    while (!finished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (result_key) {
        InfoL << "ffmpeg key:" << result_key;
        return "";
    } else {
        WarnL << "start ffmpeg source failed.";
        return result_key;
    }    
}

const char* mk_add_ffmpeg_source_default(const char* src_url, const char* dst_url, int timeout_ms, on_mk_ffmpeg_close cb, void* user_data) {

    auto key = MD5(dst_url).hexdigest();
    {
        lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
        if (s_ffmpegMap.find(key) != s_ffmpegMap.end()) {
            //已经在拉流了
            return "";
        }
    }

    InfoL << "new ffmpeg key:" << key;

    FFmpegSource::Ptr ffmpeg = std::make_shared<FFmpegSource>();
    s_ffmpegMap[key] = ffmpeg;

    ffmpeg->setOnClose([key]() {
        {
            lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
            s_ffmpegMap.erase(key);
        }
        
        {
            lock_guard<decltype(s_ffmpegContextMapMtx)> lck(s_ffmpegContextMapMtx);
            auto iter = s_ffmpegContextMap.find(key);
            if (iter != s_ffmpegContextMap.end()) {
                if (iter->second.cb) {
                    iter->second.cb(iter->second.user_data, key.c_str());
                }
                s_ffmpegContextMap.erase(iter);
            }
            
        }
    });
    ffmpeg->setupRecordFlag(false, false);

    bool finished = false;
    const char* result_key = nullptr;
    ffmpeg->play(src_url, dst_url, timeout_ms, [cb, user_data, key, &result_key, &finished](const SockException &ex) {
        if (ex) {
            WarnL << "mk_add_ffmpeg_source:" << ex.what();

            lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
            s_ffmpegMap.erase(key);        
        } else {
            FFmpegSourceContext ctx;
            ctx.cb = cb;
            ctx.user_data = user_data;
            ctx.key = key;            

            InfoL << "ctx.key:" << ctx.key << " result_key:" << result_key;
            lock_guard<decltype(s_ffmpegContextMapMtx)> lck(s_ffmpegContextMapMtx);
            s_ffmpegContextMap[key] = ctx;            
            result_key = s_ffmpegContextMap[key].key.c_str();
        }
        finished = true;   
    });

    while (!finished) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (result_key) {
        InfoL << "ffmpeg key:" << result_key;
        return result_key;
    } else {
        WarnL << "start ffmpeg source failed.";
        return result_key;
    }    
}


void mk_del_ffmpeg_source(const char* key) {
    {
        lock_guard<decltype(s_ffmpegMapMtx)> lck(s_ffmpegMapMtx);
        auto iter = s_ffmpegMap.find(key);
        if (iter != s_ffmpegMap.end()) {
            s_ffmpegMap.erase(iter);
            InfoL << "del ffmpeg source:" << key << " ok.";
        } else {
            WarnL << "del ffmpeg source:" << key << " invalid.";
        }        
    }
    
    {
        lock_guard<decltype(s_ffmpegContextMapMtx)> lck(s_ffmpegContextMapMtx);
        s_ffmpegContextMap.erase(key);
    }
}