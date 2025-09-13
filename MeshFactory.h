/*
	MeshFactory.h
	20250818  hanaue sho
	自作メッシュを作る
	作ったらそのまま MeshFilter にセットする（所有権は MeshFilter ）
*/
#ifndef MESHFACTORY_H_
#define MESHFACTORY_H_
#include <d3d11.h>
#include <cassert>
#include "MeshFilterComponent.h"
#include "Renderer.h"
#include "Vector3.h"


// 平面（XY）
struct QuadParams
{
	float width = 200.0f;
	float height = 200.0f;
	bool originCenter = true; // true = 中心原点, false = 左上原点
};

// 平面（XZ）
struct PlaneParams
{
	float width = 10.0f;
	float depth = 10.0f;
	bool originCenter = true; // true = 中心原点, false = 左上原点
};

// 球体
struct SphereParams
{
	float radius = 1.0f;
	int slices = 6; // 横分割（経度）
	int stacks = 6; // 縦分割（緯度）
	bool insideOut = false; // true: スカイドーム用
};

// 立方体
struct CubeParams
{
	Vector3 size = Vector3(1, 1, 1);
	bool originCenter = true;   // true = 中心原点, false = 左上原点
	bool insideOut = false;		// true: 内側
};

// カプセル
struct CapsuleParams
{
	float radius = 1.0f;		 // 半径
	float cylinderHeight = 1.0f; // 胴の長さ
	int slices = 24;			 // 周方向
	int stacksBody = 1;			 // 側面の縦分割
	int stacksCap  = 4;			 // 半球の縦分割
	bool insideOut = false;		 // 反転
};

// 円柱
struct CylinderParams
{
	float radius = 1.0f;
	float height = 2.0f;
	int slices = 24;		// 周方向
	int stacks = 1;			// 側面の縦分割
	bool insideOut = false;	// 反転
};



class MeshFactory
{
public:
	static void CreateQuad	  (MeshFilterComponent* filter, const QuadParams& p = {});
	static void CreatePlane   (MeshFilterComponent* filter, const PlaneParams& p = {});
	static void CreateSphere  (MeshFilterComponent* filter, const SphereParams& p = {});
	static void CreateCube	  (MeshFilterComponent* filter, const CubeParams& p = {});
	static void CreateCylinder(MeshFilterComponent* filter, const CylinderParams& p = {});
	static void CreateCapsule (MeshFilterComponent* filter, const CapsuleParams& p = {});
};



#endif