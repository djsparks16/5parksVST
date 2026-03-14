#include "PluginEditor.h"

namespace
{
    using namespace juce;

    Colour bg() { return Colour(0xff06111a); }
    Colour panel() { return Colour(0xff0b2232); }
    Colour edge() { return Colour(0x5537d6ff); }
    Colour glow() { return Colour(0xff7fe8ff); }
    Colour textHi() { return Colours::white.withAlpha(0.92f); }
    Colour textLo() { return Colours::white.withAlpha(0.62f); }

    void drawPanel(Graphics& g, Rectangle<float> r, const String& title)
    {
        g.setColour(panel());
        g.fillRoundedRectangle(r, 12.0f);
        g.setColour(edge());
        g.drawRoundedRectangle(r, 12.0f, 1.0f);
        g.setColour(textHi());
        g.setFont(FontOptions(13.0f, Font::bold));
        g.drawText(title, r.getX() + 10.0f, r.getY() + 6.0f, r.getWidth() - 20.0f, 18.0f, Justification::left);
    }
}

LabeledKnob::LabeledKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    addAndMakeVisible(footer);

    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 16);
    slider.setColour(juce::Slider::rotarySliderFillColourId, glow());
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff66d7ff));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, panel().darker(0.5f));
    slider.setColour(juce::Slider::textBoxTextColourId, textHi());
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    title.setJustificationType(juce::Justification::centred);
    footer.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::textColourId, textHi());
    footer.setColour(juce::Label::textColourId, textLo());
    title.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    footer.setFont(juce::FontOptions(9.0f, juce::Font::plain));
}

void LabeledKnob::resized()
{
    auto r = getLocalBounds();
    title.setBounds(r.removeFromTop(14));
    footer.setBounds(r.removeFromBottom(12));
    slider.setBounds(r.reduced(2));
}

void ScopeComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(panel().darker(0.4f));
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(edge());
    g.drawRoundedRectangle(area, 10.0f, 1.0f);

    juce::Path p;
    const float x0 = area.getX() + 8.0f;
    const float w = area.getWidth() - 16.0f;
    const float y0 = area.getCentreY();
    const float h = area.getHeight() - 16.0f;
    p.startNewSubPath(x0, y0);
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = x0 + w * (float) i / (float) (data.size() - 1);
        const float y = y0 - data[i] * h * 0.42f;
        p.lineTo(x, y);
    }
    g.setColour(glow().withAlpha(0.20f));
    g.strokePath(p, juce::PathStrokeType(4.0f));
    g.setColour(glow());
    g.strokePath(p, juce::PathStrokeType(1.5f));
}

void ScopeComponent::timerCallback()
{
    processor.copyScopeData(data);
    repaint();
}

void SpectrumComponent::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setColour(panel().darker(0.4f));
    g.fillRoundedRectangle(area, 10.0f);
    g.setColour(edge());
    g.drawRoundedRectangle(area, 10.0f, 1.0f);
    auto inner = area.reduced(8.0f);
    const float barW = inner.getWidth() / (float) data.size();
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float h = data[i] * inner.getHeight();
        juce::Rectangle<float> bar(inner.getX() + barW * (float) i, inner.getBottom() - h, juce::jmax(1.0f, barW - 1.0f), h);
        auto c = juce::Colour::fromHSV(0.32f - 0.18f * data[i], 0.70f, 0.96f, 0.85f);
        g.setColour(c);
        g.fillRoundedRectangle(bar, 1.5f);
    }
}

void SpectrumComponent::timerCallback()
{
    processor.copyAnalyzerData(data);
    repaint();
}

BadlineDnBAudioProcessorEditor::BadlineDnBAudioProcessorEditor(BadlineDnBAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p), spectrum(p)
{
    setSize(1570, 930);
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(subtitleLabel);
    titleLabel.setText("BLACKSIDE BASS MONSTER", juce::dontSendNotification);
    subtitleLabel.setText("Warhorn / Reese / Womp / Combo workstation", juce::dontSendNotification);
    titleLabel.setColour(juce::Label::textColourId, textHi());
    subtitleLabel.setColour(juce::Label::textColourId, textLo());
    titleLabel.setFont(juce::FontOptions(26.0f, juce::Font::bold));
    subtitleLabel.setFont(juce::FontOptions(13.0f, juce::Font::plain));

    for (auto* c : { &subPanel, &oscAPanel, &oscBPanel, &filterPanel, &modPanel, &envPanel, &lfoPanel, &performPanel, &footerPanel })
        addAndMakeVisible(*c);

    for (auto* h : { &subHeader, &oscAHeader, &oscBHeader, &filterHeader, &modHeader, &envHeader, &lfoHeader, &performHeader })
    {
        styleHeader(*h);
    }
    subHeader.setText("SUB", juce::dontSendNotification);
    oscAHeader.setText("OSC A", juce::dontSendNotification);
    oscBHeader.setText("OSC B / COMBO", juce::dontSendNotification);
    filterHeader.setText("FILTER / TONE", juce::dontSendNotification);
    modHeader.setText("MOD", juce::dontSendNotification);
    envHeader.setText("ENV", juce::dontSendNotification);
    lfoHeader.setText("LFO", juce::dontSendNotification);
    performHeader.setText("PERFORM", juce::dontSendNotification);

    for (auto pair : { std::make_pair(&subPanel, &subHeader), std::make_pair(&oscAPanel, &oscAHeader), std::make_pair(&oscBPanel, &oscBHeader),
                       std::make_pair(&filterPanel, &filterHeader), std::make_pair(&modPanel, &modHeader), std::make_pair(&envPanel, &envHeader),
                       std::make_pair(&lfoPanel, &lfoHeader), std::make_pair(&performPanel, &performHeader) })
        pair.first->addAndMakeVisible(*pair.second);

    oscAPanel.addAndMakeVisible(scope);
    filterPanel.addAndMakeVisible(spectrum);

    attachKnob(subPanel, subLevel, "sub_level", "LEVEL", "Sub body");
    attachKnob(subPanel, subOctave, "sub_octave", "OCT", "Extra depth");
    attachKnob(subPanel, subHarmony, "sub_harmony", "HARM", "Lower stack");
    attachKnob(subPanel, monoBlend, "mono_blend", "MONO", "Mono lock");

    attachKnob(oscAPanel, oscAWave, "oscA_wave", "WT POS", "Main shape");
    attachKnob(oscAPanel, oscAWarp, "oscA_warp", "WARP", "Drive curve");
    attachKnob(oscAPanel, reeseDetune, "reese_detune", "DETUNE", "Reese split");
    attachKnob(oscAPanel, reeseWidth, "reese_width", "WIDTH", "Stereo width");

    attachKnob(oscBPanel, oscBWave, "oscB_wave", "WT POS", "Counter wave");
    attachKnob(oscBPanel, oscBWarp, "oscB_warp", "WARP", "Metal shape");
    attachKnob(oscBPanel, comboMix, "combo_mix", "COMBO", "Layer glue");
    attachKnob(oscBPanel, swarmMix, "swarm_mix", "SWARM", "Stack more");

    attachKnob(filterPanel, cutoff, "filter_cutoff", "CUTOFF", "Main move");
    attachKnob(filterPanel, res, "filter_res", "RES", "Acid edge");
    attachKnob(filterPanel, warhornMix, "warhorn_mix", "WARHORN", "Brassy shout");
    attachKnob(filterPanel, talk, "talk", "TALK", "Vocal notch");

    attachKnob(modPanel, drive, "dist_drive", "DRIVE", "Final dirt");
    attachKnob(modPanel, fmGrit, "fm_grit", "FM GRIT", "Razor edge");
    attachKnob(modPanel, bite, "bite", "BITE", "Sharper attack");
    attachKnob(modPanel, air, "air", "AIR", "Top fizz");

    attachKnob(lfoPanel, lfoRate, "lfo1_rate", "RATE", "Motion speed");
    attachKnob(lfoPanel, lfoAmt, "lfo1_amt", "AMOUNT", "Movement");
    attachKnob(lfoPanel, womp, "womp_amount", "WOMP", "Donk sweep");
    attachKnob(lfoPanel, stereoSpin, "stereo_spin", "SPIN", "Stereo orbit");

    attachKnob(envPanel, envAmt, "envamt", "ENV", "Punch curve");
    attachKnob(envPanel, punch, "punch", "PUNCH", "Transient hit");
    attachKnob(envPanel, hornBend, "horn_bend", "BEND", "Horn envelope");
    attachKnob(envPanel, hornFormant, "horn_formant", "FORMANT", "Throat shape");

    attachKnob(performPanel, macro1, "macro1", "MAC 1", "Reese sweep");
    attachKnob(performPanel, macro2, "macro2", "MAC 2", "Body push");
    attachKnob(performPanel, macro3, "macro3", "MAC 3", "Texture mix");
    attachKnob(performPanel, macro4, "macro4", "MAC 4", "Tone stab");
    attachKnob(performPanel, glide, "glide_ms", "GLIDE", "Porta");
    attachKnob(performPanel, masterGain, "master_gain", "MASTER", "Output");
    attachKnob(performPanel, harmMix, "harm_mix", "HARM MIX", "Upper stack");
    attachKnob(performPanel, screechDrive, "screech_drive", "SCREECH", "Metal drive");

    performPanel.addAndMakeVisible(playModeLabel);
    performPanel.addAndMakeVisible(notePriorityLabel);
    performPanel.addAndMakeVisible(playMode);
    performPanel.addAndMakeVisible(notePriority);
    playModeLabel.setText("PLAY MODE", juce::dontSendNotification);
    notePriorityLabel.setText("NOTE PRIORITY", juce::dontSendNotification);
    playModeLabel.setColour(juce::Label::textColourId, textLo());
    notePriorityLabel.setColour(juce::Label::textColourId, textLo());
    playModeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    notePriorityLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));

    styleCombo(playMode);
    styleCombo(notePriority);
    playMode.addItem("Mono", 1); playMode.addItem("Legato", 2); playMode.addItem("Poly", 3);
    notePriority.addItem("Last", 1); notePriority.addItem("Low", 2); notePriority.addItem("High", 3);
    playModeAtt = std::make_unique<ComboAttachment>(audioProcessor.apvts, "play_mode", playMode);
    notePriorityAtt = std::make_unique<ComboAttachment>(audioProcessor.apvts, "note_priority", notePriority);

    startTimerHz(24);
}

void BadlineDnBAudioProcessorEditor::styleHeader(juce::Label& label)
{
    label.setColour(juce::Label::textColourId, textHi());
    label.setFont(juce::FontOptions(13.0f, juce::Font::bold));
}

void BadlineDnBAudioProcessorEditor::setupKnob(LabeledKnob& knob, const juce::String& t, const juce::String& f)
{
    knob.title.setText(t, juce::dontSendNotification);
    knob.footer.setText(f, juce::dontSendNotification);
}

void BadlineDnBAudioProcessorEditor::attachKnob(juce::Component& parent, LabeledKnob& knob, const juce::String& paramID, const juce::String& t, const juce::String& f)
{
    setupKnob(knob, t, f);
    parent.addAndMakeVisible(knob);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, paramID, knob.slider));
}

void BadlineDnBAudioProcessorEditor::styleCombo(juce::ComboBox& box)
{
    box.setColour(juce::ComboBox::backgroundColourId, panel().darker(0.5f));
    box.setColour(juce::ComboBox::outlineColourId, edge());
    box.setColour(juce::ComboBox::textColourId, textHi());
    box.setColour(juce::ComboBox::arrowColourId, glow());
}

void BadlineDnBAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.fillAll(bg());
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff051019), 0.0f, 0.0f, juce::Colour(0xff11324b), 0.0f, area.getBottom(), false));
    g.fillRoundedRectangle(area.reduced(10.0f), 18.0f);

    drawPanel(g, subPanel.getBounds().toFloat(), "SUB / DIRECT OUT");
    drawPanel(g, oscAPanel.getBounds().toFloat(), "OSC A");
    drawPanel(g, oscBPanel.getBounds().toFloat(), "OSC B");
    drawPanel(g, filterPanel.getBounds().toFloat(), "FILTER");
    drawPanel(g, modPanel.getBounds().toFloat(), "MOD");
    drawPanel(g, envPanel.getBounds().toFloat(), "ENV 1");
    drawPanel(g, lfoPanel.getBounds().toFloat(), "LFO 1");
    drawPanel(g, performPanel.getBounds().toFloat(), "PERFORM / MASTER");
    drawPanel(g, footerPanel.getBounds().toFloat(), "KEYBOARD / METERS");

    auto meter = [&](float x, float value, const juce::String& label, juce::Colour c)
    {
        juce::Rectangle<float> r(x, 826.0f, 16.0f, 76.0f);
        g.setColour(panel().darker(0.6f));
        g.fillRoundedRectangle(r, 6.0f);
        const float h = juce::jlimit(0.0f, 1.0f, value) * (r.getHeight() - 8.0f);
        juce::Rectangle<float> fill(r.getX() + 3.0f, r.getBottom() - 4.0f - h, r.getWidth() - 6.0f, h);
        g.setColour(c);
        g.fillRoundedRectangle(fill, 4.0f);
        g.setColour(textLo());
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(label, (int) r.getX() - 2, (int) r.getBottom() + 4, 22, 12, juce::Justification::centred);
    };
    meter(38.0f, audioProcessor.getMeterLevel(), "M", glow());
    meter(60.0f, audioProcessor.getSubMeterLevel(), "S", juce::Colour(0xff84ffb0));

    g.setColour(textLo());
    g.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    g.drawText("Inspired by modern bassline workflow: warhorns, reese, womps, stacked subs, combo layers.", 1040, 26, 480, 16, juce::Justification::right);

    auto kb = footerPanel.getBounds().reduced(16, 42);
    const int whiteKeys = 28;
    const float whiteW = kb.getWidth() / (float) whiteKeys;
    for (int i = 0; i < whiteKeys; ++i)
    {
        juce::Rectangle<float> k(kb.getX() + whiteW * (float) i, kb.getY(), whiteW - 2.0f, kb.getHeight());
        g.setColour(juce::Colour(0xffb9d6ff).withAlpha(0.85f));
        g.fillRoundedRectangle(k, 3.0f);
    }
}

void BadlineDnBAudioProcessorEditor::resized()
{
    titleLabel.setBounds(92, 18, 440, 26);
    subtitleLabel.setBounds(94, 48, 320, 18);

    subPanel.setBounds(18, 86, 110, 300);
    oscAPanel.setBounds(136, 86, 430, 300);
    oscBPanel.setBounds(574, 86, 430, 300);
    filterPanel.setBounds(1012, 86, 540, 300);

    modPanel.setBounds(18, 394, 110, 300);
    envPanel.setBounds(136, 394, 450, 300);
    lfoPanel.setBounds(594, 394, 450, 300);
    performPanel.setBounds(1052, 394, 500, 300);
    footerPanel.setBounds(18, 730, 1534, 182);

    for (auto pair : { std::make_pair(&subPanel, &subHeader), std::make_pair(&oscAPanel, &oscAHeader), std::make_pair(&oscBPanel, &oscBHeader),
                       std::make_pair(&filterPanel, &filterHeader), std::make_pair(&modPanel, &modHeader), std::make_pair(&envPanel, &envHeader),
                       std::make_pair(&lfoPanel, &lfoHeader), std::make_pair(&performPanel, &performHeader) })
        pair.second->setBounds(pair.first->getLocalBounds().reduced(10).removeFromTop(18));


    auto subArea = subPanel.getLocalBounds().reduced(8); subArea.removeFromTop(28);
    auto subRow1 = subArea.removeFromTop(130); auto subRow2 = subArea.removeFromTop(130);
    subLevel.setBounds(subRow1.removeFromLeft(47).withSizeKeepingCentre(44, 118));
    subOctave.setBounds(subRow1.removeFromLeft(55).withSizeKeepingCentre(52, 118));
    subHarmony.setBounds(subRow2.removeFromLeft(55).withSizeKeepingCentre(52, 118));
    monoBlend.setBounds(subRow2.removeFromLeft(55).withSizeKeepingCentre(52, 118));

    auto aArea = oscAPanel.getLocalBounds().reduced(10); aArea.removeFromTop(28);
    auto aTop = aArea.removeFromTop(166); scope.setBounds(aTop.removeFromLeft(250).reduced(4));
    oscAWave.setBounds(aTop.removeFromLeft(82).reduced(2)); oscAWarp.setBounds(aTop.removeFromLeft(82).reduced(2));
    reeseDetune.setBounds(aArea.removeFromLeft(90).reduced(4)); reeseWidth.setBounds(aArea.removeFromLeft(90).reduced(4));

    auto bArea = oscBPanel.getLocalBounds().reduced(10); bArea.removeFromTop(28);
    auto bTop = bArea.removeFromTop(166);
    oscBWave.setBounds(bTop.removeFromLeft(82).reduced(2)); oscBWarp.setBounds(bTop.removeFromLeft(82).reduced(2));
    comboMix.setBounds(bTop.removeFromLeft(82).reduced(2)); swarmMix.setBounds(bTop.removeFromLeft(82).reduced(2));

    auto fArea = filterPanel.getLocalBounds().reduced(10); fArea.removeFromTop(28);
    auto fTop = fArea.removeFromTop(166); spectrum.setBounds(fTop.removeFromLeft(260).reduced(4));
    cutoff.setBounds(fTop.removeFromLeft(68).reduced(2)); res.setBounds(fTop.removeFromLeft(68).reduced(2)); warhornMix.setBounds(fTop.removeFromLeft(84).reduced(2)); talk.setBounds(fTop.removeFromLeft(68).reduced(2));
    auto fBottom = fArea.removeFromTop(120); drive.setBounds(fBottom.removeFromLeft(84).reduced(4)); fmGrit.setBounds(fBottom.removeFromLeft(84).reduced(4)); bite.setBounds(fBottom.removeFromLeft(84).reduced(4)); air.setBounds(fBottom.removeFromLeft(84).reduced(4));

    auto mArea = modPanel.getLocalBounds().reduced(8); mArea.removeFromTop(28);
    lfoRate.setBounds(mArea.removeFromTop(60).reduced(2)); lfoAmt.setBounds(mArea.removeFromTop(60).reduced(2)); womp.setBounds(mArea.removeFromTop(60).reduced(2)); stereoSpin.setBounds(mArea.removeFromTop(60).reduced(2));

    auto eArea = envPanel.getLocalBounds().reduced(10); eArea.removeFromTop(28);
    auto eTop = eArea.removeFromTop(140); envAmt.setBounds(eTop.removeFromLeft(86).reduced(3)); punch.setBounds(eTop.removeFromLeft(86).reduced(3)); hornBend.setBounds(eTop.removeFromLeft(86).reduced(3)); hornFormant.setBounds(eTop.removeFromLeft(86).reduced(3));

    auto lArea = lfoPanel.getLocalBounds().reduced(10); lArea.removeFromTop(28);
    auto lTop = lArea.removeFromTop(140); macro1.setBounds(lTop.removeFromLeft(86).reduced(3)); macro2.setBounds(lTop.removeFromLeft(86).reduced(3)); macro3.setBounds(lTop.removeFromLeft(86).reduced(3)); macro4.setBounds(lTop.removeFromLeft(86).reduced(3));

    auto pArea = performPanel.getLocalBounds().reduced(10); pArea.removeFromTop(28);
    auto pRow = pArea.removeFromTop(24);
    playModeLabel.setBounds(pRow.removeFromLeft(90)); playMode.setBounds(pRow.removeFromLeft(120)); pRow.removeFromLeft(16); notePriorityLabel.setBounds(pRow.removeFromLeft(94)); notePriority.setBounds(pRow.removeFromLeft(120));
    auto pKnobs = pArea.removeFromTop(150);
    glide.setBounds(pKnobs.removeFromLeft(84).reduced(4)); masterGain.setBounds(pKnobs.removeFromLeft(84).reduced(4)); harmMix.setBounds(pKnobs.removeFromLeft(84).reduced(4)); screechDrive.setBounds(pKnobs.removeFromLeft(84).reduced(4));
}

void BadlineDnBAudioProcessorEditor::timerCallback()
{
    repaint();
}
