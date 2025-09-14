/*
	Random.h
	20250914 hanaue sho
	�����_���p�N���X
	�ق�ChatGPT�N�ɍ���ĖႢ�܂���
*/
#ifndef RANDOM_H_
#define RANDOM_H_
#include <random>	 // �����G���W���A���z
#include <cstdint>	 // �Œ蕝����
#include <algorithm> // std::swap
#include <limits>	 // ���l���E
#include <cmath>	 // std::nextafter
#include <chrono>	 // �����ŃV�[�h��������

class Random 
{
public:
	using engine_t = std::mt19937; // �����Z���k�c�C�X�^�[32bit�i�y�������i���j

private:
	static engine_t& Global()
	{
		thread_local engine_t eng{ SeedFromDevice() };
		return eng;
	}
	// �����V�[�h�����irandom_device �Ɏ�����A�h���X�l�������ăG���g���s�[�m�ہj 
	static engine_t::result_type SeedFromDevice()
	{
		std::random_device rd;
		// rd ���^���I�Ȋ��ł��B���ԂƃA�h���X�������Ă�����x�����_����
		auto now = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		auto mix = (static_cast<uint64_t>(rd()) << 32) ^ rd() ^ now ^ reinterpret_cast<uintptr_t>(&now); // rd() �Ǝ����A�X�^�b�N��̃A�h���X�� XOR ����
		return static_cast<engine_t::result_type>(mix); // mt19937 �̃V�[�h�^�Ɋۂ߂ĕԂ��i32bit�j
	}

public:
	// ���݂̃X���b�h��RNG��C�ӃV�[�h�ŏ������i�V�[�h�l���Œ肵�������p�j
	static void Seed(uint64_t seed)
	{
		// Global() �� thread_local �ȃG���W�����Q�ƂŕԂ�
		Global() = engine_t(static_cast<engine_t::result_type>(seed));
	}

	static float RandomRange(float min, float max)
	{
		if (min > max) std::swap(min, max);
		std::uniform_real_distribution<float> dist(min, 
												   std::nextafter(max, std::numeric_limits<float>::infinity()));
		return dist(Global()); // ���z�ɃG���W����H�킹�Ēl�������
	}
	// [min, max]
	static int RandomRange(int min, int max) 
	{
		if (min > max) std::swap(min, max);
		std::uniform_int_distribution<int> dist(min, max); // ������͋K�i�� [a,b]
		return dist(Global());
	}
	// [0,1) �̎����i�V���v���Łj
	static float Random01()
	{
		std::uniform_real_distribution<float> dist(0.0f, 1.0f); // [0,1)
		return dist(Global());
	}

	// ���U��l: min, min+step, min+2*step, ... <= max �̂����ꂩ�𓙊m���ŕԂ�
	static float RandomRangeStepped(float min, float max, float step)
	{
		// step �������L���ł��邱�Ƃ��m�F
		if (!(step > 0.0f) || !std::isfinite(step)) return std::numeric_limits<float>::quiet_NaN();
		if (min > max) std::swap(min, max);                        // �������t�ł�OK��

		// �v�Z�͌덷��}���邽�� double �ōs��
		const double dmin = static_cast<double>(min);
		const double dmax = static_cast<double>(max);
		const double dstep = static_cast<double>(step);

		// �O���b�h��́u�ő�C���f�b�N�X�vk_max �����߂�imin + k*step <= max�j
		// 1e-12 �͕��������ɂ��͂��Ȓ��߂��z�����邽�߂̔�����
		const double kmax_d = std::floor((dmax - dmin) / dstep + 1e-12);
		const int    kmax = (kmax_d < 0.0) ? 0 : static_cast<int>(kmax_d);

		// k �� [0, kmax] ����l�ɑI�ԁi���[�܂ށj
		const int k = RandomRange(0, kmax);

		// �l���\���i�Ō�� float �ցj�Bfmaf �Ŋۂߌ덷����i�}����̂��A��
		return static_cast<float>(dmin + dstep * static_cast<double>(k));
	}
};

#endif