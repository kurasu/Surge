#include <appd/appd.h>
#include "tinyxml/tinyxml.h"
#include <pluginterfaces/base/funknown.h>
#include <surgecids.h>
#include <vt_dsp/vt_dsp_endian.h>

#if WINDOWS
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

struct fxChunkSetCustom
{
	int chunkMagic; // 'CcnK'
	int byteSize;   // of this chunk, excl. magic + byteSize

	int fxMagic; // 'FPCh'
	int version;
	int fxID; // fx unique id
	int fxVersion;

	int numPrograms;
	char prgName[28];

	int chunkSize;
	// char chunk[8]; // variable
};

#pragma pack(push, 1)
struct patch_header
{
	char tag[4];
	unsigned int xmlsize, wtsize[2][3];
};
#pragma pack(pop)

#define TINYXML_SAFE_TO_ELEMENT(expr) ((expr)?(expr)->ToElement():NULL)

void loadXMLForAppd(appd_preset_file_metadata_receiver_t receiver, const void* data, int datasize, bool preset)
{
	TiXmlDocument doc;
	if (datasize)
	{
		assert(datasize < (1 << 22)); // something is weird if the patch is this big
		char* temp = (char*)malloc(datasize + 1);
		memcpy(temp, data, datasize);
		*(temp + datasize) = 0;
		// std::cout << "XML DOC is " << temp << std::endl;
		doc.Parse(temp, nullptr, TIXML_ENCODING_LEGACY);
		free(temp);

		TiXmlElement* patch = TINYXML_SAFE_TO_ELEMENT(doc.FirstChild("patch"));
		if (patch)
		{
			int revision = 0;
			patch->QueryIntAttribute("revision", &revision);

			TiXmlElement* meta = TINYXML_SAFE_TO_ELEMENT(patch->FirstChild("meta"));
			if (meta)
			{
				const char* s;

				s = meta->Attribute("category");
				if (s) receiver->add_tag(s);

				s = meta->Attribute("comment");
				if (s) receiver->set_description(s);

				s = meta->Attribute("author");
				if (s) receiver->add_creator(s);
			}
		}
		else
		{
			receiver->on_error("No <patch> element found");
		}
	}
}

void loadPatchForAppd(appd_preset_file_metadata_receiver_t receiver, const void* data, int datasize, bool preset)
{
	if (datasize <= 4)
		return;
	assert(datasize);
	assert(data);

	patch_header* ph = (patch_header*)data;
	ph->xmlsize = vt_read_int32LE(ph->xmlsize);

	if (!memcmp(ph->tag, "sub3", 4))
	{
		char* dr = (char*)data + sizeof(patch_header);
		loadXMLForAppd(receiver, dr, ph->xmlsize, preset);
	}
	else
	{
		loadXMLForAppd(receiver, data, datasize, preset);
	}
}


void surgePresetMetadataParser(const char* path, appd_preset_file_metadata_receiver_t receiver, void* arg)
{
	FILE* f = fopen(path, "rb");
	if (!f)
	{
		receiver->on_error("Couldn't open file");
		return;
	}

	fxChunkSetCustom fxp;
	fread(&fxp, sizeof(fxChunkSetCustom), 1, f);

	if ((vt_read_int32BE(fxp.chunkMagic) != 'CcnK') || (vt_read_int32BE(fxp.fxMagic) != 'FPCh') ||
		(vt_read_int32BE(fxp.fxID) != 'cjs3'))
	{
		fclose(f);
		return;
	}

	int cs = vt_read_int32BE(fxp.chunkSize);
	void* data = malloc(cs);
	assert(data);
	size_t actual_cs = fread(data, 1, cs, f);
	int error = ferror(f);
	fclose(f);

	if (error)
	{
		receiver->on_error("Couldn't read file content");
		return;
	}
	else
	{
		loadPatchForAppd(receiver, data, cs, true);
	}
	free(data);
}

void appd_init(const appd_host* host, int host_version)
{
	appd_plugin_id_set_t pluginIdSet = host->create_plugin_id_set("Vember Audio", "Surge");

	unsigned char* vst3id = (unsigned char*)SurgeControllerUID.toTUID();

	host->plugin_id_set_add_vst3_id(pluginIdSet, vst3id);
	host->plugin_id_set_add_vst2_id(pluginIdSet, 'cjs3');
	host->plugin_id_set_set_default_device_type(pluginIdSet, APPD_DEVICE_TYPE_INSTRUMENT);

	appd_preset_file_type_t fileType = host->create_preset_file_type("surge_preset", "Surge Preset", "fxp", surgePresetMetadataParser, nullptr);

#if WINDOWS

	PWSTR localAppData;
	if (!SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &localAppData))
	{
		CHAR path[4096];
		wsprintfA(path, "%S\\Surge\\patches_factory", localAppData);
		appd_preset_location_t factoryPresets = host->create_preset_location(path);
		host->preset_location_add_file_type(factoryPresets, fileType);
		host->preset_location_set_plugin_id_set(factoryPresets, pluginIdSet);

		wsprintfA(path, "%S\\Surge\\patches_3rdparty", localAppData);
		appd_preset_location_t thirdPartyPresets = host->create_preset_location(path);
		host->preset_location_add_file_type(thirdPartyPresets, fileType);
		host->preset_location_set_plugin_id_set(thirdPartyPresets, pluginIdSet);
	}

	PWSTR documentsFolder;
	if (!SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &documentsFolder))
	{
		CHAR path[4096];
		wsprintfA(path, "%S\\Surge\\", documentsFolder);

		appd_preset_location_t userPresets = host->create_preset_location(path);

		host->preset_location_add_file_type(userPresets, fileType);
		host->preset_location_set_plugin_id_set(userPresets, pluginIdSet);
		host->preset_location_set_is_for_user_presets(userPresets, true);
	}
#endif
}