#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace Bitwig
{
namespace Vst3
{

/**
 * @brief The IPresetContainerFileLoading interface lets the host ask a plug-in to load a preset from within a container file.
 *
 * This is useful if the host discovers the plug-in presets inside a container file using a system like APPD (Audio Plug-in Preset Discovery).
 *
 * Every method is expected to be called from the main thread.
 *
 * - [plug imp]
 * - [extends IComponent]
 */
class IPresetContainerFileLoading : public Steinberg::FUnknown
{
public:
   /** Ask the plug-in to load a preset from the supplied file path.
    * @param pathUTF16 A null terminated UTF-16 encoded string for the path of the preset to load.
    * @param presetLoadIdUTF16 A null terminated UTF-16 encoded string for the load id of the preset within the container file.
    *      The load id is provided by APPD (Audio Plug-in Preset Discovery) when reading contents of container file.
    * @param errorMessageOut A pointer to a (max. 128) char UTF-16 buffer that should be filled with error message and null terminated
    * @return true if the preset was successfully loaded, otherwise false and errorMessageOut should be filled with a useful error message */
   virtual Steinberg::TBool PLUGIN_API loadPresetFromContainerFile(const Steinberg::Vst::TChar* pathUTF16, const Steinberg::Vst::TChar* presetLoadIdUTF16, Steinberg::Vst::String128 errorMessageOut) = 0;

   static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID(IPresetContainerFileLoading, 0x8b4bc7f6, 0xc3aa493f, 0x872da5b4, 0xd779daeb)

}
}
