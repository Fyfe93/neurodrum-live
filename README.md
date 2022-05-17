# neurodrum-live
neurodrum-live: NeuroDrum Sampler VST/AU/Standalone

Generate and play percussion samples using NeuroDrum generative models inside your DAW.

## Instructions
Requires the Juce library as this is a Juce project and uses the Projucer for building. Clone from [here](https://github.com/juce-framework/JUCE)

Clone Onnx runtime and build following the instructions in the Onnyx Documentation [here](https://onnxruntime.ai/docs/how-to/build/inferencing.html)

create "include" and "lib" directories in neurodrum-live project root folder

For now you must copy header files and corresponding .a library files over to the previously created 'include' and 'lib' directories. To check you have placed the library files and includes in the correct locations open the Projucer file 'neurodrum-live.jucer' and check against the linked library/include locations.

Launch the Xcode project from the Projucer and build.

The compiled executable will be found in the 'Builds' folder.

## Acknowledgements

Credit given to the Music Technology Group, Universitat Pompeu Fabra, Barcelona who authored the NeuroDrum model architecture. You can check it out [here](https://github.com/pc2752/percussive_synth)

Check out the original paper [here](https://arxiv.org/abs/1911.11853)
