

#include "main.h"
#include "manager.h"
#include "mouse.h"
#include "keyboard.h"
#include <thread>


const char* CLASS_NAME = "AppClass";
const char* WINDOW_NAME = "DX11�Q�[��";


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


HWND g_Window;

HWND GetWindow()
{
	return g_Window;
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	// ������ -----
	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	Mouse_Initialize(GetWindow());
	Keyboard_Initialize();
	Manager::Init();

	// �f�o�b�O�E�B���h�E����
	//AllocConsole(); // �R���\�[�����쐬
	//FILE* a = freopen("CONOUT$", "w", stdout); // stdout���R���\�[���Ƀ��_�C���N�g
	//printf("Hello, World!\n"); // �R���\�[���ɏo��


	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);

	// ���[�v���� ----- �i�A�L�������[�^�����j
	using clock = std::chrono::steady_clock;
	auto prev = clock::now();

	double accumulator = 0.0f;
	const double fixedDt = 1.0 / 60.0; // �������[�g����

	// FPS�p�̐ώZ
	double fpsElapsed = 0.0;
	int fpsFrames = 0;
	int lastPhysSteps = 0; // ���߃t���[���ŉ��� FixedStep ������

	bool running = true;
	MSG msg;
	while(running)
	{
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // ���b�Z�[�W�̏���
		{
			if (msg.message == WM_QUIT)
			{
				running = false; // ���[�v�I��
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (!running) break;
		

		// ----- ���Ԍv�� -----
		auto now = clock::now(); // ���݂̃t���[���̎���
		double dt = std::chrono::duration<double>(now - prev).count(); // �O�̃t���[���̎��Ԃƍ��̃t���[���̎��Ԃ̍���
		prev = now; // ���݂̃t���[���̎��Ԃ̕ۑ�
		//dt = std::min(dt, 0.05); // �t���[���ő� 50ms

		// ----- ���� -----
		//accumulator = std::min(accumulator + dt, 0.25); // �ݐύő� 250 ms
		accumulator += dt; 
		int steps = 0; 
		const int maxSteps = 6; // �X�p�C�����΍�
		while (accumulator >= fixedDt && steps < maxSteps)
		{
			// FixedUpdate �i���̒��� PhysicsSytem ���Ă΂�Ă����j
			Manager::FixedUpdate(fixedDt);
			
			// �J�E���g
			accumulator -= fixedDt;
			steps++;
		}
		lastPhysSteps = steps;

		// ----- �Q�[���X�V�A�`�� -----
		Manager::Update(dt);
		Manager::Draw();
		MouseUpdate();
		keycopy();


		// ----- FPS�v���ƃ^�C�g���X�V�i�P�b���Ɓj -----
		fpsElapsed += dt; // �����[�v���ԃJ�E���g
		fpsFrames += 1; // �����[�v�t���[���J�E���g
		if (fpsElapsed >= 1.0f) 
		{
			double fps = fpsFrames / fpsElapsed; // �P�b����FPS
			double ms = 1000.0 / (fps > 0 ? fps : 1); // �ڈ��̂P�t�����v����
		#ifdef UNICODE
			wchar_t title[128];
			swprintf_s(title, L"%hs | FPS: %.1f (%.2f ms) | PhysSteps: %d", WINDOW_NAME, fps, ms, lastPhysSteps);
			SetWindowTextW(g_Window, title); // �E�B���h�E�̃o�[�ɕ\��
		#else
			char title[128];
			snprintf(title, sizeof(title), "%s | FPS: %.1f (%.2f ms) | PhysSteps: %d", WINDOW_NAME, fps, ms, lastPhysSteps);
			SetWindowTextA(g_Window, title); // �E�B���h�E�̃o�[�ɕ\��
		#endif		
			// �ώZ�����Z�b�g
			fpsElapsed -= 1.0f;
			fpsFrames	= 0;
		}
	} // while �I��

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	Manager::Uninit();

	CoUninitialize();

	return (int)msg.wParam;
}

// --------------------------------------------------
// WndProc
// Windows�̃��b�Z�[�W�z����
// �E�B���h�E�ɔ�������S�C�x���g�i�L�[�A�}�E�X�A���T�C�Y�A�j���Ȃǁj�������ɓ͂�
// �����ŏ����������b�Z�[�W�Freturn 0;
// �����ŏ������Ȃ����b�Z�[�W�FDefWindowProc�֓n���i���菈���j
// --------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	// �L�[�{�[�h�i�ʏ�L�[�j-----
	case WM_KEYDOWN: // �����L�[�������ꂽ�i�I�[�g���s�[�g����j
	case WM_KEYUP:	 // �����L�[�������ꂽ
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		return 0;

	// �V�X�e���L�[�iALT�^F10�������݁j-----
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		Keyboard_ProcessMessage(uMsg, wParam, lParam);
		break;
	
	// �A�v���̃A�N�e�B�u�^��A�N�e�B�u�̐؂�ւ� -----
	case WM_ACTIVATEAPP:
		Keyboard_ProcessMessage(uMsg, wParam, lParam); // �����œ��͏�Ԃ̃N���A�Ȃǂ��s���ƈ��S
		return 0;
		
	// RAW INPUT �i���̓��́F�o�^���Ă���Δ��ł����j
	case WM_INPUT:
		Mouse_ProcessMessage(uMsg, wParam, lParam);  // RAW �}�E�X���g���Ȃ炱����
		return 0;

	// �}�E�X�i��RAW�̒ʏ탁�b�Z�[�W�j-----
	case WM_MOUSEMOVE:	 // �}�E�X�ړ�
	case WM_LBUTTONDOWN: // ���N���b�N����
	case WM_LBUTTONUP:	 // ���N���b�N����
	case WM_RBUTTONDOWN: // �E�N���b�N����
	case WM_RBUTTONUP:	 // �E�N���b�N����
	case WM_MBUTTONDOWN: // �����{�^������
	case WM_MBUTTONUP:	 // �����{�^������
	case WM_MOUSEWHEEL:	 // �}�E�X�z�C�[��
	case WM_XBUTTONDOWN: // �T�C�h�{�^������
	case WM_XBUTTONUP:	 // �T�C�h�{�^������
	case WM_MOUSEHOVER:
		Mouse_ProcessMessage(uMsg, wParam, lParam);
		return 0;

	// �E�B���h�E�̔j�� -----
	case WM_DESTROY:
		PostQuitMessage(0); // ���C�����[�v�� WM_QUIT ������
		return 0;
	default:
		break;
	}

	// �����ɗ������̂͊��菈���i�T�C�Y�ύX�A��N���C�A���g�̈�A�ŏ������j
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

