/* This file was automatically generated from XML paramdefs. */
#pragma once

namespace from {
namespace paramdef {
/**
 * @brief This struct was automatically generated from XML paramdefs.
 * 
 */
struct MAP_DEFAULT_INFO_PARAM_ST {
    /**
     * @brief Do you remove it from the NT version output?
     *
     * Parameters marked with ○ are excluded in the NT version package.
     */
    bool disableParam_NT : 1 { false };

    /**
     * @brief Reserve for package output 1
     */
    unsigned char disableParamReserve1 : 7;

    /**
     * @brief Reserve for package output 2
     */
    unsigned char disableParamReserve2[3];

    /**
     * @brief Fast travel permission flag ID
     */
    unsigned int EnableFastTravelEventFlagId{ 0 };

    /**
     * @brief Weather lottery time offset (in-game seconds)
     *
     * Offset for weather lottery time (unit: in-game seconds). Divide by the
     * in-game time scale of the game properties to get real time
     */
    int WeatherLotTimeOffsetIngameSeconds{ 0 };

    /**
     * @brief ID for limiting the generation of weather generation assets
     *
     * ID used to limit the generation of assets generated by the weather asset
     * generation parameter .xlsm
     */
    signed char WeatherCreateAssetLimitId{ -1 };

    /**
     * @brief Map visibility type
     */
    unsigned char MapAiSightType{ 0 };

    /**
     * @brief Sound indoor, complete indoor distribution
     *
     * Set whether to divide the map GD setting "indoor" into "indoor" or
     * "completely indoor" for sound (SEQ09833).
     */
    unsigned char SoundIndoorType{ 0 };

    /**
     * @brief Reverb default settings
     *
     * Specifies the reverb default type for this map
     */
    signed char ReverbDefaultType{ -1 };

    /**
     * @brief Location information for BGM
     *
     * Specify location default information for BGM
     */
    short BgmPlaceInfo{ 0 };

    /**
     * @brief Location information for environmental sounds
     *
     * Specifies location default information for ambient sounds
     */
    short EnvPlaceInfo{ 0 };

    /**
     * @brief Map addition bank ID
     *
     * Specify the ID of the "map addition bank" of the sound to be additionally
     * loaded (-1: no map addition bank) (SEQ15725)
     */
    int MapAdditionalSoundBankId{ -1 };

    /**
     * @brief Map height information for sound [m]
     *
     * Map height information for sound [m] (SEQ15727)
     */
    short MapHeightForSound{ 0 };

    /**
     * @brief [For Legacy] Map Environment Map Time Blend Is Effective?
     *
     * Specifies whether to time blend the environment map. Only Texure in the
     * 0th time zone is shot and used. GPU load and memory are reduced. This
     * setting is valid only for Legacy (m00-m49) (SEQ16464).
     */
    bool IsEnableBlendTimezoneEnvmap{ true };

    /**
     * @brief GI resolution overwrite setting _XSS platform
     *
     * Overwrites the GI resolution used in XSS (Xbox SeriesS, Lockhart)
     */
    signed char OverrideGIResolution_XSS{ -1 };

    /**
     * @brief Map high hit switching judgment AABB_width Depth (XZ) [m]
     *
     * Map high hit switching judgment AABB_width Depth (XZ) [m] (SEQ16295)
     */
    float MapLoHiChangeBorderDist_XZ{ 40.f };

    /**
     * @brief Map high hit switching judgment AABB_ height (Y) [m]
     *
     * Map high hit switching judgment AABB_ height (Y) [m] (SEQ16295)
     */
    float MapLoHiChangeBorderDist_Y{ 40.f };

    /**
     * @brief Map high hit switching judgment play distance [m]
     *
     * Map high hit switching judgment play distance [m]. Normally the default
     * should be fine. For smaller AABB, adjust as needed. If the play is too
     * small, switching will occur frequently. Not expected if larger than AABB
     * size (SEQ 16295)
     */
    float MapLoHiChangePlayDist{ 5.f };

    /**
     * @brief Number of pixels to be judged on the back side in automatic
     * drawing group calculation
     */
    unsigned int MapAutoDrawGroupBackFacePixelNum{ 32400 };

    /**
     * @brief Player Light Intensity Scale Value
     *
     * Specify the scale to be applied to the PC and PC horse resident light
     * source on this map (SEQ16562).
     */
    float PlayerLigntScale{ 1.f };

    /**
     * @brief Does the Player light intensity scale change depending on the time
     * zone?
     *
     * Does this map change the Player light intensity scale depending on the
     * PC, PC horse resident light source, and time zone? (SEQ 16562)
     */
    bool IsEnableTimezonnePlayerLigntScale{ true };

    /**
     * @brief Do you want to disable automatic cliff wind SE?
     *
     * Do you want to disable automatic cliff wind SE? (SEQ15729)
     */
    bool isDisableAutoCliffWind{ false };

    /**
     * @brief Open character activation limit_evaluation value threshold
     */
    short OpenChrActivateThreshold{ -1 };

    /**
     * @brief Mimicry probability parameter ID by map
     *
     * Mimicry probability parameter ID by map (SEQ22471)
     */
    int MapMimicryEstablishmentParamId{ -1 };

    /**
     * @brief GI resolution overwrite setting _XSX platform
     *
     * Overwrites the GI resolution used on XSX (Xbox Series X, Anaconda)
     */
    signed char OverrideGIResolution_XSX{ -1 };

    unsigned char Reserve[7];
};

}; // namespace paramdef
}; // namespace from

static_assert(sizeof(from::paramdef::MAP_DEFAULT_INFO_PARAM_ST) == 64,
    "MAP_DEFAULT_INFO_PARAM_ST paramdef size does not match detected size");
