#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace Bitwig
{
namespace Vst3
{

/**
 * @brief The IPresetFileLoading interface lets the host ask a plug-in to load a preset from a file.
 *
 * This is useful if the host discovers the plug-in presets using a system like APPD (Audio Plug-in Preset Discovery).
 *
 * Every method is expected to be called from the main thread.
 *
 * - [plug imp]
 * - [extends IComponent]
 */
class IPresetFileLoading : public Steinberg::FUnknown
{
public:
   /** Ask the plug-in to load a preset from the supplied file path.
    * @param pathUTF16 A null terminated UTF-16 encoded string for the path of the preset to load.
    * @param errorMessageOut A pointer to a (max. 128) char UTF-16 buffer that should be filled with error message and null terminated
    * @return true if the preset was successfully loaded, otherwise false and errorMessageOut should be filled with a useful error message */
   virtual Steinberg::TBool PLUGIN_API loadPresetFromFile(const Steinberg::Vst::TChar* pathUTF16, Steinberg::Vst::String128 errorMessageOut) = 0;

   static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID(IPresetFileLoading, 0x4cc80877, 0x47b04a83, 0x85a71ad4, 0x9dde6f72)

}
}
