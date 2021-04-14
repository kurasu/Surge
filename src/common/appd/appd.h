/**
    APPD (Audio Plug-in Preset Discovery) API.

    APPD is a new API that is under development for plug-in hosts and plug-in manufacturers.
    This API enables a plug-in host to identify where presets are found, what extensions they have, which plug-ins they apply to,
    and other metadata associated with the presets so that they can be indexed and searched for quickly within the plug-in host's browser.
    This API is not specific to any single plug-in API but allows one preset to exist for multiple plug-in APIs (e.g VST 2, VST 3, AU, AAX).

    This has a number of advantages for the user:

    * it allows them to browse for presets from one central location in a consistent way
    * the user can browse for presets without having to commit to a particular plug-in first
    * it enables the host to talk to hardware in order to browse presets on the hardware
          connected to the computer (e.g a controller like Ableton Push)

    There are 3 kinds of objects that are used to define where presets are and how to extract metadata
        out of them:

    * Plug-in id set: This is a set of plug-in ids that define the set of plug-ins that a preset can be used for
    * Preset file type: This defines a file type for a preset (what extensions it has, a friendly name
                to show to the user, and a callback to parse metadata out of a preset when the host discovers
                files of this type.
    * Preset location: This is a path to a directory on the file system that defines where preset file can be found
                and which preset file types should be detected below the location.
                The location may optionally be associated with a Plug-in id set if the location is specific to a set of
                plug-in ids.

    The design of APPD deliberately does not define fixed tags or categories. It is the plug-in host's job
    to try to intelligently map the raw list of tags that are found for a preset and to process this list
    to generate something that makes sense for the host's tagging and categorization system. The reason for
    this is to reduce the work for a plug-in developer to add APPD support for their existing preset file
    format and not have to be concerned with all the different hosts and how they want to receive the metadata.

    Note: APPD itself is only concerned with the discovery of presets but not the loading of them.
    For that it is necessary to implement an extension interface depending on the plug-in system being used.
    The extension interfaces can be found in the include/extensions directory.

    Note: All strings are passed as null terminated UTF-8 encoded strings as const char*.

    https://github.com/bitwig/APPD
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MSC_VER) || defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
    #ifdef __GNUC__
      #define APPD_EXPORT __attribute__ ((dllexport))
    #else
      #define APPD_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
    #endif
#else
  #if __GNUC__ >= 4
    #define APPD_EXPORT __attribute__ ((visibility ("default")))
  #else
    #define APPD_EXPORT
  #endif
#endif

#define APPD_VERSION 1

/** A device type that identifies the type of device a particular preset is for. */
typedef int appd_device_type;

#define APPD_DEVICE_TYPE_INSTRUMENT 0
#define APPD_DEVICE_TYPE_AUDIO_EFFECT 1
#define APPD_DEVICE_TYPE_NOTE_EFFECT 2
#define APPD_DEVICE_TYPE_NOTE_DETECTOR 3

typedef void* appd_preset_file_type_t;
typedef void* appd_preset_location_t;
typedef void* appd_plugin_id_set_t;

/** Receiver that receives the metadata for a single preset file.
 * The host would define the various callbacks in this interface and the preset parser function would then call them. */
typedef const struct appd_preset_file_metadata_receiver
{
   /** If there is an error reading metadata from a file this should be called with an error message. */
   void (*on_error)(const char* error_message);

   /** Marks this file as a bank meaning that it can be assigned to the plug-in as a preset but will update the banks
       in the plug-in. */
   void (*mark_as_bank)();

   /** Marks this file a a container file meaning that it can contain other presets. */
   void (*mark_as_container_file)();

   /** If the file being parsed is a preset container file (mark_as_container_file has been called)
       then this should be called for every preset in the file and before any preset metadata is sent with the
       calls below. If the file is not a container file then this should not be called at all.
       The path defines a human friendly path to the preset in the container file. It should be unique within the container file.
       The preset_load_id is a machine friendly string used to load the preset inside the container via a plug-in extension
       (e.g the IPresetContainerFileLoading interface for VST3). The preset_load_id can also just be the path if that's what
       the extension wants but it could also be some other unique id like a database primary key or something. It's use is
       entirely up to the plug-in. */
   void (*begin_contained_preset)(const char* path, const char* preset_load_id);

   /** Sets the device type for the preset being parsed.
       If a plug-in id set is assocted with this preset (e.g by associating it with the location or by calling
       set_plugin_id_set) and the id set has a known device type then that will be used if this is not called.*/
    void (*set_device_type)(appd_device_type device_type);

    /** Registers a tag that exists in the preset file.
        The caller need not do any processing on this tag information but just return the raw data found in the preset.
        It is the host's job to make sense of the tag (perhaps using it as a category, sub category, or changing the
        case to match the host's tagging system). */
    void (*add_tag)(const char* tag);

    /** Adds a creator name for the preset. */
    void (*add_creator)(const char* creator);

    /** Sets a description of the preset. */
    void (*set_description)(const char* description);

    /** Sets plug-in id set that this preset can be used with.
        If a plug-in id set was already associated with the location of this preset then that
        will be used by default if this is not called.
    */
    void (*set_plugin_id_set)(appd_plugin_id_set_t id_set);

} *appd_preset_file_metadata_receiver_t;

/** The type of a preset parser function that gets registered with a preset file type. */
typedef void (*appd_preset_metadata_parser_function_t)(
      const char* path,
      appd_preset_file_metadata_receiver_t metadata_receiver,
      void* metadata_parser_function_arg);

/** Host callback interface. This is used to define the plug-in id sets used, the preset file types,
    and the preset locations. The callbacks in this interface should only be called by the appd_init function. */
struct appd_host
{
   /** Creates a new empty plug-in id set that can be used to group one or more ids for different plugin APIs. */
   appd_plugin_id_set_t (*create_plugin_id_set)(const char* plugin_vendor, const char* plugin_name);

   /** Adds a VST 2 plug-in identifier to the plug-in id set.

      @param id_set The set of ids that should be added to.

      @param id The integer identifier of the VST2 plug-in
   */
   void (*plugin_id_set_add_vst2_id)(appd_plugin_id_set_t id_set, int id);

   /** Adds a VST 3 plug-in identifier to the plug-in id set.

      @param id_set The set of ids that should be added to.

      @param id The 16-byte array defining the id of the plug-in
   */
   void (*plugin_id_set_add_vst3_id)(appd_plugin_id_set_t id_set, unsigned char id[16]);

   /** Adds a Audio Unit plug-in identifier to the plug-in id set.

      @param id_set The set of ids that should be added to.

      @param type The 4-byte sequence defining the type of the plug-in

      @param sub_type The 4-byte sequence defining the sub-type of the plug-in

      @param manufacturer_id The 4-byte sequence defining the manufacturer of the plug-in
   */
   void (*plugin_id_set_add_audio_unit_id)(appd_plugin_id_set_t id_set, unsigned char type[4], unsigned char sub_type[4], unsigned char manufacturer_id[4]);

   /** Adds an AAX plug-in identifier to the plug-in id set.

      @param id_set The set of ids that should be added to.

      @param id The string defining the id of the plug-in
   */
   void (*plugin_id_set_add_aax_id)(appd_plugin_id_set_t id_set, const char* id);

   /** Sets a default device type for the supplied plug-in id set. */
   void (*plugin_id_set_set_default_device_type)(appd_plugin_id_set_t id_set, appd_device_type device_type);

    /** Defines information for a file type for presets.

      @param id A unique string id for the file type (unique for the implementation calling it)
      @param name A user friendly name for this file type
      @param extension The extension of this file type without the preceeding "."
      @param metadata_parser_function An optional function that can be called to parse detailed metadata from a preset
              identified to be the file type being defined. If no additional metadata can be parsed then pass NULL and
              make sure to call the set_preset_location_plugin_id_for_api so the host knows which plug-in the presets are for.
    */
    appd_preset_file_type_t (*create_preset_file_type)(const char* id,
                                                       const char* name,
                                                       const char* extension,
                                                       appd_preset_metadata_parser_function_t metadata_parser_function,
                                                       void* metadata_parser_function_arg);

    /**
     * Defines a location where presets saved by the user can be found for the last file type defined by a call to add_preset_file_type.

       @param location A path to the location of the file system.
    */
    appd_preset_location_t (*create_preset_location)(const char* location);

    /**
      Adds a preset file type to the supplied location. The host will then look for these file types below this location and index them.
    */
    void (*preset_location_add_file_type)(appd_preset_location_t location, appd_preset_file_type_t file_type);

    /** Indicates whether the preset location is for user presets or not. By default a location is not for user presets. */
    void (*preset_location_set_is_for_user_presets)(appd_preset_location_t location, bool value);

    /** Associates a plug-in id set with the supplied location. This is useful if the preset does not contain plug-in id information
        but the location where it is found indicates the plug-in it is for. */
    void (*preset_location_set_plugin_id_set)(appd_preset_location_t location, appd_plugin_id_set_t id_set);
};

typedef void (*appd_init_t)(const appd_host* host, int host_version);

/**
 * Entry point to the APPD preset system. This function shoud be implemented in the plug-in library and exported.

    @param host The host callback interface. The callbacks in this interface should not be called outside of the appd_init function.
    @param host_version The version number of the APPD API that the host implements (currently this is just 1).

*/
APPD_EXPORT void appd_init(const appd_host* host, int host_version);

#ifdef __cplusplus
}
#endif
