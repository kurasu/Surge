#pragma once

#include <pluginterfaces/base/funknown.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace Bitwig
{
namespace Vst3
{

/** Defines the type of a generic section. Generic sections have an arbitrary row and column size. */
typedef int GenericRemoteControlsSectionType;

/**
 * Represents the primary knob/encoder/slider section of a controller. Usually consists of 8 items.
 *
 *  If the controller has only one section of controls, this is the one (even if they are sliders).
 *  This is usually identified by the proximity to display and page-navigation controls.
*/
const GenericRemoteControlsSectionType GENERIC_REMOTE_CONTROLS_SECTION_TYPE_MAIN = 0;

/**
 * Typically a bank of 9 sliders. This is very common among keyboard controllers having a main section
 * of 8 encoders (or knobs) along with 9 sliders, where the encoders are mapped as the main section and
 * the slider perform an auxiliary function.
 *
 *  They are most often used as:
 *  mixer for 8 tracks + Master (handled at the host level)
 *  2 ADSR envelopes + device output level
 *  9 drawbars for tonewheel organs
 *  Level controls for EQ bands
 *
 * These will usually stay at a fixed function for the device/preset and not switch with pages.
*/
const GenericRemoteControlsSectionType GENERIC_REMOTE_CONTROLS_SECTION_TYPE_AUX_SLIDERS = 1;

/**
 * A bank of encoders (or knobs) which are located away from the Main section. Will usually stay
 * at a fixed function for the device/preset and not switch with pages.
 */
const GenericRemoteControlsSectionType GENERIC_REMOTE_CONTROLS_SECTION_TYPE_AUX_KNOBS = 2;

/**
 * Toggle-buttons lined up with the Main section.
 */
const GenericRemoteControlsSectionType GENERIC_REMOTE_CONTROLS_SECTION_TYPE_MAIN_BUTTONS = 3;

/**
 * Toggle-buttons lined up with the Aux controls.
 */
const GenericRemoteControlsSectionType GENERIC_REMOTE_CONTROLS_SECTION_TYPE_AUX_BUTTONS = 4;

typedef int SpecializedRemoteControlsSectionType;

/**
 * 1 X (required)
 * 2 Y (required)
 * 3 Z (optional)
 */
const SpecializedRemoteControlsSectionType SPECIALIZED_REMOTE_CONTROLSE_SECTION_TYPE_XY_PAD = 0;

/**
 * Mapped to the parameters of an ADSR envelope. Usually present on hardware synths. For controllers
 * with 9 faders in a row, use Aux Sliders instead.
 *
 * 1 Attack Time
 * 2 Decay Time
 * 3 Sustain Level
 * 4 Release Time
 * 5 Hold Time
 * 6 Attack Curve
 * 7 Decay Curve
 * 8 Release Curve
 */
const SpecializedRemoteControlsSectionType SPECIALIZED_REMOTE_CONTROLSE_SECTION_TYPE_ADSR = 1;

/**
 * 1 Cutoff (required)
 * 2 Reso (required)
 * Envelope Amount
 * Keytrack
 * Filter type
 * High-pass
 */
const SpecializedRemoteControlsSectionType SPECIALIZED_REMOTE_CONTROLSE_SECTION_TYPE_FILTER = 2;

/** Defines the id of a page that was created. This is used to link the pages. */
typedef Steinberg::uint64 RemoteControlsPageId;

/**
 * @brief The IRemoteControlsSupport interface should be implemented by the host and
 * is called by the plug-in when it's remote controls have changed and should be queried again.
 *
 * - [host imp]
 */
class IRemoteControlsSupport : public Steinberg::FUnknown
{
public:

   /** Should be called by the plug-in if the remote controls change in any way. This requests that the
       host the gets the latest pages by calling IRemoteConrolsProvider::defineRemoteControls at a time
       convenient to the host. */
   virtual void PLUGIN_API remoteControlsChanged() = 0;

   /** Requests that the host show the remote controls page with the supplied page id.
       The plug-in may like to call this when the user is interacting with a certain part of the
       plug-in UI for example so that the selected page refers to controls the user is (or is likely to)
       interact with. */
   virtual void PLUGIN_API selectRemoteControlsPage(RemoteControlsPageId pageId) = 0;

   static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID(IRemoteControlsSupport, 0x0e3bc625, 0x089545cf, 0x959e6446, 0x38c6c9c8)

/**
 * @brief The IRemoteControlsReceiver interface should be implemented by the host and lets
 * a plug-in define remote control pages. The host provides an implementation of this interface
 * to the plug-in when it calls the IRemoteControlsProvider::defineRemoteControls method.
 *
 * Remote control pages allow the plug-in to be controlled easily from hardware by organising
 * its parameters into useful named pages of controls.
 *
 * Every method is expected to be called from the main thread. All strings should be encoded in UTF-8
 * and null terminated.
 *
 * - [host imp]
 */
class IRemoteControlsReceiver
{
public:

   /** Defines the generic section that following calls to beginRemoteControlsPage below to. */
   virtual void PLUGIN_API beginGenericControlsSection(GenericRemoteControlsSectionType sectionType, int columnCount, int rowCount) = 0;

   /** Defines the specialized section that following calls to beginRemoteControlsPage below to. */
   virtual void PLUGIN_API beginSpecializedControlsSection(SpecializedRemoteControlsSectionType sectionType) = 0;

   /** A page group allows the pages to be grouped into a hierarchy for optional organisation and navigation by the user.
       This method does not need to be called if there is no page hierarchy and the host is also free to ignore this information if it wants.
       The #endRemoteControlsPageGroup method should be called when all pages in a group have been defined.
       A modular plug-in may decide to group pages based on each module for example. */
   virtual void PLUGIN_API beginRemoteControlsPageGroup(Steinberg::Vst::CString nameUTF8) = 0;

   /** Starts a new empty page of remote controls with the supplied name. */
   virtual void PLUGIN_API beginRemoteControlsPage(RemoteControlsPageId pageId, Steinberg::Vst::CString nameUTF8) = 0;

   /** Adds a parameter mapping to the currently defined page (by calling #beginRemoteControlsPage()).
       @param slotIndex An index into the page where the mapping should be made.
       @param paramID The id of the parameter that should be mapped. */
   virtual void PLUGIN_API addRemoteControl(Steinberg::uint32 slotIndex, Steinberg::Vst::ParamID paramID) = 0;

   /** Sets an optional name for the last mapping added with #addRemoteControl. */
   virtual void PLUGIN_API setRemoteControlName(Steinberg::Vst::CString nameUTF8) = 0;

   /** Sets a range that the current remote control's parameter will be adjusted over when using the remote control.
       If not called then the default is assumed to be the full range (i.e 0...1). */
   virtual void PLUGIN_API setRemoteControlNormalizedRange(double minNormalized, double maxNormalized) = 0;

   /** Should be called when a page has finished being defined. This must be called once for every call to #beginRemoteControlsPage */
   virtual void PLUGIN_API endRemoteControlsPage() = 0;

   /** Should be called when a page group has finished being defined. This must be called once for every call to #beginRemoteControlsPageGroup */
   virtual void PLUGIN_API endRemoteControlsPageGroup() = 0;

   /** Should be called once all pages within a section (started with beginGenericControlsSection or beginSpecializedControlsSection) have been defined. */
   virtual void PLUGIN_API endRemoteControlsSection() = 0;

   /** When a page is shown it is possible to automatically cause another page to be shown (as long as it is in a different section).
       For example, when switching main pages it may be nice to switch to a different auxiliary page automatically. */
   virtual void PLUGIN_API linkRemoteControlPages(RemoteControlsPageId shownPage, RemoteControlsPageId pageToShow) = 0;
};

/**
 * IRemoteControlsProvider interface should be implemented by the plug-in if it can provide remote
 * controls to the host.
 */
class IRemoteControlsProvider : public Steinberg::FUnknown
{
public:

   /** Called by the host to ask the plug-in to define the remote controls that are available. */
   virtual void PLUGIN_API defineRemoteControls(IRemoteControlsReceiver* receiver) = 0;

   static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID(IRemoteControlsProvider, 0xf185556c, 0x5ee24fc7, 0x92f28754, 0xb7759ea8)

struct MappingSourceInfo
{
   Steinberg::Vst::String128 vendor;
   Steinberg::Vst::String128 model;
   int instanceNumber;
   Steinberg::Vst::String128 controlName;
   int controlBankIndex;
};

/**
 * Interface that can be implemented by the plug-in in order to hear about how a remote control is mapped within the host.
 * The plug-in can then show these mappings to the user in some useful way (e.g as a tooltip, or some highlight on mapped parameters, etc).
 */
class IRemoteControlsMappingListener : public Steinberg::FUnknown
{
public:

   /** Called by the host when a new mapping is added from a source to a parameter. Typically the source will be some
       hardware controller connected to the host. */
   virtual void PLUGIN_API remoteControlMappingAdded(RemoteControlsPageId pageId, Steinberg::uint32 slotIndex, MappingSourceInfo* sourceInfo) = 0;

   /** Called by the host when a new mapping is removed from a source to a parameter. Typically the source will be some
    hardware controller connected to the host. */
   virtual void PLUGIN_API remoteControlMappingRemoved(RemoteControlsPageId pageId, Steinberg::uint32 slotIndex, MappingSourceInfo* sourceInfo) = 0;

   static const Steinberg::FUID iid;
};

DECLARE_CLASS_IID(IRemoteControlsMappingListener, 0x36b944b2, 0xa60c40fa, 0xbd8ff856, 0xb438a9d7)

}
}
