/*
  ==============================================================================

    InferenceThreadJob.cpp
    Created: 2 Oct 2021 1:54:57pm
    Author:  Andrew Fyfe

  ==============================================================================
*/

#include "InferenceThreadJob.h"


InferenceThreadJob::InferenceThreadJob(NeuroDrumLiveAudioProcessor& processor)
: ThreadPoolJob("InferenceThreadPoolJob"), mProcessor(processor)
{
    
}

InferenceThreadJob::~InferenceThreadJob()
{
    
}

auto InferenceThreadJob::runJob() -> JobStatus
{
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    const File modelFile = mProcessor.getModelFile();
    
    if (!modelFile.existsAsFile() || modelFile.getFileExtension() != ".onnx")
    {
        DBG("invalid model file format");
        return JobStatus::jobHasFinished;
    }
    
    auto model_file = mProcessor.getModelFile().getFullPathName().toStdString();
    
    const float attackVal = mProcessor.mAttackVal.load();
    const float releaseVal = mProcessor.mReleaseVal.load();
    const float brightnessVal = mProcessor.mBrightnessVal.load();
    const float hardnessVal = mProcessor.mHardnessVal.load();
    const float depthVal = mProcessor.mDepthVal.load();
    const float roughnessVal = mProcessor.mRoughnessVal.load();
    const float boominessVal = mProcessor.mBoominessVal.load();
    const float warmthVal = mProcessor.mWarmthVal.load();
    const float sharpnessVal = mProcessor.mSharpnessVal.load();
    
    const Ort::SessionOptions options_ort{nullptr};
    Ort::Env env;
    Ort::Session session_{env, model_file.c_str(), options_ort};
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }

    Ort::Value env_tensor_{nullptr};
    std::array<int64_t, 3> env_shape_{16, 16000, 1};
    
    Ort::Value params_tensor_{nullptr};
    std::array<int64_t, 2> params_shape_{16, 7};
    
    Ort::Value is_train_tensor_{nullptr};
    std::array<int64_t, 1> is_train_shape_{1};

    Ort::Value output_tensor_{nullptr};
    std::array<int64_t, 2> output_shape_{16, 16000};
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    std::vector<float> input_env(16 * 16000);
    std::vector<float> input_params(16 * 7);
    bool input_is_train = false;
    std::vector<float> results_(16 * 16000);
    std::vector<Ort::Value> input_tensors;
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    /* ['brightness', 'hardness', 'depth', 'roughness', 'boominess', 'warmth', 'sharpness'] */
    const std::array<float, 7> param_vals { brightnessVal,
                                    hardnessVal,
                                    depthVal,
                                    roughnessVal,
                                    boominessVal,
                                    warmthVal,
                                    sharpnessVal };
    
    /* populate input params vector with 16 repeats of the param values */
    int counter_params = 0;
    while (counter_params < input_params.size()) {
        for (int i = 0; i < param_vals.size(); ++i) {
            input_params[counter_params + i] = param_vals[i];
        }
        counter_params += param_vals.size();
    }
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    /* populate envelope vector with 16 repeats of the set attack/release envelope */
    const int size = 16000;
    const float endVal = 1.f;
    float currentVal = 0.f;
    int counter_env = 0;
    float stepAttack = endVal / (attackVal*size);
    float stepRelease = endVal / (releaseVal*size);
    
    //std::fill(input_is_train.begin(), input_is_train.end(), false);
    std::fill(input_env.begin(), input_env.end(), 0.f);
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    while (counter_env < input_env.size()) {
        for (int i = 0; i < size; ++i)
        {
            if (i < attackVal*size) {
                currentVal += stepAttack;
                input_env[counter_env + i] = std::min(currentVal, 1.f);;
            } else {
                currentVal -= stepRelease;
                input_env[counter_env + i] = std::max(currentVal, 0.f);
            }
        }
        currentVal = 0.f;
        counter_env += size;
    }
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    auto memory_info = Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU);
    env_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_env.data(), input_env.size(), env_shape_.data(), env_shape_.size());
    
    params_tensor_ = Ort::Value::CreateTensor<float>(memory_info, input_params.data(), input_params.size(), params_shape_.data(), params_shape_.size());
    
    is_train_tensor_ = Ort::Value::CreateTensor<bool>(memory_info, &input_is_train, 1, is_train_shape_.data(), is_train_shape_.size());
    
    output_tensor_ = Ort::Value::CreateTensor<float>(memory_info, results_.data(), results_.size(), output_shape_.data(), output_shape_.size());
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    
    const char* input_names[] = {"cond_placeholder:0", "input_placeholder:0", "is_train:0"};
    const char* output_names[] = {"Squeeze:0"};
    
    input_tensors.push_back(std::move(params_tensor_));
    input_tensors.push_back(std::move(env_tensor_));
    input_tensors.push_back(std::move(is_train_tensor_));

    session_.Run(Ort::RunOptions{nullptr}, input_names, input_tensors.data(), 3, output_names, &output_tensor_, 1);
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    AudioSampleBuffer buffer;
    buffer.setSize(2, size);
    for (int c {0}; c < buffer.getNumChannels(); ++c)
    {
        for (int i {0}; i < size; ++i)
        {
            buffer.setSample(c, i, results_[i]);
        }
    }
    
    if (shouldExit())
    {
        return JobStatus::jobNeedsRunningAgain;
    }
    
    const double fs = 16000.0;
    juce::BigInteger range;
    range.setRange(0, 128, true);
    
    mProcessor.mSampler.addSound(new AudioBufferSamplerSound ("Sample", buffer, fs, range, 60, 0.1, 0.1, 10.0));
    
    DBG("inference complete");
    
    return JobStatus::jobHasFinished;
}
