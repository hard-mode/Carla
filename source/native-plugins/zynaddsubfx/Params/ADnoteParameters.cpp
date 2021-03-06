/*
  ZynAddSubFX - a software synthesizer

  ADnoteParameters.cpp - Parameters for ADnote (ADsynth)
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ADnoteParameters.h"
#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "../Misc/XMLwrapper.h"
#include "../DSP/FFTwrapper.h"
#include "../Synth/OscilGen.h"
#include "../Synth/Resonance.h"
#include "FilterParams.h"

#include <rtosc/ports.h>
#include <rtosc/port-sugar.h>
using rtosc::Ports;
using rtosc::RtData;

#define EXPAND(x) x
#define rObject ADnoteVoiceParam

static const Ports voicePorts = {
    rRecurp(OscilSmp, "Primary Oscillator"),
    rRecurp(FMSmp,    "Modulating Oscillator"),
    rRecurp(FreqLfo, "Frequency LFO"),
    rRecurp(AmpLfo, "Amplitude LFO"),
    rRecurp(FilterLfo, "Filter LFO"),
    rRecurp(FreqEnvelope,   "Frequency Envelope"),
    rRecurp(AmpEnvelope,    "Amplitude Envelope"),
    rRecurp(FilterEnvelope, "Filter Envelope"),
    rRecurp(FMFreqEnvelope, "Modulator Frequency Envelope"),
    rRecurp(FMAmpEnvelope,  "Modulator Amplitude Envelope"),
    rRecurp(VoiceFilter,    "Optional Voice Filter"),

    rToggle(Enabled, "Voice Enable"),
    rParamZyn(Unison_size, "Number of subvoices"),
    rParamZyn(Unison_phase_randomness, "Phase Randomness"),
    rParamZyn(Unison_frequency_spread, "Subvoice detune"),
    rParamZyn(Unison_stereo_spread, "Subvoice L/R Separation"),
    rParamZyn(Unison_vibratto, "Subvoice vibratto"),
    rParamZyn(Unison_vibratto_speed, "Subvoice vibratto speed"),
    rOption(Unison_invert_phase, rOptions(none, random, 50%, 33%, 25%), "Subvoice Phases"),
    rOption(Type, rOptions(Sound,Noise), "Type of Sound"),
    rParamZyn(PDelay, "Voice Startup Delay"),
    rToggle(Presonance, "Resonance Enable"),
    rParamZyn(Pextoscil, "External Oscilator Selection"),
    rParamZyn(PextFMoscil, "External FM Oscilator Selection"),
    rParamZyn(Poscilphase, "Oscillator Phase"),
    rParamZyn(PFMoscilphase, "FM Oscillator Phase"),
    rToggle(Pfilterbypass, "Filter Bypass"),

    //Freq Stuff
    rToggle(Pfixedfreq,           "If frequency is fixed"),
    rParamZyn(PfixedfreqET,          "Equal Tempermant Parameter"),
    rParamI(PDetune,              "Fine Detune"),
    rParamI(PCoarseDetune,        "Coarse Detune"),
    rParamZyn(PDetuneType,           "Magnitude of Detune"),
    rToggle(PFreqEnvelopeEnabled, "Frequency Envelope Enable"),
    rToggle(PFreqLfoEnabled,      "Frequency LFO Enable"),

    //Amplitude Stuff
    rParamZyn(PPanning,                  "Panning"),
    rParamZyn(PVolume,                   "Volume"),
    rToggle(PVolumeminus,             "Signal Inverter"), //do we really need this??
    rParamZyn(PAmpVelocityScaleFunction, "Velocity Sensing"),
    rToggle(PAmpEnvelopeEnabled,      "Amplitude Envelope Enable"),
    rToggle(PAmpLfoEnabled,           "Amplitude LFO Enable"),

    //Filter Stuff
    rToggle(PFilterEnabled,         "Filter Enable"),
    rToggle(PFilterEnvelopeEnabled, "Filter Envelope Enable"),
    rToggle(PFilterLfoEnabled,      "Filter LFO Enable"),


    //Modulator Stuff
    rParamZyn(PFMEnabled,              "Modulator Enable/Type"),
    rParamI(PFMVoice,                "Modulator Oscillator Selection"),
    rParamZyn(PFMVolume,                "Modulator Magnitude"),
    rParamZyn(PFMVolumeDamp,            "Modulator HF dampening"),
    rParamZyn(PFMVelocityScaleFunction, "Modulator Velocity Function"),
    rParamI(PFMDetune,               "Modulator Fine Detune"),
    rParamI(PFMCoarseDetune,         "Modulator Coarse Detune"),
    rParamZyn(PFMDetuneType,            "Modulator Detune Magnitude"),
    rToggle(PFMFreqEnvelopeEnabled,  "Modulator Frequency Envelope"),
    rToggle(PFMAmpEnvelopeEnabled,   "Modulator Amplitude Envelope"),

    
    //weird stuff for PCoarseDetune
    {"detunevalue:", NULL, NULL, [](const char *, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            //TODO check if this is accurate or if PCoarseDetune is utilized
            //TODO do the same for the other engines
            d.reply(d.loc, "f", getdetune(obj->PDetuneType, 0, obj->PDetune));
        }},
    {"octave::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PCoarseDetune/1024;
                if (k>=8) k-=16;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=16;
                obj->PCoarseDetune = k*1024 + obj->PCoarseDetune%1024;
            }
        }},
    {"coarsedetune::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PCoarseDetune%1024;
                if (k>=512) k-=1024;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=1024;
                obj->PCoarseDetune = k + (obj->PCoarseDetune/1024)*1024;
            }
        }},
    
    //weird stuff for PCoarseDetune
    {"FMdetunevalue:", NULL, NULL, [](const char *, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            //TODO check if this is accurate or if PCoarseDetune is utilized
            //TODO do the same for the other engines
            d.reply(d.loc, "f", getdetune(obj->PFMDetuneType, 0, obj->PFMDetune));
        }},
    {"FMoctave::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PFMCoarseDetune/1024;
                if (k>=8) k-=16;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=16;
                obj->PFMCoarseDetune = k*1024 + obj->PFMCoarseDetune%1024;
            }
        }},
    {"FMcoarsedetune::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PFMCoarseDetune%1024;
                if (k>=512) k-=1024;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=1024;
                obj->PFMCoarseDetune = k + (obj->PFMCoarseDetune/1024)*1024;
            }
        }},

    //Reader
    {"unisonFrequencySpreadCents:", NULL, NULL, [](const char *, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            d.reply(d.loc, "f", obj->getUnisonFrequencySpreadCents());
        }},
};

#undef  rObject
#define rObject ADnoteGlobalParam

static const Ports globalPorts = {
    rParamZyn(PPanning, "Panning (0 random, 1 left, 127 right)"),
    rRecurp(Reson, "Resonance"),
    rRecurp(FreqLfo, "Frequency LFO"),
    rRecurp(AmpLfo, "Amplitude LFO"),
    rRecurp(FilterLfo, "Filter LFO"),
    rRecurp(FreqEnvelope, "Frequency Envelope"),
    rRecurp(AmpEnvelope, "Frequency Envelope"),
    rRecurp(FilterEnvelope, "Frequency Envelope"),
    rRecurp(GlobalFilter, "Filter"),
    rToggle(PStereo, "Mono/Stereo Enable"),

    //Frequency
    rParamI(PDetune,       "Fine Detune"),
    rParamI(PCoarseDetune, "Coarse Detune"),
    rParamZyn(PDetuneType,   "Detune Scaling Type"),
    rParamZyn(PBandwidth,    "Relative Fine Detune Gain"),

    //Amplitude
    rParamZyn(PPanning, "Panning of ADsynth"),
    rParamZyn(PVolume, "volume control"),
    rParamZyn(PAmpVelocityScaleFunction, "Volume Velocity Control"),

    rParamZyn(PPunchStrength, "Punch Strength"),
    rParamZyn(PPunchTime, "UNKNOWN"),
    rParamZyn(PPunchStretch, "How Punch changes with note frequency"),
    rParamZyn(PPunchVelocitySensing, "Punch Velocity control"),

    //Filter
    rParamZyn(PFilterVelocityScale, "Filter Velocity Magnitude"),
    rParamZyn(PFilterVelocityScaleFunction, "Filter Velocity Function Shape"),


    //Resonance
    rParamZyn(Hrandgrouping, "How randomness is applied to multiple voices using the same oscil"),

    //weird stuff for PCoarseDetune
    {"detunevalue:", NULL, NULL, [](const char *, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            d.reply(d.loc, "f", getdetune(obj->PDetuneType, 0, obj->PDetune));
        }},
    {"octave::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PCoarseDetune/1024;
                if (k>=8) k-=16;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=16;
                obj->PCoarseDetune = k*1024 + obj->PCoarseDetune%1024;
            }
        }},
    {"coarsedetune::c:i", NULL, NULL, [](const char *msg, RtData &d)
        {
            rObject *obj = (rObject *)d.obj;
            if(!rtosc_narguments(msg)) {
                int k=obj->PCoarseDetune%1024;
                if (k>=512) k-=1024;
                d.reply(d.loc, "i", k);
            } else {
                int k=(int) rtosc_argument(msg, 0).i;
                if (k<0) k+=1024;
                obj->PCoarseDetune = k + (obj->PCoarseDetune/1024)*1024;
            }
        }},

};

#undef  rObject
#define rObject ADnoteParameters
static const Ports adPorts = {//XXX 16 should not be hard coded
    rSelf(ADnoteParameters),
    rPaste,
    rArrayPaste,
    rRecurs(VoicePar, NUM_VOICES),
    rRecur(GlobalPar, "Adnote Parameters"),
};

const Ports &ADnoteParameters::ports  = adPorts;
const Ports &ADnoteVoiceParam::ports  = voicePorts;
const Ports &ADnoteGlobalParam::ports = globalPorts;

ADnoteParameters::ADnoteParameters(const SYNTH_T &synth, FFTwrapper *fft_)
    :PresetsArray()
{
    setpresettype("Padsynth");
    fft = fft_;


    for(int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
        EnableVoice(synth, nvoice);

    defaults();
}

ADnoteGlobalParam::ADnoteGlobalParam()
{
    FreqEnvelope = new EnvelopeParams(0, 0);
    FreqEnvelope->ASRinit(64, 50, 64, 60);
    FreqLfo = new LFOParams(70, 0, 64, 0, 0, 0, 0, 0);

    AmpEnvelope = new EnvelopeParams(64, 1);
    AmpEnvelope->ADSRinit_dB(0, 40, 127, 25);
    AmpLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 1);

    GlobalFilter   = new FilterParams(2, 94, 40);
    FilterEnvelope = new EnvelopeParams(0, 1);
    FilterEnvelope->ADSRinit_filter(64, 40, 64, 70, 60, 64);
    FilterLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 2);
    Reson     = new Resonance();
}

void ADnoteParameters::defaults()
{
    //Default Parameters
    GlobalPar.defaults();

    for(int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
        defaults(nvoice);

    VoicePar[0].Enabled = 1;
}

void ADnoteGlobalParam::defaults()
{
    /* Frequency Global Parameters */
    PStereo = 1; //stereo
    PDetune = 8192; //zero
    PCoarseDetune = 0;
    PDetuneType   = 1;
    FreqEnvelope->defaults();
    FreqLfo->defaults();
    PBandwidth = 64;

    /* Amplitude Global Parameters */
    PVolume  = 90;
    PPanning = 64; //center
    PAmpVelocityScaleFunction = 64;
    AmpEnvelope->defaults();
    AmpLfo->defaults();
    PPunchStrength = 0;
    PPunchTime     = 60;
    PPunchStretch  = 64;
    PPunchVelocitySensing = 72;
    Hrandgrouping = 0;

    /* Filter Global Parameters*/
    PFilterVelocityScale = 64;
    PFilterVelocityScaleFunction = 64;
    GlobalFilter->defaults();
    FilterEnvelope->defaults();
    FilterLfo->defaults();
    Reson->defaults();
}

/*
 * Defaults a voice
 */
void ADnoteParameters::defaults(int n)
{
    VoicePar[n].defaults();
}

void ADnoteVoiceParam::defaults()
{
    Enabled = 0;

    Unison_size = 1;
    Unison_frequency_spread = 60;
    Unison_stereo_spread    = 64;
    Unison_vibratto = 64;
    Unison_vibratto_speed = 64;
    Unison_invert_phase   = 0;
    Unison_phase_randomness = 127;

    Type = 0;
    Pfixedfreq    = 0;
    PfixedfreqET  = 0;
    Presonance    = 1;
    Pfilterbypass = 0;
    Pextoscil     = -1;
    PextFMoscil   = -1;
    Poscilphase   = 64;
    PFMoscilphase = 64;
    PDelay                    = 0;
    PVolume                   = 100;
    PVolumeminus              = 0;
    PPanning                  = 64; //center
    PDetune                   = 8192; //8192=0
    PCoarseDetune             = 0;
    PDetuneType               = 0;
    PFreqLfoEnabled           = 0;
    PFreqEnvelopeEnabled      = 0;
    PAmpEnvelopeEnabled       = 0;
    PAmpLfoEnabled            = 0;
    PAmpVelocityScaleFunction = 127;
    PFilterEnabled            = 0;
    PFilterEnvelopeEnabled    = 0;
    PFilterLfoEnabled         = 0;
    PFMEnabled                = 0;

    //I use the internal oscillator (-1)
    PFMVoice = -1;

    PFMVolume       = 90;
    PFMVolumeDamp   = 64;
    PFMDetune       = 8192;
    PFMCoarseDetune = 0;
    PFMDetuneType   = 0;
    PFMFreqEnvelopeEnabled   = 0;
    PFMAmpEnvelopeEnabled    = 0;
    PFMVelocityScaleFunction = 64;

    OscilSmp->defaults();
    FMSmp->defaults();

    AmpEnvelope->defaults();
    AmpLfo->defaults();

    FreqEnvelope->defaults();
    FreqLfo->defaults();

    VoiceFilter->defaults();
    FilterEnvelope->defaults();
    FilterLfo->defaults();

    FMFreqEnvelope->defaults();
    FMAmpEnvelope->defaults();
}



/*
 * Init the voice parameters
 */
void ADnoteParameters::EnableVoice(const SYNTH_T &synth, int nvoice)
{
    VoicePar[nvoice].enable(synth, fft, GlobalPar.Reson);
}

void ADnoteVoiceParam::enable(const SYNTH_T &synth, FFTwrapper *fft, Resonance *Reson)
{
    OscilSmp = new OscilGen(synth, fft, Reson);
    FMSmp    = new OscilGen(synth, fft, NULL);

    AmpEnvelope = new EnvelopeParams(64, 1);
    AmpEnvelope->ADSRinit_dB(0, 100, 127, 100);
    AmpLfo = new LFOParams(90, 32, 64, 0, 0, 30, 0, 1);

    FreqEnvelope = new EnvelopeParams(0, 0);
    FreqEnvelope->ASRinit(30, 40, 64, 60);
    FreqLfo = new LFOParams(50, 40, 0, 0, 0, 0, 0, 0);

    VoiceFilter    = new FilterParams(2, 50, 60);
    FilterEnvelope = new EnvelopeParams(0, 0);
    FilterEnvelope->ADSRinit_filter(90, 70, 40, 70, 10, 40);
    FilterLfo = new LFOParams(50, 20, 64, 0, 0, 0, 0, 2);

    FMFreqEnvelope = new EnvelopeParams(0, 0);
    FMFreqEnvelope->ASRinit(20, 90, 40, 80);
    FMAmpEnvelope = new EnvelopeParams(64, 1);
    FMAmpEnvelope->ADSRinit(80, 90, 127, 100);
}

/*
 * Get the Multiplier of the fine detunes of the voices
 */
float ADnoteParameters::getBandwidthDetuneMultiplier() const
{
    float bw = (GlobalPar.PBandwidth - 64.0f) / 64.0f;
    bw = powf(2.0f, bw * powf(fabs(bw), 0.2f) * 5.0f);

    return bw;
}

/*
 * Get the unison spread in cents for a voice
 */

float ADnoteParameters::getUnisonFrequencySpreadCents(int nvoice) const
{
    return VoicePar[nvoice].getUnisonFrequencySpreadCents();
}

float ADnoteVoiceParam::getUnisonFrequencySpreadCents(void) const {
    return powf(Unison_frequency_spread / 127.0 * 2.0f, 2.0f) * 50.0f; //cents
}

/*
 * Kill the voice
 */
void ADnoteParameters::KillVoice(int nvoice)
{
    VoicePar[nvoice].kill();
}

void ADnoteVoiceParam::kill()
{
    delete OscilSmp;
    delete FMSmp;

    delete AmpEnvelope;
    delete AmpLfo;

    delete FreqEnvelope;
    delete FreqLfo;

    delete VoiceFilter;
    delete FilterEnvelope;
    delete FilterLfo;

    delete FMFreqEnvelope;
    delete FMAmpEnvelope;
}


ADnoteGlobalParam::~ADnoteGlobalParam()
{
    delete FreqEnvelope;
    delete FreqLfo;
    delete AmpEnvelope;
    delete AmpLfo;
    delete GlobalFilter;
    delete FilterEnvelope;
    delete FilterLfo;
    delete Reson;
}

ADnoteParameters::~ADnoteParameters()
{
    for(int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
        KillVoice(nvoice);
}

int ADnoteParameters::get_unison_size_index(int nvoice) const
{
    int index = 0;
    if(nvoice >= NUM_VOICES)
        return 0;
    int unison = VoicePar[nvoice].Unison_size;

    while(1) {
        if(ADnote_unison_sizes[index] >= unison)
            return index;

        if(ADnote_unison_sizes[index] == 0)
            return index - 1;

        index++;
    }
    return 0;
}

void ADnoteParameters::set_unison_size_index(int nvoice, int index) {
    int unison = 1;
    for(int i = 0; i <= index; ++i) {
        unison = ADnote_unison_sizes[i];
        if(unison == 0) {
            unison = ADnote_unison_sizes[i - 1];
            break;
        }
    }

    VoicePar[nvoice].Unison_size = unison;
}



void ADnoteParameters::add2XMLsection(XMLwrapper *xml, int n)
{
    int nvoice = n;
    if(nvoice >= NUM_VOICES)
        return;

    int oscilused = 0, fmoscilused = 0; //if the oscil or fmoscil are used by another voice

    for(int i = 0; i < NUM_VOICES; ++i) {
        if(VoicePar[i].Pextoscil == nvoice)
            oscilused = 1;
        if(VoicePar[i].PextFMoscil == nvoice)
            fmoscilused = 1;
    }

    xml->addparbool("enabled", VoicePar[nvoice].Enabled);
    if(((VoicePar[nvoice].Enabled == 0) && (oscilused == 0)
        && (fmoscilused == 0)) && (xml->minimal))
        return;

    VoicePar[nvoice].add2XML(xml, fmoscilused);
}

void ADnoteVoiceParam::add2XML(XMLwrapper *xml, bool fmoscilused)
{
    xml->addpar("type", Type);

    xml->addpar("unison_size", Unison_size);
    xml->addpar("unison_frequency_spread",
                Unison_frequency_spread);
    xml->addpar("unison_stereo_spread", Unison_stereo_spread);
    xml->addpar("unison_vibratto", Unison_vibratto);
    xml->addpar("unison_vibratto_speed", Unison_vibratto_speed);
    xml->addpar("unison_invert_phase", Unison_invert_phase);
    xml->addpar("unison_phase_randomness", Unison_phase_randomness);

    xml->addpar("delay", PDelay);
    xml->addparbool("resonance", Presonance);

    xml->addpar("ext_oscil", Pextoscil);
    xml->addpar("ext_fm_oscil", PextFMoscil);

    xml->addpar("oscil_phase", Poscilphase);
    xml->addpar("oscil_fm_phase", PFMoscilphase);

    xml->addparbool("filter_enabled", PFilterEnabled);
    xml->addparbool("filter_bypass", Pfilterbypass);

    xml->addpar("fm_enabled", PFMEnabled);

    xml->beginbranch("OSCIL");
    OscilSmp->add2XML(xml);
    xml->endbranch();


    xml->beginbranch("AMPLITUDE_PARAMETERS");
    xml->addpar("panning", PPanning);
    xml->addpar("volume", PVolume);
    xml->addparbool("volume_minus", PVolumeminus);
    xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);

    xml->addparbool("amp_envelope_enabled",
                    PAmpEnvelopeEnabled);
    if((PAmpEnvelopeEnabled != 0) || (!xml->minimal)) {
        xml->beginbranch("AMPLITUDE_ENVELOPE");
        AmpEnvelope->add2XML(xml);
        xml->endbranch();
    }
    xml->addparbool("amp_lfo_enabled", PAmpLfoEnabled);
    if((PAmpLfoEnabled != 0) || (!xml->minimal)) {
        xml->beginbranch("AMPLITUDE_LFO");
        AmpLfo->add2XML(xml);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    xml->addparbool("fixed_freq", Pfixedfreq);
    xml->addpar("fixed_freq_et", PfixedfreqET);
    xml->addpar("detune", PDetune);
    xml->addpar("coarse_detune", PCoarseDetune);
    xml->addpar("detune_type", PDetuneType);

    xml->addparbool("freq_envelope_enabled",
                    PFreqEnvelopeEnabled);
    if((PFreqEnvelopeEnabled != 0) || (!xml->minimal)) {
        xml->beginbranch("FREQUENCY_ENVELOPE");
        FreqEnvelope->add2XML(xml);
        xml->endbranch();
    }
    xml->addparbool("freq_lfo_enabled", PFreqLfoEnabled);
    if((PFreqLfoEnabled != 0) || (!xml->minimal)) {
        xml->beginbranch("FREQUENCY_LFO");
        FreqLfo->add2XML(xml);
        xml->endbranch();
    }
    xml->endbranch();


    if((PFilterEnabled != 0) || (!xml->minimal)) {
        xml->beginbranch("FILTER_PARAMETERS");
        xml->beginbranch("FILTER");
        VoiceFilter->add2XML(xml);
        xml->endbranch();

        xml->addparbool("filter_envelope_enabled",
                        PFilterEnvelopeEnabled);
        if((PFilterEnvelopeEnabled != 0) || (!xml->minimal)) {
            xml->beginbranch("FILTER_ENVELOPE");
            FilterEnvelope->add2XML(xml);
            xml->endbranch();
        }

        xml->addparbool("filter_lfo_enabled",
                        PFilterLfoEnabled);
        if((PFilterLfoEnabled != 0) || (!xml->minimal)) {
            xml->beginbranch("FILTER_LFO");
            FilterLfo->add2XML(xml);
            xml->endbranch();
        }
        xml->endbranch();
    }

    if((PFMEnabled != 0) || (fmoscilused != 0)
       || (!xml->minimal)) {
        xml->beginbranch("FM_PARAMETERS");
        xml->addpar("input_voice", PFMVoice);

        xml->addpar("volume", PFMVolume);
        xml->addpar("volume_damp", PFMVolumeDamp);
        xml->addpar("velocity_sensing",
                    PFMVelocityScaleFunction);

        xml->addparbool("amp_envelope_enabled",
                        PFMAmpEnvelopeEnabled);
        if((PFMAmpEnvelopeEnabled != 0) || (!xml->minimal)) {
            xml->beginbranch("AMPLITUDE_ENVELOPE");
            FMAmpEnvelope->add2XML(xml);
            xml->endbranch();
        }
        xml->beginbranch("MODULATOR");
        xml->addpar("detune", PFMDetune);
        xml->addpar("coarse_detune", PFMCoarseDetune);
        xml->addpar("detune_type", PFMDetuneType);

        xml->addparbool("freq_envelope_enabled",
                        PFMFreqEnvelopeEnabled);
        if((PFMFreqEnvelopeEnabled != 0) || (!xml->minimal)) {
            xml->beginbranch("FREQUENCY_ENVELOPE");
            FMFreqEnvelope->add2XML(xml);
            xml->endbranch();
        }

        xml->beginbranch("OSCIL");
        FMSmp->add2XML(xml);
        xml->endbranch();

        xml->endbranch();
        xml->endbranch();
    }
}

void ADnoteGlobalParam::add2XML(XMLwrapper *xml)
{
    xml->addparbool("stereo", PStereo);

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    xml->addpar("volume", PVolume);
    xml->addpar("panning", PPanning);
    xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);
    xml->addpar("punch_strength", PPunchStrength);
    xml->addpar("punch_time", PPunchTime);
    xml->addpar("punch_stretch", PPunchStretch);
    xml->addpar("punch_velocity_sensing", PPunchVelocitySensing);
    xml->addpar("harmonic_randomness_grouping", Hrandgrouping);

    xml->beginbranch("AMPLITUDE_ENVELOPE");
    AmpEnvelope->add2XML(xml);
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_LFO");
    AmpLfo->add2XML(xml);
    xml->endbranch();
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    xml->addpar("detune", PDetune);

    xml->addpar("coarse_detune", PCoarseDetune);
    xml->addpar("detune_type", PDetuneType);

    xml->addpar("bandwidth", PBandwidth);

    xml->beginbranch("FREQUENCY_ENVELOPE");
    FreqEnvelope->add2XML(xml);
    xml->endbranch();

    xml->beginbranch("FREQUENCY_LFO");
    FreqLfo->add2XML(xml);
    xml->endbranch();
    xml->endbranch();


    xml->beginbranch("FILTER_PARAMETERS");
    xml->addpar("velocity_sensing_amplitude", PFilterVelocityScale);
    xml->addpar("velocity_sensing", PFilterVelocityScaleFunction);

    xml->beginbranch("FILTER");
    GlobalFilter->add2XML(xml);
    xml->endbranch();

    xml->beginbranch("FILTER_ENVELOPE");
    FilterEnvelope->add2XML(xml);
    xml->endbranch();

    xml->beginbranch("FILTER_LFO");
    FilterLfo->add2XML(xml);
    xml->endbranch();
    xml->endbranch();

    xml->beginbranch("RESONANCE");
    Reson->add2XML(xml);
    xml->endbranch();
}

void ADnoteParameters::add2XML(XMLwrapper *xml)
{
    GlobalPar.add2XML(xml);
    for(int nvoice = 0; nvoice < NUM_VOICES; ++nvoice) {
        xml->beginbranch("VOICE", nvoice);
        add2XMLsection(xml, nvoice);
        xml->endbranch();
    }
}


void ADnoteGlobalParam::getfromXML(XMLwrapper *xml)
{
    PStereo = xml->getparbool("stereo", PStereo);

    if(xml->enterbranch("AMPLITUDE_PARAMETERS")) {
        PVolume  = xml->getpar127("volume", PVolume);
        PPanning = xml->getpar127("panning", PPanning);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing",
                                                   PAmpVelocityScaleFunction);

        PPunchStrength = xml->getpar127("punch_strength", PPunchStrength);
        PPunchTime     = xml->getpar127("punch_time", PPunchTime);
        PPunchStretch  = xml->getpar127("punch_stretch", PPunchStretch);
        PPunchVelocitySensing = xml->getpar127("punch_velocity_sensing",
                                               PPunchVelocitySensing);
        Hrandgrouping = xml->getpar127("harmonic_randomness_grouping",
                                       Hrandgrouping);

        if(xml->enterbranch("AMPLITUDE_ENVELOPE")) {
            AmpEnvelope->getfromXML(xml);
            xml->exitbranch();
        }

        if(xml->enterbranch("AMPLITUDE_LFO")) {
            AmpLfo->getfromXML(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if(xml->enterbranch("FREQUENCY_PARAMETERS")) {
        PDetune = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        PDetuneType   = xml->getpar127("detune_type", PDetuneType);
        PBandwidth    = xml->getpar127("bandwidth", PBandwidth);

        xml->enterbranch("FREQUENCY_ENVELOPE");
        FreqEnvelope->getfromXML(xml);
        xml->exitbranch();

        xml->enterbranch("FREQUENCY_LFO");
        FreqLfo->getfromXML(xml);
        xml->exitbranch();

        xml->exitbranch();
    }


    if(xml->enterbranch("FILTER_PARAMETERS")) {
        PFilterVelocityScale = xml->getpar127("velocity_sensing_amplitude",
                                              PFilterVelocityScale);
        PFilterVelocityScaleFunction = xml->getpar127(
            "velocity_sensing",
            PFilterVelocityScaleFunction);

        xml->enterbranch("FILTER");
        GlobalFilter->getfromXML(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_ENVELOPE");
        FilterEnvelope->getfromXML(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_LFO");
        FilterLfo->getfromXML(xml);
        xml->exitbranch();
        xml->exitbranch();
    }

    if(xml->enterbranch("RESONANCE")) {
        Reson->getfromXML(xml);
        xml->exitbranch();
    }
}

void ADnoteParameters::getfromXML(XMLwrapper *xml)
{
    GlobalPar.getfromXML(xml);

    for(int nvoice = 0; nvoice < NUM_VOICES; ++nvoice) {
        VoicePar[nvoice].Enabled = 0;
        if(xml->enterbranch("VOICE", nvoice) == 0)
            continue;
        getfromXMLsection(xml, nvoice);
        xml->exitbranch();
    }
}

void ADnoteParameters::getfromXMLsection(XMLwrapper *xml, int n)
{
    int nvoice = n;
    if(nvoice >= NUM_VOICES)
        return;

    VoicePar[nvoice].getfromXML(xml, nvoice);
}

void ADnoteParameters::paste(ADnoteParameters &a)
{
    this->GlobalPar.paste(a.GlobalPar);
    for(int i=0; i<NUM_VOICES; ++i)
        this->VoicePar[i].paste(a.VoicePar[i]);
}

void ADnoteParameters::pasteArray(ADnoteParameters &a, int nvoice)
{
    if(nvoice >= NUM_VOICES)
        return;

    VoicePar[nvoice].paste(a.VoicePar[nvoice]);
}

#define copy(x) this->x = a.x
#define RCopy(x) this->x->paste(*a.x)
void ADnoteVoiceParam::paste(ADnoteVoiceParam &a)
{
    //Come on C++ get some darn reflection, this is horrible

    copy(Enabled);
    copy(Unison_size);
    copy(Unison_frequency_spread);
    copy(Unison_stereo_spread);
    copy(Unison_vibratto);
    copy(Unison_vibratto_speed);
    copy(Unison_invert_phase);
    copy(Unison_phase_randomness);
    copy(Type);
    copy(PDelay);
    copy(Presonance);
    copy(Pextoscil);
    copy(PextFMoscil);
    copy(Poscilphase);
    copy(PFMoscilphase);
    copy(PFilterEnabled);
    copy(Pfilterbypass);
    copy(PFMEnabled);

    RCopy(OscilSmp);


    copy(PPanning);
    copy(PVolume);
    copy(PVolumeminus);
    copy(PAmpVelocityScaleFunction);
    copy(PAmpEnvelopeEnabled);

    RCopy(AmpEnvelope);

    copy(PAmpLfoEnabled);

    RCopy(AmpLfo);

    copy(Pfixedfreq);
    copy(PfixedfreqET);
    copy(PDetune);
    copy(PCoarseDetune);
    copy(PDetuneType);
    copy(PFreqEnvelopeEnabled);

    RCopy(FreqEnvelope);

    copy(PFreqLfoEnabled);

    RCopy(FreqLfo);

    RCopy(VoiceFilter);

    copy(PFilterEnvelopeEnabled);

    RCopy(FilterEnvelope);

    copy(PFilterLfoEnabled);

    RCopy(FilterLfo);

    copy(PFMVoice);
    copy(PFMVolume);
    copy(PFMVolumeDamp);
    copy(PFMVelocityScaleFunction);

    copy(PFMAmpEnvelopeEnabled);

    RCopy(FMAmpEnvelope);

    copy(PFMDetune);
    copy(PFMCoarseDetune);
    copy(PFMDetuneType);
    copy(PFMFreqEnvelopeEnabled);


    RCopy(FMFreqEnvelope);

    RCopy(FMSmp);
}

void ADnoteGlobalParam::paste(ADnoteGlobalParam &a)
{
    copy(PStereo);

    copy(PVolume);
    copy(PPanning);
    copy(PAmpVelocityScaleFunction);

    copy(PPunchStrength);
    copy(PPunchTime);
    copy(PPunchStretch);
    copy(PPunchVelocitySensing);
    copy(Hrandgrouping);

    RCopy(AmpEnvelope);
    RCopy(AmpLfo);

    copy(PDetune);
    copy(PCoarseDetune);
    copy(PDetuneType);
    copy(PBandwidth);

    RCopy(FreqEnvelope);
    RCopy(FreqLfo);

    copy(PFilterVelocityScale);
    copy(PFilterVelocityScaleFunction);

    RCopy(GlobalFilter);
    RCopy(FilterEnvelope);
    RCopy(FilterLfo);
    RCopy(Reson);
}
#undef copy
#undef RCopy

void ADnoteVoiceParam::getfromXML(XMLwrapper *xml, unsigned nvoice)
{
    Enabled     = xml->getparbool("enabled", 0);
    Unison_size = xml->getpar127("unison_size", Unison_size);
    Unison_frequency_spread = xml->getpar127("unison_frequency_spread",
                                             Unison_frequency_spread);
    Unison_stereo_spread = xml->getpar127("unison_stereo_spread",
                                          Unison_stereo_spread);
    Unison_vibratto = xml->getpar127("unison_vibratto", Unison_vibratto);
    Unison_vibratto_speed = xml->getpar127("unison_vibratto_speed",
                                           Unison_vibratto_speed);
    Unison_invert_phase = xml->getpar127("unison_invert_phase",
                                         Unison_invert_phase);
    Unison_phase_randomness = xml->getpar127("unison_phase_randomness",
						Unison_phase_randomness);

    Type       = xml->getpar127("type", Type);
    PDelay     = xml->getpar127("delay", PDelay);
    Presonance = xml->getparbool("resonance", Presonance);

    Pextoscil   = xml->getpar("ext_oscil", -1, -1, nvoice - 1);
    PextFMoscil = xml->getpar("ext_fm_oscil", -1, -1, nvoice - 1);

    Poscilphase    = xml->getpar127("oscil_phase", Poscilphase);
    PFMoscilphase  = xml->getpar127("oscil_fm_phase", PFMoscilphase);
    PFilterEnabled = xml->getparbool("filter_enabled", PFilterEnabled);
    Pfilterbypass  = xml->getparbool("filter_bypass", Pfilterbypass);
    PFMEnabled     = xml->getpar127("fm_enabled", PFMEnabled);

    if(xml->enterbranch("OSCIL")) {
        OscilSmp->getfromXML(xml);
        xml->exitbranch();
    }


    if(xml->enterbranch("AMPLITUDE_PARAMETERS")) {
        PPanning     = xml->getpar127("panning", PPanning);
        PVolume      = xml->getpar127("volume", PVolume);
        PVolumeminus = xml->getparbool("volume_minus", PVolumeminus);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing",
                                                   PAmpVelocityScaleFunction);

        PAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled",
                                              PAmpEnvelopeEnabled);
        if(xml->enterbranch("AMPLITUDE_ENVELOPE")) {
            AmpEnvelope->getfromXML(xml);
            xml->exitbranch();
        }

        PAmpLfoEnabled = xml->getparbool("amp_lfo_enabled", PAmpLfoEnabled);
        if(xml->enterbranch("AMPLITUDE_LFO")) {
            AmpLfo->getfromXML(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if(xml->enterbranch("FREQUENCY_PARAMETERS")) {
        Pfixedfreq    = xml->getparbool("fixed_freq", Pfixedfreq);
        PfixedfreqET  = xml->getpar127("fixed_freq_et", PfixedfreqET);
        PDetune       = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        PDetuneType   = xml->getpar127("detune_type", PDetuneType);
        PFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled",
                                               PFreqEnvelopeEnabled);

        if(xml->enterbranch("FREQUENCY_ENVELOPE")) {
            FreqEnvelope->getfromXML(xml);
            xml->exitbranch();
        }

        PFreqLfoEnabled = xml->getparbool("freq_lfo_enabled", PFreqLfoEnabled);

        if(xml->enterbranch("FREQUENCY_LFO")) {
            FreqLfo->getfromXML(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if(xml->enterbranch("FILTER_PARAMETERS")) {
        if(xml->enterbranch("FILTER")) {
            VoiceFilter->getfromXML(xml);
            xml->exitbranch();
        }

        PFilterEnvelopeEnabled = xml->getparbool("filter_envelope_enabled",
                                                 PFilterEnvelopeEnabled);
        if(xml->enterbranch("FILTER_ENVELOPE")) {
            FilterEnvelope->getfromXML(xml);
            xml->exitbranch();
        }

        PFilterLfoEnabled = xml->getparbool("filter_lfo_enabled",
                                            PFilterLfoEnabled);
        if(xml->enterbranch("FILTER_LFO")) {
            FilterLfo->getfromXML(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if(xml->enterbranch("FM_PARAMETERS")) {
        PFMVoice      = xml->getpar("input_voice", PFMVoice, -1, nvoice - 1);
        PFMVolume     = xml->getpar127("volume", PFMVolume);
        PFMVolumeDamp = xml->getpar127("volume_damp", PFMVolumeDamp);
        PFMVelocityScaleFunction = xml->getpar127("velocity_sensing",
                                                  PFMVelocityScaleFunction);

        PFMAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled",
                                                PFMAmpEnvelopeEnabled);
        if(xml->enterbranch("AMPLITUDE_ENVELOPE")) {
            FMAmpEnvelope->getfromXML(xml);
            xml->exitbranch();
        }

        if(xml->enterbranch("MODULATOR")) {
            PFMDetune = xml->getpar("detune", PFMDetune, 0, 16383);
            PFMCoarseDetune = xml->getpar("coarse_detune",
                                          PFMCoarseDetune,
                                          0,
                                          16383);
            PFMDetuneType = xml->getpar127("detune_type", PFMDetuneType);

            PFMFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled",
                                                     PFMFreqEnvelopeEnabled);
            if(xml->enterbranch("FREQUENCY_ENVELOPE")) {
                FMFreqEnvelope->getfromXML(xml);
                xml->exitbranch();
            }

            if(xml->enterbranch("OSCIL")) {
                FMSmp->getfromXML(xml);
                xml->exitbranch();
            }

            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
