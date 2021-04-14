#pragma once

namespace Bitwig {

namespace Vst2 {

namespace PresetFileLoading {

/** Identifiers to be passed to VST2's canDo() method to check if the plug-in supports preset loading */
const char* canDoPresetFileLoadingName = "BitwigPresetFileLoading";

const char* canDoPresetContainerFileLoadingName = "BitwigPresetContainerFileLoading";

/**
 * The host asks the plug-in to load a preset from a file path by calling the plug-in's dispatch function with the following arguments:
 *
 * opcode - effVendorSpecific
 * index - Bitwig::Vst2::PresetLoading::kLoadPresetFromFile
 * value - file path as a UTF-8 encoded null terminated C string
 * ptr - A pointer to an output error message in case the plug-in cannot load the preset. This should be of type char[128].
 * opt - Ignored
 *
 * The return value should be 0 on failure (in which case the error message output array should be filled with an error message)
 * or any other value to indicate success.
 *
*/
const int kLoadPresetFromFile = ('B' << 24 | 'W' << 16 | 'S' << 8 | 1);

/** Structure containing the path to the file */
struct FilePathAndPresetLoadId
{
   /** file path as a UTF-8 encoded null terminated C string */
   const char* filePath;

   /** preset load id as a UTF-8 encoded null terminated C string */
   const char* presetLoadId;
};

/**
 * The host asks the plug-in to load a preset from a container file path by calling the plug-in's dispatch function with the following arguments:
 *
 * opcode - effVendorSpecific
 * index - Bitwig::Vst2::PresetLoading::kLoadPresetFromContainerFile
 * value - A pointer to a FilePathAndPresetLoadId that defines the file path and the preset load identifier (as provided by APPD, for example).
 * ptr - A pointer to an output error message in case the plug-in cannot load the preset. This should be of type char[128].
 * opt - Ignored
 *
 * The return value should be 0 on failure (in which case the error message output array should be filled with an error message)
 * or any other value to indicate success.
 *
*/
const int kLoadPresetFromContainerFile = ('B' << 24 | 'W' << 16 | 'S' << 8 | 2);

}
}
}
