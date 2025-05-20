## Rules

Check how this works: Make sure agent does not try to delete the wrong folder.
This build command seems safe:
/opt/homebrew/bin/cmake --build build

## Vibes

-  Coding initial prompt:

I want to make a VST audio effect plugin. I want it to use Juce 8 and compile for Ableton and Logic Pro on Apply Mac. The KINA VST has an effect chain, starting from the first to last, as follows: it uses an LFO modulating a VCA. The LFO for the VCA shape can be set to sine, triangle, saw, square, or random. It can be synced to the DAW bpm at divisions up to 1/64. The LFO for the VCA can go up to 10,000Hz to create ring-mod like effects. The LFO for the VCA allows the VCA to take the sound between 0 and maximum volume.  the VCA has a control for the amount. The next stage in the effect is a VCF whose cutoff can be modulated by a LFO. The VCF has cutoff and resonance, and can be set to either low, band or high pass. The LFO for the VCF can go up to 1000hz and it has controls rate, and amount.  It can also be synced to the DAW bpm with divisions up to 1/64.

Then there are two distortion modules, called “Trashers” with individual tonality. Both feature Fuzz and Scream modes and have controls for amount and tone. Then there is an Echo, with time, feedback, and amount controls. The echo also has a sync to bpm feature, up to 1/64 divisions.  Next is the Reverb, it has room size, damping, width and amount.
There is a global Dry/wet control to handle the dry vs. wet balance. There is the option for oversampling of the entire signal chain, with settings off, 2x, 4x or 8x oversampling, to change the oversampling setting there is a drop down box, when the setting is changed the dsp::Oversampling needs to be recreated. 

There is a randomize button to set all parameters randomly, except for the dry/mix balance.

To reiterate the signal chain is VCA modulated by LFO, two modes; VCF, Trasher 1, Trasher 2, Echo, Reverb.

### To add to the vibe:

Should compile using cmake on the system.

###Vibes:

-The overall plugin loads OK and the controls are correct. But there are issues with the sound. First, let's focus on fixing the VCF. It does not work. Fix it. The filter cutoff and resonance sliders in the GUI have no effect. Fix it. Secondly, the echo does not work as expected, the sound is very distorted. Fix it. Next, the oversampling does not work, use the following information for fixing it: You must call oversampler.initProcessing() in prepareToPlay.

The AudioBlock passed into processSamplesUp must not be reused until after processSamplesDown is called.

The oversampled block is an internal buffer managed by JUCE — you don’t need to allocate it manually. To change the oversampling amount in JUCE 8 using dsp::Oversampling, you need to:

Destroy and recreate the Oversampling object with a new factor.

Call initProcessing() again with the new configuration.

Optionally add a user parameter (e.g. a combo box) to select oversampling amount dynamically.

- I ran the plugin, it is getting better. The VCA LFO amount is set too high it destroys the sound (getting too louder) and the plugin stops responding. Fix it. If I change the oversampling setting it crashes the plugin. Fix it.

- I have made a VST plugin. It is working well, so I do not want to change too much. However the oversampling code does not seem to have an effect. Can you help me verify it? I want to keep oversampling for the entire signal chain.