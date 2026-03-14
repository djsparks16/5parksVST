#include "PluginProcessor.h"
#include "PluginEditor.h"

BadlineDnBAudioProcessor::BadlineDnBAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout BadlineDnBAudioProcessor::createParameters()
{
    using APF = juce::AudioParameterFloat;
    using APC = juce::AudioParameterChoice;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    auto norm = [](const juce::String& id, const juce::String& name, float min, float max, float def, float skew = 1.0f)
    {
        return std::make_unique<APF>(id, name, juce::NormalisableRange<float>(min, max, 0.0f, skew), def);
    };

    p.push_back(std::make_unique<APC>("play_mode", "Play Mode", juce::StringArray { "Mono", "Legato", "Poly" }, 1));
    p.push_back(std::make_unique<APC>("note_priority", "Note Priority", juce::StringArray { "Last", "Low", "High" }, 0));
    p.push_back(norm("glide_ms", "Glide", 0.0f, 500.0f, 45.0f, 0.35f));

    p.push_back(norm("oscA_level", "OscA Level", 0.0f, 1.0f, 0.86f));
    p.push_back(norm("oscA_wave", "OscA Wave", 0.0f, 1.0f, 0.30f));
    p.push_back(norm("oscA_warp", "OscA Warp", 0.0f, 1.0f, 0.18f));
    p.push_back(norm("oscA_spread", "OscA Spread", 0.0f, 1.0f, 0.22f));

    p.push_back(norm("oscB_level", "OscB Level", 0.0f, 1.0f, 0.76f));
    p.push_back(norm("oscB_wave", "OscB Wave", 0.0f, 1.0f, 0.58f));
    p.push_back(norm("oscB_warp", "OscB Warp", 0.0f, 1.0f, 0.14f));
    p.push_back(norm("oscB_spread", "OscB Spread", 0.0f, 1.0f, 0.28f));

    p.push_back(norm("oscC_level", "OscC Level", 0.0f, 1.0f, 0.46f));
    p.push_back(norm("oscC_wave", "OscC Wave", 0.0f, 1.0f, 0.16f));
    p.push_back(norm("oscC_warp", "OscC Warp", 0.0f, 1.0f, 0.12f));

    p.push_back(norm("sub_level", "Sub Level", 0.0f, 1.0f, 0.72f));
    p.push_back(norm("sub_drive", "Sub Drive", 0.0f, 1.0f, 0.12f));
    p.push_back(norm("sub_octave", "Sub Octave", 0.0f, 1.0f, 0.34f));
    p.push_back(norm("noise_level", "Noise Level", 0.0f, 1.0f, 0.03f));

    p.push_back(norm("filter_cutoff", "Filter 1 Cutoff", 20.0f, 18000.0f, 1200.0f, 0.30f));
    p.push_back(norm("filter_res", "Filter 1 Res", 0.1f, 1.2f, 0.26f));
    p.push_back(norm("filter_drive", "Filter 1 Drive", 0.0f, 1.0f, 0.20f));
    p.push_back(norm("filter2_cutoff", "Filter 2 Cutoff", 20.0f, 18000.0f, 4800.0f, 0.30f));
    p.push_back(norm("filter2_res", "Filter 2 Res", 0.1f, 1.2f, 0.20f));

    p.push_back(norm("dist_drive", "Distortion Drive", 0.0f, 1.0f, 0.30f));
    p.push_back(norm("dist_mix", "Distortion Mix", 0.0f, 1.0f, 0.58f));
    p.push_back(norm("output_clip", "Output Clip", 0.0f, 1.0f, 0.25f));

    p.push_back(norm("lfo1_rate", "LFO 1 Rate", 0.01f, 20.0f, 3.4f, 0.35f));
    p.push_back(norm("lfo1_amt", "LFO 1 Amount", 0.0f, 1.0f, 0.28f));
    p.push_back(norm("envamt", "Env Amount", 0.0f, 2.0f, 0.76f));
    p.push_back(norm("envA", "Attack", 0.001f, 2.0f, 0.005f, 0.40f));
    p.push_back(norm("envD", "Decay", 0.001f, 2.0f, 0.24f, 0.40f));
    p.push_back(norm("envS", "Sustain", 0.0f, 1.0f, 0.74f));
    p.push_back(norm("envR", "Release", 0.001f, 3.0f, 0.20f, 0.40f));

    p.push_back(norm("reese_detune", "Reese Detune", 0.0f, 1.0f, 0.30f));
    p.push_back(norm("reese_width", "Reese Width", 0.0f, 1.0f, 0.42f));
    p.push_back(norm("reese_drift", "Reese Drift", 0.0f, 1.0f, 0.20f));
    p.push_back(norm("horn_bend", "Horn Bend", 0.0f, 1.0f, 0.16f));
    p.push_back(norm("horn_formant", "Horn Formant", 0.0f, 1.0f, 0.18f));
    p.push_back(norm("horn_body", "Horn Body", 0.0f, 1.0f, 0.26f));
    p.push_back(norm("screech_fm", "Screech FM", 0.0f, 1.0f, 0.12f));
    p.push_back(norm("screech_drive", "Screech Drive", 0.0f, 1.0f, 0.10f));
    p.push_back(norm("harm_mix", "Harmony Mix", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("harm_interval", "Harmony Interval", -24.0f, 24.0f, 7.0f));
    p.push_back(norm("harm_spread", "Harmony Spread", 0.0f, 1.0f, 0.24f));

    p.push_back(norm("warhorn_mix", "Warhorn Mix", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("womp_amount", "Womp", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("sub_harmony", "Sub Harmony", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("combo_mix", "Combo Stack", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("swarm_mix", "Swarm Mix", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("swarm_detune", "Swarm Detune", 0.0f, 1.0f, 0.20f));
    p.push_back(norm("stereo_spin", "Stereo Spin", 0.0f, 1.0f, 0.18f));
    p.push_back(norm("fm_grit", "FM Grit", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("bite", "Bite", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("punch", "Punch", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("talk", "Talk", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("air", "Air", 0.0f, 1.0f, 0.0f));
    p.push_back(norm("mono_blend", "Mono Blend", 0.0f, 1.0f, 0.78f));

    for (int i = 1; i <= 8; ++i)
        p.push_back(norm("macro" + juce::String(i), "Macro " + juce::String(i), 0.0f, 1.0f, 0.0f));

    p.push_back(norm("master_gain", "Output", 0.0f, 1.0f, 0.82f));
    return { p.begin(), p.end() };
}

void BadlineDnBAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    for (auto& v : voices)
        v.prepare(sampleRate);

    keyDown.fill(false);
    sustained.fill(false);
    noteVelocity.fill(1.0f);
    monoStack.clear();
    sustainPedalDown = false;
    noteCounter = 0;
    meterLevel.store(0.0f);
    subMeterLevel.store(0.0f);
    scopeWritePos.store(0);
    fftFifoIndex = 0;
    analyzerBins.fill(0.0f);
}

void BadlineDnBAudioProcessor::releaseResources() {}

bool BadlineDnBAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void BadlineDnBAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    const auto playMode = static_cast<PlayMode>((int) apvts.getRawParameterValue("play_mode")->load());
    const auto priority = static_cast<NotePriority>((int) apvts.getRawParameterValue("note_priority")->load());

    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isController() && msg.getControllerNumber() == 64)
        {
            handleSustainPedal(msg.getControllerValue() >= 64, playMode);
            continue;
        }

        if (msg.isNoteOn()) handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity(), playMode);
        if (msg.isNoteOff()) handleNoteOff(msg.getNoteNumber(), playMode);
        if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            for (auto& v : voices) v.stop();
            keyDown.fill(false);
            sustained.fill(false);
            monoStack.clear();
            sustainPedalDown = false;
        }
    }

    BlacksideVoice::RuntimeParams rp;
    auto load = [this](const char* id) { return apvts.getRawParameterValue(id)->load(); };
    rp.oscALevel = load("oscA_level"); rp.oscAWave = load("oscA_wave"); rp.oscAWarp = load("oscA_warp"); rp.oscASpread = load("oscA_spread");
    rp.oscBLevel = load("oscB_level"); rp.oscBWave = load("oscB_wave"); rp.oscBWarp = load("oscB_warp"); rp.oscBSpread = load("oscB_spread");
    rp.oscCLevel = load("oscC_level"); rp.oscCWave = load("oscC_wave"); rp.oscCWarp = load("oscC_warp");
    rp.subLevel = load("sub_level"); rp.subDrive = load("sub_drive"); rp.subOctaveMix = load("sub_octave"); rp.noiseLevel = load("noise_level");
    rp.cutoff1 = load("filter_cutoff"); rp.res1 = load("filter_res"); rp.drive1 = load("filter_drive");
    rp.cutoff2 = load("filter2_cutoff"); rp.res2 = load("filter2_res");
    rp.distDrive = load("dist_drive"); rp.distMix = load("dist_mix"); rp.outputClip = load("output_clip");
    rp.lfoRate = load("lfo1_rate"); rp.lfoAmount = load("lfo1_amt"); rp.envAmount = load("envamt");
    rp.reeseDetune = load("reese_detune"); rp.reeseWidth = load("reese_width"); rp.reeseDrift = load("reese_drift");
    rp.hornBend = load("horn_bend"); rp.hornFormant = load("horn_formant"); rp.hornBody = load("horn_body");
    rp.screechFm = load("screech_fm"); rp.screechDrive = load("screech_drive");
    rp.harmMix = load("harm_mix"); rp.harmInterval = load("harm_interval"); rp.harmSpread = load("harm_spread");
    rp.warhornMix = load("warhorn_mix"); rp.wompAmount = load("womp_amount"); rp.subHarmony = load("sub_harmony");
    rp.comboMix = load("combo_mix"); rp.swarmMix = load("swarm_mix"); rp.swarmDetune = load("swarm_detune");
    rp.stereoSpin = load("stereo_spin"); rp.fmGrit = load("fm_grit"); rp.bite = load("bite"); rp.punch = load("punch"); rp.talk = load("talk"); rp.air = load("air"); rp.monoBlend = load("mono_blend");
    rp.macro1 = load("macro1"); rp.macro2 = load("macro2"); rp.macro3 = load("macro3"); rp.macro4 = load("macro4");
    const float glideMs = load("glide_ms");
    const float envA = load("envA"); const float envD = load("envD"); const float envS = load("envS"); const float envR = load("envR");
    const float master = load("master_gain");

    for (auto& v : voices)
    {
        v.setGlideMs(glideMs);
        v.updateADSR(envA, envD, envS, envR);
    }

    float peak = 0.0f;
    float subPeak = 0.0f;

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        BlacksideStereoSample mix;

        if (playMode == PlayMode::poly)
        {
            for (auto& v : voices)
                mix += v.nextSample(rp);
            mix.left *= 0.24f;
            mix.right *= 0.24f;
        }
        else
        {
            mix = voices[0].nextSample(rp);
        }

        mix.left *= master;
        mix.right *= master;
        const float mono = 0.5f * (mix.left + mix.right);
        peak = juce::jmax(peak, juce::jmax(std::abs(mix.left), std::abs(mix.right)));
        subPeak = juce::jmax(subPeak, std::abs(mono) * rp.subLevel);
        pushScopeSample(mono);
        pushAnalyzerSample(mono);

        buffer.setSample(0, i, mix.left);
        if (buffer.getNumChannels() > 1)
            buffer.setSample(1, i, mix.right);
    }

    meterLevel.store(juce::jmax(peak, meterLevel.load() * 0.90f));
    subMeterLevel.store(juce::jmax(subPeak, subMeterLevel.load() * 0.90f));
    juce::ignoreUnused(priority);
}

void BadlineDnBAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void BadlineDnBAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* BadlineDnBAudioProcessor::createEditor()
{
    return new BadlineDnBAudioProcessorEditor(*this);
}

void BadlineDnBAudioProcessor::copyScopeData(std::array<float, 512>& dest) const noexcept
{
    const int write = scopeWritePos.load();
    const int size = (int) scopeRing.size();
    for (int i = 0; i < (int) dest.size(); ++i)
    {
        const int index = (write - (int) dest.size() + i + size) % size;
        dest[(size_t) i] = scopeRing[(size_t) index];
    }
}

void BadlineDnBAudioProcessor::copyAnalyzerData(std::array<float, 128>& dest) const noexcept
{
    dest = analyzerBins;
}

void BadlineDnBAudioProcessor::handleNoteOn(int note, float velocity, PlayMode mode)
{
    keyDown[(size_t) note] = true;
    sustained[(size_t) note] = false;
    noteVelocity[(size_t) note] = velocity;
    monoStack.erase(std::remove(monoStack.begin(), monoStack.end(), note), monoStack.end());
    monoStack.push_back(note);

    if (mode == PlayMode::poly)
    {
        startPolyVoice(note, velocity, static_cast<NotePriority>((int) apvts.getRawParameterValue("note_priority")->load()));
        return;
    }

    if (mode == PlayMode::legato && voices[0].active)
        voices[0].legatoTo(note, velocity, ++noteCounter);
    else
        startMonoVoice(note, velocity, true);
}

void BadlineDnBAudioProcessor::handleNoteOff(int note, PlayMode mode)
{
    keyDown[(size_t) note] = false;

    if (sustainPedalDown)
    {
        sustained[(size_t) note] = true;
        if (mode == PlayMode::poly)
        {
            const int idx = findVoiceForNote(note);
            if (idx >= 0)
                voices[(size_t) idx].heldBySustain = true;
        }
        return;
    }

    if (mode == PlayMode::poly)
    {
        releasePolyNote(note);
        return;
    }

    monoStack.erase(std::remove(monoStack.begin(), monoStack.end(), note), monoStack.end());
    retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::handleSustainPedal(bool down, PlayMode mode)
{
    sustainPedalDown = down;
    if (down)
        return;

    for (int note = 0; note < 128; ++note)
    {
        if (sustained[(size_t) note] && ! keyDown[(size_t) note])
        {
            sustained[(size_t) note] = false;
            if (mode == PlayMode::poly)
                releasePolyNote(note);
        }
    }

    if (mode != PlayMode::poly)
        retriggerMonoFromStack();
}

void BadlineDnBAudioProcessor::startMonoVoice(int note, float velocity, bool retrigger)
{
    voices[0].start(note, velocity, retrigger, ++noteCounter);
}

void BadlineDnBAudioProcessor::retriggerMonoFromStack()
{
    for (auto it = monoStack.rbegin(); it != monoStack.rend(); ++it)
    {
        const int nextNote = *it;
        if (keyDown[(size_t) nextNote] || sustained[(size_t) nextNote])
        {
            const float vel = noteVelocity[(size_t) nextNote];
            if (voices[0].active)
                voices[0].legatoTo(nextNote, vel, ++noteCounter);
            else
                voices[0].start(nextNote, vel, true, ++noteCounter);
            return;
        }
    }

    releaseMonoIfIdle();
}

void BadlineDnBAudioProcessor::releaseMonoIfIdle()
{
    for (bool k : keyDown)
        if (k)
            return;
    voices[0].stop();
}

int BadlineDnBAudioProcessor::findFreeVoice() const
{
    for (int i = 0; i < maxVoices; ++i)
        if (! voices[(size_t) i].active)
            return i;
    return -1;
}

int BadlineDnBAudioProcessor::findVoiceForNote(int note) const
{
    for (int i = 0; i < maxVoices; ++i)
        if (voices[(size_t) i].active && voices[(size_t) i].midiNote == note)
            return i;
    return -1;
}

int BadlineDnBAudioProcessor::stealVoice(NotePriority priority) const
{
    int candidate = 0;
    if (priority == NotePriority::low)
    {
        int lowest = 127;
        for (int i = 0; i < maxVoices; ++i)
            if (voices[(size_t) i].midiNote < lowest) { lowest = voices[(size_t) i].midiNote; candidate = i; }
        return candidate;
    }
    if (priority == NotePriority::high)
    {
        int highest = -1;
        for (int i = 0; i < maxVoices; ++i)
            if (voices[(size_t) i].midiNote > highest) { highest = voices[(size_t) i].midiNote; candidate = i; }
        return candidate;
    }

    uint64_t oldestAge = std::numeric_limits<uint64_t>::max();
    for (int i = 0; i < maxVoices; ++i)
    {
        if (voices[(size_t) i].noteAge < oldestAge)
        {
            oldestAge = voices[(size_t) i].noteAge;
            candidate = i;
        }
    }
    return candidate;
}

void BadlineDnBAudioProcessor::startPolyVoice(int note, float velocity, NotePriority priority)
{
    int idx = findFreeVoice();
    if (idx < 0)
        idx = stealVoice(priority);
    voices[(size_t) idx].start(note, velocity, true, ++noteCounter);
}

void BadlineDnBAudioProcessor::releasePolyNote(int note)
{
    const int idx = findVoiceForNote(note);
    if (idx >= 0)
        voices[(size_t) idx].stop();
}

void BadlineDnBAudioProcessor::pushScopeSample(float s) noexcept
{
    const int size = (int) scopeRing.size();
    const int pos = scopeWritePos.fetch_add(1);
    scopeRing[(size_t) (pos % size)] = s;
}

void BadlineDnBAudioProcessor::pushAnalyzerSample(float s) noexcept
{
    if (fftFifoIndex < (int) fftFifo.size())
        fftFifo[(size_t) fftFifoIndex++] = s;

    if (fftFifoIndex == (int) fftFifo.size())
    {
        computeAnalyzerFrame();
        fftFifoIndex = 0;
    }
}

void BadlineDnBAudioProcessor::computeAnalyzerFrame() noexcept
{
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    std::copy(fftFifo.begin(), fftFifo.end(), fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(), fftFifo.size());
    fft.performFrequencyOnlyForwardTransform(fftData.data());

    for (size_t i = 0; i < analyzerBins.size(); ++i)
    {
        const int index = juce::jlimit(1, 511, (int) std::pow((double) i / (double) analyzerBins.size(), 2.0) * 511 + 1);
        const float mag = juce::Decibels::gainToDecibels(fftData[(size_t) index] / 32.0f, -96.0f);
        analyzerBins[i] = juce::jlimit(0.0f, 1.0f, juce::jmap(mag, -72.0f, 12.0f, 0.0f, 1.0f));
    }
}
