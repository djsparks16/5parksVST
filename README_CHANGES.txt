Blackside Bass Monster patch

Major upgrades in this pass:
- Added a deeper stereo voice engine with left/right rendering instead of mono duplication.
- Added combo-stack, swarm, warhorn, womp, sub-harmony, bite, punch, talk, air, stereo-spin, fm-grit, and mono-blend controls in DSP.
- Added age-based voice stealing for poly mode.
- Fixed mono-note retrigger to recall note velocity instead of forcing 1.0.
- Added a fuller single-screen UI with sub, osc, filter, mod, env, lfo, and perform sections inspired by the reference layout.
- Kept the implementation on public JUCE DSP APIs only.

Notes:
- This is still a focused synth engine, not full wavetable import or a full modulation matrix yet.
- The broadest next leap would be: drawable MSEG/LFO editor, filter-mode selector, distortion-mode selector, true wavetable tables, and preset browser.
