#include "Audio.h"

#pragma comment(lib, "xaudio2.lib")

AudioManager::AudioManager() {
}

AudioManager::~AudioManager() {
    Shutdown();
}

void AudioManager::Initialize() {
	HRESULT hr;

	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));

	LoadAudio("Test", "resources/Alarm01.wav");
}

SoundData AudioManager::SoundLoadWave(const char* filename) {
	// Implementation for loading a WAV file into SoundData
    std::ifstream file;
    file.open(filename, std::ios_base::binary);
    assert(file.is_open());

    RiffHeader riff;
    file.read((char*)&riff, sizeof(riff));
    if (strncmp(riff.chunk.id, "RIFF", 4) != 0) { assert(0); }
    if (strncmp(riff.type, "WAVE", 4) != 0) { assert(0); }

    FormatChunk format = {};
    file.read((char*)&format, sizeof(ChunkHeader));
    while (strncmp(format.chunk.id, "fmt ", 4) != 0) {
        file.seekg(format.chunk.size, std::ios_base::cur);
        file.read((char*)&format, sizeof(ChunkHeader));
        assert(!file.eof());
    }

    assert(format.chunk.size <= sizeof(format.fmt));
    file.read((char*)&format.fmt, format.chunk.size);

    ChunkHeader data;
    file.read((char*)&data, sizeof(data));
    if (strncmp(data.id, "JUNK", 4) == 0) {
        file.seekg(data.size, std::ios_base::cur);
        file.read((char*)&data, sizeof(data));
    }

    if (strncmp(data.id, "data", 4) != 0) { assert(0); }

    char* pBuffer = new char[data.size];
    file.read(pBuffer, data.size);
    file.close();

    SoundData soundData = {};
    soundData.wfex = format.fmt;
    soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
    soundData.bufferSize = data.size;

    return soundData;
}

void AudioManager::UnloadSound(const std::string& key) {
    auto it = soundRegistry_.find(key);
    if (it != soundRegistry_.end()) {
        SoundUnload(&it->second); // Free PCM buffer memory
        soundRegistry_.erase(it);
    }
}

void AudioManager::UnloadAll() {
    for (auto& [key, data] : soundRegistry_) {
        SoundUnload(&data);
    }
    soundRegistry_.clear();
}

void AudioManager::SoundUnload(SoundData* soundData) {
    delete[] soundData->pBuffer;
    soundData->pBuffer = 0;
    soundData->bufferSize = 0;
    soundData->wfex = {};
}

bool AudioManager::LoadAudio(const std::string& soundName, const std::string& filePath) {
    // Avoid re-loading if already cached
    if (soundRegistry_.find(soundName) != soundRegistry_.end()) {
        return true;
    }

    // Load sound data from file (your wave loading logic)
    SoundData data = SoundLoadWave(filePath.c_str());
    if (data.pBuffer == nullptr) {
        return false; 
    }
	soundRegistry_[soundName] = data;
    return true;
}

void AudioManager::PlayAudio(const std::string& soundName) {
	auto it = soundRegistry_.find(soundName);
	if (it != soundRegistry_.end()) {
		SoundPlayWave(it->second);
	}
}

void AudioManager::SoundPlayWave(const SoundData& soundData) {
    HRESULT result;
    IXAudio2SourceVoice* pSourceVoice = nullptr;
    result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
    assert(SUCCEEDED(result));

    XAUDIO2_BUFFER buf{};
    buf.pAudioData = soundData.pBuffer;
    buf.AudioBytes = soundData.bufferSize;
    buf.Flags = XAUDIO2_END_OF_STREAM;

    result = pSourceVoice->SubmitSourceBuffer(&buf);
    result = pSourceVoice->Start();
}

void AudioManager::Shutdown() {
    UnloadAll();

    if (masterVoice_) {
        masterVoice_->DestroyVoice();
        masterVoice_ = nullptr;
    }
    xAudio2_.Reset();
}