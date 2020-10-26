// Copyright 2019 Varjo Technologies Oy. All rights reserved.

#ifndef VARJO_H
#define VARJO_H

#include "Varjo_types.h"
#include "Varjo_events.h"
#include "Varjo_export.h"
#include "Varjo_version_defines.h"

#if defined __cplusplus
extern "C" {
#endif

/**
 * Checks whether Varjo system is available.
 *
 * If varjo_IsAvailable() returns false, it's guaranteed that the session cannot
 * be initialized. If this returns true, application may try
 * initiating a new session.
 *
 * This call executes fast and can be called per frame unlike #varjo_SessionInit.
 *
 * @return 0 if Varjo system is not available, 1 if it is available.
 */
VARJO_API varjo_Bool varjo_IsAvailable();

/**
 * Gets the short version of the library. Eg. "0.7.0.0".
 *
 * @return Varjo version string.
 */
VARJO_API const char* varjo_GetVersionString();

/**
 * Gets the version number of the library.
 * Use VARJO_MAKE_VERSION macro to create a version that can be
 * then used to test against the returned version number.
 *
 * eg. VARJO_MAKE_VERSION(0, 7, 0, 0) >= varjo_GetVersion()
 *
 * @return Varjo version number.
 */
VARJO_API const uint64_t varjo_GetVersion();

/**
 * Initializes a Varjo client session.
 *
 * varjo_SessionInit initializes a new Varjo session and returns an opaque
 * pointer handle representing the session which should be passed to all Varjo
 * API calls that are related to this session. When done, the session needs to
 * be cleaned up by calling #varjo_SessionShutDown.
 *
 * There can be multiple simultaneous sessions, but generally applications
 * don't need more than one. Single session can be shared between different
 * threads.
 */
VARJO_API struct varjo_Session* varjo_SessionInit(void);

/**
 * Shuts down a session and releases all the resources internally allocated
 * by Varjo session.
 *
 * Session pointer will become invalid after calling this function.
 *
 * @param session Varjo session handle.
 */
VARJO_API void varjo_SessionShutDown(struct varjo_Session* session);

/**
 * Sets session overlay priority.
 * All sessions are grouped based on this value.
 * Only one session from a group is displayed.
 * Higher priority sessions are overlaid on top of lower priority one's.
 *
 * @param session Varjo session handle.
 * @param priority Varjo session priority. Negative values allowed. Zero on SessionInit by default.
 */
VARJO_API void varjo_SessionSetPriority(struct varjo_Session* session, int32_t priority);

/**
 * Gets the current time in a format that Varjo API uses. This is a realtime
 * monotonic counter since system-specific starting point and is not affected
 * by time-of-day setting.
 *
 * @param session Varjo session handle.
 * @return Nanoseconds since epoch.
 */
VARJO_API varjo_Nanoseconds varjo_GetCurrentTime(struct varjo_Session* session);

/**
 * Creates a frame info with initial values.
 * Allocates enough data to hold all of the views.
 *
 * Use #varjo_FreeFrameInfo to free the allocated memory.
 *
 * @param session Varjo session handle.
 * @return Frame info.
 */
VARJO_API struct varjo_FrameInfo* varjo_CreateFrameInfo(struct varjo_Session* session);

/**
 * Frees an allocated frame info.
 *
 * @param frameInfo Frame info to free.
 */
VARJO_API void varjo_FreeFrameInfo(struct varjo_FrameInfo* frameInfo);

/**
 * Creates a submit info for ending a frame with default viewport layout.
 * Other fields areinitialized with default values. Allocates enough data to
 * hold all of the views.
 *
 * Use #varjo_FreeSubmitInfo to free the allocated memory.
 *
 * @param session Varjo session handle.
 * @return Submit info.
 * @deprecated varjo_SubmitInfo is not used in Layers API
 */
VARJO_DEPRECATED_API struct varjo_SubmitInfo* varjo_CreateSubmitInfo(struct varjo_Session* session);

/**
 * Frees an allocated submit info.
 *
 * @param submitInfo Submit info to free.
 * @deprecated varjo_SubmitInfo is not used in Layers API
 */
VARJO_DEPRECATED_API void varjo_FreeSubmitInfo(struct varjo_SubmitInfo* submitInfo);

/**
 * Layout default viewport configuration.
 * The default view consists of a single texture that contains all the four
 * viewports.
 *
 * This is a helper function as the viewport can be freely configured.
 *
 * @param session Varjo session handle.
 * @param viewports Pointer to #varjo_GraphicsInfo::viewCount viewports.
 * @deprecated viewports has to be calculated based on the view count (varjo_GetViewCount()) and view's dimensions
 * (varjo_GetViewDescription())
 */
VARJO_DEPRECATED_API void varjo_LayoutDefaultViewports(struct varjo_Session* session, struct varjo_Viewport* viewports);

/**
 * Forces the provided projection matrices to be centered. This may result in suboptimal resolution.
 *
 * @param session Varjo session handle.
 * @param enabled True, if the matrices need to be centered. False if off-center projection is ok.
 */
VARJO_API void varjo_SetCenteredProjection(struct varjo_Session* session, varjo_Bool enabled);

/**
 * Gets numbers of views.
 *
 * @param session Varjo session handle.
 */
VARJO_API int32_t varjo_GetViewCount(struct varjo_Session* session);

/**
 * Gets an aligned view from a projection matrix.
 *
 * Note: the projection matrix from varjo_FrameInfo might not always be axis aligned.
 *
 * @param projectionMatrix Projection matrix.
 * @return Aligned view.
 */
VARJO_API struct varjo_AlignedView varjo_GetAlignedView(double* projectionMatrix);

/**
 * Updates clip plane distances in the projection matrix.
 * Uses graphics API convention.
 *
 * @param session Varjo session handle.
 * @param projectionMatrix Projection matrix to modify.
 * @param nearClipDistance Near clip distance to use.
 * @param farClipDistance Far clip distance to use.
 */
VARJO_DEPRECATED_API void varjo_UpdateClipPlaneDistances(
    struct varjo_Session* session, double* projectionMatrix, double nearClipDistance, double farClipDistance);

/**
 * Called at the start of a frame. Obtains required metadata for the
 * frame and blocks until optimal time to start rendering so that the
 * application can use the latest possible pose data.
 * In some cases, compositor may block the call for longer duration.
 * (e.g. if it has something else to display.)
 *
 * varjo_WaitSync() and varjo_FrameGetPose() will always derive matrixes
 * from the same pose. So it's safe to call varjo_FrameGetPose() before
 * varjo_WaitSync() is complete.
 *
 * @param session Varjo session handle.
 * @param frameInfo Output pointer for frame info.
 */
VARJO_API void varjo_WaitSync(struct varjo_Session* session, struct varjo_FrameInfo* frameInfo);

/**
 * Begin rendering the frame.
 * This function should be called only from the rendering thread and before any
 * rendering work for the given frame is done.
 *
 * @param session Varjo session handle.
 * @param submitInfo Submit info to fill in the frame metadata.
 * @deprecated use varjo_BeginFrameWithLayers()
 */
VARJO_DEPRECATED_API void varjo_BeginFrame(struct varjo_Session* session, struct varjo_SubmitInfo* submitInfo);

/**
 * End the current frame and submit rendering work using the built-in swap chains.
 *
 * This function can only be called if the built-in swap chains are initialized by
 * calling either #varjo_D3D11Init or #varjo_GLInit. Alternatively, the application
 * can submit manually created swapchains using the #varjo_EndFrameWithLayers function.
 * In that case, calling #varjo_D3D11Init or #varjo_GLInit is not necessary.
 *
 * This function should only be called from the rendering thread.
 *
 * @param session Varjo session handle.
 * @param frameInfo View and projection matrices the frame was rendered with.
 * @param submitInfo Textures and viewports to submit.
 * @deprecated use varjo_EndFrameWithLayers()
 */
VARJO_DEPRECATED_API void varjo_EndFrame(struct varjo_Session* session, struct varjo_FrameInfo* frameInfo, struct varjo_SubmitInfo* submitInfo);

/**
 * Gets the time when the frame is scheduled to be displayed. This time refers to
 * the average perceived moment of when the image is shown.
 *
 * The display time is updated in #varjo_WaitSync and is guaranteed to remain
 * constant for the duration of the frame.
 *
 * This is a helper function for when you don't have access to #varjo_FrameInfo.
 *
 * @param session Varjo session handle.
 * @return Nanoseconds since epoch.
 */
VARJO_API varjo_Nanoseconds varjo_FrameGetDisplayTime(struct varjo_Session* session);

/**
 * Gets a pose for the current frame.
 *
 * If it's called before #varjo_WaitSync() is complete, it will return new pose.
 * Following calls to varjo_FrameGetPose() will return the same matrix.
 * #varjo_WaitSync() will also derive it's matrices from that pose.
 * If varjo_FrameGetPose() is called after #varjo_WaitSync() it will derive
 * matrices from that pose.
 *
 * @param session Varjo session handle.
 * @param type Type of the pose.
 * @return The pose matrix.
 */
VARJO_API struct varjo_Matrix varjo_FrameGetPose(struct varjo_Session* session, varjo_PoseType type);

/**
 * Transform from tracking space to local client space.

 * Client space may differ from tracking space if:
 * client has explicitly called varjo_ResetPose or
 * by implicit change of tracking origin for all clients.
 * Client should use varjo_GetTrackingToLocalTransform() in order to sync tracking
 * space poses (e.g. controller poses) with client local space poses.
 *
 * @param session Varjo session handle.
 * @return The pose offset matrix.
 */
VARJO_API struct varjo_Matrix varjo_GetTrackingToLocalTransform(struct varjo_Session* session);

/**
 * Gets a relative transformation from source pose to destination pose.
 *
 * @param session Varjo session handle.
 * @param src Source pose.
 * @param dest Destination pose.
 * @return Relative transformation from source pose to destination pose.
 */
VARJO_API struct varjo_Matrix varjo_GetRelativePoseTransform(struct varjo_Session* session, varjo_PoseType src, varjo_PoseType dest);

/**
 * Applies transform using post multiplication.
 *
 * @param session Varjo session handle.
 * @param m1 First transform.
 * @param m2 Second transform.
 * @return Second transformation applied to the first one.
 */
VARJO_API struct varjo_Matrix varjo_ApplyTransform(struct varjo_Session* session, struct varjo_Matrix* m1, struct varjo_Matrix* m2);

/**
 * Resets pose tracking origin.
 *
 * This will reset the tracking origin to the current transform of the user.
 * rotation parameter can be used to reset only some components of the
 * transform.
 *
 * @param session Varjo session handle.
 * @param position Reset position
 * @param rotation Rotation reset type
 */
VARJO_API void varjo_ResetPose(struct varjo_Session* session, varjo_Bool position, varjo_RotationReset rotation);

/**
 * Gets the display information for a given view.
 *
 * Use this to retrieve information about which display or eye the given view belongs to.
 *
 * #varjo_ViewDescription also contains the default render texture dimensions.
 *
 * @param session Varjo session handle.
 * @param viewIndex View number to get the display information for. Must be in range [0, #varjo_GraphicsInfo::viewCount - 1].
 * @return View description.
 */
VARJO_API struct varjo_ViewDescription varjo_GetViewDescription(struct varjo_Session* session, int32_t viewIndex);

/**
 * Creates the occlusion mesh for a given view index.
 *
 * Due to lens distortion some display pixels are not visible in the optical
 * path so the application can stencil out pixels to reduce shading workload
 * and improve performance.
 *
 * Mesh data is a triangle list where each triplet of vertices forms a triangle.
 *
 * Use #varjo_FreeOcclusionMesh to free the created mesh.
 *
 * @param session Varjo session handle.
 * @param viewIndex View number to get the occlusion mesh for. Must be in range [0, #varjo_GraphicsInfo::viewCount - 1].
 * @param windingOrder Winding order of the triangles
 * @return Occlusion mesh, null if the viewIndex invalid.
 */
VARJO_API struct varjo_Mesh2Df* varjo_CreateOcclusionMesh(struct varjo_Session* session, int32_t viewIndex, varjo_WindingOrder windingOrder);

/**
 * Frees the memory allocated by #varjo_CreateOcclusionMesh.
 *
 * @param mesh Occlusion mesh to free.
 */
VARJO_API void varjo_FreeOcclusionMesh(struct varjo_Mesh2Df* mesh);

/**
 * Gets the latest error code.
 *
 * You should check for errors at least once a frame.
 *
 * Error status will be cleared after this function.
 *
 * Use #varjo_GetErrorDesc to get more detailed error description.
 *
 * @return Error code if there was an error, #varjo_NoError if there wasn't.
 */
VARJO_API varjo_Error varjo_GetError(struct varjo_Session* session);

/**
 * Gets the current error description string in english.
 *
 * @return Error description string.
 * @deprecated use varjo_GetErrorDesc(varjo_Error)
 */
VARJO_DEPRECATED_API const char* varjo_GetErrorDescription(struct varjo_Session* session);

/**
 * Gets error description for the specified error code.
 *
 * @return Error description string.
 */
VARJO_API const char* varjo_GetErrorDesc(varjo_Error error);

/**
 * Resets the error status.
 *
 * The errors are persistent - the error code and description refer to the
 * first Varjo API call that has failed and the following API calls may fail
 * as a cascading result. Thus you should clear error status after handling
 * the error.
 *
 * @param session Varjo session handle.
 * @deprecated varjo_GetError will reset the error status.
 */
VARJO_DEPRECATED_API void varjo_ClearError(struct varjo_Session* session);

/**
 * Initializes the gaze tracking system.
 *
 * @param session Varjo session handle.
 */
VARJO_API void varjo_GazeInit(struct varjo_Session* session);

/**
 * Is gaze allowed to be used?
 *
 * User can disallow gaze tracking completely.
 *
 * @param session Varjo session handle.
 * @return varjo_True if gaze is allowed to be used, varjo_False otherwise.
 */
VARJO_API varjo_Bool varjo_IsGazeAllowed(struct varjo_Session* session);

/**
 * Gets the current state of the user gaze.
 *
 * Gaze system must have been previously initialized by calling #varjo_GazeInit.
 *
 * @param session Varjo session handle.
 * @return Current state of the user gaze.
 */
VARJO_API struct varjo_Gaze varjo_GetGaze(struct varjo_Session* session);

/**
 * Gets gaze measurements since last query
 *
 * Gaze system must have been previously initialized by calling #varjo_GazeInit.
 *
 * @param session Varjo session handle.
 * @param array Array of varjo_Gaze structures
 * @param maxSize Number of elements in array
 * @return Number of items written to array
 */
VARJO_API int32_t varjo_GetGazeArray(struct varjo_Session* session, struct varjo_Gaze* array, int32_t maxSize);

/**
 * Updates and synchronizes system properties with the system state.
 *
 * @param session Varjo session handle.
 */
VARJO_API void varjo_SyncProperties(struct varjo_Session* session);

/**
 * Gets the number of system properties.
 *
 * @param session Varjo session handle.
 * @return Number of system properties.
 */
VARJO_API int32_t varjo_GetPropertyCount(struct varjo_Session* session);

/**
 * Gets a property key for a property index.
 *
 * @param session Varjo session handle.
 * @param index Zero-based index.
 * @return Key at index location in the properties. 0 if the key index is out of bounds.
 */
VARJO_API varjo_PropertyKey varjo_GetPropertyKey(struct varjo_Session* session, int32_t index);

/**
 * Gets the name of the property key.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return Name of the property key.
 */
VARJO_API const char* varjo_GetPropertyName(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Checks if the key has value in the properties.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return 1 if the property key has a value, 0 if the value doesn't exist.
 */
VARJO_API varjo_Bool varjo_HasProperty(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Gets a boolean value of a property.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return Value of a boolean property if exists, false if the property does not exist. Undefined if the key type is not a boolean.
 */
VARJO_API varjo_Bool varjo_GetPropertyBool(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Gets a floating point value of a property.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return Value of a floating point property if exists, 0.0 if the property does not exist. Undefined if the key type is not a float.
 */
VARJO_API double varjo_GetPropertyDouble(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Gets an integer value of a property.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return Value of an integer property if exists, 0 if the property does not exist. Undefined if the key type is not an integer.
 */
VARJO_API int32_t varjo_GetPropertyInt(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Gets a string value of a property.
 *
 * A null-terminated string will be returned to the given buffer. The string will not be copied if the size is smaller than the length
 * of the property string + 1 (null terminator), i.e. truncation will not happen.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @param buffer Pointer to a char buffer.
 * @param maxSize Size of the given char buffer.
 */
VARJO_API void varjo_GetPropertyString(struct varjo_Session* session, varjo_PropertyKey propertyKey, char* buffer, uint32_t maxSize);

/**
 * Gets the size of a buffer that is big enough to hold the property, including the null terminator.
 *
 * @param session Varjo session handle.
 * @param propertyKey The property key.
 * @return Size of a buffer that is big enough to hold the string. If the string is empty, the return value will be 1
 *         (for the null terminator), and 0 if the property key does not exist or is not of a string type.
 */
VARJO_API uint32_t varjo_GetPropertyStringSize(struct varjo_Session* session, varjo_PropertyKey propertyKey);

/**
 * Requests a HMD gaze calibration.
 *
 * This attempts to trigger the gaze calibration sequence if the user has
 * allowed gaze tracking from Varjo settings and Varjo system is in a state
 * where it can bring up the calibration UI.
 *
 * @param session Varjo session handle.
 */
VARJO_API void varjo_RequestGazeCalibration(struct varjo_Session* session);

/**
 * Requests a HMD gaze calibration with provided parameters.
 *
 * This attempts to trigger the gaze calibration sequence if the user has
 * allowed gaze tracking from Varjo settings and Varjo system is in a state
 * where it can bring up the calibration UI.
 *
 * @param session Varjo session handle.
 * @param parameters Gaze calibration key-value parameters.
 * @param parameterCount Number of parameters provided.
 */
VARJO_API void varjo_RequestGazeCalibrationWithParameters(
    struct varjo_Session* session, struct varjo_GazeCalibrationParameters* parameters, int32_t parameterCount);

/**
 * Gets the default swap chain config.
 *
 * @param session Varjo session handle.
 * @return Default swap chain config.
 * @deprecated swapchain needs to be configured for each application individually
 */
VARJO_DEPRECATED_API struct varjo_SwapChainConfig varjo_GetDefaultSwapChainConfig(struct varjo_Session* session);

/**
 * Gets swap chain limits.
 *
 * @param session Varjo session handle.
 * @return Swap chain limits.
 */
VARJO_API struct varjo_SwapChainLimits varjo_GetSwapChainLimits(struct varjo_Session* session);

/**
 * Gets supported texture formats in the order of most preferred to least preferred.
 *
 * To allocate memory for the \p formats output buffer, you should first call this
 * function by passing null \p formats and query the count for the formats.
 *
 * @param session Varjo session handle.
 * @param renderApi Rendering API being used.
 * @param formatCount Output pointer for the format count. Cannot be null.
 * @param formats Output pointer for the formats. Can be null.
 */
VARJO_API void varjo_GetSupportedTextureFormats(struct varjo_Session* session, varjo_RenderAPI renderApi, int32_t* formatCount, varjo_TextureFormat* formats);

/**
 * Is the given swap chain config supported.
 *
 * @param session Varjo session handle.
 * @param renderApi Rendering API being used.
 * @param config Config to test.
 * @return varjo_True if the config is supported. If varjo_False, check #varjo_GetError for more information.
 */
VARJO_DEPRECATED_API varjo_Bool varjo_IsSwapChainConfigSupported(
    struct varjo_Session* session, varjo_RenderAPI renderApi, const struct varjo_SwapChainConfig* config);

/**
 * Gets the current swap chain texture index.
 *
 * @param session Varjo session handle.
 * @return Index of texture to which application should draw.
 * @deprecated use varjo_GetSwapchainImage()
 */
VARJO_DEPRECATED_API int32_t varjo_GetSwapChainCurrentIndex(struct varjo_Session* session);

/**
 * Allocates an event.
 *
 * Helper function if you don't want to do the allocation by yourself.
 *
 * @return Allocated event.
 */
VARJO_API struct varjo_Event* varjo_AllocateEvent();

/**
 * Free an event.
 *
 * Must be allocated with #varjo_AllocateEvent.
 *
 * @param event Event to free.
 */
VARJO_API void varjo_FreeEvent(struct varjo_Event* event);

/**
 * Polls events.
 *
 * Holds the latest 100 events in the queue.
 * If the queue size exceeds that, starts dropping events starting from the oldest.
 * Make sure to call often enough (eg. once per frame) if you don't want to miss an event.
 *
 * Use this in a loop to go through all of the queued events.
 *
 * @param session Varjo session handle.
 * @param evt Event to receive data to.
 * @return varjo_True if there was an event, varjo_False if all events have been processed.
 */
VARJO_API varjo_Bool varjo_PollEvent(struct varjo_Session* session, struct varjo_Event* evt);

#if defined __cplusplus
}
#endif

#endif
