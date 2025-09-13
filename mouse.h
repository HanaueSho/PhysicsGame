//--------------------------------------------------------------------------------------
// File: mouse.h
//
// �֗��ȃ}�E�X���W���[��
//
//--------------------------------------------------------------------------------------
// 2020/02/11
//     DirectXTK���A�Ȃ񂿂����C����p�ɃV�F�C�v�A�b�v����
//
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------
#ifndef HAL_YOUHEI_MOUSE_H
#define HAL_YOUHEI_MOUSE_H
#pragma once


#include <windows.h>
#include <memory>


#include <DirectXMath.h>
using namespace DirectX;

// �}�E�X���[�h
typedef enum Mouse_PositionMode_tag
{
    MOUSE_POSITION_MODE_ABSOLUTE, // ��΍��W���[�h
    MOUSE_POSITION_MODE_RELATIVE, // ���΍��W���[�h
} Mouse_PositionMode;

// �}�E�X�L�[��
typedef enum Mouse_Keys_tag
{
    MS_NONE = 0x0,

    MS_CLICK_LEFT = 0x0001, // ���N���b�N 0100
    MS_CLICK_MIDDLE = 0x0002, // �z�C�[���N���b�N 0010
    MS_CLICK_RIGHT = 0x0004, // �E�N���b�N 0001

    MS_DUMMY_0 = 0x0008, // �_�~�[ 1000

} MOUSE_KEY;

// �}�E�X��ԍ\����
typedef struct MouseState_tag
{
    bool leftButton : 1;
    bool middleButton : 1;
    bool rightButton : 1;
    bool xButton1 : 1;
    bool xButton2 : 1;
    int x;
    int y;
    int scrollWheelValue;
    Mouse_PositionMode positionMode;
} Mouse_State;


// �}�E�X���W���[���̏�����
void Mouse_Initialize(HWND window);

// �}�E�X���W���[���̏I������
void Mouse_Finalize(void);

// �}�E�X�̏�Ԃ��擾����
void Mouse_GetState(Mouse_State* pState);

// �ݐς����}�E�X�X�N���[���z�C�[���l�����Z�b�g����
void Mouse_ResetScrollWheelValue(void);

// �}�E�X�̃|�W�V�������[�h��ݒ肷��i�f�t�H���g�͐�΍��W���[�h�j
void Mouse_SetMode(Mouse_PositionMode mode);

// �}�E�X�̐ڑ������o����
bool Mouse_IsConnected(void);

// �}�E�X�J�[�\�����\������Ă��邩�m�F����
bool Mouse_IsVisible(void);

// �}�E�X�J�[�\���\����ݒ肷��
void Mouse_SetVisible(bool visible);

// �}�E�X����̂��߂̃E�B���h�E���b�Z�[�W�v���V�[�W���t�b�N�֐�
void Mouse_ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

// --------------------------------------------------
// �}�E�X�X�V����
void MouseUpdate();

// �}�E�X�̃N���b�N���͂̎擾
bool Mouse_IsClick(MOUSE_KEY key);
bool Mouse_IsClick(MOUSE_KEY key, const Mouse_State* pState);
bool Mouse_IsClickUp(MOUSE_KEY key);
bool Mouse_IsClickUp(MOUSE_KEY key, const Mouse_State* pState);
bool Mouse_IsClickTrigger(MOUSE_KEY key);
bool Mouse_IsClickTrigger(MOUSE_KEY key, const Mouse_State* pState);
bool Mouse_IsWheelScroll();

XMFLOAT2 Mouse_GetPosition();
XMFLOAT2 Mouse_GetPositionOld();
XMFLOAT2 Mouse_GetPositionDiff();
int Mouse_GetPositionX();
int Mouse_GetPositionY();
int Mouse_GetWheelScroll();
int Mouse_GetWheelValue();


void Mouse_KeyCopy();

// �������@
//
// �Ώۂ̃E�B���h�E���������ꂽ�炻�̃E�B���h�E�n���h���������ɏ������֐����Ă�
//
// Mouse_Initialize(hwnd);
//
// �E�B���h�E���b�Z�[�W�v���V�[�W������}�E�X����p�t�b�N�֐����Ăяo��
//
// LResult CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_ACTIVATEAPP:
//     case WM_INPUT:
//     case WM_MOUSEMOVE:
//     case WM_LBUTTONDOWN:
//     case WM_LBUTTONUP:
//     case WM_RBUTTONDOWN:
//     case WM_RBUTTONUP:
//     case WM_MBUTTONDOWN:
//     case WM_MBUTTONUP:
//     case WM_MOUSEWHEEL:
//     case WM_XBUTTONDOWN:
//     case WM_XBUTTONUP:
//     case WM_MOUSEHOVER:
//         Mouse_ProcessMessage(message, wParam, lParam);
//         break;
//
//     }
// }
//

#endif // HAL_YOUHEI_MOUSE_H
