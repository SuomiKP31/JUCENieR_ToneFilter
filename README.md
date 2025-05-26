# JUCENieR_ToneFilter
A recreation of NieR:Automata's 8-bit tone filters in JUCE. I made this for UVic's CSC575 Music Information Retrieval course. I'll upload my final report later (with my personal information stripped out ofc), in case you are curious about the technical details.

## How to Use
You will get two VST3 plugins in the release package. They can be added to the JUCE plugin host (or other DAWs you like but I haven't tested any). Connect the audio player to the tone filter and the tone filter to output, from there, you can try to run your own audio through and see how it sounds. The plugin host is one of the internal demos that is pre-packaged with JUCE itself, so if you want to start developing your own plugin, playing with it in ProJucer might be a good first move.

Both plugins are built and tested with JUCE 8.0.6.

![image](https://github.com/user-attachments/assets/d639d5f5-afff-4958-ad53-f413e3ba348e)

## What does it do?
It converts the input to a square-wave like harmonics and mix those with the original track, creating a retro game like feel.
NieR: Automata's lead audio engineer, Masami Ueda, created it originally in Wwise, to act as a transition between the regular soundtrack and its 8-bit version. Sometimes the track does not have an 8-bit version (such as during the Amusement Park boss fight), then this effect will be used to at least created some hint of 8-bit style from the current background music.

The Maister made a recreation in 2019 and experimented with a lot of technical stuff that Masami didn't release in detail, which helped a lot. Both of their blogs will be in the referenece section. I followed the Maister's practice mostly, but took a different approach when creating parameters for the Biquad IIR Filters, as we have much better interfaces available in JUCE.

The filtered out tones are also visualized in keys.

## Demo

Demo video can be found [here](https://www.youtube.com/watch?v=IuDy-JSWunI).

## SIMD

I decided to remove my SIMD experiments from the repo when I uploaded it. This is to maintain best readability for educational purposes. (I really struggled when reading Maister's codes)
If you want to create SIMD, it's not that hard actually, JUCE has a [very nice tutorial](https://juce.com/tutorials/tutorial_simd_register_optimisation/) which is almost exactly what you want to do (run multiple IIR filters in parallel).

## Naming

This project was originally built upon my VST3 mixer. I kept the filters from that project (you'll need to LPF the output a little bit), then built most things from there. So the main C++ class is still named 561Mixer. Just don't get confused. Most of the interesting things should be named conventionally.

## Things to do...
The runtime power adjustments probably need some more work. The thresholds of tone filter outputs are currently hardcoded which is not ideal, it should be adjustable to adapt to energy distributions of different songs.

The same is more or less true for the power lerping, which is used to make the tone output transition smoother. This should scale with BPM, as it doesn't work too well with faster songs. (The faster the song, the faster the tone output should fade).

As a result, currently this thing only works very well with slow and relatively low energy inputs. I tried on some non-combat NieR:Automata music and it's great, others, no so well.

## Reference
[The Maister's Blog](https://themaister.net/blog/2019/02/23/recreating-the-tone-filter-from-nierautomata/)
Great and much better explanation on how to choose poles and zeros for your filter, as well as leveling.

[The Maister's VST Source](https://github.com/Themaister/ToneFilterVST)
Mostly acted as my code references. The leveling part is pretty much identical (I used their parameters too), but our IIR implementations are different.

[Masame Ueda's blog at Platinum Games](https://www.platinumgames.com/official-blog/article/9581)
Original materials. A few demos can be seen there, showcasing the original version of this plugin used in game.

## Endorsement
![image](https://github.com/user-attachments/assets/f58093bb-855c-4096-8aee-c7157f19068e)

My [LinkedIn Post](https://www.linkedin.com/posts/zhixin-fang-131132192_i-tried-to-recreate-masami-ueda-sans-tone-activity-7319486409088593922-mhOo) was endorsed by the original creator, Masami Ueda.
I'm happy to get recognized by a industry veteran.

