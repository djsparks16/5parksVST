#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <vector>

#include "PluginProcessor.h"

class ScopeComponent : public juce::Component, private juce::Timer
{
public:
    explicit ScopeComponent(BadlineDnBAudioProcessor& p) : processor(p) { startTimerHz(30); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    BadlineDnBAudioProcessor& processor;
    std::array<float, 512> data {};
};

class SpectrumComponent : public juce::Component, private juce::Timer
{
public:
    explicit SpectrumComponent(BadlineDnBAudioProcessor& p) : processor(p) { startTimerHz(20); }
    void paint(juce::Graphics& g) override;
private:
    void timerCallback() override;
    BadlineDnBAudioProcessor& processor;
    std::array<float, 128> data {};
};

class LabeledKnob : public juce::Component
{
public:
    LabeledKnob();
    void resized() override;
    juce::Slider slider;
    juce::Label title;
    juce::Label footer;
};

class BadlineDnBAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor&);
    ~BadlineDnBAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    void timerCallback() override;
    void setupKnob(LabeledKnob& knob, const juce::String& title, const juce::String& footer);
    void attachKnob(juce::Component& parent, LabeledKnob& knob, const juce::String& paramID, const juce::String& title, const juce::String& footer);
    void styleCombo(juce::ComboBox& box);
    void styleHeader(juce::Label& label);

    BadlineDnBAudioProcessor& audioProcessor;

    juce::Component subPanel;
    juce::Component oscAPanel;
    juce::Component oscBPanel;
    juce::Component filterPanel;
    juce::Component modPanel;
    juce::Component envPanel;
    juce::Component lfoPanel;
    juce::Component performPanel;
    juce::Component footerPanel;

    juce::Label titleLabel, subtitleLabel;
    juce::Label subHeader, oscAHeader, oscBHeader, filterHeader, modHeader, envHeader, lfoHeader, performHeader;
    juce::Label playModeLabel, notePriorityLabel;

    ScopeComponent scope;
    SpectrumComponent spectrum;

    LabeledKnob subLevel, subOctave, subHarmony, monoBlend;
    LabeledKnob oscAWave, oscAWarp, reeseDetune, reeseWidth;
    LabeledKnob oscBWave, oscBWarp, comboMix, swarmMix;
    LabeledKnob cutoff, res, warhornMix, talk;
    LabeledKnob drive, fmGrit, bite, air;
    LabeledKnob lfoRate, lfoAmt, womp, stereoSpin;
    LabeledKnob envAmt, punch, hornBend, hornFormant;
    LabeledKnob macro1, macro2, macro3, macro4;
    LabeledKnob glide, masterGain, harmMix, screechDrive;

    juce::ComboBox playMode;
    juce::ComboBox notePriority;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::unique_ptr<ComboAttachment> playModeAtt;
    std::unique_ptr<ComboAttachment> notePriorityAtt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BadlineDnBAudioProcessorEditor)
};
