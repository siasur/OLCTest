#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class PNoiseVisual : public olc::PixelGameEngine {

private:
	static const int size = 512;
	std::array<olc::vf2d, size> points;
	std::array<olc::vf2d, size> noise;
	olc::vf2d* pSelected = nullptr;
	int txtHeight = -1;
	int spacing = 3;
	int bgLayer = -1;
	int padding = 5;
	int graphHeight = 150;
	int graphWidth = -1;
	int dividerPos = -1;
	int octaves = 1;
	float baseScale = 5.0f;

public:
	PNoiseVisual() {
		sAppName = "Mein Test";
	}

	bool OnUserCreate() override
	{
		txtHeight = GetTextSize("P").y;

		int32_t lowLimit = ScreenHeight() - padding;

		bgLayer = CreateLayer();
		SetDrawTarget(bgLayer);
		Clear(olc::BLACK);
		DrawLine({ padding, lowLimit - graphHeight }, { padding, lowLimit }, olc::WHITE);
		DrawLine({ padding, lowLimit }, { ScreenWidth() - padding, lowLimit }, olc::WHITE);

		for (int32_t i = 0; i < 5; i++)
		{
			Draw({ padding - 1, lowLimit - graphHeight + (i * (graphHeight / 4)) });
		}

		dividerPos = ScreenHeight() - graphHeight - padding * 4;
		DrawLine({ 0 , dividerPos }, { ScreenWidth(), dividerPos }, olc::WHITE, 0b1111111111111111111100);

		graphWidth = ScreenWidth() - 2 * padding;

		//for (int32_t i = 0; i < size; i++)
		//{
		//	Draw({ ScreenWidth() - padding*2 - (i * (graphWidth / size)), lowLimit + 1 });
		//}

		EnableLayer(bgLayer, true);
		SetDrawTarget(nullptr);

		InitRandValues(true);
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		Clear(olc::BLANK);

		if (this->GetKey(olc::Key::NP5).bReleased) InitRandValues(false);
		if (this->GetKey(olc::Key::NP_ADD).bReleased) octaves += 1;
		if (this->GetKey(olc::Key::NP_SUB).bReleased) octaves -= 1;
		//octaves = Clamp(octaves, 1, 8);

		int32_t cnt = 0;
		olc::vf2d* last = nullptr;
		for (auto& point : points) {

			if (cnt % 8 == 0 || cnt % 16 == 0)
			{

				//DrawStringProp({ 5, 5 + cnt * (txtHeight + spacing) }, GetPositionText(cnt++, point, 1 + -1*Normalize(point.y, ScreenHeight() - padding - graphHeight, ScreenHeight() - padding)), olc::WHITE);
				FillCircle(point, 2, cnt % 16 == 0 ? olc::YELLOW : olc::DARK_YELLOW);

				if (cnt == 0)
					FillCircle(point, 2, olc::VERY_DARK_YELLOW);


				if (last != nullptr)
					DrawLine(point, *last, olc::GREY);

				last = &point;
			}

			cnt++;
		}

		PerlinNoise(size, &points, &noise, octaves);

		last = nullptr;
		for (auto& point : noise) {
			//FillCircle(point, 2, olc::RED);

			if (last != nullptr)
				DrawLine(point, *last, olc::GREY);

			last = &point;
		}

		olc::vf2d mouse = GetMousePos();

		if (GetMouse(0).bPressed) {
			pSelected = nullptr;
			for (auto& p : points) {
				if (cnt % 8 != 0 || cnt % 16 != 0)
					continue;
				if ((p - mouse).mag() < 4)
					pSelected = &p;
			}
		}

		if (GetMouse(0).bReleased)
			pSelected = nullptr;

		if (pSelected != nullptr)
			pSelected->y = Clamp(mouse.y, ScreenHeight() - padding - graphHeight, ScreenHeight() - padding);

		return true;
	}

	void InitRandValues(bool initNoise)
	{
		for (int32_t i = 0; i < size; i++)
		{
			points[i] = { padding*2 + (static_cast<float>(i) * ((graphWidth - padding) / (size - 1))), Unfold(static_cast <float> (rand()) / static_cast <float> (RAND_MAX), ScreenHeight() - padding - graphHeight, ScreenHeight() - padding) };
			if (initNoise) {
				noise[i] = { padding * 2 + (static_cast<float>(i) * ((graphWidth - padding) / (size - 1))), 0 };
			}
		}
	}

	std::string GetPositionText(int id, olc::vf2d val, float val2)
	{
		return "Position " + std::to_string(id) + ": " + std::to_string(val.x) + " | " + std::to_string(val.y) + " (" + std::to_string(val2) + ")";
	}

	float Unfold(float in, float min, float max)
	{
		return min + (max - min) * in;
	}

	float Normalize(float in, float min, float max)
	{
		return (in - min) / (max - min);
	}

	float Clamp(float in, float min, float max) {
		return std::fminf(std::fmaxf(in, min), max);
	}

	float ValOf(olc::vf2d vec) {
		return 1 + -1 * (Normalize(vec.y, ScreenHeight() - padding - graphHeight, ScreenHeight() - padding));
	}

	void PerlinNoise(int count, std::array<olc::vf2d, size>* in_seed, std::array<olc::vf2d, size>* out_noise, int _octaves) {

		for (int32_t x = 0; x < count; x++)
		{
			float noise = 0.0f;
			float scale = baseScale;
			float scaleAcc = 0;

			for (int32_t o = 0; o < _octaves; o++)
			{
				int pitch = count >> o;

				if (pitch < 1) {
					DrawString({ 1,1 }, "E R R O R !!", olc::DARK_RED);
					continue;
				}

				int sample1 = (x / pitch) * pitch;
				int sample2 = (sample1 + pitch) % count;

				float blend = (float)(x - sample1) / (float)pitch;
				float sample = (1.0f - blend) * ValOf(in_seed->at(sample1)) + blend * ValOf(in_seed->at(sample2));
				noise += sample * scale;
				scaleAcc += scale;
				scale /= 2;
			}

			out_noise->at(x).y = (Unfold(noise / scaleAcc, 0, dividerPos - padding) * -1 + (dividerPos - padding));
		}

	}

};

int main() {
	PNoiseVisual test;
	if (test.Construct(640, 480, 2, 2))
		test.Start();
	return 0;
}