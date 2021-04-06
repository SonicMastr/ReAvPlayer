#include <kernel.h>
#include <videodec.h>
#include <scetypes.h>
#include <avcdec.h>
#include <libsysmodule.h>
#include <taihen.h>
#include <libdbg.h>

#ifndef NDEBUG
#define printf sceClibPrintf
#else
#define printf 
#endif

#define HOOK_NUM 5
tai_module_info_t info;
SceUID hook[HOOK_NUM];
tai_hook_ref_t hook_ref[HOOK_NUM];

int pictureInt = 0;
int buffering_skip = 0;

static SceInt32 _sceVideodecQueryMemSize_patch(int a, void *b, int c)
{
	sceVideodecSetConfigInternal(0x1001, 2);

	sceAvcdecSetDecodeMode(0x1001, 0x80);

	printf("Ran sceVideodecQueryMemSize_patch\n");
	return sceVideodecQueryMemSizeInternal(a, b, c);
}

static SceInt32 _sceAvcdecQueryDecoderMemSize_patch(int a, void *b, int c)
{
	printf("Ran sceAvcdecQueryDecoderMemSize_patch\n");
	return sceAvcdecQueryDecoderMemSizeInternal(a, b, c);
}

static SceInt32 _sceVideodecInitLibraryWithUnmapMem_patch(int a, void *b, void *c)
{
	printf("Ran _sceVideodecInitLibraryWithUnmapMem_patch\n");
	return sceVideodecInitLibraryWithUnmapMemInternal(a, b, c);
}

static SceInt32 _sceAvcdecCreateDecoder_patch(int a, SceAvcdecCtrl *b, void *c)
{
	printf("Ran sceAvcdecCreateDecoder_patch\n");
	return sceAvcdecCreateDecoderInternal(a, b, c);
}

static SceInt32 _sceAvcdecDecode_patch(SceAvcdecCtrl *a, const SceAvcdecAu *b, SceAvcdecArrayPicture *c)
{
	SceAvcdecArrayPicture wap;
	printf("Ran sceAvcdecDecode_patch\n");
	printf("SceAvcdecCtrl frameBuf.size %d\n", a->frameBuf.size);
	int ret = sceAvcdecDecodeAuInternal(a,b,&pictureInt);
	printf("sceAvcdecDecodeAuInternal() 0x%08x\n", ret);
	ret = sceAvcdecDecodeGetPictureWithWorkPictureInternal(a, c, &wap, &pictureInt);
	printf("sceAvcdecDecodeGetPictureWithWorkPictureInternal() 0x%08x\npicture array: numOfOutput %d\n numOfElm %d\n", ret, c->numOfOutput, c->numOfElm);
	return ret;
}

int module_start(SceSize argc, void *args)
{
	int ret = sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER);
	printf("## Start AV Player Module: %d\n", ret);
	hook[0] = taiHookFunctionImport(&hook_ref[0], "SceAvcodecUser", 0xA166C96E, 0xAF9BDDA7, _sceVideodecQueryMemSize_patch);
	hook[1] = taiHookFunctionImport(&hook_ref[1], "SceAvcodecUser", 0xA166C96E, 0x0B47EBC1, _sceAvcdecQueryDecoderMemSize_patch);
	hook[2] = taiHookFunctionImport(&hook_ref[2], "SceAvcodecUser", 0xA166C96E, 0x3A8F0E2B, _sceVideodecInitLibraryWithUnmapMem_patch);
	hook[3] = taiHookFunctionImport(&hook_ref[3], "SceAvcodecUser", 0xA166C96E, 0x95EF1A0C, _sceAvcdecCreateDecoder_patch);
	hook[4] = taiHookFunctionImport(&hook_ref[4], "SceAvcodecUser", 0xA166C96E, 0xCDB74E5D, _sceAvcdecDecode_patch);
	printf("Hook 0: 0x%08x\n", hook[0]);
	printf("Hook 1: 0x%08x\n", hook[1]);
	printf("Hook 2: 0x%08x\n", hook[2]);
	printf("Hook 3: 0x%08x\n", hook[3]);
	printf("Hook 4: 0x%08x\n", hook[4]);

	info.size = sizeof(info);
	taiGetModuleInfo("SceAvPlayer", &info);
	taiInjectData(info.modid, 0x00, 0xC2A6, "\xBE\xF5\x80\x5F", 0x04);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop()
{
	sceSysmoduleUnloadModule(SCE_SYSMODULE_AVPLAYER);
	printf("## MODULE STOP ##\n\n");
	int i = 0;
	while (i < HOOK_NUM){
		taiHookRelease(hook[i], hook_ref[i]);
		i++;
	}
	return SCE_KERNEL_STOP_SUCCESS;
}
