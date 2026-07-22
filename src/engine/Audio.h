#pragma once

#include <xaudio2.h>
#include <stdint.h>
#include <wrl.h>
#include <assert.h>
#include <fstream>
#include <unordered_map>
#include <string>
#include <memory>


struct ChunkHeader {
	char id[4];
	int32_t size;
};

struct RiffHeader {
	ChunkHeader chunk;
	char type[4];
};

struct FormatChunk {
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData {
	WAVEFORMATEX wfex;
	BYTE* pBuffer;
	unsigned int bufferSize;
};

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    void Initialize();

	SoundData SoundLoadWave(const char* filename);
	
	bool LoadAudio(const std::string& soundName, const std::string& filePath);
	void PlayAudio(const std::string& soundName);

	void UnloadSound(const std::string& key);
	void SoundUnload(SoundData* soundData);
	void UnloadAll();

	void Shutdown();
private:

	void SoundPlayWave(const SoundData& soundData);

	std::unordered_map<std::string, SoundData> soundRegistry_;

    Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;	
    IXAudio2MasteringVoice* masterVoice_;	

};