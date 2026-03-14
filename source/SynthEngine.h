#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

struct BlacksideStereoSample
{
    float left = 0.0f;
    float right = 0.0f;

    BlacksideStereoSample& operator+=(const BlacksideStereoSample& other)
    {
        left += other.left;
        right += other.right;
        return *this;
    }
};

struct BlacksideOscillator
{
    double sampleRate = 44100.0;
    float phase = 0.0f;
    float frequency = 110.0f;
    float wavePos = 0.0f;
    float warp = 0.0f;
    float pulseWidth = 0.5f;

    void prepare(double sr)
    {
        sampleRate = sr;
        phase = 0.0f;
    }

    void setFrequency(float hz) { frequency = juce::jmax(0.0f, hz); }
    void setWavePos(float p) { wavePos = juce::jlimit(0.0f, 1.0f, p); }
    void setWarp(float w) { warp = juce::jlimit(0.0f, 1.0f, w); }
    void setPulseWidth(float p) { pulseWidth = juce::jlimit(0.05f, 0.95f, p); }

    float renderAt(float p) const
    {
        const float sine = std::sin(p * juce::MathConstants<float>::twoPi);
        const float saw = 2.0f * p - 1.0f;
        const float tri = 1.0f - 4.0f * std::abs(p - 0.5f);
        const float square = p < pulseWidth ? 1.0f : -1.0f;
        const float bend = std::sin((p + warp * 0.12f) * juce::MathConstants<float>::twoPi * (1.0f + warp * 7.0f));
        const float formant = std::tanh((saw * 0.65f + sine * 0.40f) * (1.1f + warp * 7.0f));
        const float horn = std::tanh((square * 0.5f + saw * 0.7f + sine * 0.35f) * (1.0f + warp * 5.2f));
        const float growl = std::sin((sine + square * 0.3f + saw * 0.2f) * juce::MathConstants<float>::pi * (1.0f + warp * 4.5f));
        const float metallic = std::sin((p * (2.0f + warp * 5.5f) + horn * 0.2f) * juce::MathConstants<float>::twoPi);
        const std::array<float, 9> waves { sine, saw, tri, square, bend, formant, horn, growl, metallic };
        const float sel = juce::jlimit(0.0f, 0.999f, wavePos) * (float) (waves.size() - 1);
        const int i0 = (int) sel;
        const int i1 = juce::jmin((int) waves.size() - 1, i0 + 1);
        const float frac = sel - (float) i0;
        return juce::jmap(frac, waves[(size_t) i0], waves[(size_t) i1]);
    }

    float process(float phaseOffset = 0.0f)
    {
        const float dt = juce::jlimit(0.0f, 0.49f, frequency / (float) sampleRate);
        phase += dt;
        if (phase >= 1.0f)
            phase -= 1.0f;

        float p = phase + phaseOffset;
        while (p >= 1.0f) p -= 1.0f;
        while (p < 0.0f) p += 1.0f;

        return std::tanh(renderAt(p) * (1.0f + warp * 3.0f));
    }
};

struct BlacksideVoice
{
    struct RuntimeParams
    {
        float oscALevel = 0.8f, oscAWave = 0.3f, oscAWarp = 0.1f, oscASpread = 0.2f;
        float oscBLevel = 0.7f, oscBWave = 0.6f, oscBWarp = 0.1f, oscBSpread = 0.2f;
        float oscCLevel = 0.4f, oscCWave = 0.2f, oscCWarp = 0.1f;
        float subLevel = 0.7f, subDrive = 0.1f, noiseLevel = 0.02f;
        float cutoff1 = 1200.0f, res1 = 0.2f, drive1 = 0.2f;
        float cutoff2 = 4500.0f, res2 = 0.2f;
        float distDrive = 0.3f, distMix = 0.5f;
        float lfoRate = 3.0f, lfoAmount = 0.3f, envAmount = 0.7f;
        float macro1 = 0.0f, macro2 = 0.0f, macro3 = 0.0f, macro4 = 0.0f;
        float reeseDetune = 0.3f, reeseWidth = 0.4f, reeseDrift = 0.2f;
        float hornBend = 0.2f, hornFormant = 0.2f, hornBody = 0.25f;
        float screechFm = 0.1f, screechDrive = 0.1f;
        float harmMix = 0.0f, harmInterval = 7.0f, harmSpread = 0.2f;
        float subOctaveMix = 0.3f, outputClip = 0.25f;
        float warhornMix = 0.0f, wompAmount = 0.0f, subHarmony = 0.0f;
        float comboMix = 0.0f, swarmMix = 0.0f, swarmDetune = 0.0f;
        float stereoSpin = 0.0f, fmGrit = 0.0f, bite = 0.0f, punch = 0.0f, talk = 0.0f, air = 0.0f, monoBlend = 1.0f;
    };

    double sampleRate = 44100.0;
    bool active = false;
    bool heldBySustain = false;
    int midiNote = -1;
    float velocity = 0.0f;
    uint64_t noteAge = 0;

    float currentHz = 110.0f;
    float targetHz = 110.0f;
    float glideMs = 45.0f;
    float lfoPhase1 = 0.0f;
    float lfoPhase2 = 0.0f;
    float voicePan = 0.5f;
    float hornEnvState = 0.0f;
    float lastLeft = 0.0f;
    float lastRight = 0.0f;
    float dcLeft = 0.0f;
    float dcRight = 0.0f;

    BlacksideOscillator oscA, oscB, oscC, subOsc, subOctOsc, noiseOsc, harmonyOsc, swarmOsc1, swarmOsc2;
    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;
    juce::ADSR modEnv;
    juce::ADSR::Parameters modParams;

    juce::dsp::StateVariableTPTFilter<float> lowL, lowR, bandL, bandR;

    void prepare(double sr)
    {
        sampleRate = sr;
        for (auto* osc : { &oscA, &oscB, &oscC, &subOsc, &subOctOsc, &noiseOsc, &harmonyOsc, &swarmOsc1, &swarmOsc2 })
            osc->prepare(sr);

        juce::dsp::ProcessSpec spec { sr, 512, 1 };
        for (auto* f : { &lowL, &lowR, &bandL, &bandR })
            f->prepare(spec);

        lowL.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        lowR.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        bandL.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        bandR.setType(juce::dsp::StateVariableTPTFilterType::bandpass);

        ampEnv.setSampleRate(sr);
        modEnv.setSampleRate(sr);
        reset();
    }

    void reset()
    {
        ampEnv.reset();
        modEnv.reset();
        lowL.reset(); lowR.reset(); bandL.reset(); bandR.reset();
        active = false;
        heldBySustain = false;
        midiNote = -1;
        lastLeft = lastRight = dcLeft = dcRight = 0.0f;
        lfoPhase1 = lfoPhase2 = 0.0f;
        hornEnvState = 0.0f;
    }

    void setGlideMs(float ms) { glideMs = juce::jlimit(0.0f, 500.0f, ms); }

    void updateADSR(float a, float d, float s, float r)
    {
        ampParams.attack = a;
        ampParams.decay = d;
        ampParams.sustain = s;
        ampParams.release = r;
        modParams = ampParams;
        ampEnv.setParameters(ampParams);
        modEnv.setParameters(modParams);
    }

    void start(int note, float vel, bool retriggerEnvelope, uint64_t age)
    {
        midiNote = note;
        velocity = vel;
        noteAge = age;
        targetHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);

        if (! active)
            currentHz = targetHz;

        if (retriggerEnvelope || ! active)
        {
            ampEnv.noteOn();
            modEnv.noteOn();
            hornEnvState = 1.0f;
        }

        voicePan = juce::jlimit(0.0f, 1.0f, 0.5f + std::sin((float) note * 0.47f) * 0.18f);
        active = true;
        heldBySustain = false;
    }

    void legatoTo(int note, float vel, uint64_t age)
    {
        midiNote = note;
        velocity = vel;
        noteAge = age;
        targetHz = (float) juce::MidiMessage::getMidiNoteInHertz(note);
        active = true;
    }

    void stop()
    {
        ampEnv.noteOff();
        modEnv.noteOff();
        heldBySustain = false;
    }

    static float panLeft(float pan) { return std::cos(juce::MathConstants<float>::halfPi * pan); }
    static float panRight(float pan) { return std::sin(juce::MathConstants<float>::halfPi * pan); }

    BlacksideStereoSample nextSample(const RuntimeParams& s)
    {
        if (! active)
            return {};

        const float glideCoeff = (glideMs <= 0.0f)
            ? 1.0f
            : (1.0f - std::exp(-1.0f / (0.001f * glideMs * (float) sampleRate)));
        currentHz += (targetHz - currentHz) * glideCoeff;

        lfoPhase1 += s.lfoRate / (float) sampleRate;
        lfoPhase2 += (s.lfoRate * 0.29f + 0.17f + s.stereoSpin * 0.8f) / (float) sampleRate;
        if (lfoPhase1 >= 1.0f) lfoPhase1 -= 1.0f;
        if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;

        const float sinLfo = std::sin(lfoPhase1 * juce::MathConstants<float>::twoPi);
        const float skewLfo = std::sin(lfoPhase2 * juce::MathConstants<float>::twoPi);
        const float stepLfo = (lfoPhase1 < 0.5f ? -1.0f : 1.0f);
        const float wobbleShape = 0.65f * stepLfo + 0.35f * skewLfo;
        const float wobble = juce::jmap(s.wompAmount, sinLfo, wobbleShape) * s.lfoAmount;
        const float env = modEnv.getNextSample();

        hornEnvState = juce::jmax(0.0f, hornEnvState * 0.99925f - 0.00011f);
        const float hornPitchMod = (hornEnvState * s.hornBend + s.macro4 * 0.12f) * 0.7f;
        const float fmOffset = s.screechFm * 0.3f + s.fmGrit * 0.2f;

        const float driftA = std::sin((lfoPhase2 + 0.13f) * juce::MathConstants<float>::twoPi) * s.reeseDrift * 0.01f;
        const float driftB = std::sin((lfoPhase2 + 0.39f) * juce::MathConstants<float>::twoPi) * s.reeseDrift * 0.01f;
        const float swarmRatio = s.swarmDetune * 0.035f + s.reeseWidth * 0.01f + s.stereoSpin * 0.005f;

        oscA.setFrequency(currentHz * (1.0f - s.reeseDetune * 0.02f + driftA + hornPitchMod));
        oscB.setFrequency(currentHz * (1.0f + s.reeseDetune * 0.02f + driftB + hornPitchMod));
        oscC.setFrequency(currentHz * (0.5f + s.macro2 * 0.25f + s.comboMix * 0.25f + s.subHarmony * 0.06f));
        subOsc.setFrequency(currentHz * 0.5f);
        subOctOsc.setFrequency(currentHz * 0.25f);
        harmonyOsc.setFrequency(currentHz * std::pow(2.0f, s.harmInterval / 12.0f));
        swarmOsc1.setFrequency(currentHz * (1.0f - swarmRatio));
        swarmOsc2.setFrequency(currentHz * (1.0f + swarmRatio));
        noiseOsc.setFrequency(1200.0f + currentHz * (0.15f + s.air * 0.2f) + s.screechFm * 3000.0f + s.warhornMix * 900.0f);

        oscA.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscAWave + s.macro3 * 0.12f + s.talk * 0.08f));
        oscB.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscBWave + s.macro1 * 0.06f + s.wompAmount * 0.06f));
        oscC.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscCWave + s.hornFormant * 0.22f + s.comboMix * 0.12f));
        harmonyOsc.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscBWave + 0.2f + s.subHarmony * 0.14f));
        subOsc.setWavePos(0.04f + s.subDrive * 0.10f);
        subOctOsc.setWavePos(0.02f + s.subHarmony * 0.08f);
        noiseOsc.setWavePos(0.82f);
        swarmOsc1.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscAWave + 0.10f));
        swarmOsc2.setWavePos(juce::jlimit(0.0f, 1.0f, s.oscBWave + 0.10f));

        oscA.setWarp(juce::jlimit(0.0f, 1.0f, s.oscAWarp + s.macro1 * 0.18f + s.screechFm * 0.10f + s.warhornMix * 0.10f + s.bite * 0.10f));
        oscB.setWarp(juce::jlimit(0.0f, 1.0f, s.oscBWarp + s.macro1 * 0.14f + s.wompAmount * 0.15f + s.fmGrit * 0.12f));
        oscC.setWarp(juce::jlimit(0.0f, 1.0f, s.oscCWarp + s.macro2 * 0.16f + s.hornBody * 0.18f + s.comboMix * 0.20f));
        harmonyOsc.setWarp(juce::jlimit(0.0f, 1.0f, 0.10f + s.harmMix * 0.25f + s.subHarmony * 0.14f));
        subOsc.setWarp(juce::jlimit(0.0f, 1.0f, s.subDrive * 0.30f));
        subOctOsc.setWarp(juce::jlimit(0.0f, 1.0f, s.subDrive * 0.15f + s.subHarmony * 0.25f));
        noiseOsc.setWarp(juce::jlimit(0.0f, 1.0f, s.screechDrive * 0.7f + s.warhornMix * 0.2f + s.air * 0.2f));
        swarmOsc1.setWarp(juce::jlimit(0.0f, 1.0f, s.oscAWarp + s.comboMix * 0.14f));
        swarmOsc2.setWarp(juce::jlimit(0.0f, 1.0f, s.oscBWarp + s.comboMix * 0.14f));

        const float spreadA = (s.oscASpread + s.reeseWidth * 0.45f + s.stereoSpin * 0.3f) * 0.02f;
        const float spreadB = (s.oscBSpread + s.reeseWidth * 0.45f + s.stereoSpin * 0.3f) * 0.02f;
        const float reeseA_L = oscA.process(-spreadA) * s.oscALevel;
        const float reeseA_R = oscA.process(spreadA) * s.oscALevel;
        const float reeseB_L = oscB.process(spreadB + fmOffset * 0.01f) * s.oscBLevel;
        const float reeseB_R = oscB.process(-spreadB - fmOffset * 0.01f) * s.oscBLevel;
        const float cL = oscC.process(wobble * 0.010f) * s.oscCLevel;
        const float cR = oscC.process(-wobble * 0.008f) * s.oscCLevel;
        const float hL = harmonyOsc.process(-s.harmSpread * 0.02f) * s.harmMix * 0.55f;
        const float hR = harmonyOsc.process(s.harmSpread * 0.02f) * s.harmMix * 0.55f;
        const float sub = subOsc.process(-0.005f * s.subDrive) * s.subLevel;
        const float sub2 = subOctOsc.process(0.004f) * s.subOctaveMix * 0.65f;
        const float subH = subOctOsc.process(0.12f) * s.subHarmony * 0.35f;
        const float swarmL = (swarmOsc1.process(-0.014f) + swarmOsc2.process(0.017f)) * 0.5f * s.swarmMix;
        const float swarmR = (swarmOsc1.process(0.014f) + swarmOsc2.process(-0.017f)) * 0.5f * s.swarmMix;
        const float noiseL = noiseOsc.process(-0.03f) * s.noiseLevel * (0.12f + s.screechDrive * 0.25f + s.air * 0.4f);
        const float noiseR = noiseOsc.process(0.03f) * s.noiseLevel * (0.12f + s.screechDrive * 0.25f + s.air * 0.4f);

        const float hornCore = std::tanh((reeseA_L * 0.35f + reeseB_R * 0.28f + cL * 0.22f + noiseL * 0.45f)
                               * (1.3f + s.hornBody * 2.8f + s.warhornMix * 3.0f)
                               + hornEnvState * s.hornBend * 1.6f);
        const float hornShout = std::sin((hornCore + s.hornFormant * 0.35f) * juce::MathConstants<float>::pi * (1.0f + s.warhornMix * 2.2f));
        const float warhorn = std::tanh((hornCore * 0.75f + hornShout * 0.6f) * (1.0f + s.warhornMix * 3.4f));

        const float wompL = std::tanh((reeseA_L + reeseB_L * 0.8f + cL * 0.65f) * (1.0f + std::abs(wobble) * 4.2f + s.wompAmount * 2.2f));
        const float wompR = std::tanh((reeseA_R + reeseB_R * 0.8f + cR * 0.65f) * (1.0f + std::abs(wobble) * 4.2f + s.wompAmount * 2.2f));
        const float comboL = std::tanh((cL * 0.65f + hL * 0.55f + swarmL * 0.85f + noiseL * 0.2f) * (1.0f + s.comboMix * 4.0f));
        const float comboR = std::tanh((cR * 0.65f + hR * 0.55f + swarmR * 0.85f + noiseR * 0.2f) * (1.0f + s.comboMix * 4.0f));

        float xL = reeseA_L + reeseB_L + cL + hL + noiseL + swarmL * 0.6f;
        float xR = reeseA_R + reeseB_R + cR + hR + noiseR + swarmR * 0.6f;
        const float subLayer = std::tanh((sub + sub2 + subH) * (1.0f + s.subDrive * 4.8f + s.subHarmony * 2.0f + s.punch * 0.8f));
        xL += subLayer;
        xR += subLayer;
        xL = juce::jmap(s.comboMix * 0.5f, xL, xL + comboL * 0.7f);
        xR = juce::jmap(s.comboMix * 0.5f, xR, xR + comboR * 0.7f);
        xL += warhorn * s.warhornMix * 0.72f + wompL * s.wompAmount * 0.65f;
        xR += warhorn * s.warhornMix * 0.68f + wompR * s.wompAmount * 0.65f;

        const float talkMod = s.talk * (0.25f + 0.75f * std::abs(skewLfo));
        const float cutoff1 = juce::jlimit(30.0f, 18000.0f,
            s.cutoff1 * (1.0f + env * s.envAmount + wobble * 0.7f + s.macro2 * 0.20f + s.comboMix * 0.18f));
        const float cutoff2 = juce::jlimit(30.0f, 18000.0f,
            s.cutoff2 * (1.0f + skewLfo * 0.45f + s.hornFormant * 0.55f + s.warhornMix * 0.24f + talkMod));
        const float res1 = juce::jlimit(0.1f, 1.35f, s.res1 + s.reeseWidth * 0.12f + s.wompAmount * 0.08f + s.bite * 0.1f);
        const float res2 = juce::jlimit(0.1f, 1.25f, s.res2 + s.hornBody * 0.10f + s.hornFormant * 0.10f + talkMod * 0.2f);

        for (auto* f : { &lowL, &lowR })
        {
            f->setCutoffFrequency(cutoff1);
            f->setResonance(res1);
        }
        for (auto* f : { &bandL, &bandR })
        {
            f->setCutoffFrequency(cutoff2);
            f->setResonance(res2);
        }

        xL = std::tanh(xL * (1.0f + s.drive1 * 5.0f + s.screechDrive * 2.5f + s.comboMix * 2.2f + s.fmGrit * 1.8f));
        xR = std::tanh(xR * (1.0f + s.drive1 * 5.0f + s.screechDrive * 2.5f + s.comboMix * 2.2f + s.fmGrit * 1.8f));
        const float lowOutL = lowL.processSample(0, xL);
        const float lowOutR = lowR.processSample(0, xR);
        const float bandOutL = bandL.processSample(0, xL);
        const float bandOutR = bandR.processSample(0, xR);

        xL = lowOutL + bandOutL * (0.25f + s.hornFormant * 0.26f + s.wompAmount * 0.22f + s.talk * 0.3f);
        xR = lowOutR + bandOutR * (0.25f + s.hornFormant * 0.26f + s.wompAmount * 0.22f + s.talk * 0.3f);
        const float biteShapeL = std::sin(xL * juce::MathConstants<float>::pi * (1.0f + s.bite * 2.0f));
        const float biteShapeR = std::sin(xR * juce::MathConstants<float>::pi * (1.0f + s.bite * 2.0f));
        xL = juce::jmap(s.bite * 0.5f, xL, biteShapeL);
        xR = juce::jmap(s.bite * 0.5f, xR, biteShapeR);

        const float dirtyL = std::tanh(xL * (1.0f + s.distDrive * 7.5f + s.screechDrive * 4.5f + s.comboMix * 2.2f + s.punch * 1.2f));
        const float dirtyR = std::tanh(xR * (1.0f + s.distDrive * 7.5f + s.screechDrive * 4.5f + s.comboMix * 2.2f + s.punch * 1.2f));
        xL = juce::jmap(s.distMix, xL, dirtyL);
        xR = juce::jmap(s.distMix, xR, dirtyR);

        xL += (xL - lastLeft) * s.punch * 0.35f;
        xR += (xR - lastRight) * s.punch * 0.35f;
        xL = 0.82f * xL + 0.18f * lastLeft;
        xR = 0.82f * xR + 0.18f * lastRight;
        lastLeft = xL;
        lastRight = xR;

        const float hpL = xL - dcLeft + 0.995f * dcLeft;
        const float hpR = xR - dcRight + 0.995f * dcRight;
        dcLeft = xL;
        dcRight = xR;
        xL = hpL + noiseL * s.air * 0.2f;
        xR = hpR + noiseR * s.air * 0.2f;

        const float amp = ampEnv.getNextSample() * velocity;
        xL *= amp;
        xR *= amp;

        const float panMod = juce::jlimit(0.0f, 1.0f, voicePan + skewLfo * s.stereoSpin * 0.12f);
        const float mono = 0.5f * (xL + xR);
        xL = juce::jmap(s.monoBlend, mono, xL) * panLeft(panMod);
        xR = juce::jmap(s.monoBlend, mono, xR) * panRight(panMod);
        xL = std::tanh(xL * (1.0f + s.outputClip * 3.0f));
        xR = std::tanh(xR * (1.0f + s.outputClip * 3.0f));

        if (! ampEnv.isActive())
        {
            active = false;
            heldBySustain = false;
            midiNote = -1;
        }

        return { xL, xR };
    }
};
