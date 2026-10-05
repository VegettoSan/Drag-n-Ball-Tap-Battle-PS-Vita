#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int32_t dbtb_start(void);
int32_t dbtb_frame(void * events);
void dbtb_present(void);
int32_t dbtb_resource(void * name);
int32_t dbtb_resourceEncoding(void);
int32_t dbtb_installedData(void);
int32_t dbtb_textEncoding(int32_t source);
void dbtb_copyResource(void * target, int32_t size);
int32_t dbtb_readSave(void * name);
int32_t dbtb_writeSave(void * name, void * bytes, int32_t size, int32_t position, int32_t truncate);
int32_t dbtb_deleteSave(void * name);
int32_t dbtb_loadTexture(void * bytes, int32_t size, int32_t linear);
int32_t dbtb_textureWidth(int32_t id);
int32_t dbtb_textureHeight(int32_t id);
int32_t dbtb_emptyTexture(int32_t w, int32_t h);
int32_t dbtb_createText(int32_t w, int32_t h);
void dbtb_clearText(int32_t id);
int32_t dbtb_drawText(int32_t id, void * text, int32_t length, int32_t size, int32_t r, int32_t g, int32_t b, int32_t a, void * bounds);
int32_t dbtb_textTexture(int32_t id);
void dbtb_disposeText(int32_t id);
int32_t dbtb_effectLoad(void * name);
void dbtb_effectPlay(int32_t id, float gain);
void dbtb_effectStop(void);
void dbtb_audioDispose(void);
int32_t dbtb_voiceLoad(void * bytes, int32_t size);
void dbtb_voicePlay(int32_t id, float gain);
void dbtb_voiceRelease(void);
void dbtb_voiceStop(void);
int32_t dbtb_bgmPlay(void * name, float gain, int32_t loop);
void dbtb_bgmStop(void);
void dbtb_unsupported(void * message);
void dbtb_glBindTexture(void *receiver, int32_t target, int32_t texture);
void dbtb_glBlendFunc(void *receiver, int32_t src, int32_t dst);
void dbtb_glClear(void *receiver, int32_t mask);
void dbtb_glClearColor(void *receiver, float r, float g, float b, float a);
void dbtb_glColor4f(void *receiver, float r, float g, float b, float a);
void dbtb_glDisable(void *receiver, int32_t cap);
void dbtb_glDisableClientState(void *receiver, int32_t cap);
void dbtb_glEnable(void *receiver, int32_t cap);
void dbtb_glEnableClientState(void *receiver, int32_t cap);
void dbtb_glHint(void *receiver, int32_t target, int32_t mode);
void dbtb_glLoadIdentity(void *receiver);
void dbtb_glMatrixMode(void *receiver, int32_t mode);
void dbtb_glOrthof(void *receiver, float l, float r, float b, float t, float near, float far);
void dbtb_glPopMatrix(void *receiver);
void dbtb_glPushMatrix(void *receiver);
void dbtb_glScalef(void *receiver, float x, float y, float z);
void dbtb_glShadeModel(void *receiver, int32_t mode);
void dbtb_glTexEnvf(void *receiver, int32_t target, int32_t name, float value);
void dbtb_glTexParameterf(void *receiver, int32_t target, int32_t name, float value);
void dbtb_glTranslatef(void *receiver, float x, float y, float z);
void dbtb_glViewport(void *receiver, int32_t x, int32_t y, int32_t w, int32_t h);
void dbtb_glBindFramebuffer(void *receiver, int32_t target, int32_t id);
int32_t dbtb_glCheckFramebufferStatus(void *receiver, int32_t target);
void dbtb_glFramebufferTexture2D(void *receiver, int32_t target, int32_t attachment, int32_t textarget, int32_t texture, int32_t level);
void dbtb_glArray(int32_t operation, int32_t n, void * data);
void dbtb_glPointer(int32_t kind, int32_t size, int32_t type, int32_t stride, void * data, int32_t bytes);
void dbtb_glDraw(int32_t mode, int32_t count, int32_t type, void * data, int32_t bytes);
#ifdef __cplusplus
}
#endif
