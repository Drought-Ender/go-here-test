#ifndef _JSYSTEM_JAI_JAIGLOBALPARAMETER
#define _JSYSTEM_JAI_JAIGLOBALPARAMETER

#include "types.h"

namespace JAIGlobalParameter {
void setParamInitDataPointer(void*);
void setParamSoundSceneMax(u32);
void setParamSeTrackMax(u32);
void setParamSeqPlayTrackMax(u32);
void setParamSeqControlBufferMax(u32);
void setParamAutoHeapMax(u32);
void setParamStayHeapMax(u32);
void setParamDistanceMax(f32);
void setParamMaxVolumeDistance(f32);
void setParamMinDistanceVolume(f32);
void setParamStreamInsideBufferCut(bool);
void setParamAutoHeapRoomSize(u32);
void setParamStayHeapSize(u32);
void setParamInitDataFileName(char*);
void setParamSequenceArchivesFileName(char*);
void setParamDistanceParameterMoveTime(u8);
void setParamAudioCameraMax(u32);
void setParamSystemTrackMax(s32);
void setParamSystemRootTrackMax(s32);
void setParamSoundOutputMode(u32);
u32 getParamSeCategoryMax();
u32 getParamSoundSceneMax();
u32 getParamSeRegistMax();
u32 getParamSeTrackMax();
u32 getParamSeqTrackMax();
u32 getParamSeqControlBufferMax();
u32 getParamAutoHeapMax();
u32 getParamStayHeapMax();
u32 getParamSeqPlayTrackMax();
f32 getParamDistanceMax();
f32 getParamMaxVolumeDistance();
f32 getParamMinDistanceVolume();
u32 getParamStreamDecodedBufferBlocks();
u32 getParamAutoHeapRoomSize();
u32 getParamStayHeapSize();
f32 getParamSeDolbyCenterValue();
char* getParamInitDataFileName();
char* getParamWavePath();
char* getParamSequenceArchivesPath();
char* getParamStreamPath();
char* getParamAudioResPath();
char* getParamSequenceArchivesFileName();
u32 getParamDopplarMoveTime();
u8 getParamDistanceParameterMoveTime();
u32 getParamDummyObjectLifeTime();
u32 getParamDummyObjectMax();
u8 getParamSeqMuteVolumeSePlay();
u32 getParamSeqMuteMoveSpeedSePlay();
u32 getParamAudioCameraMax();
u8 getParamSeqParameterLines();
u8 getParamStreamParameterLines();
u16 getParamSeDistanceWaitMax();

// unused/inlined:
u8 getParamAudioDvdThreadPriority();
void setParamAudioDvdThreadPriority(u8);
void setParamAudioResPath(char*);
u8 getParamAudioSystemThreadPriority();
void setParamAudioSystemThreadPriority(u8);
void setParamDopplarMoveTime(u32);
f32 getParamDopplarParameter();
void setParamDopplarParameter(f32);
void setParamDummyObjectLifeTime(u32);
void setParamDummyObjectMax(u32);
bool getParamInitDataLoadOffFlag();
void setParamInitDataLoadOffFlag(bool);
void* getParamInitDataPointer();
u8 getParamInitFileLoadSwitch();
void setParamInitFileLoadSwitch(u8);
f32 getParamInputGainDown();
void setParamInputGainDown(f32);
u32 getParamInterfaceHeapSize();
void setParamInterfaceHeapSize(u32);
f32 getParamOutputGainUp();
void setParamOutputGainUp(f32);
void setParamPanAngleParameter(f32);
void setParamPanDistanceMax(f32);
u16 getParamSeDistanceFxParameter();
void setParamSeDistanceFxParameter(u16);
void setParamSeDistancepitchMax(f32);
void setParamSeDistanceWaitMax(u16);
f32 getParamSeDolbyBehindDistanceMax();
void setParamSeDolbyBehindDistanceMax(f32);
void setParamSeDolbyCenterValue(u8);
f32 getParamSeDolbyFrontDistanceMax();
void setParamSeDolbyFrontDistanceMax(f32);
bool getParamSeqEntryCancelFlag();
void setParamSeqEntryCancelFlag(bool);
void setParamSeqMuteMoveSpeedSePlay(u32);
void setParamSeqMuteVolumeSePlay(u8);
void setParamSeqParameterLines(u8);
void setParamSeqTrackMax(u32);
void setParamSequenceArchivesPath(char*);
void setParamSeRegistMax(u32);
u32 getParamSoundOutputMode();
char* getParamSoundTableFileName();
void setParamSoundTableFileName(char*);
u32 getParamStreamControlBufferMax();
void setParamStreamControlBufferMax(u32);
void setParamStreamDecodedBufferBlocks(u32);
bool getParamStreamEntryCancelFlag();
void setParamStreamEntryCancelFlag(bool);
u32 getParamStreamParameterBufferMax();
void setParamStreamParameterBufferMax(u32);
void setParamStreamParameterLines(u8);
void setParamStreamPath(char*);
bool getParamStreamUseOffFlag();
void setParamStreamUseOffFlag(bool);
s32 getParamSystemTrackMax();
s32 getParamSystemRootTrackMax();
void setParamWavePath(char*);

extern u8 distanceParameterMoveTime;
extern u8 audioSystemThreadPriority;
extern u8 audioDvdThreadPriority;
extern u8 seqMuteVolumeSePlay;
extern u8 seqParameterLines;
extern u8 streamParameterLines;
extern u16 seDistanceFxParameter;
extern u32 soundSceneMax;
extern u32 seRegistMax;
extern u32 seTrackMax;
extern u32 seqTrackMax;
extern u32 seqPlayTrackMax;
extern u32 seqControlBufferMax;
extern u32 streamControlBufferMax;   // unused/inlined
extern u32 streamParameterBufferMax; // unused/inlined
extern u32 autoHeapMax;
extern u32 stayHeapMax;
extern u32 autoHeapRoomSize;
extern u32 stayHeapSize;
extern char* soundTableFileName; // unused/inlined
extern char* initDataFileName;
extern char* wavePath;
extern char* sequenceArchivesPath;
extern char* streamPath;
extern char* audioResPath;
extern char* sequenceArchivesFileName;
extern f32 inputGainDown;
extern f32 outputGainUp;
extern f32 distanceMax;
extern f32 maxVolumeDistance;
extern f32 seDolbyCenterValue;
extern f32 seDolbyFrontDistanceMax;
extern f32 seDolbyBehindDistanceMax;
extern u32 dopplarMoveTime;
extern u32 dummyObjectLifeTime;
extern u32 dummyObjectMax;
extern u32 seqMuteMoveSpeedSePlay;
extern u32 audioCameraMax;
extern s32 systemTrackMax;
extern s32 systemRootTrackMax;
extern f32 panDistanceMax;
extern f32 panDistance2Max;
extern f32 panAngleParameter;
extern f32 panAngleParameter2;
extern f32 dopplarParameter;
extern u16 seDistanceWaitMax;
extern f32 seDistancepitchMax;

extern u16 seDefaultFx;
extern u32 interfaceHeapSize;
extern f32 minDistanceVolume;
} // namespace JAIGlobalParameter

#endif
