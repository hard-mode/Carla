/*
 * Carla Native Plugin API (C++)
 * Copyright (C) 2012-2013 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#ifndef CARLA_NATIVE_HPP_INCLUDED
#define CARLA_NATIVE_HPP_INCLUDED

#include "CarlaNative.h"
#include "CarlaMIDI.h"
#include "CarlaJuceUtils.hpp"

/*!
 * @defgroup CarlaNativeAPI Carla Native API
 * @{
 */

// -----------------------------------------------------------------------
// Native Plugin Class

class NativePluginClass
{
public:
    NativePluginClass(const NativeHostDescriptor* const host)
        : pHost(host)
    {
        CARLA_ASSERT(host != nullptr);
    }

    virtual ~NativePluginClass()
    {
    }

protected:
    // -------------------------------------------------------------------
    // Host calls

    const NativeHostDescriptor* getHostHandle() const noexcept
    {
        return pHost;
    }

    const char* getResourceDir() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, nullptr);

        return pHost->resourceDir;
    }

    const char* getUiName() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, nullptr);

        return pHost->uiName;
    }

    uint32_t getBufferSize() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, 0);

        return pHost->get_buffer_size(pHost->handle);
    }

    double getSampleRate() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, 0.0);

        return pHost->get_sample_rate(pHost->handle);
    }

    bool isOffline() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, false);

        return pHost->is_offline(pHost->handle);
    }

    const NativeTimeInfo* getTimeInfo() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, nullptr);

        return pHost->get_time_info(pHost->handle);
    }

    void writeMidiEvent(const NativeMidiEvent* const event) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->write_midi_event(pHost->handle, event);
    }

    void uiParameterChanged(const uint32_t index, const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->ui_parameter_changed(pHost->handle, index, value);
    }

    void uiMidiProgramChanged(const uint8_t channel, const uint32_t bank, const uint32_t program) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->ui_midi_program_changed(pHost->handle, channel, bank, program);
    }

    void uiCustomDataChanged(const char* const key, const char* const value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->ui_custom_data_changed(pHost->handle, key, value);
    }

    void uiClosed() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->ui_closed(pHost->handle);
    }

    const char* uiOpenFile(const bool isDir, const char* const title, const char* const filter) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, nullptr);

        return pHost->ui_open_file(pHost->handle, isDir, title, filter);
    }

    const char* uiSaveFile(const bool isDir, const char* const title, const char* const filter) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, nullptr);

        return pHost->ui_save_file(pHost->handle, isDir, title, filter);
    }

    // -------------------------------------------------------------------
    // Host dispatcher calls

    void hostSetVolume(const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_VOLUME, 0, 0, nullptr, value);
    }

    void hostSetDryWet(const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_DRYWET, 0, 0, nullptr, value);
    }

    void hostSetBalanceLeft(const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_BALANCE_LEFT, 0, 0, nullptr, value);
    }

    void hostSetBalanceRight(const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_BALANCE_RIGHT, 0, 0, nullptr, value);
    }

    void hostSetPanning(const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_PANNING, 0, 0, nullptr, value);
    }

    intptr_t hostGetParameterMidiCC(const int32_t index) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr, -1);

        return pHost->dispatcher(pHost->handle, HOST_OPCODE_GET_PARAMETER_MIDI_CC, index, 0, nullptr, 0.0f);
    }

    void hostSetParameterMidiCC(const int32_t index, const intptr_t value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_PARAMETER_MIDI_CC, index, value, nullptr, 0.0f);
    }

    void hostSetProcessPrecision(const intptr_t value) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_SET_PROCESS_PRECISION, 0, value, nullptr, 0.0f);
    }

    void hostUpdateParameter(const int32_t index) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_UPDATE_PARAMETER, index, 0, nullptr, 0.0f);
    }

    void hostUpdateAllParameters() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_UPDATE_PARAMETER, -1, 0, nullptr, 0.0f);
    }

    void hostUpdateMidiProgram(const int32_t index, const intptr_t channel = 0) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_UPDATE_MIDI_PROGRAM, index, channel, nullptr, 0.0f);
    }

    void hostUpdateAllMidiPrograms(const intptr_t channel = 0) const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_UPDATE_MIDI_PROGRAM, -1, channel, nullptr, 0.0f);
    }

    void hostReloadParameters() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_RELOAD_PARAMETERS, 0, 0, nullptr, 0.0f);
    }

    void hostReloadMidiPrograms() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_RELOAD_MIDI_PROGRAMS, 0, 0, nullptr, 0.0f);
    }

    void hostReloadAll() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_RELOAD_ALL, 0, 0, nullptr, 0.0f);
    }

    void hostUiUnavailable() const
    {
        CARLA_SAFE_ASSERT_RETURN(pHost != nullptr,);

        pHost->dispatcher(pHost->handle, HOST_OPCODE_UI_UNAVAILABLE, 0, 0, nullptr, 0.0f);
    }

    // -------------------------------------------------------------------
    // Plugin parameter calls

    virtual uint32_t getParameterCount() const
    {
        return 0;
    }

    virtual const NativeParameter* getParameterInfo(const uint32_t index) const
    {
        CARLA_SAFE_ASSERT_RETURN(index < getParameterCount(), nullptr);
        return nullptr;
    }

    virtual float getParameterValue(const uint32_t index) const
    {
        CARLA_SAFE_ASSERT_RETURN(index < getParameterCount(), 0.0f);
        return 0.0f;
    }

    virtual const char* getParameterText(const uint32_t index, const float value) const
    {
        CARLA_SAFE_ASSERT_RETURN(index < getParameterCount(), nullptr);
        return nullptr;

        // unused
        (void)value;
    }

    // -------------------------------------------------------------------
    // Plugin midi-program calls

    virtual uint32_t getMidiProgramCount() const
    {
        return 0;
    }

    virtual const NativeMidiProgram* getMidiProgramInfo(const uint32_t index) const
    {
        CARLA_SAFE_ASSERT_RETURN(index < getMidiProgramCount(), nullptr);
        return nullptr;
    }

    // -------------------------------------------------------------------
    // Plugin state calls

    virtual void setParameterValue(const uint32_t index, const float value)
    {
        CARLA_SAFE_ASSERT_RETURN(index < getParameterCount(),);
        return;

        // unused
        (void)value;
    }

    virtual void setMidiProgram(const uint8_t channel, const uint32_t bank, const uint32_t program)
    {
        CARLA_SAFE_ASSERT_RETURN(channel < MAX_MIDI_CHANNELS,);
        return;

        // unused
        (void)bank;
        (void)program;
    }

    virtual void setCustomData(const char* const key, const char* const value)
    {
        CARLA_SAFE_ASSERT_RETURN(key != nullptr,);
        CARLA_SAFE_ASSERT_RETURN(value != nullptr,);
    }

    // -------------------------------------------------------------------
    // Plugin process calls

    virtual void activate()
    {
    }

    virtual void deactivate()
    {
    }

    virtual void process(float** const inBuffer, float** const outBuffer, const uint32_t frames, const NativeMidiEvent* const midiEvents, const uint32_t midiEventCount) = 0;

    // -------------------------------------------------------------------
    // Plugin UI calls

    virtual void uiShow(const bool show)
    {
        return;

        // unused
        (void)show;
    }

    virtual void uiIdle()
    {
    }

    virtual void uiSetParameterValue(const uint32_t index, const float value)
    {
        CARLA_SAFE_ASSERT_RETURN(index < getParameterCount(),);
        return;

        // unused
        (void)value;
    }

    virtual void uiSetMidiProgram(const uint8_t channel, const uint32_t bank, const uint32_t program)
    {
        CARLA_SAFE_ASSERT_RETURN(channel < MAX_MIDI_CHANNELS,);
        return;

        // unused
        (void)bank;
        (void)program;
    }

    virtual void uiSetCustomData(const char* const key, const char* const value)
    {
        CARLA_SAFE_ASSERT_RETURN(key != nullptr,);
        CARLA_SAFE_ASSERT_RETURN(value != nullptr,);
    }

    // -------------------------------------------------------------------
    // Plugin state calls

    virtual char* getState() const
    {
        return nullptr;
    }

    virtual void setState(const char* const data)
    {
        CARLA_SAFE_ASSERT_RETURN(data != nullptr,);
    }

    // -------------------------------------------------------------------
    // Plugin dispatcher calls

    virtual void bufferSizeChanged(const uint32_t bufferSize)
    {
        return;

        // unused
        (void)bufferSize;
    }

    virtual void sampleRateChanged(const double sampleRate)
    {
        CARLA_SAFE_ASSERT_RETURN(sampleRate > 0.0,);
    }

    virtual void offlineChanged(const bool isOffline)
    {
        return;

        // unused
        (void)isOffline;
    }

    virtual void uiNameChanged(const char* const uiName)
    {
        CARLA_SAFE_ASSERT_RETURN(uiName != nullptr,);
    }

    // -------------------------------------------------------------------

private:
    const NativeHostDescriptor* const pHost;

    // -------------------------------------------------------------------

#ifndef DOXYGEN
public:
    #define handlePtr ((NativePluginClass*)handle)

    static uint32_t _get_parameter_count(NativePluginHandle handle)
    {
        return handlePtr->getParameterCount();
    }

    static const NativeParameter* _get_parameter_info(NativePluginHandle handle, uint32_t index)
    {
        return handlePtr->getParameterInfo(index);
    }

    static float _get_parameter_value(NativePluginHandle handle, uint32_t index)
    {
        return handlePtr->getParameterValue(index);
    }

    static const char* _get_parameter_text(NativePluginHandle handle, uint32_t index, float value)
    {
        return handlePtr->getParameterText(index, value);
    }

    static uint32_t _get_midi_program_count(NativePluginHandle handle)
    {
        return handlePtr->getMidiProgramCount();
    }

    static const NativeMidiProgram* _get_midi_program_info(NativePluginHandle handle, uint32_t index)
    {
        return handlePtr->getMidiProgramInfo(index);
    }

    static void _set_parameter_value(NativePluginHandle handle, uint32_t index, float value)
    {
        handlePtr->setParameterValue(index, value);
    }

    static void _set_midi_program(NativePluginHandle handle, uint8_t channel, uint32_t bank, uint32_t program)
    {
        handlePtr->setMidiProgram(channel, bank, program);
    }

    static void _set_custom_data(NativePluginHandle handle, const char* key, const char* value)
    {
        handlePtr->setCustomData(key, value);
    }

    static void _ui_show(NativePluginHandle handle, bool show)
    {
        handlePtr->uiShow(show);
    }

    static void _ui_idle(NativePluginHandle handle)
    {
        handlePtr->uiIdle();
    }

    static void _ui_set_parameter_value(NativePluginHandle handle, uint32_t index, float value)
    {
        handlePtr->uiSetParameterValue(index, value);
    }

    static void _ui_set_midi_program(NativePluginHandle handle, uint8_t channel, uint32_t bank, uint32_t program)
    {
        handlePtr->uiSetMidiProgram(channel, bank, program);
    }

    static void _ui_set_custom_data(NativePluginHandle handle, const char* key, const char* value)
    {
        handlePtr->uiSetCustomData(key, value);
    }

    static void _activate(NativePluginHandle handle)
    {
        handlePtr->activate();
    }

    static void _deactivate(NativePluginHandle handle)
    {
        handlePtr->deactivate();
    }

    static void _process(NativePluginHandle handle, float** inBuffer, float** outBuffer, const uint32_t frames, const NativeMidiEvent* midiEvents, uint32_t midiEventCount)
    {
        handlePtr->process(inBuffer, outBuffer, frames, midiEvents, midiEventCount);
    }

    static char* _get_state(NativePluginHandle handle)
    {
        return handlePtr->getState();
    }

    static void _set_state(NativePluginHandle handle, const char* data)
    {
        handlePtr->setState(data);
    }

    static intptr_t _dispatcher(NativePluginHandle handle, NativePluginDispatcherOpcode opcode, int32_t index, intptr_t value, void* ptr, float opt)
    {
        switch(opcode)
        {
        case PLUGIN_OPCODE_NULL:
            return 0;
        case PLUGIN_OPCODE_BUFFER_SIZE_CHANGED:
            CARLA_SAFE_ASSERT_RETURN(value > 0, 0);
            handlePtr->bufferSizeChanged(static_cast<uint32_t>(value));
            return 0;
        case PLUGIN_OPCODE_SAMPLE_RATE_CHANGED:
            handlePtr->sampleRateChanged(static_cast<double>(opt));
            return 0;
        case PLUGIN_OPCODE_OFFLINE_CHANGED:
            handlePtr->offlineChanged(value != 0);
            return 0;
        case PLUGIN_OPCODE_UI_NAME_CHANGED:
            handlePtr->uiNameChanged(static_cast<const char*>(ptr));
            return 0;
        }

        return 0;

        // unused
        (void)index;
    }

    #undef handlePtr

    CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NativePluginClass)
#endif
};

/**@}*/

// -----------------------------------------------------------------------

#define PluginClassEND(ClassName)                   \
public:                                             \
    static NativePluginHandle _instantiate(const NativeHostDescriptor* host) \
    {                                               \
        return new ClassName(host);                 \
    }                                               \
    static void _cleanup(NativePluginHandle handle) \
    {                                               \
        delete (ClassName*)handle;                  \
    }

#define PluginDescriptorFILL(ClassName) \
    ClassName::_instantiate,            \
    ClassName::_cleanup,                \
    PluginDescriptorFILL2(ClassName)

#define PluginDescriptorFILL2(ClassName) \
    ClassName::_get_parameter_count,    \
    ClassName::_get_parameter_info,     \
    ClassName::_get_parameter_value,    \
    ClassName::_get_parameter_text,     \
    ClassName::_get_midi_program_count, \
    ClassName::_get_midi_program_info,  \
    ClassName::_set_parameter_value,    \
    ClassName::_set_midi_program,       \
    ClassName::_set_custom_data,        \
    ClassName::_ui_show,                \
    ClassName::_ui_idle,                \
    ClassName::_ui_set_parameter_value, \
    ClassName::_ui_set_midi_program,    \
    ClassName::_ui_set_custom_data,     \
    ClassName::_activate,               \
    ClassName::_deactivate,             \
    ClassName::_process,                \
    ClassName::_get_state,              \
    ClassName::_set_state,              \
    ClassName::_dispatcher

// -----------------------------------------------------------------------

#endif // CARLA_NATIVE_HPP_INCLUDED